#include "LogicalDriveInfo.h"
#include <windows.h>
#include <string>
#include "Utility.h"

using namespace std;


char* LogicalDriveInfo::GetLogicalDrivesName()
{

	DWORD dr = GetLogicalDrives();
	int curBit;
	string letters;

	for (int x = 0; x < 26; x++) // проходимся циклом по битам
	{
		curBit = ((dr >> x) & 1); // узнаём значение текущего бита
		if (curBit) // если единица - диск с номером x есть
		{
			char letter = 65 + x;
			letters.push_back(letter);
		}
	}

	char* names = StringToCharP(letters);
	names[letters.length()] = '\0';

	return names;
}

void  LogicalDriveInfo::SetLogicalDriveType(LogicalDriveInfo* info)
{
	info->driveType = GetDriveType(info->drivePath); // узнаём тип диска
}

bool  LogicalDriveInfo::isDriveReady(LogicalDriveInfo* info)
{
	WORD OldErrorMode;
	OldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS); // убираем показ ошибок
	bool ready = isDirectory(info->drivePath); // пытаемcя открыть корневую директорию
	SetErrorMode(OldErrorMode); // восстанавливаем старый режим показа ошибок

	return ready;
}

void  LogicalDriveInfo::SetLogicalDriveSpace(LogicalDriveInfo* info)
{
	bool success;
	success = ::GetDiskFreeSpaceEx(info->drivePath,
		(PULARGE_INTEGER)&info->freeBytesAvailable,
		(PULARGE_INTEGER)&info->totalNumberOfBytes,
		(PULARGE_INTEGER)&info->totalNumberOfFreeBytes
	);

	if (!success)
	{
		info->freeBytesAvailable = NULL;
		info->totalNumberOfBytes = NULL;
		info->totalNumberOfFreeBytes = NULL;
	}
}

void  LogicalDriveInfo::SetLogicalDriveVolumeInfo(LogicalDriveInfo* info)
{
	info->driveNameSize = sizeof(info->driveLabel);
	info->driveLabel[0] = '\0';
	info->driveFat[0] = '\0';
	bool success;
	success = GetVolumeInformation(info->drivePath,
		info->driveLabel,
		sizeof(info->driveLabel),
		&info->driveSerialNumber,
		&info->driveNameSize,
		NULL,
		info->driveFat,
		sizeof(info->driveFat)
	);

	if (!success)
	{
		
	}
}

LogicalDriveInfo* LogicalDriveInfo::SetLogicalDrivesInfo()
{
	char* names = GetLogicalDrivesName();

	LogicalDriveInfo* info = new LogicalDriveInfo[strlen(names)];

	for (int i = 0; i < strlen(names); i++)
	{
		info[i].drivePath = new char[4];
		info[i].drivePath[0] = names[i];
		info[i].drivePath[1] = ':';
		info[i].drivePath[2] = '\\';
		info[i].drivePath[3] = '\0';

		SetLogicalDriveType(&info[i]);

		if (isDriveReady(&info[i]))
		{
			SetLogicalDriveSpace(&info[i]);

			SetLogicalDriveVolumeInfo(&info[i]);
		}
	}
	info[strlen(names) - 1].lastDrive = true;

	return info;
}

char* LogicalDriveInfo::GetLogicalDriveType(LogicalDriveInfo * info)
{
	if (info->driveType == DRIVE_REMOVABLE)		 return "REMOVABLE";
	else if (info->driveType == DRIVE_FIXED)     return "FIXED";
	else if (info->driveType == DRIVE_REMOTE)    return "REMOTE";
	else if (info->driveType == DRIVE_CDROM)     return "CD-ROM";
	else if (info->driveType == DRIVE_RAMDISK)   return "RAMDISK";
	else										 return "UNKNOWN";
}

void LogicalDriveInfo::GetLogicalDriveFreeSpace(LogicalDriveInfo * info, int sizeType, char* source)
{
	unsigned int sizeTypeDivisior = pow(1024, sizeType);
	double pointingSize = info->totalNumberOfFreeBytes / sizeTypeDivisior;
	unsigned long long normalSize = floor(pointingSize);
	string tmp = to_string(normalSize);
	
	strcpy_s(source, tmp.length() + 1, tmp.c_str());
}

void LogicalDriveInfo::GetLogicalDriveAllSpace(LogicalDriveInfo * info, int sizeType, char* source)
{	
	unsigned int sizeTypeDivisior = pow(1024, sizeType);
	double pointingSize = info->totalNumberOfBytes / sizeTypeDivisior;
	unsigned long long normalSize = floor(pointingSize);
	string tmp = to_string(normalSize);

	strcpy_s(source, tmp.length() + 1, tmp.c_str());
}
