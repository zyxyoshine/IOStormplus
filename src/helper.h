#pragma once
#include <bits/stdc++.h>
#include <stdexcept>
#include <windows.h>

using namespace std;


string exec_command(const char* cmd) {
	char buffer[128];
	string result = "";
		FILE* pipe = _popen(cmd, "r");
	if (!pipe)
		throw runtime_error("popen() failed!");
	try {
		while (!feof(pipe)) {
			if (fgets(buffer, 128, pipe) != NULL)
				result += buffer;
		}
	} catch (...) {
		_pclose(pipe);
		throw;
	}
	_pclose(pipe);
	return result;
}

vector<string> list_files_in_directory(string dirna) {
    WIN32_FIND_DATA data;
    HANDLE hFind = FindFirstFile((dirna + "*").c_str(), &data);      // DIRECTORY
    vector<string> res;
    if ( hFind != INVALID_HANDLE_VALUE ) {
        do {
            string file_name = data.cFileName;
            if (file_name != "." && file_name != "..")
                res.push_back(file_name);
        } while (FindNextFile(hFind, &data));
        FindClose(hFind);
    }
    return res;
}
///////////////////////////////////////////////////

const string agents_config_file = "agents.json";
const string workload = "workload\\";
const string tempfolder = "temp\\";
const string output = "output\\";
