#pragma once


#include <string>
using namespace std;

#ifdef __cplusplus
extern "C"
{
#endif


namespace IOStormPlus{
    const string AgentsConfigFilename = "agents.json";
	const string storageConfigFileName = "AzureStorage.config";
	const string workloadConfigFileName = "workload.json";
    const string WorkloadFolder = "workload\\";
    const string OutputFolder = "output\\";
    const string AgentInfoFolder = "agents\\";
	const string emptyErrorMessage = "#";
	const utility::string_t storageTempTableName = U("IOStormTemp");
	const utility::string_t tableCommandColumnName = U("CMD");
	const utility::string_t tableOSColumnName = U("OS");
	const utility::string_t tablePoolColumnName = U("POOL");
	const utility::string_t tableIPColumnName = U("IP");
	const utility::string_t tableSizeColumnName = U("SIZE");
	const utility::string_t tableErrorColumnName = U("ERR");
	const utility::string_t workloadBlobContainerName = U("workload");
	const utility::string_t outputBlobContainerName = U("output");
	const utility::string_t logBlobContainerName = U("log");
	const int maxWaitTimeInSec = 60 * 60;
	const int maxHeartbeatGapInSec = 2 * 60;
}

#ifdef __cplusplus
}
#endif
