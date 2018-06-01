#include "helper.h"


using namespace std;

namespace IOStormPlus{

	string ExecCommand(const char* cmd) {
		char buffer[128];
		string result = "";
		FILE* pipe = _popen(cmd, "r");
		if (!pipe){
			throw runtime_error("popen() failed!");
        }
		try {
			while (!feof(pipe)) {
				if (fgets(buffer, 128, pipe) != NULL)
					result += buffer;
			}
		} 
        catch (...) {
			_pclose(pipe);
			throw;
		}
		_pclose(pipe);
		return result;
	}

	vector<string> ListFilesInDirectory(string root) {
		WIN32_FIND_DATA data;
		HANDLE hFind = FindFirstFile((root + "*").c_str(), &data);   
		vector<string> res;
		if ( hFind != INVALID_HANDLE_VALUE ) {
			do {
				string file_name = data.cFileName;
				if (file_name != "." && file_name != "..") {
					res.push_back(file_name);
                }
			} 
            while (FindNextFile(hFind, &data));
			FindClose(hFind);
		}
		return res;
	}

}
