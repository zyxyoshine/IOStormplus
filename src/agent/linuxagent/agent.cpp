#include "constant.h"
#include "..\agent.h"
#include "..\..\common\logger.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <ctime>
//#include <unistd.h>

extern int pclose(FILE* stream);
extern FILE* popen(const char *command, const char *mode);
extern int sleep(double milliseconds);

using namespace std;



namespace IOStormPlus{

	class LinuxAgent:public IAgent{
	public:        

		string LinuxAgent::ExecuteCommand(string cmd) {
			char buffer[128];
			string result = "";
			FILE* pipe = popen(cmd.c_str(), "r");
			if (!pipe)
				throw runtime_error("popen() failed!");
			try {
				while (!feof(pipe)) {
					if (fgets(buffer, 128, pipe) != NULL) {
						result += buffer;
					}
				}
			} 
			catch (...) {
				pclose(pipe);
				throw;
			}
			pclose(pipe);
			return result;
		}

		vector<string> LinuxAgent::ListFilesInDirectory(string rootPath) {
			vector<string> res;
			string lsCmd = "ls " + rootPath;
			string lsResut = ExecuteCommand(lsCmd.c_str());
			int pre = 0;
			for (int i = 0;i < lsResut.length();i++) {
				if (lsResut[i] == '\n') {
					res.push_back(lsResut.substr(pre,i - pre));
					pre = i + 1;
				}
			}
			return res;
		}

		void LinuxAgent::Sync() {
			ifstream fin;
			fin.open(ControllerTempFilePath, ios_base::in);
			while (fin.fail()) {
				sleep(SyncWaitTime);
				cerr << strerror(errno) << endl;
				fin.open(ControllerTempFilePath , ios_base::in);
			}
			string reader;
			cout << "waiting for controller requests" << endl;
			while(1) {
				fin.close();
				sleep(SyncWaitTime);
				fin.open(ControllerTempFilePath , ios_base::in);
				fin >> reader;
				if (reader == "PRESYNC") {
					cout << "pre-sync succeeded" << endl;
					break;
				}
			}
			fin.close();
			ofstream fout(ClientTempFilePath, ios_base::out | ios_base::trunc);
			fout << "SYNCDONE";
			fout.flush();
			fout.close();
		}

		void LinuxAgent::Run() {
			string hostname = ExecuteCommand("hostname");
			if (hostname.find('\n') != string::npos)
				hostname = hostname.replace(hostname.find('\n'),1,"");
			cout << hostname << endl;
			ifstream fin(ControllerTempFilePath, ios_base::in);
			string reader;
			while(true) {
				fin.seekg(0, ios::beg);
				fin >> reader;
				if (reader == "START") {
					vector<string> jobs = ListFilesInDirectory(WorkLoadFolderPath);
					cout << "Running fio workload." << endl;
					for (int i = 0 ;i < jobs.size();i++){
						string jobname = jobs[i];
						if (jobname.find(".job") != string::npos)
							jobname = jobname.replace(jobname.find(".job"),4,"");
						string cmd_run_fio = "fio --output=" + jobname + ".out " + " " + WorkLoadFolderPath + jobs[i];
						ExecuteCommand(cmd_run_fio.c_str());
						string cmd_copy_output = "cp -pf " + jobname + ".out " + " " + OutputFolderPath + hostname + "_" + jobname + ".out";
						ExecuteCommand(cmd_copy_output.c_str());
						string cmd_rm_temp_file = "rm -f " + jobname + "*";
						ExecuteCommand(cmd_rm_temp_file.c_str());
					}
					break;
				}
				sleep(10);
			}
			fin.close();
			string cmd_rm_job_files = "rm -f " + WorkLoadFolderPath + "*";
			ExecuteCommand(cmd_rm_job_files.c_str());
			ofstream fout(ClientTempFilePath, ios_base::out | ios_base::trunc);
			fout << "DONE" << endl;
			cout << "Done !" << endl;
			fout.close();
		}
	};
}

int main(int argc,char *argv[]) {
	IOStormPlus::LinuxAgent agent;
    
	string hostname = agent.ExecuteCommand("hostname");
    if (hostname.find('\n') != string::npos)
        hostname = hostname.replace(hostname.find('\n'),1,"");
    ofstream fout("/samba/info/" + hostname,ios_base::out | ios_base::trunc);
    fout << argv[1] << " linux " << argv[2] << endl;
    fout.close();


	while(true) {
        agent.Sync();
        agent.Run();
    }
	return 0;
}
