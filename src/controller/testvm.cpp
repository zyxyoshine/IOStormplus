#include "testvm.h"
#include <sstream>
#include <algorithm>

namespace IOStormPlus {

    string TestVM::GetName(){
        return m_name;
    }

    string TestVM::GetInternalIP(){
        return m_internalIP;
    }

    string TestVM::GetSize(){
        return m_size;
    }

    OSType TestVM::GetOSType(){
        return m_osType;
    }
    
    string TestVM::GetSharePath(){
        return "\\\\" + m_internalIP + "\\";
    }

    string TestVM::GetOSTypeName() {
        return m_osType == Linux ? "Linux" : "Windows";
    }

    string TestVM::GetVMInfo() {
        stringstream tempStream;
        tempStream << m_name << "\t" << m_internalIP << "\t" << (GetOSTypeName() + "\t" + m_size);
        return tempStream.str();
    }

    string TestVM::GetVMResult(string jobNum) {
        int readMinIOPS = 1 << 30;
        int readMaxIOPS = 0;
        int readAvgIOPS = 0;
        int writeMinIOPS = 1 << 30;;
        int writeMaxIOPS = 0;
        int writeAvgIOPS = 0;
        
        for (auto iter : TestResults[jobNum].ReadIOPS) {
            readMinIOPS = min(readMinIOPS, iter);
            readMaxIOPS = max(readMaxIOPS, iter);
            readAvgIOPS += iter;
        }

        if (TestResults[jobNum].ReadIOPS.size() != 0)
            readAvgIOPS /= TestResults[jobNum].ReadIOPS.size();

        for (auto iter : TestResults[jobNum].WriteIOPS) {
            writeMinIOPS = min(writeMinIOPS, iter);
            writeMaxIOPS = max(writeMaxIOPS, iter);
            writeAvgIOPS += iter;
        }
        if (TestResults[jobNum].WriteIOPS.size() != 0)
            writeAvgIOPS /= TestResults[jobNum].WriteIOPS.size();

        stringstream tempStream;
        tempStream << m_name << "\t" << m_internalIP << "\t" << (GetOSTypeName() + "\t" + m_size) 
                   << "\t" << readMinIOPS << "\t" << readMaxIOPS << "\t" << readAvgIOPS << "\t" 
                   << writeMinIOPS << "\t" << writeMaxIOPS << "\t" << writeAvgIOPS;
        return tempStream.str();
    }
}