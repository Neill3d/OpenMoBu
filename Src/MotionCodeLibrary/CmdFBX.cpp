
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

#include "CmdFBX.h"
#include "Logger.h"

// DONE: run cmdFBX application and fill data into the shared memory block

//#ifdef CMD_SEND_CODE
#include "FileUtils.h"

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

		FBString out_path, out_fullpath;
		if ( FindEffectLocation( "\\cmdFBX.exe", out_path, out_fullpath ) == false)
			throw "failed to find cmdFBX";

		// filename is a path for saving temp fbx !!

		//if( IsFileExists(filename) == false )
		//	throw " file is not exist\n";
	
		// prepare command line with filename and unique name
		//out_fullpath += " ";
		//out_fullpath += filename;
		//out_fullpath += " ";
		//out_fullpath += uniqueName;

		//std::string		out_fullpath("D:\\Program Files\\Autodesk\\MotionBuilder 2017\\bin\\x64\\plugins\\cmdFBX.exe");

		// prepare command line with filename and unique name
		out_fullpath += " ";
		out_fullpath += filename;
		out_fullpath += " ";
		out_fullpath += uniqueName;

		// Start the child process. 
		if( !CreateProcess( NULL,   // No module name (use command line)
			out_fullpath,        // Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			0,              // No creation flags
			NULL,           // Use parent's environment block
			NULL,           // Use parent's starting directory 
			&si,            // Pointer to STARTUPINFO structure
			&pi )           // Pointer to PROCESS_INFORMATION structure
		) 
		{
			throw "CreateProcess failed";
		}

		// Wait until child process exits.
		WaitForSingleObject( pi.hProcess, INFINITE );

		// Close process and thread handles. 
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
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