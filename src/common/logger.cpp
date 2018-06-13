#include "header\logger.h"
#include <cassert>
#include <iostream>
#include <ctime>

namespace IOStormPlus{
    
    ////////////////////////////////////////////////////////////////////////////////////////////
    // Public function
    ///////////////////////////////////////////////////////////////////////////////////////////

    void Logger::Init(string filename, LogLevel level, bool needConsoleOutput) {
        cout << "call log init";
        if (s_logfileStream.is_open()){
            s_logfileStream.close();
        }
        s_level = level;
        s_consoleOutput = needConsoleOutput;
        cout << filename << endl;
        s_logfileStream.open(filename, ios_base::out | ios_base::trunc);
        if (s_logfileStream.fail()){
            cerr << "Open " + filename + " for Logger failed!" << endl;
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

        // get time now
        time_t t = std::time(0);  
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
            cout << message << endl;
        }
        
        // Skip log to file if file is not ready
        //assert(s_logfileStream.is_open());
        if(!s_logfileStream.is_open()) return;

        s_logfileStream << ('[')
                        << (now->tm_year + 1900) << '-'
                        << (now->tm_mon + 1) << '-'
                        << now->tm_mday
                        << ' '
                        << now->tm_hour << ':'
                        << now->tm_min << ':'
                        << now->tm_sec << ']';
        s_logfileStream << ('[' + GetLabel(level) + ']');
        s_logfileStream << message << endl;
        s_logfileStream.flush();        
    }

    ////////////////////////////////////////////////////////////////////////////////////////////
    // Private function
    ///////////////////////////////////////////////////////////////////////////////////////////
    LogLevel Logger::s_level;
    ofstream Logger::s_logfileStream;
    bool Logger::s_consoleOutput;

    string Logger::GetLabel(LogLevel level) {
        switch(level) {
            case Verbose: return "VERBOSE";
            case Info:  return "INFO";
            case Warning:  return "WARNING";
            case Error: return "ERROR";
        }
        assert(true);
        return "UNKNOWN";
    }

}
