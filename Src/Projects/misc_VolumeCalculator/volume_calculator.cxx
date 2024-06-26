
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
#include <fbsdk/fbsdk.h>

#ifdef KARCH_ENV_WIN
	#include <windows.h>
#endif

#include "volume_calculator_model_display.h"

//--- Library declaration.
FBLibraryDeclare( ModelVolumeCalculator )
{
	FBLibraryRegisterStorable(ModelVolumeCalculator);
	FBLibraryRegisterElement(ModelVolumeCalculator);
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()		{ return true; }
bool FBLibrary::LibOpen()		{ 
	ModelVolumeCalculator::AddPropertiesToPropertyViewManager();
	return true; 
}
bool FBLibrary::LibReady()		{ return true; }
bool FBLibrary::LibClose()		{ return true; }
bool FBLibrary::LibRelease()	{ return true; }

/**
*	\mainpage	Model Template
*	\section	intro	Introduction
*	Template showing what needs to be done
*	in order to create a new model type in FiLMBOX.
*/
