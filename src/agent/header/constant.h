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
    const string InfoTempFolderName = "temp";
    const string OutputFolderName = "output";
    const string VMInfoFolderName = "info";
    const string AgentsInfoFolderName = "agents";
    const string ControllerTempFilename = "controller.tmp";
    const string ClientTempFilename = "client.tmp";
    const string LogFilename = "log.txt";
	const string storageConfigFileName = "AzureStorage.config";
	const utility::string_t storageTempTableName = U("IOStormTemp");
	const utility::string_t tableCommandColumnName = U("CMD");
	const utility::string_t tableOSColumnName = U("OS");
	const utility::string_t tablePoolColumnName = U("POOL");
	const utility::string_t tableIPColumnName = U("IP");
	const utility::string_t tableSizeColumnName = U("SIZE");
	const utility::string_t workloadBlobContainerName = U("WORKLOAD");
	const utility::string_t outputBlobContainerName = U("OUTPUT");
}

#ifdef __cplusplus
}
#endif
