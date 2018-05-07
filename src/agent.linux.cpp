#include <bits/stdc++.h>
#include <stdexcept>
#include <unistd.h>

using namespace std;

const string share_path = "/samba/";
const string workspace = "~/fiojob/";
const string workload = "workload/";
const string infotemp = "temp/";
const string output = "output/";
const string controller_temp_file = "controller.tmp";
const string client_temp_file = "client.tmp";

string exec(const char* cmd) {
	char buffer[128];
	string result = "";
	FILE* pipe = popen(cmd, "r");
	if (!pipe)
		throw runtime_error("popen() failed!");
	try {
		while (!feof(pipe)) {
			if (fgets(buffer, 128, pipe) != NULL)
				result += buffer;
		}
	} catch (...) {
		pclose(pipe);
		throw;
	}
	pclose(pipe);
	return result;
}

void pre_sync() {
	string control_file_path = share_path + infotemp;
	ifstream fin;
	fin.open(control_file_path + controller_temp_file, ios_base::in);
	while (fin.fail()) {
		sleep(10);
		cerr << strerror(errno) << endl;
		fin.open(control_file_path + controller_temp_file , ios_base::in);
	}
	string reader;
	cout << "waiting for controller requests" << endl;
	while(1) {
		fin.close();
		sleep(10);
		fin.open(control_file_path + controller_temp_file , ios_base::in);
		fin >> reader;
		if (reader == "PRESYNC") {
			cout << "pre-sync succeeded" << endl;
			break;
		}
	}
	fin.close();
	ofstream fout(control_file_path + client_temp_file, ios_base::out | ios_base::trunc);
	fout << "SYNCDONE";
	fout.flush();
	fout.close();
}

void worker() {
    string hostname = exec("hostname");
	string control_file_path = share_path + infotemp;
	ifstream fin(control_file_path + controller_temp_file, ios_base::in);
	string reader;
	while(1) {
		fin.seekg(0, ios::beg);
		fin >> reader;
		//cout << reader;
		if (reader == "START") {
            string cmd_ls_workload = "ls " + share_path + workload;
            string ls_res = exec(cmd_ls_workload.c_str());
            int pre = 0;
            vector<string> jobs;
            for (int i = 0;i < ls_res.length();i++) {
                if (ls_res[i] == '\n') {
                    jobs.push_back(ls_res.substr(pre,i - pre));
                    pre = i + 1;
                }
            }
            cout << "Running fio workload." << endl;
            for (int i = 0 ;i < jobs.size();i++){
                string jobname = jobs[i];
                if (jobname.find(".job") != string::npos)
                    jobname = jobname.replace(jobname.find(".job"),4,"");
                string cmd_run_fio = "fio --output=" + jobname + ".out " + " " + share_path + workload + jobs[i];
                exec(cmd_run_fio.c_str());
                string cmd_rm_temp_file = "rm -f " + jobname;
                exec(cmd_rm_temp_file.c_str());
                string cmd_copy_output = "cp -pf " + workspace + jobname + ".out " + " " + share_path + output + hostname + "_" + jobname + ".out";
                exec(cmd_copy_output.c_str());
            }
			break;
		}
		sleep(10);
	}
	fin.close();
	ofstream fout(control_file_path + client_temp_file, ios_base::out | ios_base::trunc);
	fout << "DONE" << endl;
	cout << "Done !" << endl;
	fout.close();
}

int main(int argc,char *argv[]) {
	while(1) {
        pre_sync();
        worker();
    }
	return 0;
}
