#pragma once

#include "agent.h"
#include "../../common/header/command.h"
#include "constant.h"
#include <string>
#include <vector>
#include <was/storage_account.h>
#include <was/table.h>

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
		void CreateStorageClient(string storageConfigFileName);
        virtual vector<string> ListFilesInDirectory(string rootPath) = 0;
        virtual string RunScript(AgentCommand command, vector<string> &params);
        virtual void Wait(){};
        virtual string GetControlTempFilePath() = 0;
        virtual string GetClientTempFilePath() = 0;
        virtual string GetLogFilePath() = 0;
        virtual string GetWorkloadFolderPath() = 0;
        virtual string GetVMInfoFolderPath() = 0;
        void RegisterOnAzure();
        bool GetControllerCmd(azure::storage::cloud_table& table, SCCommand &command);
        void Acknowledge(azure::storage::cloud_table& table, SCCommand command = EmptyCmd);
        void RunJobs();
		void SetAgentInfo(string vmIP, string vmSize, string vmOS, string vmPool);

	private:
		//Azure Storage Client
		azure::storage::cloud_table_client tableClient;

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