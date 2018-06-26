#include "header/controller.h"
#include "header/constant.h"
#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "../common/header/logger.h"
#include "header/helper.h"
#include <windows.h>
#include <algorithm>
#include <ctime>
#include <iostream>
#include <cassert>

using namespace rapidjson;
using namespace std;

namespace IOStormPlus{

    ////////////////////////////////////////////////////////////////////////////////////////////
    // Public function
    ///////////////////////////////////////////////////////////////////////////////////////////
    Controller::Controller(string configFilename) {
        InitLogger();
        m_isReady = false;

        /* Load JSON file */
        fstream fin(configFilename);
        Logger::LogInfo("Open Configuration file: " + configFilename);
        string data, content = "";
        while (!fin.eof()) {
            getline(fin, data);
            content += data;
        }
        fin.close();

        // TODO: Handle configFilename not exisi

        Document VMConfig;
        if (VMConfig.Parse(content.c_str()).HasParseError()) {
            stringstream logStream;
            logStream << "parse test VM configuration failed! " << content.c_str() << endl;
            Logger::LogError(logStream.str());
            return;
        }
        Logger::LogInfo("Configure File has been parsed successfully!");

        int vmCount = VMConfig["count"].GetInt();
        auto vmInfo = VMConfig["value"].GetArray();
        
        stringstream logStream;
        logStream << "Test VM Count " << vmCount;      
        Logger::LogInfo(logStream.str());
        
        for (int i = 0; i < vmCount; ++i) {
            TestVMs.push_back(TestVM(vmInfo[i]["name"].GetString(), vmInfo[i]["ip"].GetString(), vmInfo[i]["info"]["type"].GetString(), vmInfo[i]["info"]["size"].GetString()));
        }

        m_isReady = true;
    }

    /// Set the Agent info to the controller
    void Controller::InitAgents() {
        TestVMs.clear();
        vector<string> agents = ListFilesInDirectory(AgentInfoFolder);
        ifstream fin;
        string internalIP = "", osType = "", size = "";
        for (auto agentName : agents) {
            fin.open(AgentInfoFolder + agentName);
            fin >> internalIP >> osType >> size;
            TestVMs.push_back(TestVM(agentName, internalIP, osType, size));
            Logger::LogInfo("Register test VM " + agentName + " succeeded.");
            fin.close();
        }
        
        WriteConfig();
        PrintTestVMInfo();
    }

    /// True: if the Controller is ready for operation
    bool Controller::IsReady() {
        return m_isReady;
    }

    /// Run Configure Agent command
    void Controller::ConfigureAgent(int argc, char *argv[]) {
        if (argc == 0) {
            PrintUsage(ControllerCommand::AgentGeneral);
        }
        else if (strcmp(argv[0], "add") == 0) {
            RegisterAgent(argc - 1, argv + 1);
        }
        else if (strcmp(argv[0], "show") == 0) {
            ShowAgent();
        }
        else if (strcmp(argv[0], "test") == 0) {
            TestAgent();
        }
        else if (strcmp(argv[0], "rm") == 0) {
            RemoveAgent(argc - 1, argv + 1);
        }
        else {
            PrintUsage(ControllerCommand::AgentGeneral);
        }
    }
    
    /// Run Test command
    void Controller::RunTest(int argc, char *argv[]) {
        if (argc == 0) {
            Logger::LogVerbose("Start custom test.");
            CheckTestVMHealth();
            RunCustomTest();
            Logger::LogVerbose("End custom test.");
        }
        else if ( argc == 1 && (strcmp(argv[0], "-std") == 0)) {
            Logger::LogVerbose("Start standard test.");
            CheckTestVMHealth();
            RunStandardTest();
            Logger::LogVerbose("End standard test.");
        }
        else {
            PrintUsage(ControllerCommand::WorkerGeneral);
        }
    }

    /// Usage
    void Controller::PrintUsage(ControllerCommand command) {
        cout<<"Usage";
        switch(command) {
            case ControllerCommand::General: {
                cout << "USAGE: IOStormplus [options] {parameters}        " << endl;
                cout << "Options:                                             " << endl;
                cout << "help                   Display usage.                " << endl;
                cout << "agent                  Configure the test VM agents. " << endl;
                cout << "start                  Start test job.               " << endl;
                break;
            }
            case ControllerCommand::AgentGeneral: {
                cout << "USAGE: IOStormplus agent [options] {parameters}                         " << endl;
                cout << "Options:                                                                    " << endl;
                cout << "help                Display usage.                                          " << endl;
                cout << "add                 Register a test VM.                                     " << endl;
                cout << "show                Display all currently registered test VM information.   " << endl;
                cout << "rm                  remove a test VM.                                       " << endl;
            //   cout << "test                Determine the working status of all registered test VM. " << endl;
                break;
            }
            case ControllerCommand::AgentRegister:{
                cout << "USAGE: IOStormplus agent add [VM name] [VM internal ip] [VM OS(linux / windows)] [VM size]" << endl;
                break;
            }
            case ControllerCommand::AgentRemove:{
                cout << "USAGE: IOStormplus agent rm [VM name]" << endl;
                break;
            }
            case ControllerCommand::WorkerGeneral:{
                cout << "USAGE: IOStormplus start {parameters}            " << endl;
                cout << "parameters:                                          " << endl;
                cout << "-std                   Start standard test.          " << endl;
                cout << "{default}              Start custom test.            " << endl;
                break;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////
    // Private function
    ///////////////////////////////////////////////////////////////////////////////////////////
    void Controller::InitLogger() {
        time_t t = std::time(0);   
        tm* now = std::localtime(&t);
        stringstream logFilename;
        logFilename << now->tm_year + 1900 << now->tm_mon + 1 << now->tm_mday << ".log" ;
        Logger::Init(logFilename.str());
    }

    // Health Check
    void Controller::CheckTestVMHealth() {

        // Ask VM to sync    
        for (auto &vm : TestVMs) {
            Logger::LogInfo("Sending pre-sync request to test VM " + vm.GetName() + "(" + vm.GetInternalIP() + ")");
            vm.SendCommand(SCCommand::SyncCmd);
        }

        // Check all VMs to response sync
        WaitForAllVMs(SCCommand::SyncDoneCmd);
        Logger::LogInfo("All test VM pre-sync succeeded!");
    }

    void Controller::WaitForAllVMs(SCCommand command){
        // Check all VMs to response sync
        Logger::LogInfo("Waiting for agents response.");
        bool allDone = false;
        map<string, bool> doneJobs;

        while (!allDone) {
            allDone = true;
            for (auto vm : TestVMs) {
                
                if (doneJobs[vm.GetInternalIP()]) {
                    continue;
                }

                Sleep(1000);
                if (vm.GetResponse(command)) {
                    doneJobs[vm.GetInternalIP()] = true;
                    Logger::LogInfo("Test VM " + vm.GetName() + "(" + vm.GetInternalIP() + ")" + " successfully executed command.");                    
                }
                else {
                    allDone = false;
                }
            }
        }
    }

    // Test Execution
    void Controller::RunStandardTest() {
        RunTest(StandardWorkloadFolder);
    }

    void Controller::RunCustomTest() {
        RunTest(WorkloadFolder);
    }

    void Controller::RunTest(string rootPath){
        for (auto &vm : TestVMs) {
            if (vm.GetOSType() == OSType::Linux) {
                ExecCommand(string("xcopy " + rootPath + "linux " + vm.GetSharePath() + WorkloadFolder + " /s /h /d /y"));
            }
            else if(vm.GetOSType() == OSType::Windows) {
                ExecCommand(string("xcopy " + rootPath + "windows " + vm.GetSharePath() + WorkloadFolder + " /s /h /d /y"));
            }
            else {
                assert(false);
            }

            Logger::LogInfo("Sending work request to test VM " + vm.GetName() + "(" + vm.GetInternalIP() + ")");
            vm.SendCommand(SCCommand::StartJobCmd);
        }

        WaitForAllVMs(SCCommand::JobDoneCmd);

        Logger::LogVerbose("All jobs done!");

        AnalyzeData(rootPath);

        PrintTestResultSummary(rootPath);
    }

    // Agent management
    void Controller::RegisterAgent(int argc, char *argv[]) {
        string name, internalIP, osType, size = "";
        if (argc != 4) {
            PrintUsage(ControllerCommand::AgentRegister);
            return;
        }
        //TODO: Add error check
        else if (1) {
            name = argv[0];
            internalIP = argv[1];
            osType = argv[2];
            size = argv[3];
            if (osType == "linux" || osType == "windows") {
                bool vmExists = false;
                for (auto &iter : TestVMs) {
                    if (iter.GetName() == name) {
                        vmExists = true;
                        break;
                    }
                }
                if (vmExists) {
                    Logger::LogWarning("VM " + name + " has already been registered.");
                }
                else {
                    TestVMs.push_back(TestVM(name, internalIP, osType, size));
                    Logger::LogInfo("Register test VM " + name + " succeeded.");
                    WriteConfig();
                    PrintTestVMInfo();
                }
            }
            else {
                Logger::LogError("ERROR: Illegal VM OS type");
                PrintUsage(ControllerCommand::AgentRegister);
            }
        }
        else{
            Logger::LogError("ERROR: Illegal VM IP address");
            PrintUsage(ControllerCommand::AgentRegister);
        }
    }

    void Controller::RemoveAgent(int argc, char *argv[]) {
        string vmName;
        if (argc != 1) {
            PrintUsage(ControllerCommand::AgentRemove);
            return;
        }
        
        vmName = argv[0];

        bool found = false;
        for (auto iter = TestVMs.begin(); iter != TestVMs.end(); iter++) {
            if (iter->GetName() == vmName) {
                found = true;
                TestVMs.erase(iter);
                break;
            }
        }
        WriteConfig();
        
        if (found){
            Logger::LogInfo("Remove test VM " + vmName + " succeeded.");
            PrintTestVMInfo();
        }
        else {
            Logger::LogWarning("Can not find registered test VM " + vmName + ".");
        }
    }

    // TODO: Change name
    void Controller::ShowAgent() {
        PrintTestVMInfo();
    }

    void Controller::TestAgent() {
        Logger::LogVerbose("Using pre-sync to determine the working status of all registered test VM.");
        CheckTestVMHealth();
    }

    void Controller::WriteConfig() {
        ofstream fout(AgentsConfigFilename, ios_base::out | ios_base::trunc);
        fout << "{\n";
        fout << "\t\"count\":" << TestVMs.size() << ",\n";
        fout << "\t\"value\":\n";
        fout << "\t\t[\n";
        for (int i = 0; i < TestVMs.size(); i++) {
            fout << "\t\t\t{\n";
            fout << "\t\t\t\t\"name\":\"" << TestVMs[i].GetName() << "\",\n";
            fout << "\t\t\t\t\"ip\":\"" << TestVMs[i].GetInternalIP() << "\",\n";
            fout << "\t\t\t\t\"info\":{\n";
            fout << "\t\t\t\t\t\"type\":\"" << (TestVMs[i].GetOSType() == OSType::Linux ? "linux" : "windows") << "\",\n";
            fout << "\t\t\t\t\t\"size\":\"" << TestVMs[i].GetSize() << "\"\n";
            fout << "\t\t\t\t}\n";
            if (i != TestVMs.size() - 1) {
                fout << "\t\t\t},\n";
            }
            else {
                fout << "\t\t\t}\n";
            }
        }
        fout << "\t\t]\n";
        fout << "}\n";
        fout.close();
    }

    // Reporting
    void Controller::PrintTestVMInfo() {
        stringstream logStream;
        logStream << "VM Count: " << TestVMs.size();
        Logger::LogVerbose(logStream.str());
        Logger::LogVerbose("ID\tName\tIP Address\tOS\tSize");
        
        logStream.clear();
        logStream.str("");
        for (int i = 0;i < TestVMs.size(); ++i) {
            logStream.clear();
            logStream.str("");
            logStream << i + 1 << "\t" << TestVMs[i].GetInfo() << endl;
            Logger::LogVerbose(logStream.str());
        }
    }

    void Controller::PrintTestResultSummary(string workloadRootPath) {
        vector<string> linuxJobs = ListFilesInDirectory(workloadRootPath + "linux\\");
        vector<string> jobs = ListFilesInDirectory(workloadRootPath + "windows\\");
        jobs.insert(jobs.end(), linuxJobs.begin(), linuxJobs.end());
        
        sort(jobs.begin(), jobs.end());
        vector<string>::iterator iter = unique(jobs.begin(),jobs.end());       
        jobs.erase(iter,jobs.end());

        stringstream tempStream;
        string summaryOutputFile;
        time_t t = std::time(0);
        tm* now = std::localtime(&t);
        tempStream << (now->tm_year + 1900) << '-'
                   << (now->tm_mon + 1) << '-'
                   << now->tm_mday;
        tempStream >> summaryOutputFile;
        summaryOutputFile = OutputFolder + summaryOutputFile + "_summary.out";
        ofstream fout(summaryOutputFile, ios_base::out | ios_base::app);
        
        // Title
        tempStream.clear();
        tempStream.str(""); // Clear() do not clear stream buffer
        tempStream << "VM Count: " << TestVMs.size();
        Logger::LogInfo(tempStream.str());
        fout << tempStream.str() << endl;

        for (auto job : jobs) {
            Logger::LogInfo("Job: " + job);
            fout << "Job: " + job << endl;
            Logger::LogInfo("ID\tName\tIP Address\tOS\tSize\tR(MIN)\tR(MAX)\tR(AVG)\tW(MIN)\tW(MAX)\tW(AVG)");
            fout << "ID\tName\tIP Address\tOS\tSize\tR(MIN)\tR(MAX)\tR(AVG)\tW(MIN)\tW(MAX)\tW(AVG)" << endl;
            string vm_id;
            for (int i = 0;i < TestVMs.size();i++) {
                // Logger::LogVerbose("Job: " + job);
                if (TestVMs[i].CountTestResult(job)) {
                    tempStream.clear();
                    tempStream.str("");
                    tempStream << i + 1;
                    tempStream >> vm_id;
                    Logger::LogInfo(vm_id + "\t" + TestVMs[i].GetTestResult(job));
                    fout << vm_id + "\t" + TestVMs[i].GetTestResult(job) << endl;
                }
                else{
                    Logger::LogWarning("No Job");
                }
            }
        }

        fout.close();
    }

    void Controller::AnalyzeData(string workloadRootPath) {
        vector<string> linuxJobs = ListFilesInDirectory(workloadRootPath + "linux\\");
        vector<string> windowsJobs = ListFilesInDirectory(workloadRootPath + "windows\\");

        for (auto &iter : TestVMs) {
            if (iter.GetOSType() == OSType::Linux) {
                for (auto job : linuxJobs) {
                    AnalyzeJob(job, iter);
                }
            }
            else if (iter.GetOSType() == OSType::Windows) {
                for (auto job : windowsJobs) {
                   AnalyzeJob(job, iter);
                }
            }
            else {
                assert(false);
            }
        }
    }

    void Controller::AnalyzeJob(const string& job, TestVM& vm){
        string jobName = job;
        string res;
        
        if (jobName.find(".job") != string::npos) {
            jobName = jobName.replace(jobName.find(".job"),4,"");
        }
        Logger::LogVerbose("Start AnalyzeJob "+ jobName);
        
        const string outputFile = OutputFolder + vm.GetName() + "_" + jobName + ".out";
        const string copyOutputCmd = "copy " + vm.GetSharePath() + outputFile + " " + outputFile + " /y";
        
        do {
            res = ExecCommand(copyOutputCmd);   
        } while (res.find("cannot") != string::npos);

        Logger::LogInfo("Done execute command \"" + copyOutputCmd + "\"");

        Logger::LogVerbose("Jobname: "+jobName);
        vm.SetTestResult(job, AnalyzeStandardOutput(outputFile));
    }

    ReportSummary Controller::AnalyzeStandardOutput(string outputFile) {
        Logger::LogVerbose("Start AnalyzeStandardOutput "+ outputFile);
        
        ifstream fin(outputFile, ios_base::in);
        string buf;
        ReportSummary res;
        
        while (!fin.eof()) {
            getline(fin, buf);
            int pos = buf.find("read: IOPS=");
            if (pos != string::npos) {
                pos += 11;
                res.ReadIOPS.push_back(GetIOPSNumber(buf, pos));
            }

            pos = buf.find("write: IOPS=");
            if (pos != string::npos) {
                pos += 12;
                res.WriteIOPS.push_back(GetIOPSNumber(buf, pos));
            }
        }

        Logger::LogVerbose("End AnalyzeStandardOutput "+ outputFile);
        
        return res;
    }

    int Controller::GetIOPSNumber(string buf, int pos){
        double num = 0;
        int pointCount = 0;
        bool pointFlag = false;

        // TODO: Need Refactor here
        for (int i = pos;i < buf.size();i++) {
            if (buf[i] >= '0' && buf[i] <= '9') {
                num = (num * 10) + buf[i] - '0';
                if (pointFlag) {
                    pointCount++;
                }
            }
            else if (buf[i] == 'k') {
                num *= 1000;
            }
            else if (buf[i] == '.') {
                pointFlag = 1;
            }
            else {
                break;
            }
        }
        num /= pow(10,pointCount);
        // Logger::LogVerbose("IOPS number done");
        return (int)num;
    }

}

int main(int argc,char *argv[]) {
    IOStormPlus::Controller controller(IOStormPlus::AgentsConfigFilename);
    if (!controller.IsReady()){
        return 0;
    }

    if (argc == 1)
        controller.PrintUsage(IOStormPlus::ControllerCommand::General);
    else if (strcmp(argv[1], "agent") == 0) {
        controller.ConfigureAgent(argc - 2, argv + 2);
    }
    else if (strcmp(argv[1], "start") == 0) {
        controller.RunTest(argc - 2, argv + 2);
    }
    else if (strcmp(argv[1], "init") == 0) {
        controller.InitAgents();
    }
    else if (strcmp(argv[1], "test") == 0) {
        controller.CheckTestVMHealth();
    }
    else {
        controller.PrintUsage(IOStormPlus::ControllerCommand::General);
    }
    return 0;
}



