#include "header/constant.h"
#include "../header/baseagent.h"
#include "../../common/header/logger.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <ctime>
#include <unistd.h>

using namespace std;

namespace IOStormPlus{

	class LinuxAgent: public BaseAgent{
	public:
		LinuxAgent(int argc = 0,char *argv[] = NULL) {
			InitLogger();
			CreateStorageClient(argv[4]);
			SetAgentInfo(argv[1], argv[2], "linux", argv[3]);
			RegisterOnAzure();
		}
		
		string ExecuteScript(string command) {
			Logger::LogInfo("Run Script " + command);
			char buffer[128];
			string result = "";
			FILE* pipe = popen(command.c_str(), "r");
			if (!pipe){
				Logger::LogError("popen() failed!");
				throw runtime_error("popen() failed!");
			}
			try {
				while (!feof(pipe)) {
					if (fgets(buffer, 128, pipe) != NULL) {
						result += buffer;
					}
				}
			} 
			catch (...) {
				pclose(pipe);
				Logger::LogError("Execute script error");
				throw;
			}
			pclose(pipe);
			Logger::LogInfo("Execute script succeed " + result);
			return result;
		}

	protected:        

		vector<string> ListFilesInDirectory(string rootPath) {
			Logger::LogInfo("Start List Files under directory");
			vector<string> res;
			string lsCmd = "ls " + rootPath;
			string lsResut = ExecuteScript(lsCmd.c_str());
			int pre = 0;
			for (int i = 0;i < lsResut.length();i++) {
				if (lsResut[i] == '\n') {
					res.push_back(lsResut.substr(pre,i - pre));
					pre = i + 1;
				}
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
					striptCmdString = "rm -f " + filename + "*";
					Logger::LogInfo("Stript command " + striptCmdString);
					ExecuteScript(striptCmdString);	
					break;		
				}
				case DelJobFilesCmd: {
					string striptCmdString = "rm -f " + GetWorkloadFolderPath() + "*";
					Logger::LogVerbose("Stript command "+striptCmdString);
					ExecuteScript(striptCmdString);
					striptCmdString = "rm -f " + IOStormPlus::workloadConfigFileName;
					Logger::LogInfo("Stript command " + striptCmdString);
					ExecuteScript(striptCmdString);
					break;					
				}
				case AgentCommand::DelLocalOutputCmd: {
					striptCmdString = "rm -f " + GetOutputFolderPath() + "*";
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
			usleep(SyncWaitTime);
		}

        string GetLogFilePath(){
			return LogFilePath;
		}	

		string GetWorkloadFolderPath(){
			return WorkLoadFolderPath;
		}

		string GetOutputFolderPath() {
			return OutputFolder;
		}
		
	};
}

int main(int argc,char *argv[]) {   
	IOStormPlus::LinuxAgent agent(argc, argv);

	while(true){
		agent.Run();
	}
	return 0;
}
