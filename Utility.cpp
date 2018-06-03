#include <windows.h>
#include <string>
#include "Utility.h"
#include "LogicalDriveInfo.h"

using namespace std;

char* StringToCharP(string str)
{
	int length = str.length();
	char* arr = new char[length - sizeof(arr) / sizeof(char)];

	for (int i = 0; i < length; i++)
	{
		arr[i] = str[i];
	}

	return arr;
}

bool isRootFolder(LogicalDriveInfo* drivesInfo, string path)
{
	int i = 0;
	while (!drivesInfo[i - 1].lastDrive)
	{
		if (path == drivesInfo[i].drivePath) return true;
		i++;
	}
	return false;
}

string EraseLastDirectory(string currentDirectory)
{
	int length = currentDirectory.length();
	currentDirectory.erase(length - 1, length); //erase '\\' symbol
	currentDirectory.erase(currentDirectory.find_last_of("\\") + 1, length);
	return currentDirectory;
}

bool isPreRootFolder(LogicalDriveInfo* drivesInfo, string path)
{
	string preRootPath = path;
	preRootPath = EraseLastDirectory(preRootPath);
	if (isRootFolder(drivesInfo, preRootPath)) return true;
	return false;
}

bool isDirectory(string fullPath)
{
	DWORD dwAttribute = GetFileAttributes(fullPath.c_str());

	if ((FILE_ATTRIBUTE_DIRECTORY & dwAttribute) == FILE_ATTRIBUTE_DIRECTORY && 
		(FILE_ATTRIBUTE_DEVICE & dwAttribute) != FILE_ATTRIBUTE_DEVICE) return true;
	if (dwAttribute == INVALID_FILE_ATTRIBUTES) return false;
	return false;
}