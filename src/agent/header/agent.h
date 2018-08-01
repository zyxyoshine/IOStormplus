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
        DelTempFileCmd = 1,
        DelJobFilesCmd = 2,
        HostnameCmd = 3,
		DelLocalOutputCmd = 4
    };

    class IAgent{
    public:
        void Run();
    };

}


#ifdef __cplusplus
}
#endif
