#pragma once
#include <stdexcept>
#include <windows.h>
#include <vector>
#include <sstream>

using namespace std;

namespace IOStormPlus{
	string ExecCommand(const string cmd);
	vector<string> ListFilesInDirectory(string dirna);
}

