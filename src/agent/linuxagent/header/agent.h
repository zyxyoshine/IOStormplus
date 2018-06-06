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
    class LinuxAgent: public BaseAgent{
    public:
        virtual string ExecuteScript(string command);  
    };
}


#ifdef __cplusplus
}
#endif
