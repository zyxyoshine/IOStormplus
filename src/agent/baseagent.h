#pragma once

#include "agent.h"
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
        void Sync();
        void Run();
    
    protected:
        // 
        void InitLogger();
        virtual string ExecuteCommand(string command){return NULL;};
        virtual vector<string> ListFilesInDirectory(string rootPath){vector<string> a; return a;};
        virtual string RunCommand(AgentCommand command, ...);
        virtual void Wait(){};
        virtual string GetControlTempFilePath(){return NULL;};
        virtual string GetClientTempFilePath(){return NULL;};
        virtual string GetLogFilePath(){return NULL;};
        virtual string GetWorkloadFolderPath(){return NULL;};
    };

}


#ifdef __cplusplus
}
#endif