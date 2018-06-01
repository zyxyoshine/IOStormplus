#include "constant.h"
#include "..\defaultagent.h"
#include "..\..\common\logger.h"
#include <stdexcept>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <ctime>

using namespace std;

namespace IOStormPlus{

	class WindowsAgent:DefaultAgent{
	public:        

		string WindowsAgent::ExecuteCommand(string command) {
			char buffer[128];
			string result = "";
			FILE* pipe = _popen(command.c_str(), "r");
			if (!pipe) {
				throw runtime_error("popen() failed!");
			}
			try {
				while (!feof(pipe)) {
					if (fgets(buffer, 128, pipe) != NULL){
						result += buffer;
					}
				}
			} 
			catch (...) {
				_pclose(pipe);
				throw;
			}
			_pclose(pipe);
			return result;
		}

		vector<string> WindowsAgent::ListFilesInDirectory(string rootPath) {
			WIN32_FIND_DATA data;
			HANDLE hFind = FindFirstFile((rootPath + "*").c_str(), &data);      // DIRECTORY
			vector<string> res;
			if ( hFind != INVALID_HANDLE_VALUE ) {
				do {
					string file_name = data.cFileName;
					if (file_name != "." && file_name != "..")
						res.push_back(file_name);
				} 
				while (FindNextFile(hFind, &data));
				FindClose(hFind);
			}
			return res;
		}

		void WindowsAgent::Sync() {
			ifstream fin;
			
			fin.open(ControllerTempFilePath, ios_base::in);
			while (fin.fail()) {
				Sleep(SyncWaitTime);
				cerr << strerror(errno) << endl;
				fin.open(ControllerTempFilePath , ios_base::in);
			}
			
			string buf;
			Logger::LogInfo("waiting for controller requests");
			while(true) {
				fin.close();
				Sleep(SyncWaitTime);
				fin.open(ControllerTempFilePath, ios_base::in);
				fin >> buf;
				if (buf == "PRESYNC") {
					Logger::LogInfo("pre-sync succeeded");
					break;
				}
			}
			fin.close();

			ofstream fout(ClientTempFilePath, ios_base::out | ios_base::trunc);
			fout << "SYNCDONE";
			fout.flush();
			fout.close();
		}

		void WindowsAgent::Run() {
			string hostname = ExecuteCommand("hostname");
			if (hostname.find('\n') != string::npos) {
				hostname = hostname.replace(hostname.find('\n'),1,"");
			}

			ifstream fin(ControllerTempFilePath, ios_base::in);
			string buf;
			while(true) {
				fin.seekg(0, ios::beg);
				fin >> buf;
				if (buf == "START") {
					vector<string> jobs = ListFilesInDirectory(WorkLoadFolderPath);
					Logger::LogVerbose("Running fio workload.");
					for (int i = 0 ;i < jobs.size();i++){
						string jobname = jobs[i];
						if (jobname.find(".job") != string::npos) {
							jobname = jobname.replace(jobname.find(".job"),4,"");
						}
						string runFIOCmd = "fio --output=" + jobname + ".out " + " " + WorkLoadFolderPath + jobs[i];
						ExecuteCommand(runFIOCmd);
						string copyOutputCmd = "copy " + jobname + ".out " + " " + OutputFolder + hostname + "_" + jobname + ".out";
						ExecuteCommand(copyOutputCmd);
						string rmTempFileCmd = "DEL /F /Q " + jobname + "*";
						ExecuteCommand(rmTempFileCmd);
					}
					break;
				}
				Sleep(SyncWaitTime);
			}
			fin.close();

			string rmJobFilesCmd = "DEL /F /Q " + WorkLoadFolderPath + "*";
			ExecuteCommand(rmJobFilesCmd);
			ofstream fout(ClientTempFilePath, ios_base::out | ios_base::trunc);
			if (fout.fail()) {
				Logger::LogError(strerror(errno));
			}
			fout << "DONE" << endl;
			Logger::LogVerbose("Done !");
			fout.close();
		}		
	};

}

int main(int argc,char *argv[]) {
	ofstream fout("\\\\" + string(argv[1]) + "\\agents\\" + string(argv[2]));
	fout << string(argv[3]) << " windows " << string(argv[4]) << endl;
	fout.close();

	IOStormPlus::WindowsAgent agent;
	while(true) {
		agent.Sync();
		agent.Run();
	}
	return 0;
}
