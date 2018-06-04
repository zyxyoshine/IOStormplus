#include "constant.h"
#include "..\agent.h"
#include "..\..\common\logger.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <ctime>
#include <stdarg.h>
//#include <unistd.h>

extern int pclose(FILE* stream);
extern FILE* popen(const char *command, const char *mode);
extern int sleep(double milliseconds);

using namespace std;



namespace IOStormPlus{

	class LinuxAgent:public BaseAgent{
	protected:        

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

		string LinuxAgent::RunCommand(AgentCommand command, ...){
			va_list args;
			int count;
			va_start(args, count);
			string cmdString;
			switch(command){
				case AgentCommand::CopyOutputCmd: {
					string jobname = va_arg(args, string);
					string hostname = va_arg(args, string);
					string cmdString = "cp -pf " + jobname + ".out " + " " + OutputFolder + hostname + "_" + jobname + ".out";
					ExecuteCommand(cmdString);	
					break;				
				}
				case AgentCommand::DelTempFileCmd: {
					string jobname = va_arg(args, string);
					string cmdString = "rm -f " + jobname + "*";
					ExecuteCommand(cmdString);	
					break;		
				}
				case AgentCommand::DelJobFilesCmd: {
					string cmdString = "rm -f " + GetWorkloadFolderPath() + "*";
					ExecuteCommand(cmdString);
					break;					
				}
				default: {
					BaseAgent::RunCommand(command, args);
				}
			}
			return NULL;
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
