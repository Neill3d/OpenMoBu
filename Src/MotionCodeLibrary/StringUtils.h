
#pragma once


/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: neill3d@gmail.com
// 
/////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include "fbsdk/fbsdk.h"

// return filename from a full path
FBString ExtractFileName(FBString str);

FBString ExtractFilePath(FBString &str);

bool SplitPath(FBString &path, FBStringList &list);

// change name in full path str to the new_name (inc extension)
void ChangeFileName(FBString &str, FBString new_name);

// change filename extension to the specified new_ext
void ChangeFileExt(FBString &str, FBString new_ext);