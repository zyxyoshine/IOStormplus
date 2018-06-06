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
    
    protected:
        // 
        void InitLogger();
        virtual string ExecuteScript(string command){return NULL;};
        virtual vector<string> ListFilesInDirectory(string rootPath){vector<string> a; return a;};
        virtual string RunScript(AgentCommand command, vector<string> &params);
        virtual void Wait(){};
        virtual string GetControlTempFilePath(){return NULL;};
        virtual string GetClientTempFilePath(){return NULL;};
        virtual string GetLogFilePath(){return NULL;};
        virtual string GetWorkloadFolderPath(){return NULL;};
        bool GetControllerCmd(SCCommand &command);
        void Acknowledge(SCCommand command);
        void RunJobs();
    };

}


#ifdef __cplusplus
}
#endif