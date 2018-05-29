#pragma once
#include "TestVM.h"
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>

using namespace std;

#ifdef __cplusplus
extern "C"
{
#endif

namespace IOStormPlus{

    class Controller{
    public:
        vector<TestVM> TestVMs;

    public:
        Controller(string TestVMs_config);
        ~Controller(){};

        inline bool IsReady();
        void PrintUsage();
        void start_worker(int argc, char *argv[]);
        void agent_worker(int argc, char *argv[]);
        void initialize();
        bool IsReady();

    private:
        bool m_isReady;
        void InitLogger();
        void PrintTestVMInfo();
        void PrintTestResultSummary(string workloadRootPath);
        void pre_sync();
        void standard_worker();
        void custom_worker();
    
        void analyze_data(string workloadRootPath);
        ReportSummary analyze_standard_output(string output_file);

        inline void write_log(string msg, bool flag_stdout);
        inline void write_config();
        //////////////////////// USAGE ////////////////////////

        void start_worker_PrintUsage();

        void agent_worker_PrintUsage();
        void agent_worker_register(int argc, char *argv[]);
        void agent_worker_register_PrintUsage();
        void agent_worker_remove(int argc, char *argv[]);
        void agent_worker_remove_PrintUsage();
        void agent_worker_show();
        void agent_worker_test();

    };

}

#ifdef __cplusplus
}
#endif