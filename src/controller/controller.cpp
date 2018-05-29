#include "Controller.h"
#include <windows.h>
#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "logger.h"
#include "helper.h"
#include <algorithm>
#include <ctime>
#include <iostream>

using namespace rapidjson;
using namespace std;


#ifdef __cplusplus
extern "C"
{
#endif

namespace IOStormPlus{

    void Controller::InitLogger() {
        time_t t = std::time(0);   
        tm* now = std::localtime(&t);
        stringstream tempStream;
        tempStream << "%d%02d%02d.log" << now->tm_year + 1900 << now->tm_mon + 1 << now->tm_mday << endl;
        Logger::Init(tempStream.str());
    }

    void Controller::PrintTestVMInfo() {
        stringstream tempStream;
        tempStream << "VM Count: " << TestVMs.size() << endl;
        Logger::LogVerbose(tempStream.str());
        Logger::LogVerbose("ID\tName\tIP Address\tOS\tSize");
        
        tempStream.clear();
        for (int i = 0;i < TestVMs.size(); ++i) {
            tempStream.clear();
            tempStream << i + 1 << "\t" << TestVMs[i].GetVMInfo() << endl;
            Logger::LogVerbose(tempStream.str());
        }
    }

    void Controller::PrintTestResultSummary(string workloadRootPath) {
        vector<string> linux_jobs = list_files_in_directory(workloadRootPath + "linux\\");
        vector<string> jobs = list_files_in_directory(workloadRootPath + "windows\\");
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
        sprintf(title, "VM Count: %d", TestVMs.size());
        Logger::LogVerbose(title);
        fout << title << endl;
        for (auto job : jobs) {
            Logger::LogVerbose("Job: " + job);
            fout << "Job: " + job << endl;
            Logger::LogVerbose("ID\tName\tIP Address\tOS\tSize\tR(MIN)\tR(MAX)\tR(AVG)\tW(MIN)\tW(MAX)\tW(AVG)");
            fout << "ID\tName\tIP Address\tOS\tSize\tR(MIN)\tR(MAX)\tR(AVG)\tW(MIN)\tW(MAX)\tW(AVG)" << endl;
            string vm_id;
            for (int i = 0;i < TestVMs.size();i++) {
                if (TestVMs[i].TestResults.count(job)) {
                    temp_stream.clear();
                    temp_stream << i + 1;
                    temp_stream >> vm_id;
                    Logger::LogVerbose(vm_id + "\t" + TestVMs[i].GetVMResult(job));
                    fout << vm_id + "\t" + TestVMs[i].GetVMResult(job) << endl;
                }
            }
            cout << endl;
        }
        fout.close();
    }

    Controller::Controller(string configFilename) {
        InitLogger();
        m_isReady = false;

        /* Load JSON file */
        fstream fin(configFilename);
        string data, content = "";
        while (!fin.eof()) {
            getline(fin, data);
            content += data;
        }
        fin.close();

        Document VMConfig;
        if (VMConfig.Parse(content.c_str()).HasParseError()) {
            stringstream tempStream;
            tempStream << "parse test VM configuration failed! " << content.c_str() << endl;
            Logger::LogError(tempStream.str());
            return;
        }
        
        int vmCount = VMConfig["count"].GetInt();
        auto vmInfo = VMConfig["value"].GetArray();
        for (int i = 0; i < vmCount; ++i) {
            TestVMs.push_back(TestVM(vmInfo[i]["name"].GetString(), vmInfo[i]["ip"].GetString(), vmInfo[i]["info"]["type"].GetString(), vmInfo[i]["info"]["size"].GetString()));
        }

        m_isReady = true;
    }

    void Controller::initialize() {
        TestVMs.clear();
        vector<string> agents = list_files_in_directory(agent_info);
        ifstream fin;
        string internalIP,osType,size;
        for (auto agentName : agents) {
            fin.open(agent_info + agentName);
            fin >> internalIP >> osType >> size;
            TestVMs.push_back(TestVM(agentName, internalIP, osType, size));
            Logger::LogVerbose("Register test VM " + agentName + " succeeded.");
            fin.close();
        }
        write_config();
        PrintTestVMInfo();
    }

    bool Controller::IsReady() {
        return m_isReady;
    }

    void Controller::pre_sync() {
        ofstream fout;
        string reader;
        ifstream fin;
        for (auto vm_itr : TestVMs) {
            Logger::LogVerbose("Sending pre-sync request to test VM " + vm_itr.GetName() + "(" + vm_itr.GetInternalIP() + ")");
            exec_command(string("del /f " + vm_itr.GetSharePath() + tempfolder + "client.tmp").c_str());
            fout.open(vm_itr.GetSharePath() + tempfolder + "Controller.tmp", ios_base::out | ios_base::trunc);
            fout << "PRESYNC";
            fout.close();
        }

        Logger::LogVerbose("Waiting for agents response.");
        bool all_done = false;
        map<string, bool> done_job;
        done_job.clear();
        while (!all_done) {
            all_done = true;
            for (auto vm_itr : TestVMs) {
                if (done_job[vm_itr.GetInternalIP()] == 1)
                    continue;
                ifstream fin;
                fin.open(vm_itr.GetSharePath() + tempfolder + "client.tmp", ios_base::in);
                if (!fin.fail()) {
                    fin >> reader;
                    if(reader == "SYNCDONE") {
                        done_job[vm_itr.GetInternalIP()] = 1;
                        Logger::LogVerbose("Test VM " + vm_itr.GetName() + "(" + vm_itr.GetInternalIP() + ")" + " pre-sync succeeded.");
                    }
                    else
                        all_done = false;
                }
                else
                    all_done = false;
                fin.close();
            }
        }
        Logger::LogVerbose("All test VM pre-sync succeeded!");
        cout << endl;
    }

    void Controller::standard_worker() {
        ifstream fin;
        ofstream fout;
        string reader;
        for (auto vm_itr : TestVMs) {
            if (vm_itr.GetOSType() == OSType::Linux)
                exec_command(string("xcopy workload\\std\\linux " + vm_itr.GetSharePath() + workload + " /s /h /d /y").c_str());
            else
                exec_command(string("xcopy workload\\std\\windows " + vm_itr.GetSharePath() + workload + " /s /h /d /y").c_str());
            Logger::LogVerbose("Sending work request to test VM " + vm_itr.GetName() + "(" + vm_itr.GetInternalIP() + ")");
            exec_command(string("del /f " + vm_itr.GetSharePath() + tempfolder + "client.tmp").c_str());
            fout.open(vm_itr.GetSharePath() + tempfolder + "Controller.tmp", ios_base::out | ios_base::trunc);
            fout << "START";
            fout.close();
        }
        Logger::LogVerbose("Waiting for agents response.");
        map<string, bool> done_job;
        done_job.clear();

        bool all_done = false;
        while (!all_done) {
            all_done = true;
            for (auto vm_itr : TestVMs) {
                if (done_job[vm_itr.GetInternalIP()] == 1)
                    continue;
                fin.close();
                fin.clear();
                Sleep(1000);
                fin.open(vm_itr.GetSharePath() + tempfolder + "client.tmp", ios_base::in);
                fin >> reader;
                if (reader == "DONE") {
                    done_job[vm_itr.GetInternalIP()] = 1;
                    Logger::LogVerbose("Test VM " + vm_itr.GetName() + "(" + vm_itr.GetInternalIP() + ")" + " successfully completed work.");
                }
                else
                    all_done = false;
            }
            fin.close();
        }
        Logger::LogVerbose("All jobs done!");
        analyze_data("workload\\std\\");
        PrintTestResultSummary("workload\\std\\");
        cout << endl;
    }

    void Controller::custom_worker() {
        ifstream fin;
        ofstream fout;
        string reader;
        for (auto vm_itr : TestVMs) {
            if (vm_itr.GetOSType() == OSType::Linux)
                exec_command(string("xcopy workload\\linux " + vm_itr.GetSharePath() + workload + " /s /h /d /y").c_str());
            else
                exec_command(string("xcopy workload\\windows " + vm_itr.GetSharePath() + workload + " /s /h /d /y").c_str());
            Logger::LogVerbose("Sending work request to test VM " + vm_itr.GetName() + "(" + vm_itr.GetInternalIP() + ")");
            exec_command(string("del /f " + vm_itr.GetSharePath() + tempfolder + "client.tmp").c_str());
            fout.open(vm_itr.GetSharePath() + tempfolder + "Controller.tmp", ios_base::out | ios_base::trunc);
            fout << "START";
            fout.close();
        }
        Logger::LogVerbose("Waiting for agents response.");
        map<string, bool> done_job;
        done_job.clear();

        bool all_done = false;
        while (!all_done) {
            all_done = true;
            for (auto vm_itr : TestVMs) {
                if (done_job[vm_itr.GetInternalIP()] == 1)
                    continue;
                fin.close();
                fin.clear();
                Sleep(1000);
                fin.open(vm_itr.GetSharePath() + tempfolder + "client.tmp", ios_base::in);
                fin >> reader;
                if (reader == "DONE") {
                    done_job[vm_itr.GetInternalIP()] = 1;
                    Logger::LogVerbose("Test VM " + vm_itr.GetName() + "(" + vm_itr.GetInternalIP() + ")" + " successfully completed work.");
                }
                else
                    all_done = false;
            }
            fin.close();
        }
        Logger::LogVerbose("All jobs done!");
        analyze_data("workload\\");
        PrintTestResultSummary("workload\\");
        cout << endl;
    }

    void Controller::write_config() {
        ofstream fout(agents_config_file, ios_base::out | ios_base::trunc);
        fout << "{\n";
        fout << "\t\"count\":" << TestVMs.size() << ",\n";
        fout << "\t\"value\":\n";
        fout << "\t\t[\n";
        for (int i = 0; i < TestVMs.size(); i++) {
            fout << "\t\t\t{\n";
            fout << "\t\t\t\t\"name\":\"" << TestVMs[i].GetName() << "\",\n";
            fout << "\t\t\t\t\"ip\":\"" << TestVMs[i].GetInternalIP() << "\",\n";
            fout << "\t\t\t\t\"info\":{\n";
            fout << "\t\t\t\t\t\"type\":\"" << (TestVMs[i].GetOSType() == OSType::Linux ? "linux" : "windows") << "\",\n";
            fout << "\t\t\t\t\t\"size\":\"" << TestVMs[i].GetSize() << "\"\n";
            fout << "\t\t\t\t}\n";
            if (i != TestVMs.size() - 1)
                fout << "\t\t\t},\n";
            else
                fout << "\t\t\t}\n";
        }
        fout << "\t\t]\n";
        fout << "}\n";
        fout.close();
    }

    void Controller::PrintUsage() {
        cout << "USAGE: IOStormplus [options] {parameters}        " << endl;
        cout << "Options:                                             " << endl;
        cout << "help                   Display usage.                " << endl;
        cout << "agent                  Configure the test VM agents. " << endl;
        cout << "start                  Start test job.               " << endl;
    }

    void Controller::start_worker_PrintUsage() {
        cout << "USAGE: IOStormplus start {parameters}            " << endl;
        cout << "parameters:                                          " << endl;
        cout << "-std                   Start standard test.          " << endl;
        cout << "{default}              Start custom test.            " << endl;
    }

    void Controller::start_worker(int argc, char *argv[]) {
        if (argc == 0) {
            Logger::LogVerbose("Start custom test.");
            pre_sync();
            custom_worker();
        }else if (argc > 1)
            start_worker_PrintUsage();
        else if (strcmp(argv[0], "-std") == 0) {
            Logger::LogVerbose("Start standard test.");
            pre_sync();
            standard_worker();
        }else
            start_worker_PrintUsage();
    }


    void Controller::agent_worker_PrintUsage() {
        cout << "USAGE: IOStormplus agent [options] {parameters}                         " << endl;
        cout << "Options:                                                                    " << endl;
        cout << "help                Display usage.                                          " << endl;
        cout << "add                 Register a test VM.                                     " << endl;
        cout << "show                Display all currently registered test VM information.   " << endl;
        cout << "rm                  remove a test VM.                                       " << endl;
    //   cout << "test                Determine the working status of all registered test VM. " << endl;
    }

    void Controller::agent_worker_show() {
        PrintTestVMInfo();
    }

    void Controller::agent_worker_test() {
        Logger::LogVerbose("Using pre-sync to determine the working status of all registered test VM.");
        pre_sync();
    }

    void Controller::agent_worker_register(int argc, char *argv[]) {
        string name, internalIP, osType, size = "";
        if (argc != 4)
            agent_worker_register_PrintUsage();
        else if (1) { //TODO: Add error check
            name = argv[0];
            internalIP = argv[1];
            osType = argv[2];
            osType = argv[3];
            if (osType == "linux" || osType == "windows") {
                bool exist_flag = 0;
                for (auto vm_itr : TestVMs) {
                    if (vm_itr.GetName() == name) {
                        exist_flag = 1;
                        break;
                    }
                }
                if (exist_flag) {
                    cout << "VM " + name + " has already been registered.";
                }else{
                    //TestVMs.push_back(TestVm(name, internalIP, osType, size);
                    write_config();
                    Logger::LogVerbose("Register test VM " + name + " succeeded.");
                    PrintTestVMInfo();
                }
            }else {
                cout << "ERROR: Illegal VM OS type";
                agent_worker_register_PrintUsage();
            }
        }else{
            cout << "ERROR: Illegal VM IP address" << endl;
            agent_worker_register_PrintUsage();
        }
    }

    void Controller::agent_worker_register_PrintUsage() {
        cout << "USAGE: IOStormplus agent add [VM name] [VM internal ip] [VM OS(linux / windows)] [VM size]" << endl;
    }

    void Controller::agent_worker_remove_PrintUsage() {
        cout << "USAGE: IOStormplus agent rm [VM name]" << endl;
    }

    void Controller::agent_worker_remove(int argc, char *argv[]) {
        string vmName;
        if (argc != 1)
            agent_worker_remove_PrintUsage();
        else
            vmName = argv[0];
        bool find_flag = 0;
        for (auto vm_itr = TestVMs.begin(); vm_itr != TestVMs.end(); vm_itr++) {
            if (vm_itr->GetName() == vmName) {
                find_flag = 1;
                TestVMs.erase(vm_itr);
                break;
            }
        }
        write_config();
        if (find_flag){
            Logger::LogVerbose("Remove test VM " + vmName + " succeeded.");
            PrintTestVMInfo();
        }
        else
            Logger::LogVerbose("Can not find registered test VM " + vmName + ".");
    }


    void Controller::agent_worker(int argc, char *argv[]) {
        if (argc == 0)
            agent_worker_PrintUsage();
        else if (strcmp(argv[0], "add") == 0) {
            agent_worker_register(argc - 1, argv + 1);
        }else if (strcmp(argv[0], "show") == 0) {
            agent_worker_show();
        }else if (strcmp(argv[0], "test") == 0) {
            agent_worker_test();
        }else if (strcmp(argv[0], "rm") == 0) {
            agent_worker_remove(argc - 1, argv + 1);
        }else
            agent_worker_PrintUsage();
    }

    ReportSummary Controller::analyze_standard_output(string output_file) {
        ifstream fin(output_file, ios_base::in);
        string buffer;
        ReportSummary res;
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
                res.ReadIOPS.push_back((int)num);
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
                res.WriteIOPS.push_back((int)num);
            }
        }
        return res;
    }

    void Controller::analyze_data(string workloadRootPath) {
        string output_file;
        vector<string> linux_jobs = list_files_in_directory(workloadRootPath + "linux\\");
        vector<string> windows_jobs = list_files_in_directory(workloadRootPath + "windows\\");
        for (auto &vm_itr : TestVMs) {
            if (vm_itr.GetOSType() == OSType::Linux) {
                for (auto job : linux_jobs) {
                    string jobname = job;
                    if (jobname.find(".job") != string::npos)
                        jobname = jobname.replace(jobname.find(".job"),4,"");
                    output_file = output + vm_itr.GetName() + "_" + jobname + ".out";
                    string cmd_copy_output = "copy " + vm_itr.GetSharePath() + output_file + " " + output_file + " /y";
                    string res = exec_command(cmd_copy_output.c_str());
                    while (res.find("cannot") != string::npos) {   //retry
                        res = exec_command(cmd_copy_output.c_str());
                    }
                    vm_itr.TestResults[job] = analyze_standard_output(output_file);
                }
            }else{
                for (auto job : windows_jobs) {
                    string jobname = job;
                    if (jobname.find(".job") != string::npos)
                        jobname = jobname.replace(jobname.find(".job"),4,"");
                    output_file = output + vm_itr.GetName() + "_" + jobname + ".out";
                    string cmd_copy_output = "copy " + vm_itr.GetSharePath() + output_file + " " + output_file + " /y";
                    string res = exec_command(cmd_copy_output.c_str());
                    while (res.find("cannot") != string::npos) {
                        res = exec_command(cmd_copy_output.c_str());
                    }
                    vm_itr.TestResults[job] = analyze_standard_output(output_file);
                }
            }
        }
    }

}

#ifdef __cplusplus
}
#endif



