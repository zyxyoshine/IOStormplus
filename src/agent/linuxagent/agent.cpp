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
			if (argc >= 3)
				RegisterOnController(argv[1], argv[2], "linux");
		}
		
		string ExecuteScript(string command) {
			char buffer[128];
			string result = "";
			FILE* pipe = popen(command.c_str(), "r");
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

	protected:        

		vector<string> ListFilesInDirectory(string rootPath) {
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
			return res;
		}

		string RunScript(AgentCommand command, vector<string> &params){
			string striptCmdString;
			switch(command){
				case CopyOutputCmd: {
					string jobname = params[0];
					string hostname = params[1];
					string striptCmdString = "cp -pf " + jobname + ".out " + " " + OutputFolder + hostname + "_" + jobname + ".out";
					ExecuteScript(striptCmdString);	
					break;				
				}
				case DelTempFileCmd: {
					string filename = params[0];
					string striptCmdString = "rm -f " + filename + "*";
					Logger::LogVerbose("Stript command "+striptCmdString);
					ExecuteScript(striptCmdString);	
					break;		
				}
				case DelJobFilesCmd: {
					string striptCmdString = "rm -f " + GetWorkloadFolderPath() + "*";
					Logger::LogVerbose("Stript command "+striptCmdString);
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

        string GetControlTempFilePath(){
			return ControllerTempFilePath;
		}

        string GetClientTempFilePath(){
			return ClientTempFilePath;
		}

        string GetLogFilePath(){
			return LogFilePath;
		}	

		string GetWorkloadFolderPath(){
			return WorkLoadFolderPath;
		}

		string GetVMInfoFolderPath() {
			return VMInfoFolderPath;
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
