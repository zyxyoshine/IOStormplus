#pragma once
#include "../../common/header/command.h"
#include <map>
#include <vector>
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
        TestVM(string name, string internalIP, OSType osType, string size):
            m_name(name), m_internalIP(internalIP), m_osType(osType), m_size(size){}

        TestVM(string name, string internalIP, string osType, string size):
            m_name(name), m_internalIP(internalIP), m_osType(osType == "linux" ? Linux : Windows), m_size(size){}

        string GetInfo();
        string GetTestResult(const string& jobName);
        void SetTestResult(const string& jobName, const ReportSummary& report);
        int CountTestResult(const string& jobName);
        string GetSharePath();
        string GetName();
        string GetInternalIP();
        string GetSize();
        OSType GetOSType();

        void SendCommand(SCCommand command);
        bool GetResponse(SCCommand command);

    private:
        string m_name;
        string m_internalIP;
        string m_size;
        OSType m_osType;
        map<string, ReportSummary> m_testResults;

        string GetOSTypeName();

    };
}

#ifdef __cplusplus
}
#endif
