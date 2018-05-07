#pragma once
#include <bits/stdc++.h>

using namespace std;


struct report_summary{
    vector<int> read_iops;
    vector<int> write_iops;
    report_summary() {
        read_iops.clear();
        write_iops.clear();
    }
};

struct vm {
    string vm_name;
    string internal_ip;
    string vm_size;
    enum os_type{
        linux,
        windows
    } vm_os;
    map<string, report_summary> test_result;

    vm(string _vm_name, string _ip, os_type _vm_os, string _vm_size):
        vm_name(_vm_name), internal_ip(_ip), vm_os(_vm_os), vm_size(_vm_size){}

    vm(string _vm_name, string _ip, string _vm_os, string _vm_size):
        vm_name(_vm_name), internal_ip(_ip), vm_os(_vm_os == "linux" ? linux : windows), vm_size(_vm_size){}

    inline string get_share_path() {
        return "\\\\" + internal_ip + "\\";
    }

    inline string get_vm_os_name() {
        return vm_os == linux ? "Linux" : "Windows";
    }

    inline string get_vm_info() {
        stringstream temp_stream;
        temp_stream.clear();
        temp_stream << vm_name << "\t" << internal_ip << "\t" << (get_vm_os_name() + "\t" + vm_size);
        string res;
        getline(temp_stream,res);
        return res;
    }

    inline string get_vm_result(string jobna) {
        int read_min_iops = 1 << 30;
        int read_max_iops = 0;
        int read_avg_iops = 0;
        int write_min_iops = 1 << 30;;
        int write_max_iops = 0;
        int write_avg_iops = 0;
        for (auto res_itr : test_result[jobna].read_iops) {
            read_min_iops = min(read_min_iops, res_itr);
            read_max_iops = max(read_max_iops, res_itr);
            read_avg_iops += res_itr;
        }
        if (test_result[jobna].read_iops.size() != 0)
            read_avg_iops /= test_result[jobna].read_iops.size();
        for (auto res_itr : test_result[jobna].write_iops) {
            write_min_iops = min(write_min_iops, res_itr);
            write_max_iops = max(write_max_iops, res_itr);
            write_avg_iops += res_itr;
        }
        if (test_result[jobna].write_iops.size() != 0)
            write_avg_iops /= test_result[jobna].write_iops.size();
        stringstream temp_stream;
        temp_stream.clear();
        temp_stream << vm_name << "\t" << internal_ip << "\t" << (get_vm_os_name() + "\t" + vm_size) << "\t" << read_min_iops << "\t" << read_max_iops << "\t" << read_avg_iops << "\t" << write_min_iops << "\t" << write_max_iops << "\t" << write_avg_iops;
        string res;
        getline(temp_stream,res);
        return res;
    }

};


class controller{
public:
    vector<vm> test_vm;
    bool flag_err;
    inline void initialize_logger();

public:
    controller(string test_vm_config);
    ~controller(){};

    inline bool has_error();
    inline void print_test_vm_info();
    inline void print_test_result_summary(string workload_path);
    void pre_sync();
    void standard_worker();
    void custom_worker();

    void analyze_data(string workload_path);
    report_summary analyze_standard_output(string output_file);

    inline void write_log(string msg, bool flag_stdout);
    inline void write_config();
    //////////////////////// USAGE ////////////////////////
    void print_usage();

    void start_worker(int argc, char *argv[]);
    void start_worker_print_usage();

    void agent_worker(int argc, char *argv[]);
    void agent_worker_print_usage();
    void agent_worker_register(int argc, char *argv[]);
    void agent_worker_register_print_usage();
    void agent_worker_remove(int argc, char *argv[]);
    void agent_worker_remove_print_usage();
    void agent_worker_show();
    void agent_worker_test();

};
