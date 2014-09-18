
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://code.google.com/p/motioncodelibrary/wiki/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Windows.h>

//--- SDK include
#include <fbsdk/fbsdk.h>




/************************************************
 *  Execution callback.
 ************************************************/
void DisplayErrorText(DWORD dwLastError);

//
// additive from character animation (you need 3 takes)
//
void AdditiveCalculate();

// In scene you should have 2 models: srcModel and dstModel
// this function replaced geometry from src to dst
// !! function is under construction
//
void GeometryReplace();