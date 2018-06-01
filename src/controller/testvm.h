#pragma once
#include <map>
#include <vector>
using namespace std;

#ifdef __cplusplus
extern "C"
{
#endif

namespace IOStormPlus{
    
    enum OSType{
        Linux = 0,
        Windows = 1
    };

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

        map<string, ReportSummary> TestResults;

        string GetVMInfo();
        string GetVMResult(string jobNum);
        string GetSharePath();
        string GetName();
        string GetInternalIP();
        string GetSize();
        OSType GetOSType();

    private:
        string GetOSTypeName();

        string m_name;
        string m_internalIP;
        string m_size;
        OSType m_osType;

    };
}

#ifdef __cplusplus
}
#endif
