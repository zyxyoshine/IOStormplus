#pragma once

#include <string>
using namespace std;

namespace IOStormPlus{
	enum SCCommand {
		InvaildCmd = 0,
		EmptyCmd = 1,
		SyncCmd = 10,
		SyncDoneCmd = 11,
		StartJobCmd = 12,
		StartStdJobCmd = 13,
        JobDoneCmd = 14
    };

    inline string GetCommandString(SCCommand command){
        switch(command){
			case EmptyCmd: return "#";
            case SyncCmd: return "PRESYNC";
            case SyncDoneCmd: return "SYNCDONE";
            case StartJobCmd: return "START";
			case StartStdJobCmd: return "STARTSTD";
            case JobDoneCmd: return "DONE";
        }
        return "";
    };

    inline SCCommand GetCommondFromString(string cmdString){
		if (cmdString.compare("#") == 0) return EmptyCmd;
        if (cmdString.compare("PRESYNC") == 0) return SyncCmd;
        if (cmdString.compare("SYNCDONE") == 0) return SyncDoneCmd;
        if (cmdString.compare("START") == 0) return StartJobCmd;
		if (cmdString.compare("STARTSTD") == 0) return StartStdJobCmd;
        if (cmdString.compare("DONE") == 0) return JobDoneCmd;

        return InvaildCmd;
    }
}

