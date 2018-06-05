#include "header/baseagent.h"
#include "../common/header/logger.h"
#include <iostream>
#include <cassert>
#include <stdarg.h>

using namespace std;

namespace IOStormPlus{
    void BaseAgent::InitLogger(){
		Logger::Init(GetLogFilePath());
    }
    
	void BaseAgent::Run() {
		Logger::LogVerbose("Start Running");
		while (true){
			Wait();
			SCCommand cmd;
			if (!GetControllerCmd(cmd)) {
				continue;
			} 
			Logger::LogVerbose("Get one command");
			switch(cmd){
				case SCCommand::SyncCmd:{
					Acknowledge(SCCommand::SyncDoneCmd);
					break;
				}
				case SCCommand::StartJobCmd:{
					RunJobs();
					Acknowledge(SCCommand::JobDoneCmd);
					break;						
					}
				default: break;					
			}
		}
	}	

    string BaseAgent::RunScript(AgentCommand command, ...){
		va_list args;
		int count;
		va_start(args, count);
		switch(command){
			case AgentCommand::HostnameCmd: {
				return ExecuteScript("hostname");
				break; 
			}
			case AgentCommand::RunFIOCmd: {
				string job = va_arg(args, string);
				string jobname = va_arg(args, string);
				string runFIOCmd = "fio --output=" + jobname + ".out " + " " + GetWorkloadFolderPath() + job;
				ExecuteScript(runFIOCmd);
				break; 
			}
			default: {
				assert(false);
			}
		}
		return NULL;
	}	

    bool BaseAgent::GetControllerCmd(SCCommand &command){
	
		Logger::LogVerbose("Start open controller command file " + GetControlTempFilePath());
		
		ifstream fin;	
		fin.open(GetControlTempFilePath(), ios_base::in);
		if (!fin.is_open()) {
			Logger::LogError("Failed to open controller temp file");	
			return false;
		}

		Logger::LogVerbose("ControlTempFile Open successfully " + GetControlTempFilePath());

		string buf;
		fin >> buf;
		fin.close();	

		command = GetCommondFromString(buf);
		Logger::LogInfo("Get command " + buf);
		cout << buf << command << endl;
		if (command != SCCommand::InvaildCmd) {
			return true;
		}
		return false;
	}


    void BaseAgent::Acknowledge(SCCommand command){
		string cmdstring = GetCommandString(command);
		Logger::LogInfo("Start ack " + cmdstring + " command");

		Logger::LogVerbose("Open Client Acknowledge file "+GetClientTempFilePath());
		
		ofstream fout(GetClientTempFilePath(), ios_base::out | ios_base::trunc);
		fout << GetCommandString(command);
		fout.flush();
		fout.close();
        
		Logger::LogInfo("Done ack " + cmdstring + " command");
	}

    void BaseAgent::RunJobs(){
		Logger::LogVerbose("Start Job");
		string hostname = RunScript(AgentCommand::HostnameCmd);
		if (hostname.find('\n') != string::npos) {
			hostname = hostname.replace(hostname.find('\n'),1,"");
		}
		Logger::LogInfo("Hostname" + hostname);

		vector<string> jobs = ListFilesInDirectory(GetWorkloadFolderPath());
		Logger::LogVerbose("Running fio workload.");
		for (int i = 0 ;i < jobs.size();i++){
			string jobname = jobs[i];
			if (jobname.find(".job") != string::npos) {
				jobname = jobname.replace(jobname.find(".job"),4,"");
			}
				
			RunScript(AgentCommand::RunFIOCmd, jobs[i], jobname);
			RunScript(AgentCommand::CopyOutputCmd, jobname, hostname);
			RunScript(AgentCommand::DelTempFileCmd, jobname);
			Logger::LogInfo("Done Job "+jobname);
		}

		RunScript(AgentCommand::DelJobFilesCmd);

		Logger::LogVerbose("Job Done");		
	}
}