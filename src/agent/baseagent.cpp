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
    
	void BaseAgent::CreateStorageClient(string storageConfigFile) {
		fstream fin(storageConfigFile);
		Logger::LogInfo("Open Azure Storage configuration file: " + storageConfigFile);
		string storageAccountName, storageAccountKey, storageEndpointSuffix;
		getline(fin, storageAccountName);
		getline(fin, storageAccountKey);
		getline(fin, storageEndpointSuffix);
		fin.close();

		storageAccountName.replace(storageAccountName.find("NAME="), 5, "");
		storageAccountKey.replace(storageAccountKey.find("KEY="), 4, "");
		storageEndpointSuffix.replace(storageEndpointSuffix.find("ENDPOINTSUF="),12, "");
		const utility::string_t storageConnectionString = utility::conversions::to_string_t("DefaultEndpointsProtocol=https;AccountName=" + storageAccountName + ";AccountKey=" + storageAccountKey + ";EndpointSuffix=" + storageEndpointSuffix);
		azure::storage::cloud_storage_account storageAccount = azure::storage::cloud_storage_account::parse(storageConnectionString);

		tableClient = storageAccount.create_cloud_table_client();
	}


	void BaseAgent::Run() {
		Logger::LogVerbose("Start Running");
		azure::storage::cloud_table table = tableClient.get_table_reference(IOStormPlus::storageTempTableName);
		while (true){
			Wait();
			SCCommand cmd;
			if (!GetControllerCmd(table, cmd)) {
				Logger::LogVerbose("No valid command, waiting");
				continue;
			}
			Logger::LogVerbose("Get one command");
			switch(cmd){
				case SCCommand::SyncCmd:{
					Acknowledge(table, SCCommand::SyncDoneCmd);
					break;
				}
				case SCCommand::StartJobCmd:{
					RunJobs();
					Acknowledge(table, SCCommand::JobDoneCmd);
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

    bool BaseAgent::GetControllerCmd(azure::storage::cloud_table& table, SCCommand &command){
		azure::storage::table_operation retrieveOperation = azure::storage::table_operation::retrieve_entity(utility::conversions::to_string_t(m_vmPool), utility::conversions::to_string_t(m_vmName));

		azure::storage::table_result retrieveResult = table.execute(retrieveOperation);
		azure::storage::table_entity agentEntity = retrieveResult.entity();
		const azure::storage::table_entity::properties_type& properties = agentEntity.properties();
		string cmdString = utility::conversions::to_utf8string(properties.at(IOStormPlus::tableCommandColumnName).string_value());
		command = GetCommondFromString(cmdString);
		if (command != SCCommand::InvaildCmd && command != SCCommand::EmptyCmd && command != SCCommand::SyncDoneCmd && command != SCCommand::JobDoneCmd) {
			return true;
		}

		Logger::LogWarning("Unknown control command");
		return false;
	}


    void BaseAgent::Acknowledge(azure::storage::cloud_table& table, SCCommand command){
		string cmdstring = GetCommandString(command);
		Logger::LogInfo("Start ack " + cmdstring + " command");

		azure::storage::table_entity agent(utility::conversions::to_string_t(m_vmPool), utility::conversions::to_string_t(m_vmName));
		azure::storage::table_entity::properties_type& properties = agent.properties();
		//properties.reserve(1);
		properties[tableCommandColumnName] = azure::storage::entity_property(utility::conversions::to_string_t(GetCommandString(command)));

		azure::storage::table_operation opt = azure::storage::table_operation::insert_or_merge_entity(agent);
		azure::storage::table_result insert_result = table.execute(opt);
        
		Logger::LogInfo("Done ack " + cmdstring + " command");
	}

	void BaseAgent::RegisterOnAzure() {
		azure::storage::cloud_table table = tableClient.get_table_reference(IOStormPlus::storageTempTableName);
		table.create_if_not_exists();
		azure::storage::table_entity agent(utility::conversions::to_string_t(m_vmPool), utility::conversions::to_string_t(m_vmName));
		azure::storage::table_entity::properties_type& properties = agent.properties();

		properties.reserve(5);
		properties[tableCommandColumnName] = azure::storage::entity_property(utility::conversions::to_string_t(GetCommandString(SCCommand::EmptyCmd)));
		properties[tableOSColumnName] = azure::storage::entity_property(utility::conversions::to_string_t(m_vmOS));
		properties[tablePoolColumnName] = azure::storage::entity_property(utility::conversions::to_string_t(m_vmPool));
		properties[tableIPColumnName] = azure::storage::entity_property(utility::conversions::to_string_t(m_vmIP));
		properties[tableSizeColumnName] = azure::storage::entity_property(utility::conversions::to_string_t(m_vmSize));

		azure::storage::table_operation opt = azure::storage::table_operation::insert_or_replace_entity(agent);
		azure::storage::table_result insert_result = table.execute(opt);

		Logger::LogInfo("Register agent succeeded");
	}
	
	void BaseAgent::SetAgentInfo(string vmIP, string vmSize, string vmOS, string vmPool) {
		vector<string> params;
		m_vmName = BaseAgent::RunScript(AgentCommand::HostnameCmd, params);
		if (m_vmName.find('\n') != string::npos)
			m_vmName = m_vmName.replace(m_vmName.find('\n'), 1, "");

		m_vmIP = vmIP;
		m_vmSize = vmSize;
		m_vmOS = vmOS;
		m_vmPool = vmPool;
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