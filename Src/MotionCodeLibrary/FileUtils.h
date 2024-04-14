
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