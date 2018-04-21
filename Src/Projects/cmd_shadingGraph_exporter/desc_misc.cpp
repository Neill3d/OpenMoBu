
// desc_misc.cpp
// Sergei <Neill3d> Solokhin 2018

#include "desc.h"

#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>

////////////////////////////////////////////////////////////////////////////////////////////
// definitions

// lpszString - pointer to buffer to receive string

int FileDate(const char* name, __int64 &fileDate)
{
	WIN32_FILE_ATTRIBUTE_DATA fad;
	if (!GetFileAttributesEx(name, GetFileExInfoStandard, &fad))
		return -1; // error condition, could call GetLastError to find out more
	LARGE_INTEGER date;

	date.HighPart = fad.ftLastWriteTime.dwHighDateTime;
	date.LowPart = fad.ftLastWriteTime.dwLowDateTime;

	fileDate = date.QuadPart;

	return 1;
}

// return date in UTC +0

BOOL FileSizeAndDate(const char* name, __int64 &fileSize, LPTSTR lpszString, DWORD dwSize)
{
	WIN32_FILE_ATTRIBUTE_DATA fad;
	if (!GetFileAttributesEx(name, GetFileExInfoStandard, &fad))
		return -1; // error condition, could call GetLastError to find out more
	LARGE_INTEGER size;

	size.HighPart = fad.nFileSizeHigh;
	size.LowPart = fad.nFileSizeLow;

	FILETIME	ftWrite;
	SYSTEMTIME	stUTC, stLocal;
	DWORD dwRet;

	ftWrite = fad.ftLastWriteTime;
	FileTimeToSystemTime(&ftWrite, &stUTC);

	TIME_ZONE_INFORMATION timeZone;
	memset(&timeZone, 0, sizeof(timeZone));
	SystemTimeToTzSpecificLocalTime(&timeZone, &stUTC, &stLocal);


	// Build a string with last-write time
	dwRet = StringCchPrintf(lpszString, dwSize,
		TEXT("%02d/%02d/%d  %02d:%02d:%02d"),
		stLocal.wDay, stLocal.wMonth, stLocal.wYear,
		stLocal.wHour, stLocal.wMinute, stLocal.wSecond);

	fileSize = size.QuadPart;

	if (S_OK == dwRet)
		return TRUE;
	else return FALSE;
}

bool FileExists(LPCTSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
