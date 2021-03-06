#pragma once

#include <fstream>
#include <string>
using namespace std;

#ifdef __cplusplus
extern "C"
{
#endif

namespace IOStormPlus{

    enum LogLevel {
        Verbose = 0,
        Info = 1,
        Warning = 2,
        Error = 3
    };

    class Logger {
    
    public:
        /// Default logger will output both logfile and cout, log all
        static void Init(string filename, LogLevel level = Verbose, bool needConsoleOutput = true);
        static void SetLogLevel(LogLevel level);
        static LogLevel GetLogLevel();
        static void LogError(string message);
        static void LogWarning(string message);
        static void LogInfo(string message);
        static void LogVerbose(string message);
        static void Log(LogLevel level, string message);

    private:
        static LogLevel s_level;
        static ofstream s_logfileStream;
        static bool s_consoleOutput;
        static string GetLabel(LogLevel level);
    };

}

#ifdef __cplusplus
}
#endif
