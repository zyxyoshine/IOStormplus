#pragma once

#include "../../header/baseagent.h"
#include <string>
#include <vector>
using namespace std;

#ifdef __cplusplus
extern "C"
{
#endif

namespace IOStormPlus{
    class WindowsAgent: public BaseAgent{
    public:
        virtual string ExecuteScript(string command);

    private:
        string ControllerIP;
    };
}


#ifdef __cplusplus
}
#endif
