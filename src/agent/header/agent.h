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
        HostnameCmd = 4,
		DelLocalOutputCmd = 5
    };

    class IAgent{
    public:
        void Run();
    };

}


#ifdef __cplusplus
}
#endif
