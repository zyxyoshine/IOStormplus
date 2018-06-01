#pragma once

#include <string>
using namespace std;

#ifdef __cplusplus
extern "C"
{
#endif

namespace IOStormPlus{
    const string WorkLoadFolderName = "workload";
    const string InfoTempFolderName = "temp";
    const string OutputFolderName = "output";
    const string ControllerTempFilename = "controller.tmp";
    const string ClientTempFilename = "client.tmp";
    const string LogFilename = "log.txt";
    const int SyncWaitTime = 10000;    
}

#ifdef __cplusplus
}
#endif
