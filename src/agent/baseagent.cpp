#include "baseagent.h"
#include "../common/logger.h"
#include <iostream>
#include <cassert>
#include <stdarg.h>

using namespace std;

namespace IOStormPlus{
    void BaseAgent::InitLogger(){
		Logger::Init(GetLogFilePath());
    }
    
	// void ExecuteCommand(string command);
    // vector<string> ListFilesInDirectory(string rootPath);
    void BaseAgent::Sync(){
		ifstream fin;
			
		fin.open(GetControlTempFilePath(), ios_base::in);
		while (fin.fail()) {
			Wait();
			cerr << strerror(errno) << endl;
			fin.open(GetControlTempFilePath() , ios_base::in);
		}
			
		string buf;
		Logger::LogInfo("waiting for controller requests");
		while(true) {
			fin.close();
			Wait();
			fin.open(GetControlTempFilePath(), ios_base::in);
			fin >> buf;
			if (buf == "PRESYNC") {
				Logger::LogInfo("pre-sync succeeded");
				break;
			}
		}
		fin.close();

		ofstream fout(GetClientTempFilePath(), ios_base::out | ios_base::trunc);
		fout << "SYNCDONE";
		fout.flush();
		fout.close();
            
    }

	void BaseAgent::Run() {
		string hostname = RunCommand(AgentCommand::HostnameCmd);
		if (hostname.find('\n') != string::npos) {
			hostname = hostname.replace(hostname.find('\n'),1,"");
		}
		Logger::LogInfo("Hostname" + hostname);

		ifstream fin(GetControlTempFilePath(), ios_base::in);
		string buf;
		while(true) {
			fin.seekg(0, ios::beg);
			fin >> buf;
			if (buf == "START") {
				vector<string> jobs = ListFilesInDirectory(GetWorkloadFolderPath());
				Logger::LogVerbose("Running fio workload.");
				for (int i = 0 ;i < jobs.size();i++){
					string jobname = jobs[i];
					if (jobname.find(".job") != string::npos) {
						jobname = jobname.replace(jobname.find(".job"),4,"");
					}
				
					RunCommand(AgentCommand::RunFIOCmd, jobs[i], jobname);
					RunCommand(AgentCommand::CopyOutputCmd, jobname, hostname);
					RunCommand(AgentCommand::DelTempFileCmd, jobname);
					
				}
				break;
			}
			Wait();
		}
		fin.close();

		RunCommand(AgentCommand::DelJobFilesCmd);

		ofstream fout(GetClientTempFilePath(), ios_base::out | ios_base::trunc);
		if (fout.fail()) {
			Logger::LogError(strerror(errno));
		}
		fout << "DONE" << endl;
		Logger::LogVerbose("Done !");
		fout.close();
	}	

    string BaseAgent::RunCommand(AgentCommand command, ...){
		va_list args;
		int count;
		va_start(args, count);
		switch(command){
			case AgentCommand::HostnameCmd: {
				return ExecuteCommand("hostname");
				break; 
			}
			case AgentCommand::RunFIOCmd: {
				string job = va_arg(args, string);
				string jobname = va_arg(args, string);
				string runFIOCmd = "fio --output=" + jobname + ".out " + " " + GetWorkloadFolderPath() + job;
				ExecuteCommand(runFIOCmd);
				break; 
			}
			default: {
				assert(false);
			}
		}
		return NULL;
	}	

}