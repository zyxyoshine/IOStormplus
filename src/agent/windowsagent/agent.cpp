#include "constant.h"
#include "..\baseagent.h"
#include "..\..\common\logger.h"
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
	protected:        

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

		string WindowsAgent::RunCommand(AgentCommand command, ...){
			va_list args;
			int count;
			va_start(args, count);
			switch(command){
				case AgentCommand::CopyOutputCmd: {
					string jobname = va_arg(args, string);
					string hostname = va_arg(args, string);
					string copyOutputCmd = "copy " + jobname + ".out " + " " + OutputFolder + hostname + "_" + jobname + ".out";
					ExecuteCommand(copyOutputCmd);	
					break;				
				}
				case AgentCommand::DelTempFileCmd: {
					string jobname = va_arg(args, string);
					string rmTempFileCmd = "DEL /F /Q " + jobname + "*";
					ExecuteCommand(rmTempFileCmd);	
					break;		
				}
				case AgentCommand::DelJobFilesCmd: {
					string rmJobFilesCmd = "DEL /F /Q " + GetWorkloadFolderPath() + "*";
					ExecuteCommand(rmJobFilesCmd);
					break;					
				}
				default: {
					BaseAgent::RunCommand(command, args);
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
