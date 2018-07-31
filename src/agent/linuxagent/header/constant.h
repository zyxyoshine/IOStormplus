#pragma once

#include "../../header/constant.h"

#include <string>
using namespace std;

#ifdef __cplusplus
extern "C"
{
#endif

namespace IOStormPlus{
    const string BinFolderPath = "/home/IOStormplus";
    const string DirSpliter = "/";
    const string WorkspaceFolderPath = "/home/fiojob/";
    const string WorkLoadFolderPath = BinFolderPath + WorkLoadFolderName + DirSpliter;
    const string OutputFolder = BinFolderPath + OutputFolderName + DirSpliter;
    const string LogFilePath = BinFolderPath + LogFilename;
    const int SyncWaitTime = 1000 * 1000;    
}

#ifdef __cplusplus
}
#endif
