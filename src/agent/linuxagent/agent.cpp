#include "header/constant.h"
#include "../header/agent.h"
#include "../../common/header/logger.h"
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

	class LinuxAgent:public BaseAgent{
	protected:        

		string LinuxAgent::ExecuteScript(string cmd) {
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

		string LinuxAgent::RunScript(AgentCommand command, vector<string> &params){
			string striptCmdString;
			switch(command){
				case AgentCommand::CopyOutputCmd: {
					string jobname = params[0];
					string hostname = params[1];
					string striptCmdString = "cp -pf " + jobname + ".out " + " " + OutputFolder + hostname + "_" + jobname + ".out";
					ExecuteScript(striptCmdString);	
					break;				
				}
				case AgentCommand::DelTempFileCmd: {
					string filename = params[0];
					string striptCmdString = "rm -f " + jobname + "*";
					Logger::LogVerbose("Stript command "+striptCmdString);
					ExecuteScript(striptCmdString);	
					break;		
				}
				case AgentCommand::DelJobFilesCmd: {
					string striptCmdString = "rm -f " + GetWorkloadFolderPath() + "*";
					Logger::LogVerbose("Stript command "+striptCmdString);
					ExecuteScript(striptCmdString);
					break;					
				}
				default: {
					BaseAgent::RunCommand(command, params);
				}
			}
			return "";
		}

		void LinuxAgent::Wait(){
			Sleep(SyncWaitTime);
		}

        string LinuxAgent::GetControlTempFilePath(){
			return ControllerTempFilePath;
		}

        string LinuxAgent::GetClientTempFilePath(){
			return ClientTempFilePath;
		}

        string LinuxAgent::GetLogFilePath(){
			return LogFilePath;
		}	

		string LinuxAgent::GetWorkloadFolderPath(){
			return WorkLoadFolderPath;
		}		
	};
}

int main(int argc,char *argv[]) {   
	string hostname = agent.ExecuteCommand("hostname");
    if (hostname.find('\n') != string::npos)
        hostname = hostname.replace(hostname.find('\n'),1,"");
    ofstream fout("/samba/info/" + hostname,ios_base::out | ios_base::trunc);
    fout << argv[1] << " linux " << argv[2] << endl;
    fout.close();


	IOStormPlus::LinuxAgent agent;
	while(true){
		agent.Run();
	}
	return 0;
}
