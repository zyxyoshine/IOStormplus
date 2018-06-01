#pragma once

#include <string>
#include <vector>
using namespace std;

#ifdef __cplusplus
extern "C"
{
#endif

namespace IOStormPlus{

    class IAgent{
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
