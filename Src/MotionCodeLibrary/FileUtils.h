
#pragma once

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

//--- SDK include
#include <fbsdk/fbsdk.h>

/////////////////////////////////////////////////////////////

bool IsFileExists ( const char *filename );

//
// search first of all in mobu config folder, then in all plugins folders
//
bool FindEffectLocation(const char *effect, FBString &out_path, FBString &out_fullname);

/// <summary>
/// open file for reading and keep it open for a class life scope
/// </summary>
class FileReadScope
{
public:
	FileReadScope(const char* filename)
	{
		fopen_s(&fp, filename, "r");
	}

	~FileReadScope()
	{
		if (fp)
			fclose(fp);
	}

	FILE* Get() const { return fp; }

private:

	FILE* fp{ nullptr };
};