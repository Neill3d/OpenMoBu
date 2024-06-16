
#include <windows.h>
#include "FileUtils.h"
#include <filesystem>

/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/OpenMoBu
//
// Author Sergei Solokhin (Neill3d) 2014-2017
//  e-mail to: neill3d@gmail.com
//
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////

bool IsFileExists ( const TCHAR *filename ) {
	
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFile( filename, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	else
	{
		FindClose(hFind);
		return true;
	}
}


////////////////////////////////////////////////////////////

bool FindEffectLocation(const char *effect, FBString &out_path, FBString &out_fullname)
{
	FBSystem	&lSystem = FBSystem::TheOne();
	
	bool found = std::filesystem::exists(effect);

	if (found)
	{
		out_fullname = effect;
		const int delimPos = std::max(0, std::max(out_fullname.ReverseFind('/'), out_fullname.ReverseFind('\\')));
		out_path = out_fullname.Left(delimPos);
	}
	else
	{
		out_path = lSystem.UserConfigPath;

		char buffer[MAX_PATH];
		memset(buffer, 0, sizeof(char) * MAX_PATH);
		sprintf_s(buffer, sizeof(char) * MAX_PATH, "%s\\%s", static_cast<const char*>(out_path), effect);

		if (std::filesystem::exists(buffer))
		{
			out_fullname = buffer;
			return true;
		}
	}
	
	if (!found)
	{
		FBStringList paths;
#ifndef ORSDK2013
		paths = lSystem.GetPluginPath();
#endif
		char buffer[MAX_PATH];

		for (int i=0; i<paths.GetCount(); ++i)
		{
			out_path = paths[i];

			memset(buffer, 0, sizeof(char) * MAX_PATH);
			sprintf_s(buffer, sizeof(char) * MAX_PATH, "%s\\%s", static_cast<const char*>(out_path), effect);

			if (std::filesystem::exists(buffer))
			{
				out_fullname = buffer;
				found = true;
				break;
			}
		}
	}

	return found;
}