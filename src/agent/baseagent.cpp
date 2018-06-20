#include "header/baseagent.h"
#include "../common/header/logger.h"
#include <iostream>
#include <cassert>
#include <cstdarg>

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
				Logger::LogVerbose("No valid command, waiting");
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

    string BaseAgent::RunScript(AgentCommand command, vector<string> &params){
		switch(command){
			case AgentCommand::HostnameCmd: {
				return ExecuteScript("hostname");
			}
			case AgentCommand::RunFIOCmd: {
				assert(params.size() == 2);
				string job = params[0];
				string jobname = params[1];
				string striptCmdString = "fio --output=" + jobname + ".out " + " " + GetWorkloadFolderPath() + job;
				ExecuteScript(striptCmdString);
				break; 
			}
			default: {
				assert(false);
			}
		}
		return "";
	}	

    bool BaseAgent::GetControllerCmd(SCCommand &command){
	
		Logger::LogVerbose("Start open controller command file " + GetControlTempFilePath());
		
		ifstream fin;	
		fin.open(GetControlTempFilePath(), ios_base::in);
		if (!fin.is_open()) {
			Logger::LogInfo("Failed to open controller temp file");	
			return false;
		}

		Logger::LogVerbose("ControlTempFile Open successfully " + GetControlTempFilePath());
		string buf;
		fin >> buf;
		fin.close();

		command = GetCommondFromString(buf);
		Logger::LogInfo("Get command " + buf);
		if (command != SCCommand::InvaildCmd) {
			return true;
		}

		Logger::LogWarning("Unknown control command");
		return false;
	}


    void BaseAgent::Acknowledge(SCCommand command){
		string cmdstring = GetCommandString(command);
		Logger::LogInfo("Start ack " + cmdstring + " command");

		Logger::LogVerbose("Open Client Acknowledge file " + GetClientTempFilePath());
		
		ofstream fout(GetClientTempFilePath(), ios_base::out | ios_base::trunc);
		fout << GetCommandString(command);
		fout.flush();
		fout.close();

		vector<string> params;
		params.push_back(GetControlTempFilePath());		
		RunScript(AgentCommand::DelTempFileCmd,params);
        
		Logger::LogInfo("Done ack " + cmdstring + " command");
	}

	void BaseAgent::RegisterOnController(string vmIP, string vmSize, string VMOS) {
		vector<string> params;
		string hostname = BaseAgent::RunScript(AgentCommand::HostnameCmd, params);
		if (hostname.find('\n') != string::npos)
			hostname = hostname.replace(hostname.find('\n'),1,"");
		ofstream fout(GetVMInfoFolderPath() + hostname,ios_base::out | ios_base::trunc);
		fout << vmIP << ' ' << VMOS << ' ' << vmSize << endl;
		fout.close();
	}
	
    void BaseAgent::RunJobs(){
		Logger::LogVerbose("Start Job");
		vector<string> params;		
		string hostname = BaseAgent::RunScript(AgentCommand::HostnameCmd, params);
		if (hostname.find('\n') != string::npos) {
			hostname = hostname.replace(hostname.find('\n'),1,"");
		}
		Logger::LogInfo("Hostname " + hostname);

		vector<string> jobs = ListFilesInDirectory(GetWorkloadFolderPath());
		Logger::LogVerbose("Running fio workload.");
		for (int i = 0 ;i < jobs.size();i++){
			string jobname = jobs[i];
			if (jobname.find(".job") != string::npos) {
				jobname = jobname.replace(jobname.find(".job"),4,"");
			}

			vector<string> params;
			params.push_back(jobs[i]);
			params.push_back(jobname);	
			BaseAgent::RunScript(AgentCommand::RunFIOCmd, params);
			
			params.clear();
			params.push_back(jobname);
			params.push_back(hostname);
			RunScript(AgentCommand::CopyOutputCmd, params);

			params.clear();
			params.push_back(jobname);			
			RunScript(AgentCommand::DelTempFileCmd,params);
			Logger::LogInfo("Done Job "+jobname);
		}

		RunScript(AgentCommand::DelJobFilesCmd,params);

		Logger::LogVerbose("Job Done");		
	}
}