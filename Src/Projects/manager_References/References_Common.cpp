
// References_Common.cpp
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//--- SDK include
#include <fbsdk/fbsdk.h>
#include <Windows.h>
#include "References_Common.h"

FBString	gScriptPath(ACTION_DEV_PATH);
FBString	gTempPath("C:\\");

const char *GetActionPath()
{
	return gScriptPath;
}

const char *GetSystemTempPath()
{
	return gTempPath;
}

bool FileExists(const char *szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool FindAScriptPath()
{
	bool lStatus = false;
	FBSystem	mSystem;

	gTempPath = mSystem.TempPath;
	const FBStringList &paths = mSystem.GetPythonStartupPath();

	for (int i = 0, count = paths.GetCount(); i < count; ++i)
	{
		FBString scriptPath(paths[i], ACTION_REFAFILE);
		if (FileExists(scriptPath))
		{
			gScriptPath = paths[i];
			lStatus = true;
			break;
		}
	}
	
	return lStatus;
}