#pragma once
#include "../../common/header/command.h"
#include <map>
#include <vector>
#include <was/storage_account.h>
#include <was/table.h>

using namespace std;

#ifdef __cplusplus
extern "C"
{
#endif

namespace IOStormPlus{
    
    // Test OS only support Windows and Linux
    enum OSType{
        Linux = 0,
        Windows = 1
    };

    // TODO: Move to report class
    struct ReportSummary {
        vector<int> ReadIOPS;
        vector<int> WriteIOPS;
        ReportSummary() {
            ReadIOPS.clear();
            WriteIOPS.clear();
        }
    };

    class TestVM {

    public:
		TestVM(string name, string internalIP, OSType osType, string size, string pool) :
            m_name(name), m_internalIP(internalIP), m_osType(osType), m_size(size), m_pool(pool){}

        TestVM(string name, string internalIP, string osType, string size, string pool):
            m_name(name), m_internalIP(internalIP), m_osType(osType == "linux" ? Linux : Windows), m_size(size), m_pool(pool) {}

        string GetInfo();
        string GetTestResult(const string& jobName);
        void SetTestResult(const string& jobName, const ReportSummary& report);
        int CountTestResult(const string& jobName);
        string GetSharePath();
        string GetName();
        string GetInternalIP();
        string GetSize();
		string GetPool();
        OSType GetOSType();

        void SendCommand(azure::storage::cloud_table& table, SCCommand command = EmptyCmd);
        bool GetResponse(azure::storage::cloud_table& table, SCCommand command, SCCommand retryCMD = SCCommand::InvaildCmd);
		
    private:
        string m_name;
        string m_internalIP;
        string m_size;
		string m_pool;
        OSType m_osType;
        map<string, ReportSummary> m_testResults;

        string GetOSTypeName();

    };
}

#ifdef __cplusplus
}
#endif
