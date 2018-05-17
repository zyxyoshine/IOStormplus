#pragma once
#include <iostream>
#include <ctime>
#include <fstream>
#include "controller.h"
using namespace std;

enum typelog {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
};

struct structlog {
    bool headers = false;
    bool timestamp = true;
    typelog level = LOG_WARN;
    ofstream fout;
    void init(string _file_name) {
        fout.close();
        fout.open(_file_name, ios_base::out | ios_base::trunc);
        if (fout.fail())
            cerr << "Open " + _file_name + " for log failed!" << endl;
    }
};

extern structlog LOGCFG;

class LOG {
public:
    LOG() {}
    LOG(typelog type) {
        msglevel = type;
        if(LOGCFG.timestamp) {
            time_t t = std::time(0);   // get time now
            tm* now = std::localtime(&t);
            operator << ('[')
                     << (now->tm_year + 1900) << '-'
                     << (now->tm_mon + 1) << '-'
                     << now->tm_mday
                     << ' '
                     << now->tm_hour << ':'
                     << now->tm_min << ':'
                     << now->tm_sec << ']';
        }
        if(LOGCFG.headers) {
            operator << ('[' + getLabel(type) + ']');
        }
    }
    ~LOG() {
        if(opened) {
            LOGCFG.fout << endl;
        }
        opened = false;
    }
    template<class T>
    LOG &operator <<(const T &msg) {
        if(msglevel >= LOGCFG.level) {
            LOGCFG.fout << msg;
            opened = true;
        }
        return *this;
    }
private:
    bool opened = false;
    typelog msglevel = LOG_DEBUG;
    inline string getLabel(typelog type) {
        string label;
        switch(type) {
            case LOG_DEBUG: label = "DEBUG"; break;
            case LOG_INFO:  label = "INFO "; break;
            case LOG_WARN:  label = "WARN "; break;
            case LOG_ERROR: label = "ERROR"; break;
        }
        return label;
    }
};

