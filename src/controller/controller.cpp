#include "header/controller.h"
#include "header/constant.h"
#include "../common/rapidjson/document.h"     // rapidjson's DOM-style API
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
    Controller::Controller(string configFilename, string storageConfigFileName) {
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
            TestVMs.push_back(TestVM(vmInfo[i]["name"].GetString(), vmInfo[i]["ip"].GetString(), vmInfo[i]["info"]["type"].GetString(), vmInfo[i]["info"]["size"].GetString(), vmInfo[i]["info"]["pool"].GetString()));
        }

		fin.open(storageConfigFileName, ios_base::in);
		Logger::LogInfo("Open Azure Storage configuration file: " + storageConfigFileName);
		string storageAccountName, storageAccountKey, storageEndpointSuffix;
		getline(fin, storageAccountName);
		getline(fin, storageAccountKey);
		getline(fin, storageEndpointSuffix);
		fin.close();
		storageAccountName.replace(storageAccountName.find("NAME="), 5, "");
		storageAccountKey.replace(storageAccountKey.find("KEY="), 4, "");
		storageEndpointSuffix.replace(storageEndpointSuffix.find("ENDPOINTSUF="), 12, "");
		const utility::string_t storageConnectionString = utility::conversions::to_string_t("DefaultEndpointsProtocol=https;AccountName=" + storageAccountName + ";AccountKey=" + storageAccountKey + ";EndpointSuffix=" + storageEndpointSuffix);
		try {
			azure::storage::cloud_storage_account storageAccount = azure::storage::cloud_storage_account::parse(storageConnectionString);
			tableClient = storageAccount.create_cloud_table_client();
			blobClient = storageAccount.create_cloud_blob_client();
		}
		catch(const std::exception& e) {
			Logger::LogError(e.what());
		}
        m_isReady = true;
    }

    /// Set the Agent info to the controller
    void Controller::InitAgents() {
        TestVMs.clear();
		azure::storage::cloud_table table = tableClient.get_table_reference(IOStormPlus::storageTempTableName);
		table.create_if_not_exists();

		azure::storage::table_query query;
		query.set_filter_string(azure::storage::table_query::generate_filter_condition(IOStormPlus::tableCommandColumnName, azure::storage::query_comparison_operator::equal, utility::conversions::to_string_t(GetCommandString(SCCommand::EmptyCmd))));
		azure::storage::table_query_iterator itr = table.execute_query(query);
		azure::storage::table_query_iterator end_of_results;
		string agentName, internalIP, osType, size, pool;
		for (; itr != end_of_results; ++itr) {
			if ((utility::datetime::utc_now() - itr->timestamp()) > IOStormPlus::maxHeartbeatGapInSec)
				continue;
			const azure::storage::table_entity::properties_type& properties = itr->properties();

			agentName = utility::conversions::to_utf8string(itr->row_key());
			pool = utility::conversions::to_utf8string(itr->partition_key());
			internalIP = utility::conversions::to_utf8string(properties.at(IOStormPlus::tableIPColumnName).string_value());
			osType = utility::conversions::to_utf8string(properties.at(IOStormPlus::tableOSColumnName).string_value());
			size = utility::conversions::to_utf8string(properties.at(IOStormPlus::tableSizeColumnName).string_value());
			pool = utility::conversions::to_utf8string(properties.at(IOStormPlus::tablePoolColumnName).string_value());

			TestVMs.push_back(TestVM(agentName, internalIP, osType, size, pool));
			Logger::LogInfo("Register test VM " + agentName + " succeeded.");
		}

        WriteConfig();
        PrintTestVMInfo();
    }

    /// True: if the Controller is ready for operation
    bool Controller::IsReady() {
        return m_isReady;
    }

	void Controller::InitWorkload(string configFilename) {
        workload.clear();
		/* Load JSON file */
		fstream fin(WorkloadFolder + configFilename);
		Logger::LogInfo("Open workload configuration file: " + configFilename);
		string data, content = "";
		while (!fin.eof()) {
			getline(fin, data);
			content += data;
		}
		fin.close();

		// TODO: Handle configFilename not exisi

		Document workloadConfig;
		if (workloadConfig.Parse(content.c_str()).HasParseError()) {
			stringstream logStream;
			logStream << "parse workload configuration failed! " << content.c_str() << endl;
			Logger::LogError(logStream.str());
			return;
		}
		Logger::LogInfo("Configure File has been parsed successfully!");

		int workloadCount = workloadConfig["count"].GetInt();
		auto workloadInfo = workloadConfig["value"].GetArray();

		
        for (int i = 0; i < workloadCount; i++) {
            string poolName = workloadInfo[i]["pool"].GetString();
            int jobCount = workloadInfo[i]["count"].GetInt();
            auto jobs = workloadInfo[i]["jobs"].GetArray();
            for (int j = 0; j < jobCount; j++) {
                workload[poolName].push_back(jobs[j].GetString());
            }
        }

        if (workload.count("std") == 0) {
			Logger::LogWarning("No standard test job!");
        }
    
		Logger::LogInfo("Initialize workload settings succeeded.");
	}


	/// Upload workload to blob service
	void Controller::UploadWorkload() {
		vector<string> workloadFiles;
		workloadFiles = ListFilesInDirectory(WorkloadFolder);
		// Retrieve a reference to a container.
		azure::storage::cloud_blob_container container = blobClient.get_container_reference(IOStormPlus::workloadBlobContainerName);
		// Create the container if it doesn't already exist.
		container.create_if_not_exists();
		for (auto fna : workloadFiles) {
			azure::storage::cloud_block_blob blockBlob = container.get_block_blob_reference(utility::conversions::to_string_t(fna));
			blockBlob.upload_from_file(utility::conversions::to_string_t(WorkloadFolder + fna));
		}
		Logger::LogInfo("Upload workload files to blob succeeded.");
	}

	void Controller::UploadLog() {
		// Retrieve a reference to a container.
		azure::storage::cloud_blob_container container = blobClient.get_container_reference(IOStormPlus::logBlobContainerName);
		// Create the container if it doesn't already exist.
		container.create_if_not_exists();
		azure::storage::cloud_block_blob blockBlob = container.get_block_blob_reference(utility::conversions::to_string_t("controller_" + logFileName));
		blockBlob.upload_from_file(utility::conversions::to_string_t(logFileName));
		Logger::LogInfo("Upload log to blob succeeded.");
	}

	/// Download output files from blob
	void Controller::DownloadOutput() {
		azure::storage::cloud_blob_container container = blobClient.get_container_reference(IOStormPlus::outputBlobContainerName);
		azure::storage::list_blob_item_iterator end_of_results;
		for (auto it = container.list_blobs(); it != end_of_results; ++it) {
			if (it->is_blob()) {
				std::wcout << U("Blob: ") << it->as_blob().uri().primary_uri().to_string() << std::endl;
				string blobName = utility::conversions::to_utf8string(it->as_blob().name());
				it->as_blob().download_to_file(utility::conversions::to_string_t(OutputFolder + blobName));
			}
		}
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
            Logger::LogInfo("Start custom test.");
            CheckTestVMHealth();
			InitWorkload(IOStormPlus::workloadConfigFileName);
			UploadWorkload();
            RunCustomTest();
            Logger::LogInfo("End custom test.");
			UploadLog();
        }
        else if ( argc == 1 && (strcmp(argv[0], "-std") == 0)) {
            Logger::LogInfo("Start standard test.");
            CheckTestVMHealth();
			InitWorkload(IOStormPlus::workloadConfigFileName);
			UploadWorkload();
            RunStandardTest();
            Logger::LogInfo("End standard test.");
			UploadLog();
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
                cout << "USAGE: IOStormplus agent add [VM name] [VM internal ip] [VM OS(linux / windows)] [VM size] [VM pool]" << endl;
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
        stringstream logFileNameStream;
		logFileNameStream << now->tm_year + 1900 << now->tm_mon + 1 << now->tm_mday << ".log" ;
		logFileName = logFileNameStream.str();
        Logger::Init(logFileName);
    }

    // Health Check
    void Controller::CheckTestVMHealth() {
		azure::storage::cloud_table table = tableClient.get_table_reference(IOStormPlus::storageTempTableName);
        // Ask VM to sync
        for (auto &vm : TestVMs) {
            Logger::LogInfo("Sending pre-sync request to test VM " + vm.GetName() + "(" + vm.GetInternalIP() + ")");
            vm.SendCommand(table, SCCommand::SyncCmd);
        }
        // Check all VMs to response sync
        bool allDone = WaitForAllVMs(table, SCCommand::SyncDoneCmd, IOStormPlus::maxHeartbeatGapInSec);
		if (!allDone) {
			for (auto i = TestVMs.begin(); i != TestVMs.end(); i++) {
				if (!doneJobs[i->GetInternalIP()]) {
					Logger::LogWarning("Test VM " + i->GetName() + " pre-sync failed!");
					TestVMs.erase(i);
				}
			}
			WriteConfig();
		}
		else
			Logger::LogInfo("Test VM pre-sync succeeded!");
    }

    bool Controller::WaitForAllVMs(azure::storage::cloud_table& table, SCCommand command, int timeLimitInSec, SCCommand retryCMD){
        // Check all VMs to response sync
        Logger::LogInfo("Waiting for agents response.");
        bool allDone = false;
		doneJobs.clear();
		
		clock_t startTime = clock();
		double duration;
        while (!allDone) {
            allDone = true;
            for (auto vm : TestVMs) {

                if (doneJobs[vm.GetInternalIP()]) {
                    continue;
                }

                if (vm.GetResponse(table, command, retryCMD)) {
                    doneJobs[vm.GetInternalIP()] = true;
                    Logger::LogInfo("Test VM " + vm.GetName() + "(" + vm.GetInternalIP() + ")" + " successfully executed command.");
                }
                else {
                    allDone = false;
                }
            }
			duration = (std::clock() - startTime) / (double)CLOCKS_PER_SEC;
			if (duration > timeLimitInSec)
				break;
        }
		if (!allDone) {
			for (auto vm : TestVMs) {
				if (!doneJobs[vm.GetInternalIP()]) {
					Logger::LogWarning("Test VM " + vm.GetName() + "(" + vm.GetInternalIP() + ") time out!");
				}
			}
		}
		return allDone;
    }

    // Test Execution
    void Controller::RunStandardTest() {
        RunTest(SCCommand::StartStdJobCmd);
    }

    void Controller::RunCustomTest() {
        RunTest(SCCommand::StartJobCmd);
    }

    void Controller::RunTest(SCCommand startCMD){
		azure::storage::cloud_table table = tableClient.get_table_reference(IOStormPlus::storageTempTableName);
		for (auto &vm : TestVMs) {
			vm.SendCommand(table, startCMD);
		}

        bool allDone = WaitForAllVMs(table, SCCommand::JobDoneCmd, IOStormPlus::maxWaitTimeInSec, startCMD);
		if (allDone)
			Logger::LogInfo("All jobs done!");

		DownloadOutput();
        AnalyzeData(startCMD);
        PrintTestResultSummary(startCMD);

		for (auto &vm : TestVMs) {
			vm.SendCommand(table, SCCommand::EmptyCmd);
		}
    }

    // Agent management
	// TODO: Ues Azure
    void Controller::RegisterAgent(int argc, char *argv[]) {
        string name, internalIP, osType, size, pool;
        if (argc != 5) {
            PrintUsage(ControllerCommand::AgentRegister);
            return;
        }
        //TODO: Add error check
        else if (1) {
            name = argv[0];
            internalIP = argv[1];
            osType = argv[2];
            size = argv[3];
			pool = argv[4];
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
                    TestVMs.push_back(TestVM(name, internalIP, osType, size, pool));
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

	// TODO: Ues Azure
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
		InitAgents();
    }

    void Controller::TestAgent() {
        Logger::LogInfo("Using pre-sync to determine the working status of all registered test VM.");
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
            fout << "\t\t\t\t\t\"size\":\"" << TestVMs[i].GetSize() << "\",\n";
			fout << "\t\t\t\t\t\"pool\":\"" << TestVMs[i].GetPool() << "\"\n";
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
        Logger::LogInfo(logStream.str());
        Logger::LogInfo("ID\tName\tIP Address\tOS\tSize\tPool");

        logStream.clear();
        logStream.str("");
        for (int i = 0;i < TestVMs.size(); ++i) {
            logStream.clear();
            logStream.str("");
            logStream << i + 1 << "\t" << TestVMs[i].GetInfo() << endl;
            Logger::LogInfo(logStream.str());
        }
    }

    void Controller::PrintTestResultSummary(SCCommand jobCMD) {
        stringstream tempStream;
        string summaryOutputFile;
        time_t t = std::time(0);
        tm* now = std::localtime(&t);
        tempStream << (now->tm_year + 1900) << '-'
                   << (now->tm_mon + 1) << '-'
                   << now->tm_mday;
        tempStream >> summaryOutputFile;
        summaryOutputFile = summaryOutputFile + "_summary.out";
        ofstream fout(OutputFolder + summaryOutputFile, ios_base::out | ios_base::app);

        // Title
        tempStream.clear();
        tempStream.str(""); // Clear() do not clear stream buffer
        tempStream << "VM Count: " << TestVMs.size();
        Logger::LogInfo(tempStream.str());
        fout << tempStream.str() << endl;

		for (auto &iter : workload) {
			string poolName = iter.first;
			if (jobCMD == SCCommand::StartStdJobCmd && poolName != "std")
				continue;
			vector<string> jobs = iter.second;
			Logger::LogInfo("Pool: " + poolName);
			fout << "Pool: " + poolName << endl;
			for (auto job : jobs) {
				Logger::LogInfo("Job: " + job);
				fout << "Job: " + job << endl;
				Logger::LogInfo("ID\tName\tIP Address\tOS\tSize\tPool\tR(MIN)\tR(MAX)\tR(AVG)\tW(MIN)\tW(MAX)\tW(AVG)");
				fout << "ID\tName\tIP Address\tOS\tSize\tPool\tR(MIN)\tR(MAX)\tR(AVG)\tW(MIN)\tW(MAX)\tW(AVG)" << endl;
				string vm_id;
				for (int i = 0; i < TestVMs.size(); i++) {
					if (!doneJobs[TestVMs[i].GetInternalIP()]) {
						continue;
					}
					if (TestVMs[i].CountTestResult(job)) {
						tempStream.clear();
						tempStream.str("");
						tempStream << i + 1;
						tempStream >> vm_id;
						Logger::LogInfo(vm_id + "\t" + TestVMs[i].GetTestResult(job));
						fout << vm_id + "\t" + TestVMs[i].GetTestResult(job) << endl;
					}
				}
			}
		}
        fout.close();

		//Upload summary
		azure::storage::cloud_blob_container container = blobClient.get_container_reference(IOStormPlus::outputBlobContainerName);
		azure::storage::cloud_block_blob blockBlob = container.get_block_blob_reference(utility::conversions::to_string_t(summaryOutputFile));
		blockBlob.upload_from_file(utility::conversions::to_string_t(OutputFolder + summaryOutputFile));
		Logger::LogInfo("Upload result summary file to blob succeeded.");
    }

    void Controller::AnalyzeData(SCCommand jobCMD) {		
		for (auto &iter : TestVMs) {
			if (!doneJobs[iter.GetInternalIP()]) {
				continue;
			}
			if (jobCMD == SCCommand::StartStdJobCmd) {
				for (auto job : workload["std"]) {
					AnalyzeJob(job, iter);
				}
			}
			else if (jobCMD == SCCommand::StartJobCmd) {
				string vmPool = iter.GetPool();
				if (workload.count(vmPool) == 0)
					vmPool = "std";
				for (auto job : workload[vmPool]) {
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

        Logger::LogInfo("Start AnalyzeJob " + jobName + "(" + vm.GetName() + ")");
        const string outputFile = OutputFolder + vm.GetName() + "_" + jobName + ".out";

        vm.SetTestResult(job, AnalyzeStandardOutput(outputFile));
    }

    ReportSummary Controller::AnalyzeStandardOutput(string outputFile) {
        Logger::LogInfo("Start AnalyzeStandardOutput " + outputFile);

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

        Logger::LogInfo("End AnalyzeStandardOutput " + outputFile);

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
        // Logger::LogInfo("IOPS number done");
        return (int)num;
    }

}

int main(int argc,char *argv[]) {
    IOStormPlus::Controller controller(IOStormPlus::AgentsConfigFilename, IOStormPlus::storageConfigFileName);
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



