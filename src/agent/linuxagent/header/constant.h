#pragma once

#include "../constant.h"

#include <string>
using namespace std;

#ifdef __cplusplus
extern "C"
{
#endif

namespace IOStormPlus{
    const string BinFolderPath = "/samba/";
    const string WorkspaceFolderPath = "~/fiojob/";
    const string DirSpliter = "/";
    const string WorkLoadFolderPath = BinFolderPath + WorkspaceFolderPath + DirSpliter;
    const string OutputFolderPath = BinFolderPath + OutputFolderName + DirSpliter;
    const string ControllerTempFilePath = BinFolderPath + InfoTempFolderName + DirSpliter + ControllerTempFilename;
    const string ClientTempFilePath = BinFolderPath + InfoTempFolderName + DirSpliter + ClientTempFilename;
    const string LogFilePath = BinFolderPath + LogFilename;   
    const int SyncWaitTime = 10;    
}

#ifdef __cplusplus
}
#endif
