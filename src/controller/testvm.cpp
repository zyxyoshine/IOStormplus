#include "testvm.h"
#include "logger.h"
#include "constant.h"
#include "helper.h"
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

    string TestVM::GetTestResult(string jobName) {
        int readMinIOPS = 1 << 30;
        int readMaxIOPS = 0;
        int readAvgIOPS = 0;
        int writeMinIOPS = 1 << 30;;
        int writeMaxIOPS = 0;
        int writeAvgIOPS = 0;
        
        for (auto &iter : TestResults[jobName].ReadIOPS) {
            readMinIOPS = min(readMinIOPS, iter);
            readMaxIOPS = max(readMaxIOPS, iter);
            readAvgIOPS += iter;
        }

        if (TestResults[jobName].ReadIOPS.size() != 0) {
            readAvgIOPS /= TestResults[jobName].ReadIOPS.size();
        }

        for (auto &iter : TestResults[jobName].WriteIOPS) {
            writeMinIOPS = min(writeMinIOPS, iter);
            writeMaxIOPS = max(writeMaxIOPS, iter);
            writeAvgIOPS += iter;
        }
        
        if (TestResults[jobName].WriteIOPS.size() != 0) {
            writeAvgIOPS /= TestResults[jobName].WriteIOPS.size();
        }

        stringstream tempStream;
        tempStream << GetName() << "\t" << GetInternalIP() << "\t" << GetOSTypeName() + "\t" + GetSize()
                   << "\t" << readMinIOPS << "\t" << readMaxIOPS << "\t" << readAvgIOPS << "\t" 
                   << writeMinIOPS << "\t" << writeMaxIOPS << "\t" << writeAvgIOPS;
        return tempStream.str();
    }

    
    void TestVM::SendCommand(string command){
        ofstream fout;

        Logger::LogInfo("Sending pre-sync request to test VM " + GetName() + "(" + GetInternalIP() + ")");
        ExecCommand(string("del /f " + GetSharePath() + TempFolder + "client.tmp"));
        fout.open(GetSharePath() + TempFolder + "Controller.tmp", ios_base::out | ios_base::trunc);
        fout << command;
        fout.close();        
    }

    bool TestVM::GetResponse(string command){
        ifstream fin;
        string buf;

        fin.open(GetSharePath() + TempFolder + "client.tmp", ios_base::in);
        if (!fin.fail()) {
            fin >> buf;
            if(buf == command) {
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