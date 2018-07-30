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
		WindowsAgent(int argc = 0,char *argv[] = NULL) {
			InitLogger();
			CreateStorageClient(IOStormPlus::storageConfigFileName);
			SetAgentInfo(argv[1], argv[2], "windows", argv[3]);
			RegisterOnAzure();
		}

		string ExecuteScript(string command) {
			Logger::LogInfo("Run Script " + command);
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
			Logger::LogInfo("Execute script succeed " + result);
			return result;
		}

	protected:        
		vector<string> ListFilesInDirectory(string rootPath) {
			Logger::LogInfo("Start List Files under directory");
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
			Logger::LogInfo("Done List Files");
			return res;
		}

		string RunScript(AgentCommand command, vector<string> &params){
			Logger::LogInfo("RunScript Start");
			string striptCmdString;
			switch(command){
				case AgentCommand::DelTempFileCmd: {
					string filename = params[0];
					striptCmdString = "DEL /F /Q " + filename + "*";
					Logger::LogInfo("Stript command " + striptCmdString);
					ExecuteScript(striptCmdString);	
					break;		
				}
				case AgentCommand::DelJobFilesCmd: {
					striptCmdString = "DEL /F /Q " + GetWorkloadFolderPath() + "*";
					Logger::LogInfo("Stript command " + striptCmdString);
					ExecuteScript(striptCmdString);
					striptCmdString = "DEL /F /Q " + IOStormPlus::workloadConfigFileName;
					Logger::LogInfo("Stript command " + striptCmdString);
					ExecuteScript(striptCmdString);
					break;					
				}
				case AgentCommand::DelLocalOutputCmd: {
					striptCmdString = "DEL /F /Q " + GetOutputFolderPath() + "*";
					Logger::LogInfo("Stript command " + striptCmdString);
					ExecuteScript(striptCmdString);
					break;
				}
				default: {
					return BaseAgent::RunScript(command, params);
				}
			}
			return "";
		}

		void Wait(){
			Sleep(SyncWaitTime);
		}

        string GetLogFilePath() {
			return LogFilePath;
		}	

		string GetWorkloadFolderPath() {
			return WorkLoadFolderPath;
		}

		string GetOutputFolderPath() {
			return OutputFolder;
		}

		string GetVMInfoFolderPath() {
			return DirSpliter + DirSpliter + ControllerIP + DirSpliter + AgentsInfoFolderName + DirSpliter;
		}

	private:
		string ControllerIP;
		
	};

}

int main(int argc,char *argv[]) {
	IOStormPlus::WindowsAgent agent(argc, argv);

	while(true){
		agent.Run();
	}
	return 0;
}

