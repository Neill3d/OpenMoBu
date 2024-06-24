
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/OpenMoBu
//
// Author Sergei Solokhin (Neill3d) 2014-2024
//  e-mail to: neill3d@gmail.com
//
/////////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "FileUtils.h"
#include <filesystem>

//--- SDK include
#include <fbsdk/fbsdk.h>

/////////////////////////////////////////////////////////////

bool IsFileExists ( const char* filename ) 
{	
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

bool FindEffectLocation(const char *effect, char* outPath, const int outPathLength)
{
	char buffer[MAX_PATH];

	auto fn_checkLocation = [&buffer](const char* location, const char* fileName) -> bool
		{
			memset(buffer, 0, sizeof(char) * MAX_PATH);
			sprintf_s(buffer, sizeof(char) * MAX_PATH, "%s\\%s", location, fileName);

			return std::filesystem::exists(buffer);
		};

	auto fn_copyLocation = [](const char* location, char* outPath, const int maxPath)
		{
			const int len = std::min(MAX_PATH, maxPath);
			memset(outPath, 0, sizeof(char) * len);

			const int userConfigPathLen = std::min(static_cast<int>(strlen(location)), maxPath);
			memcpy(outPath, location, sizeof(char) * userConfigPathLen);
		};

	FBSystem& lSystem = FBSystem::TheOne();

	// check if effect is in absolute path and could be found directly
	
	if (std::filesystem::exists(effect) 
		&& strstr(effect, ":") != nullptr)
	{
		const int effectLen = std::min(static_cast<int>(strlen(effect)), outPathLength);
		
		for (int i = effectLen - 1; i >= 0; --i)
		{
			if (effect[i] == '/' || effect[i] == '\\')
			{
				memcpy(outPath, effect, sizeof(char) * i);
				break;
			}
		}
	}
	else
	{
		const char* userConfigPath = static_cast<const char*>(lSystem.UserConfigPath);

		if (fn_checkLocation(userConfigPath, effect))
		{
			fn_copyLocation(userConfigPath, outPath, outPathLength);
			return true;
		}

	}
	
	// look in alternative plugin paths
	FBStringList paths;
#ifndef ORSDK2013
	paths = lSystem.GetPluginPath();
#endif
	
	for (int i=0; i<paths.GetCount(); ++i)
	{
		const char* location = static_cast<const char*>(paths[i]);

		if (fn_checkLocation(location, effect))
		{
			fn_copyLocation(location, outPath, outPathLength);
			return true;
		}
	}
	
	return false;
}


bool FindEffectLocation(std::function<bool(const char* testPath)> const& checkLocationFn, char* outPath, const int outPathLength)
{
	auto fn_copyLocation = [](const char* location, char* outPath, const int maxPath)
		{
			const int len = std::min(MAX_PATH, maxPath);
			memset(outPath, 0, sizeof(char) * len);

			const int userConfigPathLen = std::min(static_cast<int>(strlen(location)), maxPath);
			memcpy(outPath, location, sizeof(char) * userConfigPathLen);
		};

	FBSystem& lSystem = FBSystem::TheOne();

	// check if effect is in absolute path and could be found directly

	const char* userConfigPath = static_cast<const char*>(lSystem.UserConfigPath);

	if (checkLocationFn(userConfigPath))
	{
		fn_copyLocation(userConfigPath, outPath, outPathLength);
		return true;
	}

	// look in alternative plugin paths

	FBStringList paths;
#ifndef ORSDK2013
	paths = lSystem.GetPluginPath();
#endif

	for (int i = 0; i < paths.GetCount(); ++i)
	{
		const char* location = static_cast<const char*>(paths[i]);

		if (checkLocationFn(location))
		{
			fn_copyLocation(location, outPath, outPathLength);
			return true;
		}
	}

	return false;
}