#include <bits/stdc++.h>
#include <windows.h>
#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "logger.h"
#include "controller.h"
#include "helper.h"

using namespace rapidjson;
using namespace std;


inline void controller::initialize_logger() {
    LOGCFG.headers = false;
    LOGCFG.level = LOG_DEBUG;
    time_t t = std::time(0);   // get time now
    tm* now = std::localtime(&t);
    char log_file_name[32];
    sprintf(log_file_name, "%d%02d%02d.log", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday);
    LOGCFG.init(log_file_name);
}


inline void controller::write_log(string msg, bool flag_stdout = true) {
    if (flag_stdout)
        cout << msg << endl;
    LOG(LOG_INFO) << msg;
}

inline void controller::print_test_vm_info() {
    char title[64];
    sprintf(title, "VM Count: %d", test_vm.size());
    write_log(title);
    write_log("ID\tName\tIP Address\tOS\tSize");
    stringstream temp_stream;
    string vm_id;
    for (int i = 0;i < test_vm.size();i++) {
        temp_stream.clear();
        temp_stream << i + 1;
        temp_stream >> vm_id;
        write_log(vm_id + "\t" + test_vm[i].get_vm_info());
    }
    cout << endl;
}

inline void controller::print_test_result_summary(string workload_path) {
    vector<string> linux_jobs = list_files_in_directory(workload_path + "linux\\");
    vector<string> jobs = list_files_in_directory(workload_path + "windows\\");
    jobs.insert(jobs.end(), linux_jobs.begin(), linux_jobs.end());
    sort(jobs.begin(), jobs.end());
    vector<string>::iterator iter = unique(jobs.begin(),jobs.end());
    jobs.erase(iter,jobs.end());
    stringstream temp_stream;
    string summary_output_file;
    time_t t = std::time(0);
    tm* now = std::localtime(&t);
    temp_stream <<  (now->tm_year + 1900) << '-'
                     << (now->tm_mon + 1) << '-'
                     << now->tm_mday;
    temp_stream >> summary_output_file;
    summary_output_file = output + summary_output_file + "_summary.out";
    ofstream fout(summary_output_file, ios_base::out | ios_base::app);
    char title[64];
    sprintf(title, "VM Count: %d", test_vm.size());
    write_log(title);
    fout << title << endl;
    for (auto job : jobs) {
        write_log("Job: " + job);
        fout << "Job: " + job << endl;
        write_log("ID\tName\tIP Address\tOS\tSize\tR(MIN)\tR(MAX)\tR(AVG)\tW(MIN)\tW(MAX)\tW(AVG)");
        fout << "ID\tName\tIP Address\tOS\tSize\tR(MIN)\tR(MAX)\tR(AVG)\tW(MIN)\tW(MAX)\tW(AVG)" << endl;
        string vm_id;
        for (int i = 0;i < test_vm.size();i++) {
            if (test_vm[i].test_result.count(job)) {
                temp_stream.clear();
                temp_stream << i + 1;
                temp_stream >> vm_id;
                write_log(vm_id + "\t" + test_vm[i].get_vm_result(job));
                fout << vm_id + "\t" + test_vm[i].get_vm_result(job) << endl;
            }
        }
        cout << endl;
    }
    fout.close();
}

controller::controller(string test_vm_config) {
    initialize_logger();
    flag_err = 0;
    test_vm.clear();
    /* Load JSON file */
    fstream fin(test_vm_config);
    string reader, config_string = "";
    while (!fin.eof()) {
        getline(fin, reader);
        config_string += reader;
    }
    fin.close();

    Document vm_config;
    if (vm_config.Parse(config_string.c_str()).HasParseError()) {
        write_log("ERROR: parse test VM configuration failed!");
        flag_err = 1;
        return;
    }
    int vm_count = vm_config["count"].GetInt();
    auto vm_info = vm_config["value"].GetArray();
    for (int i = 0; i < vm_count; i++)
        test_vm.push_back(vm(vm_info[i]["name"].GetString(), vm_info[i]["ip"].GetString(), vm_info[i]["info"]["type"].GetString(), vm_info[i]["info"]["size"].GetString()));
}

void controller::initialize() {
    test_vm.clear();
    vector<string> agents = list_files_in_directory(agent_info);
    ifstream fin;
    string _ip,_os,_size;
    for (auto na : agents) {
        fin.open(agent_info + na);
        fin >> _ip >> _os >> _size;
        test_vm.push_back(vm(na, _ip, _os, _size));
        write_log("Register test VM " + na + " succeeded.");
        fin.close();
    }
    write_config();
    print_test_vm_info();
}

bool controller::has_error() {
    return flag_err;
}

void controller::pre_sync() {
    ofstream fout;
    string reader;
    ifstream fin;
    for (auto vm_itr : test_vm) {
        write_log("Sending pre-sync request to test VM " + vm_itr.vm_name + "(" + vm_itr.internal_ip + ")");
        exec_command(string("del /f " + vm_itr.get_share_path() + tempfolder + "client.tmp").c_str());
        fout.open(vm_itr.get_share_path() + tempfolder + "controller.tmp", ios_base::out | ios_base::trunc);
        fout << "PRESYNC";
        fout.close();
    }

    write_log("Waiting for agents response.");
    bool all_done = false;
    map<string, bool> done_job;
    done_job.clear();
    while (!all_done) {
        all_done = true;
        for (auto vm_itr : test_vm) {
            if (done_job[vm_itr.internal_ip] == 1)
                continue;
            ifstream fin;
            fin.open(vm_itr.get_share_path() + tempfolder + "client.tmp", ios_base::in);
            if (!fin.fail()) {
                fin >> reader;
                if(reader == "SYNCDONE") {
                    done_job[vm_itr.internal_ip] = 1;
                    write_log("Test VM " + vm_itr.vm_name + "(" + vm_itr.internal_ip + ")" + " pre-sync succeeded.");
                }
                else
                    all_done = false;
            }
            else
                all_done = false;
            fin.close();
        }
    }
    write_log("All test VM pre-sync succeeded!");
    cout << endl;
}

void controller::standard_worker() {
	ifstream fin;
	ofstream fout;
    string reader;
    for (auto vm_itr : test_vm) {
        if (vm_itr.vm_os == vm::os_type::linux)
            exec_command(string("xcopy workload\\std\\linux " + vm_itr.get_share_path() + workload + " /s /h /d /y").c_str());
        else
            exec_command(string("xcopy workload\\std\\windows " + vm_itr.get_share_path() + workload + " /s /h /d /y").c_str());
        write_log("Sending work request to test VM " + vm_itr.vm_name + "(" + vm_itr.internal_ip + ")");
        exec_command(string("del /f " + vm_itr.get_share_path() + tempfolder + "client.tmp").c_str());
        fout.open(vm_itr.get_share_path() + tempfolder + "controller.tmp", ios_base::out | ios_base::trunc);
        fout << "START";
        fout.close();
    }
    write_log("Waiting for agents response.");
    map<string, bool> done_job;
    done_job.clear();

    bool all_done = false;
    while (!all_done) {
        all_done = true;
        for (auto vm_itr : test_vm) {
            if (done_job[vm_itr.internal_ip] == 1)
                continue;
            fin.close();
            fin.clear();
            Sleep(1000);
            fin.open(vm_itr.get_share_path() + tempfolder + "client.tmp", ios_base::in);
            fin >> reader;
            if (reader == "DONE") {
                done_job[vm_itr.internal_ip] = 1;
                write_log("Test VM " + vm_itr.vm_name + "(" + vm_itr.internal_ip + ")" + " successfully completed work.");
            }
            else
                all_done = false;
        }
        fin.close();
    }
    Sleep(10000);
    for (auto vm_itr : test_vm) {
        exec_command(string("xcopy " + vm_itr.get_share_path() + output + " output /s /h /d /y").c_str());
        Sleep(1000);
    }
    write_log("All jobs done!");
    analyze_data("workload\\std\\");
    print_test_result_summary("workload\\std\\");
    cout << endl;
}

void controller::custom_worker() {
    ifstream fin;
	ofstream fout;
    string reader;
    for (auto vm_itr : test_vm) {
        if (vm_itr.vm_os == vm::os_type::linux)
            exec_command(string("xcopy workload\\linux " + vm_itr.get_share_path() + workload + " /s /h /d /y").c_str());
        else
            exec_command(string("xcopy workload\\windows " + vm_itr.get_share_path() + workload + " /s /h /d /y").c_str());
        write_log("Sending work request to test VM " + vm_itr.vm_name + "(" + vm_itr.internal_ip + ")");
        exec_command(string("del /f " + vm_itr.get_share_path() + tempfolder + "client.tmp").c_str());
        fout.open(vm_itr.get_share_path() + tempfolder + "controller.tmp", ios_base::out | ios_base::trunc);
        fout << "START";
        fout.close();
    }
    write_log("Waiting for agents response.");
    map<string, bool> done_job;
    done_job.clear();

    bool all_done = false;
    while (!all_done) {
        all_done = true;
        for (auto vm_itr : test_vm) {
            if (done_job[vm_itr.internal_ip] == 1)
                continue;
            fin.close();
            fin.clear();
            Sleep(1000);
            fin.open(vm_itr.get_share_path() + tempfolder + "client.tmp", ios_base::in);
            fin >> reader;
            if (reader == "DONE") {
                done_job[vm_itr.internal_ip] = 1;
                write_log("Test VM " + vm_itr.vm_name + "(" + vm_itr.internal_ip + ")" + " successfully completed work.");
            }
            else
                all_done = false;
        }
        fin.close();
    }
    Sleep(10000);
    for (auto vm_itr : test_vm) {
        exec_command(string("xcopy " + vm_itr.get_share_path() + output + " output /s /h /d /y").c_str());
        Sleep(1000);
    }
    write_log("All jobs done!");
    analyze_data("workload\\");
    print_test_result_summary("workload\\");
    cout << endl;
}

void controller::write_config() {
    ofstream fout(agents_config_file, ios_base::out | ios_base::trunc);
    fout << "{\n";
    fout << "\t\"count\":" << test_vm.size() << ",\n";
    fout << "\t\"value\":\n";
    fout << "\t\t[\n";
    for (int i = 0; i < test_vm.size(); i++) {
        fout << "\t\t\t{\n";
        fout << "\t\t\t\t\"name\":\"" << test_vm[i].vm_name << "\",\n";
        fout << "\t\t\t\t\"ip\":\"" << test_vm[i].internal_ip << "\",\n";
        fout << "\t\t\t\t\"info\":{\n";
        fout << "\t\t\t\t\t\"type\":\"" << (test_vm[i].vm_os == vm::os_type::linux ? "linux" : "windows") << "\",\n";
        fout << "\t\t\t\t\t\"size\":\"" << test_vm[i].vm_size << "\"\n";
        fout << "\t\t\t\t}\n";
        if (i != test_vm.size() - 1)
            fout << "\t\t\t},\n";
        else
            fout << "\t\t\t}\n";
    }
    fout << "\t\t]\n";
    fout << "}\n";
    fout.close();
}

void controller::print_usage() {
    cout << "USAGE: IOStormplus [options] {parameters}        " << endl;
    cout << "Options:                                             " << endl;
    cout << "help                   Display usage.                " << endl;
    cout << "agent                  Configure the test VM agents. " << endl;
    cout << "start                  Start test job.               " << endl;
}

void controller::start_worker_print_usage() {
    cout << "USAGE: IOStormplus start {parameters}            " << endl;
    cout << "parameters:                                          " << endl;
    cout << "-std                   Start standard test.          " << endl;
    cout << "{default}              Start custom test.            " << endl;
}

void controller::start_worker(int argc, char *argv[]) {
    if (argc == 0) {
        write_log("Start custom test.");
        pre_sync();
        custom_worker();
    }else if (argc > 1)
        start_worker_print_usage();
    else if (strcmp(argv[0], "-std") == 0) {
        write_log("Start standard test.");
        pre_sync();
        standard_worker();
    }else
        start_worker_print_usage();
}


void controller::agent_worker_print_usage() {
    cout << "USAGE: IOStormplus agent [options] {parameters}                         " << endl;
    cout << "Options:                                                                    " << endl;
    cout << "help                Display usage.                                          " << endl;
    cout << "add                 Register a test VM.                                     " << endl;
    cout << "show                Display all currently registered test VM information.   " << endl;
    cout << "rm                  remove a test VM.                                       " << endl;
 //   cout << "test                Determine the working status of all registered test VM. " << endl;
}

void controller::agent_worker_show() {
    print_test_vm_info();
}

void controller::agent_worker_test() {
    write_log("Using pre-sync to determine the working status of all registered test VM.");
    pre_sync();
}

void controller::agent_worker_register(int argc, char *argv[]) {
    string vm_name, vm_ip, vm_os, vm_size = "";
    if (argc != 4)
        agent_worker_register_print_usage();
    else if (1) { //TODO: Add error check
        vm_name = argv[0];
        vm_ip = argv[1];
        vm_os = argv[2];
        vm_size = argv[3];
        if (vm_os == "linux" || vm_os == "windows") {
            bool exist_flag = 0;
            for (auto vm_itr : test_vm) {
                if (vm_itr.vm_name == vm_name) {
                    exist_flag = 1;
                    break;
                }
            }
            if (exist_flag) {
                cout << "VM " + vm_name + " has already been registered.";
            }else{
                test_vm.push_back(vm(vm_name, vm_ip, vm_os, vm_size));
                write_config();
                write_log("Register test VM " + vm_name + " succeeded.");
                print_test_vm_info();
            }
        }else {
            cout << "ERROR: Illegal VM OS type";
            agent_worker_register_print_usage();
        }
    }else{
        cout << "ERROR: Illegal VM IP address" << endl;
        agent_worker_register_print_usage();
    }
}

void controller::agent_worker_register_print_usage() {
    cout << "USAGE: IOStormplus agent add [VM name] [VM internal ip] [VM OS(linux / windows)] [VM size]" << endl;
}

void controller::agent_worker_remove_print_usage() {
    cout << "USAGE: IOStormplus agent rm [VM name]" << endl;
}

void controller::agent_worker_remove(int argc, char *argv[]) {
    string vm_name;
    if (argc != 1)
        agent_worker_remove_print_usage();
    else
        vm_name = argv[0];
    bool find_flag = 0;
    for (auto vm_itr = test_vm.begin(); vm_itr != test_vm.end(); vm_itr++) {
        if (vm_itr->vm_name == vm_name) {
            find_flag = 1;
            test_vm.erase(vm_itr);
            break;
        }
    }
    write_config();
    if (find_flag){
        write_log("Remove test VM " + vm_name + " succeeded.");
        print_test_vm_info();
    }
    else
        write_log("Can not find registered test VM " + vm_name + ".");
}


void controller::agent_worker(int argc, char *argv[]) {
    if (argc == 0)
        agent_worker_print_usage();
    else if (strcmp(argv[0], "add") == 0) {
        agent_worker_register(argc - 1, argv + 1);
    }else if (strcmp(argv[0], "show") == 0) {
        agent_worker_show();
    }else if (strcmp(argv[0], "test") == 0) {
        agent_worker_test();
    }else if (strcmp(argv[0], "rm") == 0) {
        agent_worker_remove(argc - 1, argv + 1);
    }else
        agent_worker_print_usage();
}

report_summary controller::analyze_standard_output(string output_file) {
    ifstream fin(output_file, ios_base::in);
    string buffer;
    report_summary res;
    while (!fin.eof()) {
        getline(fin, buffer);
        int pos = buffer.find("read: IOPS=");
        if (pos != string::npos) {
            pos += 11;
            double num = 0;
            int point_count = 0;
            bool point_flag = 0;
            for (int i = pos;i < buffer.size();i++) {
                if (buffer[i] >= '0' && buffer[i] <= '9') {
                    num = (num * 10) + buffer[i] - '0';
                    if (point_flag)
                        point_count++;
                }
                else if (buffer[i] == 'k')
                    num *= 1000;
                else if (buffer[i] == '.') {
                    point_flag = 1;
                }else
                    break;
            }
            num /= pow(10,point_count);
            res.read_iops.push_back((int)num);
        }
        pos = buffer.find("write: IOPS=");
        if (pos != string::npos) {
            pos += 12;
            double num = 0;
            int point_count = 0;
            bool point_flag = 0;
            for (int i = pos;i < buffer.size();i++) {
                if (buffer[i] >= '0' && buffer[i] <= '9') {
                    num = (num * 10) + buffer[i] - '0';
                    if (point_flag)
                        point_count++;
                }
                else if (buffer[i] == 'k')
                    num *= 1000;
                else if (buffer[i] == '.') {
                    point_flag = 1;
                }else
                    break;
            }
            num /= pow(10,point_count);
            res.write_iops.push_back((int)num);
        }
    }
    return res;
}

void controller::analyze_data(string workload_path) {
    string output_file;
    vector<string> linux_jobs = list_files_in_directory(workload_path + "linux\\");
    vector<string> windows_jobs = list_files_in_directory(workload_path + "windows\\");
    for (auto &vm_itr : test_vm) {
        if (vm_itr.vm_os == vm::os_type::linux) {
            for (auto job : linux_jobs) {
                string jobname = job;
                if (jobname.find(".job") != string::npos)
                    jobname = jobname.replace(jobname.find(".job"),4,"");
                output_file = output + vm_itr.vm_name + "_" + jobname + ".out";
                vm_itr.test_result[job] = analyze_standard_output(output_file);
            }
        }else{
            for (auto job : windows_jobs) {
                string jobname = job;
                if (jobname.find(".job") != string::npos)
                    jobname = jobname.replace(jobname.find(".job"),4,"");
                output_file = output + vm_itr.vm_name + "_" + jobname + ".out";
                vm_itr.test_result[job] = analyze_standard_output(output_file);
            }
        }
    }
}

structlog LOGCFG = {};

int main(int argc,char *argv[]) {
    controller controller_instance(agents_config_file);
    if (controller_instance.has_error())
        return 0;

    if (argc == 1)
        controller_instance.print_usage();
    else if (strcmp(argv[1], "agent") == 0) {
        controller_instance.agent_worker(argc - 2, argv + 2);
    }else if (strcmp(argv[1], "start") == 0) {
        controller_instance.start_worker(argc - 2, argv + 2);
    }else if (strcmp(argv[1], "init") == 0) {
        controller_instance.initialize();
    }else
        controller_instance.print_usage();
    return 0;
}
