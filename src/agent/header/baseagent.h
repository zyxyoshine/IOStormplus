#pragma once

#include "agent.h"
#include "../../common/header/command.h"
#include "constant.h"
#include <string>
#include <vector>
#include <was/storage_account.h>
#include <was/table.h>
#include <was/blob.h>
#include <cpprest/filestream.h>  
#include <cpprest/containerstream.h> 

using namespace std;

#ifdef __cplusplus
extern "C"
{
#endif

namespace IOStormPlus{

    class BaseAgent:IAgent{
    public:
        // IAgent 
        void Run();
        virtual string ExecuteScript(string command) = 0;
    
    protected:
        // 
        void InitLogger();
		void CreateStorageClient(string storageConnectionString);
        virtual vector<string> ListFilesInDirectory(string rootPath) = 0;
        virtual string RunScript(AgentCommand command, vector<string> &params);
        virtual void Wait(){};
		virtual string GetOutputFolderPath() = 0;
        virtual string GetLogFilePath() = 0;
        virtual string GetWorkloadFolderPath() = 0;
        void RegisterOnAzure();
        bool GetControllerCmd(azure::storage::cloud_table& table, SCCommand &command);
        void Acknowledge(azure::storage::cloud_table& table, SCCommand command = EmptyCmd);
        void RunJobs();
		void SetAgentInfo(string vmIP, string vmSize, string vmOS, string vmPool);

		void DownloadWorkload(SCCommand jobCMD, string configFilename);
		void UploadOutput();

	private:
		//Azure Storage Client
		azure::storage::cloud_table_client tableClient;
		azure::storage::cloud_blob_client blobClient;

		string m_vmName;
		string m_vmPool;
		string m_vmIP;
		string m_vmSize;
		string m_vmOS;
    };

}


#ifdef __cplusplus
}
#endif