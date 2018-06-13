#pragma once
#include "TestVM.h"
#include "../common/header/command.h"
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>

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
        Controller(string TestVMs_config);
        ~Controller(){};

        void InitAgents();
        bool IsReady();
        void ConfigureAgent(int argc, char *argv[]);
        void RunTest(int argc, char *argv[]);
        void PrintUsage(ControllerCommand command);
        void CheckTestVMHealth();

    private:
        bool m_isReady;
        
        void InitLogger();
        
        // Health Check
        void WaitForAllVMs(SCCommand command);

        // Test Execution
        void RunStandardTest();
        void RunCustomTest();
        void RunTest(string rootPath);
    
        // Agent Management
        void RegisterAgent(int argc, char *argv[]);
        void RemoveAgent(int argc, char *argv[]);
        void ShowAgent();
        void TestAgent();
        void WriteConfig();

        // Reporting
        void PrintTestVMInfo();
        void PrintTestResultSummary(string workloadRootPath);
        void AnalyzeData(string workloadRootPath);
        void AnalyzeJob(const string& job, TestVM& vm);
        ReportSummary AnalyzeStandardOutput(string output_file);
        int GetIOPSNumber(string buf, int pos);

    };

}

#ifdef __cplusplus
}
#endif