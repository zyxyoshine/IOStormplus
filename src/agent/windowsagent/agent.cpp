#include "header/constant.h"
#include "../header/baseagent.h"
#include "../../common/header/logger.h"
#include <stdexcept>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <ctime>
#include <stdarg.h>

using namespace std;

namespace IOStormPlus{

	class WindowsAgent:public BaseAgent{
	public:
		WindowsAgent(){
			InitLogger();
		}
	protected:        
		string WindowsAgent::ExecuteScript(string command) {
			Logger::LogVerbose("Run Script "+command);
			char buffer[128];
			string result = "";
			FILE* pipe = _popen(command.c_str(), "r");
			if (!pipe) {
				Logger::LogError("_popen() failed!");
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
				Logger::LogError("Execute script error");
				throw;
			}
			_pclose(pipe);
			Logger::LogInfo("Execute script succeed");
			return result;
		}

		vector<string> WindowsAgent::ListFilesInDirectory(string rootPath) {
			Logger::LogVerbose("Start List Files under directory");
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
			Logger::LogVerbose("Done List Files");
			return res;
		}

		string WindowsAgent::RunScript(AgentCommand command, ...){
			va_list args;
			int count;
			va_start(args, count);
			switch(command){
				case AgentCommand::CopyOutputCmd: {
					string jobname = va_arg(args, string);
					string hostname = va_arg(args, string);
					string copyOutputCmd = "copy " + jobname + ".out " + " " + OutputFolder + hostname + "_" + jobname + ".out";
					ExecuteScript(copyOutputCmd);	
					break;				
				}
				case AgentCommand::DelTempFileCmd: {
					string jobname = va_arg(args, string);
					string rmTempFileCmd = "DEL /F /Q " + jobname + "*";
					ExecuteScript(rmTempFileCmd);	
					break;		
				}
				case AgentCommand::DelJobFilesCmd: {
					string rmJobFilesCmd = "DEL /F /Q " + GetWorkloadFolderPath() + "*";
					ExecuteScript(rmJobFilesCmd);
					break;					
				}
				default: {
					BaseAgent::RunScript(command, args);
				}
			}
			return NULL;
		}

		void WindowsAgent::Wait(){
			Sleep(SyncWaitTime);
		}

        string WindowsAgent::GetControlTempFilePath(){
			return ControllerTempFilePath;
		}

        string WindowsAgent::GetClientTempFilePath(){
			return ClientTempFilePath;
		}

        string WindowsAgent::GetLogFilePath(){
			return LogFilePath;
		}	

		string WindowsAgent::GetWorkloadFolderPath(){
			return WorkLoadFolderPath;
		}
	};

}

int main(int argc,char *argv[]) {
	// cout << "hh222" << argv[1];
	// ofstream fout("\\\\" + string(argv[1]) + "\\agents\\" + string(argv[2]));
	// cout << "hh0";
	// fout << string(argv[3]) << " windows " << string(argv[4]) << endl;
	// fout.close();
	IOStormPlus::WindowsAgent agent;
	while(true){
		agent.Run();
	}
	return 0;
}

