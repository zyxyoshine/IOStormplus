#include <bits/stdc++.h>
//#include <boost/filesystem.hpp>
//#include <boost/progress.hpp>
#include <stdexcept>
#include <unistd.h>

//namespace fs = boost::filesystem;
using namespace std;

/*
int main1(int argc, char* argv[]) {
	boost::progress_timer t(clog);
	fs::path full_path(fs::initial_path<fs::path>() );
	if (argc > 1)
		full_path = fs::system_complete(fs::path(argv[1]));
	else
		cout << "\n err1" << endl;

	int file_count = 0;
	int dir_count = 0;
	int other_count = 0;
	int err_count = 0;

	if (!fs::exists(full_path)) {
		cout << "\n err 2" << endl;
		return -1;
	}
	if (fs::is_directory(full_path)) {
		cout << "\nIn directory: " << "full_path.directory_string()" << "\n\n";
		fs::directory_iterator end_iter;
		for (fs::directory_iterator dir_itr(full_path); dir_itr != end_iter;++dir_itr) {
			try {
				if (fs::is_directory(dir_itr->status())) {
					++dir_count;
					cout << dir_itr->path().filename() << " [directory]\n";
				}
				else if (fs::is_regular_file(dir_itr->status())) {
					++file_count;
					cout << dir_itr->path().filename() << endl;
				}
				else {
					++other_count;
					cout << dir_itr->path().filename() << endl;
				}
			}
			catch (const exception &ex) {
				err_count++;
				cout << dir_itr->path().filename() << " [other]\n";
			}
		}
		cout << "\n" << file_count << " files\n"
			<< dir_count << " directories\n"
			<< other_count << " others\n"
			<< err_count << " errors\n";
	}
	else{
		cout << "found: " << full_path.filename() << "\n";
	}
	return 0;
}
*/

const string share_path = "/samba/";
const string workspace = "~/fiojob/";
const string workload = "workload/";
const string infotemp = "temp/";
const string output = "output/";
const string command_copy_config = "cp -r " + share_path + workload + " " + workspace;
const string command_copy_output = "cp -pf " + workspace + "linuxvm0_rand-RW.out" + " " + share_path + output + "linuxvm0_rand-RW.out";
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
	fin.open(control_file_path + "controller.tmp", ios_base::in);
	while (fin.fail()) {
		sleep(10);
		cerr << strerror(errno) << endl;
		fin.open(control_file_path + "controller.tmp" , ios_base::in);
	}
	string reader;
	cout << "waiting for controller requests" << endl;
	while(1) {
		//cout << "waiting for controller requests" << endl;
		fin.close();
		sleep(10);
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
			exec(command_copy_output.c_str());
			break;
		}
		sleep(10);
	}
	fin.close();
	//exec("rm -f /samba/info/client.tmp");
	ofstream fout(control_file_path + "client.tmp", ios_base::out | ios_base::trunc);
	fout << "DONE" << endl;
	cout << "Done !" << endl;
	fout.close();
}

int main(int argc,char *argv[]) {

//	if (argc > 0 && argv[1] == "-demo")
//		cout << "Demo mode (mock the fio output) " << endl;
//cout << exec(command_copy_config.c_str()) << endl;
	while(1) {
	pre_sync();
	worker();
}
	//cout << exec("fio --output=fio-test.out fio-rand-RW.job") << endl;
	//cout << "finish!" << endl;
	return 0;
}
