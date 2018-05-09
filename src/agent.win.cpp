#include <bits/stdc++.h>
#include <stdexcept>
#include <windows.h>

using namespace std;

const string share_path = "share\\";
const string workspace = "fiojob\\";
const string workload = "workload\\";
const string infotemp = "temp\\";
const string output = "output\\";
const string controller_temp_file = "controller.tmp";
const string client_temp_file = "client.tmp";

string exec_command(const char* cmd) {
	char buffer[128];
	string result = "";
		FILE* pipe = _popen(cmd, "r");
	if (!pipe)
		throw runtime_error("popen() failed!");
	try {
		while (!feof(pipe)) {
			if (fgets(buffer, 128, pipe) != NULL)
				result += buffer;
		}
	} catch (...) {
		_pclose(pipe);
		throw;
	}
	_pclose(pipe);
	return result;
}

vector<string> list_files_in_directory(string dirna) {
    WIN32_FIND_DATA data;
    HANDLE hFind = FindFirstFile((dirna + "*").c_str(), &data);      // DIRECTORY
    vector<string> res;
    if ( hFind != INVALID_HANDLE_VALUE ) {
        do {
            string file_name = data.cFileName;
            if (file_name != "." && file_name != "..")
                res.push_back(file_name);
        } while (FindNextFile(hFind, &data));
        FindClose(hFind);
    }
    return res;
}

void pre_sync() {
	string control_file_path = share_path + infotemp;
	ifstream fin;
	fin.open(control_file_path + controller_temp_file, ios_base::in);
	while (fin.fail()) {
		Sleep(10000);
		cerr << strerror(errno) << endl;
		fin.open(control_file_path + controller_temp_file , ios_base::in);
	}
	string reader;
    cout << "waiting for controller requests" << endl;
	while(1) {
		fin.close();
		Sleep(10000);
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
    string hostname = exec_command("hostname");
    if (hostname.find('\n') != string::npos)
        hostname = hostname.replace(hostname.find('\n'),1,"");
	string control_file_path = share_path + infotemp;
	ifstream fin(control_file_path + controller_temp_file, ios_base::in);
	string reader;
	while(1) {
		fin.seekg(0, ios::beg);
		fin >> reader;
		if (reader == "START") {
            vector<string> jobs = list_files_in_directory(share_path + workload);
			cout << "Running fio workload." << endl;
			for (int i = 0 ;i < jobs.size();i++){
                string jobname = jobs[i];
                if (jobname.find(".job") != string::npos)
                    jobname = jobname.replace(jobname.find(".job"),4,"");
                string cmd_run_fio = "fio --output=" + jobname + ".out " + " " + share_path + workload + jobs[i];
                exec_command(cmd_run_fio.c_str());
                string cmd_copy_output = "copy " + jobname + ".out " + " " + share_path + output + hostname + "_" + jobname + ".out";
                exec_command(cmd_copy_output.c_str());
                string cmd_rm_temp_file = "DEL /F " + jobname + "*";
                exec_command(cmd_rm_temp_file.c_str());
            }
			break;
		}
		Sleep(10000);
	}
	fin.close();
	string cmd_rm_job_files = "DEL /F " + share_path + workload + "*";
    exec_command(cmd_rm_job_files.c_str());
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
