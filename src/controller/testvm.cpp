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

	string TestVM::GetPool() {
		return m_pool;
	}

    OSType TestVM::GetOSType(){
        return m_osType;
    }

    string TestVM::GetSharePath(){
        return "\\\\" + m_internalIP + "\\";
    }

    string TestVM::GetInfo() {
        stringstream tempStream;
        tempStream << GetName() << "\t" << GetInternalIP() << "\t" << GetOSTypeName() + "\t" + GetSize() + "\t" + GetPool();
        return tempStream.str();
    }

    string TestVM::GetTestResult(const string& jobName) {
        stringstream tempStream;
		tempStream << GetName();
		vector<double> outputData;
		for (int i = 0; i < 2; i++)
			outputData.push_back(m_testResults[jobName].IOPS[i]);
		for (int i = 0; i < 2; i++)
			outputData.push_back(m_testResults[jobName].bandWidth[i]);
		for (int i = 0; i < 2; i++)
			for (int j = 0;j < 3;j++)
				outputData.push_back(m_testResults[jobName].cLat[i][j]);
		for (auto data : outputData) {
			if (data == 0)
				tempStream << "\tN/A";
			else {
				if (data > 1000 * 1000)
					tempStream << "\t" << data / (1000 * 1000) << "M";
				else if (data > 1000)
					tempStream << "\t" << data / 1000  << "k";
				else
					tempStream << "\t" << data;
			}
		}
        return tempStream.str();
    }

    void TestVM::SetTestResult(const string& jobName, const ReportSummary& report) {
        m_testResults[jobName] = report;
    }

    int TestVM::CountTestResult(const string& jobName) {
        return m_testResults.count(jobName);
    }

    int TestVM::GetResponse(azure::storage::cloud_table& table, SCCommand command, SCCommand retryCMD){
		azure::storage::table_operation retrieveOperation = azure::storage::table_operation::retrieve_entity(utility::conversions::to_string_t(m_pool), utility::conversions::to_string_t(m_name));
		azure::storage::table_result retrieveResult = table.execute(retrieveOperation);
		azure::storage::table_entity agentEntity = retrieveResult.entity();
		const azure::storage::table_entity::properties_type& properties = agentEntity.properties();

		string cmdString = utility::conversions::to_utf8string(properties.at(IOStormPlus::tableCommandColumnName).string_value());
        if(cmdString.compare(GetCommandString(command)) == 0)
			return 1; // done

		string errString = utility::conversions::to_utf8string(properties.at(IOStormPlus::tableErrorColumnName).string_value());
		if (errString != emptyErrorMessage) {
			Logger::LogWarning("Test VM " + m_name + "(" + m_internalIP + ") encounters error: " + errString);
			Reinit(table);
			return -1; //will ignore the vm for this run.
		}
		if ((retryCMD != SCCommand::InvaildCmd) && (cmdString.compare(GetCommandString(SCCommand::EmptyCmd)) == 0)) {
			Logger::LogInfo("Resending command " + GetCommandString(retryCMD) + " to " + m_name);
			SendCommand(table, retryCMD);
		}
        return 0;
    }

	void TestVM::Reinit(azure::storage::cloud_table& table) {
		SendCommand(table, SCCommand::EmptyCmd);
		azure::storage::table_entity agent(utility::conversions::to_string_t(m_pool), utility::conversions::to_string_t(m_name));
		azure::storage::table_entity::properties_type& properties = agent.properties();
		properties[tableCommandColumnName] = azure::storage::entity_property(utility::conversions::to_string_t(GetCommandString(SCCommand::EmptyCmd)));
		properties[tableErrorColumnName] = azure::storage::entity_property(utility::conversions::to_string_t(emptyErrorMessage));
		azure::storage::table_operation opt = azure::storage::table_operation::insert_or_merge_entity(agent);
		azure::storage::table_result insert_result = table.execute(opt);
	}

	void TestVM::SendCommand(azure::storage::cloud_table& table, SCCommand command) {
		azure::storage::table_entity agent(utility::conversions::to_string_t(m_pool), utility::conversions::to_string_t(m_name));
		azure::storage::table_entity::properties_type& properties = agent.properties();
		properties[tableCommandColumnName] = azure::storage::entity_property(utility::conversions::to_string_t(GetCommandString(command)));
		azure::storage::table_operation opt = azure::storage::table_operation::insert_or_merge_entity(agent);
		azure::storage::table_result insert_result = table.execute(opt);
	}

    ////////////////////////////////////////////////////////////////////////////////////////////
    // Private function
    ///////////////////////////////////////////////////////////////////////////////////////////

    string TestVM::GetOSTypeName() {
        return m_osType == Linux ? "linux" : "windows";
    }


}
