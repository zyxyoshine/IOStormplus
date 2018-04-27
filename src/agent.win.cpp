#include <bits/stdc++.h>
#include <stdexcept>
#include <windows.h>

using namespace std;

const string share_path = "share\\";
const string workspace = "fiojob\\";
const string workload = "workload\\";
const string infotemp = "temp\\";
const string output = "output\\";
const string command_copy_config = "cp -r " + share_path + workload + " " + workspace;
const string command_copy_output = "copy " + workspace + "windowsvm0_rand-RW.out" + " " + share_path + output + "windowsvm0_rand-RW.out";

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

void pre_sync() {

	string control_file_path = share_path + infotemp;
	ifstream fin;
	fin.open(control_file_path + "controller.tmp", ios_base::in);
	while (fin.fail()) {
		Sleep(10000);
		cerr << strerror(errno) << endl;
		fin.open(control_file_path + "controller.tmp" , ios_base::in);
	}
	string reader;
    cout << "waiting for controller requests" << endl;
	while(1) {
		fin.close();
		Sleep(10000);
		fin.open(control_file_path + "controller.tmp" , ios_base::in);
		fin >> reader;
		if (reader == "PRESYNC") {
			cout << "pre-sync succeeded" << endl;
			break;
		}
	}
	fin.close();
	ofstream fout(control_file_path + "client.tmp", ios_base::out | ios_base::trunc);
	fout << "SYNCDONE";
	fout.flush();
	fout.close();
}

void worker() {
	string control_file_path = share_path + infotemp;
	ifstream fin(control_file_path + "controller.tmp", ios_base::in);
	string reader;
	while(1) {
		fin.seekg(0, ios::beg);
		fin >> reader;
		//cout << reader;
		if (reader == "START") {
			cout << "Running fio workload." << endl;
			exec_command(command_copy_output.c_str());
			break;
		}
		Sleep(10000);
	}
	fin.close();
	//exec("rm -f /samba/info/client.tmp");
	ofstream fout(control_file_path + "client.tmp", ios_base::out | ios_base::trunc);
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
