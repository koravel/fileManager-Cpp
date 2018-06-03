#pragma once
#include <string>
#include "LogicalDriveInfo.h"

using namespace std;

char* StringToCharP(string str);

//bool DirectoryExists(const string& dirName_in);

bool isRootFolder(LogicalDriveInfo* drivesInfo, string path);

string EraseLastDirectory(string currentDirectory);

bool isPreRootFolder(LogicalDriveInfo * drivesInfo, string path);

bool isDirectory(string fullPath);
