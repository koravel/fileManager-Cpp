#pragma once
#include <windows.h>
#include <string>

struct LogicalDriveInfo
{
public:
	UINT driveType = DRIVE_UNKNOWN;

	unsigned __int64 freeBytesAvailable = 0;
	unsigned __int64 totalNumberOfBytes = 0;
	unsigned __int64 totalNumberOfFreeBytes = 0;

	char* drivePath;
	char driveLabel[30] = { '\0' };
	char driveFat[30] = { '\0' };
	DWORD driveSerialNumber = NULL;
	DWORD driveNameSize = NULL;

	bool driveReady;
	bool lastDrive = false;

	static char* GetLogicalDrivesName();

	static void SetLogicalDriveType(LogicalDriveInfo* info);

	static bool isDriveReady(LogicalDriveInfo* info);

	static void SetLogicalDriveSpace(LogicalDriveInfo* info);

	static void SetLogicalDriveVolumeInfo(LogicalDriveInfo* info);

	static LogicalDriveInfo* SetLogicalDrivesInfo();

	static char* GetLogicalDriveType(LogicalDriveInfo* info);

	static void GetLogicalDriveFreeSpace(LogicalDriveInfo* info, int sizeType, char* source);

	static void GetLogicalDriveAllSpace(LogicalDriveInfo* info, int sizeType, char* source);

};

enum Size
{
	_byte = 0,
	_Kb = 1,
	_Mb = 2,
	_Gb = 3,
	_Tb = 4,
};