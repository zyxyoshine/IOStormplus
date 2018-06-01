#include "defaultagent.h"
#include "..\common\logger.h"

namespace IOStormPlus{
    void DefaultAgent::InitLogger(){
		Logger::Init(LogFilePath);
    }
    
	// void ExecuteCommand(string command);
    // vector<string> ListFilesInDirectory(string rootPath);
    void DefaultAgent::Sync(){
		ifstream fin;
			
		fin.open(ControllerTempFilePath, ios_base::in);
		while (fin.fail()) {
			Sleep(SyncWaitTime);
			cerr << strerror(errno) << endl;
			fin.open(ControllerTempFilePath , ios_base::in);
		}
			
		string buf;
		Logger::LogInfo("waiting for controller requests");
		while(true) {
			fin.close();
			Sleep(SyncWaitTime);
			fin.open(ControllerTempFilePath, ios_base::in);
			fin >> buf;
			if (buf == "PRESYNC") {
				Logger::LogInfo("pre-sync succeeded");
				break;
			}
		}
		fin.close();

		ofstream fout(ClientTempFilePath, ios_base::out | ios_base::trunc);
		fout << "SYNCDONE";
		fout.flush();
		fout.close();
            
    }
}