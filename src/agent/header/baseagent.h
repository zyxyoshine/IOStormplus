#pragma once

#include "agent.h"
#include "../../common/header/command.h"
#include <string>
#include <vector>
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
        virtual vector<string> ListFilesInDirectory(string rootPath) = 0;
        string RunScript(AgentCommand command, vector<string> &params);
        virtual void Wait(){};
        virtual string GetControlTempFilePath() = 0;
        virtual string GetClientTempFilePath() = 0;
        virtual string GetLogFilePath() = 0;
        virtual string GetWorkloadFolderPath() = 0;
        bool GetControllerCmd(SCCommand &command);
        void Acknowledge(SCCommand command);
        void RunJobs();
    };

}


#ifdef __cplusplus
}
#endif