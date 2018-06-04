#pragma once

#include <string>
#include <vector>
using namespace std;

#ifdef __cplusplus
extern "C"
{
#endif

namespace IOStormPlus{
    enum AgentCommand{
        RunFIOCmd = 0,
        CopyOutputCmd = 1,
        DelTempFileCmd = 2,
        DelJobFilesCmd = 3,
        HostnameCmd = 4
    };

    class IAgent{
    public:
        void Sync();
        void Run();
    protected:
        void InitLogger();
        vector<string> ListFilesInDirectory(string rootPath);
        string RunCommand(AgentCommand command, ...);
        string ExecuteCommand(string command);
    };

}


#ifdef __cplusplus
}
#endif
