
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/OpenMoBu
//
// Author Sergey Solokhin (Neill3d) 2014-2017
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>

// call this on dll init
void InitResourceUtils();
HMODULE		GetThisModuleHandle();

const char *LoadStringFromResource1(int id);
const char *LoadStringFromResource2(int id);

const char *LoadVersionFromResource();