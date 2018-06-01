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

    class DefaultAgent:IAgent{
    public:
        void InitLogger();
        void ExecuteCommand(string command);
        vector<string> ListFilesInDirectory(string rootPath);
        void Sync();
        void Run();
    };

}


#ifdef __cplusplus
}
#endif