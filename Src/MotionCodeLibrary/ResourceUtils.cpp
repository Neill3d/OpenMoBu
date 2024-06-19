
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/OpenMoBu
//
// Author Sergey Solokhin (Neill3d) 2014-2017
//  e-mail to: neill3d@gmail.com
// 
/////////////////////////////////////////////////////////////////////////////////////////

#include "ResourceUtils.h"

#include <Windows.h>
#include <stdio.h>

////////////////////////////////////

HMODULE		gInstance = 0;


#define RESOURCE_STRING_BUFFER_SIZE		256

char szBuffer1[RESOURCE_STRING_BUFFER_SIZE];
char szBuffer2[RESOURCE_STRING_BUFFER_SIZE];
char szBufferVersion[RESOURCE_STRING_BUFFER_SIZE];

char szModulePath[MAX_PATH];

////////////////////////////////////////////

HMODULE	GetThisModuleHandle()
{
	return gInstance;
}

void localFunc()
{
}

void InitResourceUtils()
{
	if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | 
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR) &localFunc, 
        &gInstance))
	{
		int ret = GetLastError();
		fprintf(stderr, "GetModuleHandle returned %d\n", ret);
	}
	GetModuleFileNameA(gInstance, szModulePath, sizeof(szModulePath));
}

/////////////////////////////////////////////

const char *LoadStringFromResource1(int id)
{
	memset( szBuffer1, 0, sizeof(char) * RESOURCE_STRING_BUFFER_SIZE );
	LoadString( gInstance, id, szBuffer1, RESOURCE_STRING_BUFFER_SIZE );

	return szBuffer1;
}


const char *LoadStringFromResource2(int id)
{
	memset( szBuffer2, 0, sizeof(char) * RESOURCE_STRING_BUFFER_SIZE );
	LoadString( gInstance, id, szBuffer2, RESOURCE_STRING_BUFFER_SIZE );

	return szBuffer2;
}

BOOL GetAppVersion( char *LibName, WORD *MajorVersion, WORD *MinorVersion, WORD *BuildNumber, WORD *RevisionNumber )
{
	DWORD dwHandle, dwLen;
	UINT BufLen;
	LPTSTR lpData;
	VS_FIXEDFILEINFO *pFileInfo;

	dwLen = GetFileVersionInfoSize( LibName, &dwHandle );
	if (!dwLen) 
		return FALSE;

	lpData = (LPTSTR) malloc (dwLen);
	if (!lpData) 
		return FALSE;

	if(!dwHandle || !GetFileVersionInfo( LibName, dwHandle, dwLen, lpData ) )
	{
		free (lpData);
		return FALSE;
	}
	
	if( VerQueryValue( lpData, "\\", (LPVOID *) &pFileInfo, (PUINT)&BufLen ) ) 
	{
		*MajorVersion = HIWORD(pFileInfo->dwFileVersionMS);
		*MinorVersion = LOWORD(pFileInfo->dwFileVersionMS);
		*BuildNumber = HIWORD(pFileInfo->dwFileVersionLS);
		*RevisionNumber = LOWORD(pFileInfo->dwFileVersionLS);
		free (lpData);
		return TRUE;
	}
	
	free (lpData);
	return FALSE;
}



const char *LoadVersionFromResource()
{
	WORD MajorVersion			=0; 
	WORD MinorVersion			=0; 
	WORD BuildNumber			=0; 
	WORD RevisionNumber			=0;

	GetAppVersion(szModulePath,
				  &MajorVersion,    
				  &MinorVersion,    
				  &BuildNumber,    
				  &RevisionNumber);

	sprintf_s( szBufferVersion, RESOURCE_STRING_BUFFER_SIZE, "%d.%d.%d.%d", MajorVersion, MinorVersion, BuildNumber, RevisionNumber );
	return szBufferVersion;
}

