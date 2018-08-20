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
        Controller(string storageConfigFileName);
        ~Controller(){};

        void InitAgents();
        bool IsReady();
        void ConfigureAgent(int argc, char *argv[]);
		void RunTest(int argc, char *argv[]);
        void PrintUsage(ControllerCommand command);
        void CheckTestVMHealth();
		void ShowAgent();
		void SetMaxWaitTime(int timeInSec); //TODO: Add custom setting for the time limit.

    private:
        bool m_isReady;
		int m_maxWaitTimeInSec;
		string logFileName;

		map<string, vector<string> > workload;
		map<string, bool> doneJobs;
		map<string, bool> failedJobs;


        void InitLogger();
        
		//Azure Storage Client
		azure::storage::cloud_table_client tableClient;
		azure::storage::cloud_blob_client blobClient;

		void InitWorkload(string configFilename);
		void UploadWorkload();
		void DownloadOutput();
		void UploadLog();

        bool WaitForAllVMs(azure::storage::cloud_table& table, SCCommand command, int timeLimitInSec, SCCommand retryCMd = SCCommand::InvaildCmd);

        // Test Execution
        void RunStandardTest();
        void RunCustomTest();
        void RunTest(SCCommand startCMD);
    
        // Agent Management
        void RegisterAgent(int argc, char *argv[]);
        void RemoveAgent(int argc, char *argv[]);
        void TestAgent();
        void WriteConfig();

        // Reporting
        void PrintTestVMInfo();
        void PrintTestResultSummary(SCCommand jobCMD);
        void AnalyzeData(SCCommand jobCMD);
        void AnalyzeJob(const string& job, TestVM& vm);
        ReportSummary AnalyzeStandardOutput(string output_file);
        double GetNumber(string buf, int pos);

    };

}

#ifdef __cplusplus
}
#endif