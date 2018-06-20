#include "header/testvm.h"
#include "../common/header/logger.h"
#include "header/constant.h"
#include "header/helper.h"
#include <sstream>
#include <algorithm>
#include <iostream>

using namespace std;

namespace IOStormPlus {

    ////////////////////////////////////////////////////////////////////////////////////////////
    // Public function
    ///////////////////////////////////////////////////////////////////////////////////////////

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

    string TestVM::GetInfo() {
        stringstream tempStream;
        tempStream << GetName() << "\t" << GetInternalIP() << "\t" << GetOSTypeName() + "\t" + GetSize();
        return tempStream.str();
    }

    string TestVM::GetTestResult(const string& jobName) {
        int readMinIOPS = 1 << 30;
        int readMaxIOPS = 0;
        int readAvgIOPS = 0;
        int writeMinIOPS = 1 << 30;;
        int writeMaxIOPS = 0;
        int writeAvgIOPS = 0;
        
        for (auto &iter : m_testResults[jobName].ReadIOPS) {
            readMinIOPS = min(readMinIOPS, iter);
            readMaxIOPS = max(readMaxIOPS, iter);
            readAvgIOPS += iter;
        }

        if (m_testResults[jobName].ReadIOPS.size() != 0) {
            readAvgIOPS /= m_testResults[jobName].ReadIOPS.size();
        }

        for (auto &iter : m_testResults[jobName].WriteIOPS) {
            writeMinIOPS = min(writeMinIOPS, iter);
            writeMaxIOPS = max(writeMaxIOPS, iter);
            writeAvgIOPS += iter;
        }
        
        if (m_testResults[jobName].WriteIOPS.size() != 0) {
            writeAvgIOPS /= m_testResults[jobName].WriteIOPS.size();
        }

        stringstream tempStream;
        tempStream << GetName() << "\t" << GetInternalIP() << "\t" << GetOSTypeName() + "\t" + GetSize()
                   << "\t" << readMinIOPS << "\t" << readMaxIOPS << "\t" << readAvgIOPS << "\t" 
                   << writeMinIOPS << "\t" << writeMaxIOPS << "\t" << writeAvgIOPS;
        // Logger::LogVerbose(tempStream.str());
        return tempStream.str();
    }

    void TestVM::SetTestResult(const string& jobName, const ReportSummary& report) {
        m_testResults[jobName] = report;
    }
    
    int TestVM::CountTestResult(const string& jobName) {
        return m_testResults.count(jobName);
    }
    
    void TestVM::SendCommand(SCCommand command){
        ofstream fout;
        ExecCommand(string("del /f " + GetSharePath() + TempFolder + "client.tmp"));
        fout.open(GetSharePath() + TempFolder + "controller.tmp", ios_base::out | ios_base::trunc);
        fout << GetCommandString(command);
        fout.close();        
    }

    bool TestVM::GetResponse(SCCommand command){
        ifstream fin;
        string buf;

        fin.open(GetSharePath() + TempFolder + "client.tmp", ios_base::in);
        if (!fin.fail()) {
            fin >> buf;
            if(buf.compare(GetCommandString(command)) == 0) {
                return true;
            }
        }
        fin.close();

        return false;       
    }

    ////////////////////////////////////////////////////////////////////////////////////////////
    // Private function
    ///////////////////////////////////////////////////////////////////////////////////////////

    string TestVM::GetOSTypeName() {
        return m_osType == Linux ? "linux" : "windows";
    }


}