#include "logger.h"
#include <cassert>
#include <iostream>
#include <ctime>

// #ifdef __cplusplus
// extern "C"
// {
// #endif

namespace IOStormPlus{
    
    LogLevel Logger::s_level;
    ofstream Logger::s_logfileStream;
    bool Logger::s_consoleOutput;

    void Logger::Init(string filename, LogLevel level, bool needConsoleOutput) {
        Cleanup();
        s_level = level;
        s_consoleOutput = needConsoleOutput;
        s_logfileStream.open(filename, ios_base::out | ios_base::trunc);
        if (s_logfileStream.fail()){
            cerr << "Open " + filename + " for Logger failed!" << endl;
        }     
    }

    void Logger::Cleanup(){
        if (s_logfileStream.is_open()){
            s_logfileStream.close();
        }
    }

    void Logger::SetLogLevel(LogLevel level){
        s_level = level;
    }
    
    LogLevel Logger::GetLogLevel(){
        return s_level;
    }
    
    void Logger::LogError(string message){
        Logger::Log(LogLevel::Error, message);
    }
    
    void Logger::LogWarning(string message){
        Logger::Log(LogLevel::Warning, message);
    }
    
    void Logger::LogInfo(string message){
        Logger::Log(LogLevel::Info, message);
    }

    void Logger::LogVerbose(string message){
        Logger::Log(LogLevel::Verbose, message);
    }

    void Logger::Log(LogLevel level, string message){
        // skip log lower level message
        if (level < s_level) return;

        time_t t = std::time(0);   // get time now
        tm* now = std::localtime(&t);

        if (s_consoleOutput){
            cout << ('[')
                            << (now->tm_year + 1900) << '-'
                            << (now->tm_mon + 1) << '-'
                            << now->tm_mday
                            << ' '
                            << now->tm_hour << ':'
                            << now->tm_min << ':'
                            << now->tm_sec << ']';
            cout << ('[' + GetLabel(level) + ']');
        }
        
        assert(s_logfileStream.is_open());
        s_logfileStream << ('[')
                        << (now->tm_year + 1900) << '-'
                        << (now->tm_mon + 1) << '-'
                        << now->tm_mday
                        << ' '
                        << now->tm_hour << ':'
                        << now->tm_min << ':'
                        << now->tm_sec << ']';
        s_logfileStream << ('[' + GetLabel(level) + ']');
        s_logfileStream.flush();        
    }

    string Logger::GetLabel(LogLevel level) {
        switch(level) {
            case Verbose: return "Verbose:";
            case Info:  return "Info:";
            case Warning:  return "Warning:";
            case Error: return "Error:";
        }
        assert(true);
    }

}

// #ifdef __cplusplus
// }
// #endif