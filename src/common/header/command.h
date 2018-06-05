#pragma once

#include <string>
using namespace std;

namespace IOStormPlus{
    enum SCCommand{
        InvaildCmd = 0,
        SyncCmd = 10,
        SyncDoneCmd = 11,
        StartJobCmd = 12,
        JobDoneCmd = 13
    };

    inline string GetCommandString(SCCommand command){
        switch(command){
            case SyncCmd: return "PRESYNC";
            case SyncDoneCmd: return "SYNCDONE";
            case StartJobCmd: return "START";
            case JobDoneCmd: return "DONE";
        }
        return "";
    };

    inline SCCommand GetCommondFromString(string cmdString){
        if(cmdString.compare("PRESYNC") == 0) return SCCommand::SyncCmd;
        if(cmdString.compare("SYNCDONE") == 0) return SCCommand::SyncDoneCmd;
        if(cmdString.compare("START") == 0) return SCCommand::StartJobCmd;
        if(cmdString.compare("DONE") == 0) return SCCommand::JobDoneCmd;

        return SCCommand::InvaildCmd;
    }
}

