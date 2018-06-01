#pragma once

#include "..\constant.h"

#include <string>
using namespace std;

#ifdef __cplusplus
extern "C"
{
#endif

namespace IOStormPlus{
    const string BinFolderPath = "c:\\IOStormplus\\";
    const string DirSpliter = "\\";
    const string WorkspaceFolderPath = "fiojob\\";
    const string WorkLoadFolderPath = BinFolderPath + WorkLoadFolderName + DirSpliter;
    const string OutputFolder = BinFolderPath + OutputFolderName + DirSpliter;
    const string ControllerTempFilePath = BinFolderPath + InfoTempFolderName + DirSpliter + ControllerTempFilename;
    const string ClientTempFilePath = BinFolderPath + InfoTempFolderName + DirSpliter + ClientTempFilename;
    const string LogFilePath = BinFolderPath + LogFilename;
    const int SyncWaitTime = 10000;    
}

#ifdef __cplusplus
}
#endif
