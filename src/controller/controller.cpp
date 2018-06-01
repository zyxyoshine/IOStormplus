#include "controller.h"
#include "constant.h"
#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "logger.h"
#include "helper.h"
#include <windows.h>
#include <algorithm>
#include <ctime>
#include <iostream>
#include <cassert>

using namespace rapidjson;
using namespace std;

#ifdef __cplusplus
extern "C"
{
#endif

namespace IOStormPlus{

    ////////////////////////////////////////////////////////////////////////////////////////////
    // Public function
    ///////////////////////////////////////////////////////////////////////////////////////////
    Controller::Controller(string configFilename) {
        InitLogger();
        m_isReady = false;

        /* Load JSON file */
        fstream fin(configFilename);
        string data, content = "";
        while (!fin.eof()) {
            getline(fin, data);
            content += data;
        }
        fin.close();

        Document VMConfig;
        if (VMConfig.Parse(content.c_str()).HasParseError()) {
            stringstream tempStream;
            tempStream << "parse test VM configuration failed! " << content.c_str() << endl;
            Logger::LogError(tempStream.str());
            return;
        }
        
        int vmCount = VMConfig["count"].GetInt();
        auto vmInfo = VMConfig["value"].GetArray();
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
        string internalIP,osType,size;
        for (auto agentName : agents) {
            fin.open(AgentInfoFolder + agentName);
            fin >> internalIP >> osType >> size;
            TestVMs.push_back(TestVM(agentName, internalIP, osType, size));
            Logger::LogVerbose("Register test VM " + agentName + " succeeded.");
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
        if (argc == 0)
            PrintUsage(ControllerCommand::AgentGeneral);
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
        else if (argc > 1) {
            PrintUsage(ControllerCommand::WorkerGeneral);
        }
        else if (strcmp(argv[0], "-std") == 0) {
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
        switch(command) {
            case ControllerCommand::General: {
                cout << "USAGE: IOStormplus [options] {parameters}        " << endl;
                cout << "Options:                                             " << endl;
                cout << "help                   Display usage.                " << endl;
                cout << "agent                  Configure the test VM agents. " << endl;
                cout << "start                  Start test job.               " << endl;
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
        stringstream tempStream;
        tempStream << "%d%02d%02d.log" << now->tm_year + 1900 << now->tm_mon + 1 << now->tm_mday << endl;
        Logger::Init(tempStream.str());
    }

    // Health Check
    void Controller::CheckTestVMHealth() {
        ofstream fout;
        string reader;
        ifstream fin;
        for (auto iter : TestVMs) {
            Logger::LogVerbose("Sending pre-sync request to test VM " + iter.GetName() + "(" + iter.GetInternalIP() + ")");
            ExecCommand(string("del /f " + iter.GetSharePath() + TempFolder + "client.tmp").c_str());
            fout.open(iter.GetSharePath() + TempFolder + "Controller.tmp", ios_base::out | ios_base::trunc);
            fout << "PRESYNC";
            fout.close();
        }

        Logger::LogVerbose("Waiting for agents response.");
        bool allDone = false;
        map<string, bool> doneJobs;
        doneJobs.clear();
        while (!allDone) {
            allDone = true;
            for (auto iter : TestVMs) {
                
                if (doneJobs[iter.GetInternalIP()]) {
                    continue;
                }

                ifstream fin;
                fin.open(iter.GetSharePath() + TempFolder + "client.tmp", ios_base::in);
                if (!fin.fail()) {
                    fin >> reader;
                    if(reader == "SYNCDONE") {
                        doneJobs[iter.GetInternalIP()] = true;
                        Logger::LogVerbose("Test VM " + iter.GetName() + "(" + iter.GetInternalIP() + ")" + " pre-sync succeeded.");
                    }
                    else {
                        allDone = false;
                    }
                }
                else {
                    allDone = false;
                }
                fin.close();
            }
        }
        Logger::LogVerbose("All test VM pre-sync succeeded!");
    }

    // Test Execution
    void Controller::RunStandardTest() {
        ifstream fin;
        ofstream fout;
        string reader;
        for (auto iter : TestVMs) {
            if (iter.GetOSType() == OSType::Linux) {
                ExecCommand(string("xcopy workload\\std\\linux " + iter.GetSharePath() + WorkloadFolder + " /s /h /d /y").c_str());
            }
            else if (iter.GetOSType() == OSType::Windows){           
                ExecCommand(string("xcopy workload\\std\\windows " + iter.GetSharePath() + WorkloadFolder + " /s /h /d /y").c_str());
            }
            else {
                assert(false);
            }

            Logger::LogVerbose("Sending work request to test VM " + iter.GetName() + "(" + iter.GetInternalIP() + ")");
            ExecCommand(string("del /f " + iter.GetSharePath() + TempFolder + "client.tmp").c_str());
            fout.open(iter.GetSharePath() + TempFolder + "Controller.tmp", ios_base::out | ios_base::trunc);
            fout << "START";
            fout.close();
        }

        Logger::LogVerbose("Waiting for agents response.");
        map<string, bool> doneJobs;
        doneJobs.clear();

        bool allDone = false;
        while (!allDone) {
            allDone = true;
            for (auto iter : TestVMs) {
                if (doneJobs[iter.GetInternalIP()] == 1) {
                    continue;
                }
                fin.close();
                fin.clear();
                Sleep(1000);
                fin.open(iter.GetSharePath() + TempFolder + "client.tmp", ios_base::in);
                fin >> reader;
                if (reader == "DONE") {
                    doneJobs[iter.GetInternalIP()] = 1;
                    Logger::LogVerbose("Test VM " + iter.GetName() + "(" + iter.GetInternalIP() + ")" + " successfully completed work.");
                }
                else {
                    allDone = false;
                }
            }
            fin.close();
        }
        
        Logger::LogVerbose("All jobs done!");
        AnalyzeData("workload\\std\\");
        PrintTestResultSummary("workload\\std\\");
    }

    void Controller::RunCustomTest() {
        ifstream fin;
        ofstream fout;
        string reader;
        for (auto iter : TestVMs) {
            if (iter.GetOSType() == OSType::Linux) {
                ExecCommand(string("xcopy workload\\linux " + iter.GetSharePath() + WorkloadFolder + " /s /h /d /y").c_str());
            }
            else if(iter.GetOSType() == OSType::Windows) {
                ExecCommand(string("xcopy workload\\windows " + iter.GetSharePath() + WorkloadFolder + " /s /h /d /y").c_str());
            }
            else {
                assert(false);
            }
            Logger::LogVerbose("Sending work request to test VM " + iter.GetName() + "(" + iter.GetInternalIP() + ")");
            ExecCommand(string("del /f " + iter.GetSharePath() + TempFolder + "client.tmp").c_str());
            fout.open(iter.GetSharePath() + TempFolder + "Controller.tmp", ios_base::out | ios_base::trunc);
            fout << "START";
            fout.close();
        }
        Logger::LogVerbose("Waiting for agents response.");
        map<string, bool> doneJobs;
        doneJobs.clear();

        bool allDone = false;
        while (!allDone) {
            allDone = true;
            for (auto vm_itr : TestVMs) {
                if (doneJobs[vm_itr.GetInternalIP()] == 1) {
                    continue;
                }
                fin.close();
                fin.clear();
                Sleep(1000);
                fin.open(vm_itr.GetSharePath() + TempFolder + "client.tmp", ios_base::in);
                fin >> reader;
                if (reader == "DONE") {
                    doneJobs[vm_itr.GetInternalIP()] = 1;
                    Logger::LogVerbose("Test VM " + vm_itr.GetName() + "(" + vm_itr.GetInternalIP() + ")" + " successfully completed work.");
                }
                else {
                    allDone = false;
                }
            }
            fin.close();
        }
        Logger::LogVerbose("All jobs done!");
        AnalyzeData("workload\\");
        PrintTestResultSummary("workload\\");
    }

    // Agent management
    void Controller::RegisterAgent(int argc, char *argv[]) {
        string name, internalIP, osType, size = "";
        if (argc != 4) {
            PrintUsage(ControllerCommand::AgentRegister);
            return;
        }
        else if (1) { //TODO: Add error check
            name = argv[0];
            internalIP = argv[1];
            osType = argv[2];
            osType = argv[3];
            if (osType == "linux" || osType == "windows") {
                bool vmExists = false;
                for (auto iter : TestVMs) {
                    if (iter.GetName() == name) {
                        vmExists = true;
                        break;
                    }
                }
                if (vmExists) {
                    cout << "VM " + name + " has already been registered.";
                }
                else{
                    TestVMs.push_back(TestVM(name, internalIP, osType, size));
                    WriteConfig();
                    Logger::LogVerbose("Register test VM " + name + " succeeded.");
                    PrintTestVMInfo();
                }
            }
            else {
                cout << "ERROR: Illegal VM OS type";
                PrintUsage(ControllerCommand::AgentRegister);
            }
        }
        else{
            cout << "ERROR: Illegal VM IP address" << endl;
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
            Logger::LogVerbose("Remove test VM " + vmName + " succeeded.");
            PrintTestVMInfo();
        }
        else {
            Logger::LogVerbose("Can not find registered test VM " + vmName + ".");
        }
    }

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
        stringstream tempStream;
        tempStream << "VM Count: " << TestVMs.size() << endl;
        Logger::LogVerbose(tempStream.str());
        Logger::LogVerbose("ID\tName\tIP Address\tOS\tSize");
        
        tempStream.clear();
        for (int i = 0;i < TestVMs.size(); ++i) {
            tempStream.clear();
            tempStream << i + 1 << "\t" << TestVMs[i].GetVMInfo() << endl;
            Logger::LogVerbose(tempStream.str());
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
        tempStream << "VM Count: " << TestVMs.size() << endl;
        Logger::LogVerbose(tempStream.str());
        fout << tempStream.str() << endl;

        for (auto job : jobs) {
            Logger::LogVerbose("Job: " + job);
            fout << "Job: " + job << endl;
            Logger::LogVerbose("ID\tName\tIP Address\tOS\tSize\tR(MIN)\tR(MAX)\tR(AVG)\tW(MIN)\tW(MAX)\tW(AVG)");
            fout << "ID\tName\tIP Address\tOS\tSize\tR(MIN)\tR(MAX)\tR(AVG)\tW(MIN)\tW(MAX)\tW(AVG)" << endl;
            string vm_id;
            for (int i = 0;i < TestVMs.size();i++) {
                if (TestVMs[i].TestResults.count(job)) {
                    tempStream.clear();
                    tempStream << i + 1;
                    tempStream >> vm_id;
                    Logger::LogVerbose(vm_id + "\t" + TestVMs[i].GetVMResult(job));
                    fout << vm_id + "\t" + TestVMs[i].GetVMResult(job) << endl;
                }
            }
        }
        
        fout.close();
    }

    void Controller::AnalyzeData(string workloadRootPath) {
        string outputFile;
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
        
        const string outputFile = OutputFolder + vm.GetName() + "_" + jobName + ".out";
        const string copyOutputCmd = "copy " + vm.GetSharePath() + outputFile + " " + outputFile + " /y";
        
        do {
            res = ExecCommand(copyOutputCmd.c_str());   
        } while (res.find("cannot") != string::npos);

        vm.TestResults[jobName] = AnalyzeStandardOutput(outputFile);
    }

    ReportSummary Controller::AnalyzeStandardOutput(string outputFile) {
        ifstream fin(outputFile, ios_base::in);
        string buffer;
        ReportSummary res;
        
        while (!fin.eof()) {
            getline(fin, buffer);
            int pos = buffer.find("read: IOPS=");
            if (pos != string::npos) {
                pos += 11;
                res.ReadIOPS.push_back(GetIOPSNumber(buffer, pos));
            }

            pos = buffer.find("write: IOPS=");
            if (pos != string::npos) {
                pos += 12;
                res.WriteIOPS.push_back(GetIOPSNumber(buffer, pos));
            }
        }
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
        return (int)num;
    }

    int main(int argc,char *argv[]) {
        Controller controller(AgentsConfigFilename);
        if (controller.IsReady())
            return 0;

        if (argc == 1)
            controller.PrintUsage(ControllerCommand::General);
        else if (strcmp(argv[1], "agent") == 0) {
            controller.ConfigureAgent(argc - 2, argv + 2);
        }
        else if (strcmp(argv[1], "start") == 0) {
            controller.RunTest(argc - 2, argv + 2);
        }
        else if (strcmp(argv[1], "init") == 0) {
            controller.InitAgents();
        }
        else {
            controller.PrintUsage(ControllerCommand::General);
        }
        return 0;
    }
}

#ifdef __cplusplus
}
#endif



