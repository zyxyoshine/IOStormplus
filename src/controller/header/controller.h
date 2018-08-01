#pragma once
#include "TestVM.h"
#include "../../common/header/command.h"
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
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

    enum ControllerCommand {
        General = 0,
        AgentGeneral = 10,
        AgentRegister = 11,
        AgentRemove = 12,
        WorkerGeneral = 50
    };

    enum TestType {
        StandardTest = 0,
        CustomTest = 1
    };

    class Controller{
    public:
        vector<TestVM> TestVMs;

    public:
        Controller(string configFilename, string storageConfigFileName);
        ~Controller(){};

        void InitAgents();
        bool IsReady();
        void ConfigureAgent(int argc, char *argv[]);
		void RunTest(int argc, char *argv[]);
        void PrintUsage(ControllerCommand command);
        void CheckTestVMHealth();

    private:
        bool m_isReady;
		
		map<string, vector<string> > workload;

        void InitLogger();
        
		//Azure Storage Client
		azure::storage::cloud_table_client tableClient;
		azure::storage::cloud_blob_client blobClient;

		void InitWorkload(string configFilename);
		void UploadWorkload();
		void DownloadOutput();

        // Health Check
        void WaitForAllVMs(azure::storage::cloud_table& table, SCCommand command);

        // Test Execution
        void RunStandardTest();
        void RunCustomTest();
        void RunTest(SCCommand startCMD);
    
        // Agent Management
        void RegisterAgent(int argc, char *argv[]);
        void RemoveAgent(int argc, char *argv[]);
        void ShowAgent();
        void TestAgent();
        void WriteConfig();

        // Reporting
        void PrintTestVMInfo();
        void PrintTestResultSummary(SCCommand jobCMD);
        void AnalyzeData(SCCommand jobCMD);
        void AnalyzeJob(const string& job, TestVM& vm);
        ReportSummary AnalyzeStandardOutput(string output_file);
        int GetIOPSNumber(string buf, int pos);

    };

}

#ifdef __cplusplus
}
#endif