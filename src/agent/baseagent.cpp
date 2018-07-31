#include "header/baseagent.h"
#include "../common/header/logger.h"
#include "../common/rapidjson/document.h"     // rapidjson's DOM-style API
#include <iostream>
#include <cassert>
#include <cstdarg>

using namespace rapidjson;
using namespace std;

namespace IOStormPlus{
    void BaseAgent::InitLogger(){
		Logger::Init(GetLogFilePath());
    }
    
	void BaseAgent::CreateStorageClient(string storageConnectionString) {
		Logger::LogInfo("storageConnectionString: " + storageConnectionString);
		try {
			azure::storage::cloud_storage_account storageAccount = azure::storage::cloud_storage_account::parse(utility::conversions::to_string_t(storageConnectionString));
			tableClient = storageAccount.create_cloud_table_client();
			blobClient = storageAccount.create_cloud_blob_client();
		}
		catch (const std::exception& e){
			Logger::LogInfo(e.what());
		}
	}


	void BaseAgent::Run() {
		Logger::LogInfo("Start Running");
		azure::storage::cloud_table table = tableClient.get_table_reference(IOStormPlus::storageTempTableName);
		while (true){
			Wait();
			SCCommand cmd;
			if (!GetControllerCmd(table, cmd)) {
				Logger::LogInfo("No valid command, waiting");
				Acknowledge(table, SCCommand::EmptyCmd);
				continue;
			}
			Logger::LogInfo("Get one command");
			switch(cmd){
				case SCCommand::SyncCmd:{
					Acknowledge(table, SCCommand::SyncDoneCmd);
					break;
				}
				case SCCommand::StartJobCmd:{
					DownloadWorkload(SCCommand::StartJobCmd, IOStormPlus::workloadConfigFileName);
					RunJobs();
					Acknowledge(table, SCCommand::JobDoneCmd);
					break;						
				}
				case SCCommand::StartStdJobCmd: {
					DownloadWorkload(SCCommand::StartStdJobCmd, IOStormPlus::workloadConfigFileName);
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
				assert(params.size() == 3);
				string job = params[0];
				string jobname = params[1];
				string hostname = params[2];
				string striptCmdString = "fio --output=" + GetOutputFolderPath() + hostname + "_" + jobname + ".out " + " " + GetWorkloadFolderPath() + job;
				ExecuteScript(striptCmdString);
				break; 
			}
			default: {
				assert(false);
			}
		}
		return "";
	}

	void BaseAgent::DownloadWorkload(SCCommand jobCMD, string configFilename) {
		cout << jobCMD << endl;
		//Download configuration file
		azure::storage::cloud_blob_container container = blobClient.get_container_reference(IOStormPlus::workloadBlobContainerName);
		azure::storage::cloud_block_blob blockBlob = container.get_block_blob_reference(utility::conversions::to_string_t(configFilename));
		blockBlob.download_to_file(utility::conversions::to_string_t(configFilename));
		fstream fin(configFilename);
		Logger::LogInfo("Open workload configuration file: " + configFilename);
		string data, workloadConfigContent = "";
		while (!fin.eof()) {
			getline(fin, data);
			workloadConfigContent += data;
		}
		fin.close();

		Document workloadConfig;
		if (workloadConfig.Parse(workloadConfigContent.c_str()).HasParseError()) {
			stringstream logStream;
			logStream << "parse workload configuration failed! " << workloadConfigContent.c_str() << endl;
			Logger::LogError(logStream.str());
			return;
		}
		Logger::LogInfo("Configure File has been parsed successfully!");
		
		int workloadCount = workloadConfig["count"].GetInt();
		auto workloadInfo = workloadConfig["value"].GetArray();

		map<string, vector<string> > workload;

		for (int i = 0; i < workloadCount; i++) {
			string poolName = workloadInfo[i]["pool"].GetString();
			int jobCount = workloadInfo[i]["count"].GetInt();
			auto jobs = workloadInfo[i]["jobs"].GetArray();
			for (int j = 0; j < jobCount; j++) {
				workload[poolName].push_back(jobs[j].GetString());
			}
		}

		string targetPool = m_vmPool;
		if (jobCMD == SCCommand::StartStdJobCmd || workload.count(m_vmPool) == 0) {
			targetPool = "std";
		}

		for (auto jna : workload[targetPool]) {
			blockBlob = container.get_block_blob_reference(utility::conversions::to_string_t(jna));
			blockBlob.download_to_file(utility::conversions::to_string_t(GetWorkloadFolderPath() + jna));
		}
	}

	void BaseAgent::UploadOutput() {
		vector<string> outputFiles;
		outputFiles = ListFilesInDirectory(GetOutputFolderPath());
		// Retrieve a reference to a container.
		azure::storage::cloud_blob_container container = blobClient.get_container_reference(IOStormPlus::outputBlobContainerName);
		// Create the container if it doesn't already exist.
		container.create_if_not_exists();
		for (auto fna : outputFiles) {
			azure::storage::cloud_block_blob blockBlob = container.get_block_blob_reference(utility::conversions::to_string_t(fna));
			blockBlob.upload_from_file(utility::conversions::to_string_t(GetOutputFolderPath() + fna));
		}
		Logger::LogInfo("Upload workload files to blob succeeded.");
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
		Logger::LogInfo("Start Job");
		vector<string> params;		
		string hostname = BaseAgent::RunScript(AgentCommand::HostnameCmd, params);
		if (hostname.find('\n') != string::npos) {
			hostname = hostname.replace(hostname.find('\n'),1,"");
		}
		Logger::LogInfo("Hostname " + hostname);

		vector<string> jobs = ListFilesInDirectory(GetWorkloadFolderPath());
		Logger::LogInfo("Running fio workload.");
		for (int i = 0 ;i < jobs.size();i++){
			string jobname = jobs[i];
			if (jobname.find(".job") != string::npos) {
				jobname = jobname.replace(jobname.find(".job"),4,"");
			}

			vector<string> params;
			params.push_back(jobs[i]);
			params.push_back(jobname);
			params.push_back(hostname);
			BaseAgent::RunScript(AgentCommand::RunFIOCmd, params);
			
			params.clear();
			params.push_back(jobname);			
			RunScript(AgentCommand::DelTempFileCmd, params);
			Logger::LogInfo("Done Job "+jobname);
		}

		RunScript(AgentCommand::DelJobFilesCmd, params);
		UploadOutput();
		RunScript(AgentCommand::DelLocalOutputCmd, params);
		
		Logger::LogInfo("Job Done");		
	}
}