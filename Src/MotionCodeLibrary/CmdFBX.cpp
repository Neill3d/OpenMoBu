
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: CmdFBX.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.h>
#include <tchar.h>
#include <string>

#include "CmdFBX.h"
#include "Logger.h"

// DONE: run cmdFBX application and fill data into the shared memory block

//#ifdef CMD_SEND_CODE
#include "FileUtils.h"

namespace fs = std::filesystem;

namespace
{
	std::wstring QuoteWindowsArgument(std::wstring_view value)
	{
		std::wstring result;
		result.push_back(L'"');

		size_t backslashCount = 0;

		for (const wchar_t character : value)
		{
			if (character == L'\\')
			{
				++backslashCount;
				continue;
			}

			if (character == L'"')
			{
				result.append(backslashCount * 2 + 1, L'\\');
				result.push_back(L'"');
				backslashCount = 0;
				continue;
			}

			result.append(backslashCount, L'\\');
			backslashCount = 0;
			result.push_back(character);
		}

		// Backslashes preceding the closing quote must be doubled.
		result.append(backslashCount * 2, L'\\');
		result.push_back(L'"');

		return result;
	}
}

bool CmdMakeSnapshotFBX_Send(const char *filename, const char *uniqueName, InputModelData &data, const bool ResetXForm)
{
	LPCSTR	szMemoryName = _T("Local\\cmdfbxmem");
	LPCSTR	szEventName = _T("Local\\cmdfbxevent");

	HANDLE hMem, hEvent;
	void *memory = nullptr;

	bool result = true;

	hEvent = CreateEvent(NULL, FALSE, FALSE, szEventName);

	size_t size = data.ComputeTotalSize();

	if ((hMem = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, szMemoryName)) == NULL) {
		// cannot open shared memory, so we are the first process: create it
		hMem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, szMemoryName);
		if (hMem)
		{
			memory = MapViewOfFile(hMem, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
		}
	}
	else
	{
		memory = MapViewOfFile(hMem, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	}

	if (memory)
	{
		data.CopyToMemory(memory);
		// let the other process know we've written something
		if (hEvent)
		{
			SetEvent(hEvent);
		}
	}


	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

	try
	{
		const fs::path inputFilename(AnsiToWide(filename));
		const std::wstring wideUniqueName = AnsiToWide(uniqueName);

		const auto executable = FindEffectLocation(fs::path(L"cmdFBX.exe"));

		if (!executable)
		{
			throw std::runtime_error("Failed to find cmdFBX.exe");
		}

		std::wstring commandLine = QuoteWindowsArgument(executable->native());

		commandLine += L' ';
		commandLine += QuoteWindowsArgument(inputFilename.native());

		commandLine += L' ';
		commandLine += QuoteWindowsArgument(wideUniqueName);

		// CreateProcessW is allowed to modify its command-line buffer.
		std::vector<wchar_t> mutableCommandLine(
			commandLine.begin(),
			commandLine.end());

		mutableCommandLine.push_back(L'\0');

		STARTUPINFOW startupInfo{};
		startupInfo.cb = sizeof(startupInfo);

		PROCESS_INFORMATION processInfo{};

		const fs::path workingDirectory = executable->parent_path();

		if (!CreateProcessW(
			executable->c_str(),          // Exact executable
			mutableCommandLine.data(),    // Mutable command line
			nullptr,
			nullptr,
			FALSE,
			0,
			nullptr,
			workingDirectory.c_str(),
			&startupInfo,
			&processInfo))
		{
			const DWORD error = GetLastError();

			throw std::system_error(
				static_cast<int>(error),
				std::system_category(),
				"CreateProcessW failed");
		}

		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);
	}
	catch (const char *msg)
	{
		LOGE( "%s (error - %d)\n", msg, GetLastError() );
		result = false;
	}

	if (hMem) CloseHandle( hMem );
	if (hEvent) CloseHandle( hEvent );

	return result;
}

//#endif

bool CmdMakeSnapshotFBX_Receive(InputModelData *data)
{

	LPCSTR	szMemoryName = _T("Local\\cmdfbxmem");
	LPCSTR	szEventName = _T("Local\\cmdfbxevent");

	HANDLE hMem, hEvent;
	void *memory = nullptr;

	hEvent = CreateEvent(NULL, FALSE, FALSE, szEventName);

	if ((hMem = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, szMemoryName)) != NULL) {
	
		memory = MapViewOfFile(hMem, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);

		if(hEvent && WaitForSingleObject(hEvent, 5000) == WAIT_OBJECT_0 ) {
			
			data->InitFromMemory( memory );
			
			return true;
		}
	}

	return false;
}