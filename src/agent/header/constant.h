#pragma once

#include <string>
#include <was/storage_account.h>
using namespace std;

#ifdef __cplusplus
extern "C"
{
#endif

namespace IOStormPlus{
    const string WorkLoadFolderName = "workload";
    const string OutputFolderName = "output";
    const string VMInfoFolderName = "info";
    const string LogFilename = "log.txt";
	const string storageConfigFileName = "AzureStorage.config";
	const string workloadConfigFileName = "workload.json";
	const utility::string_t storageTempTableName = U("IOStormTemp");
	const utility::string_t tableCommandColumnName = U("CMD");
	const utility::string_t tableOSColumnName = U("OS");
	const utility::string_t tablePoolColumnName = U("POOL");
	const utility::string_t tableIPColumnName = U("IP");
	const utility::string_t tableSizeColumnName = U("SIZE");
	const utility::string_t workloadBlobContainerName = U("workload");
	const utility::string_t outputBlobContainerName = U("output");
	const utility::string_t logBlobContainerName = U("log");
}

#ifdef __cplusplus
}
#endif
