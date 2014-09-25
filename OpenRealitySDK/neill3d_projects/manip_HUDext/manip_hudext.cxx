
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>

#ifdef KARCH_ENV_WIN
	#include <windows.h>
#endif

//--- Library declaration
FBLibraryDeclare( manip_hudext )
{
	FBLibraryRegister( Manip_HUDext			);
	FBLibraryRegister( Manip_HUDext_Layout	);
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 *************** *********************************/
bool FBLibrary::LibInit()		{ return true; }
bool FBLibrary::LibOpen()		{ return true; }
bool FBLibrary::LibReady()		{ return true; }
bool FBLibrary::LibClose()		{ return true; }
bool FBLibrary::LibRelease()	{ return true; }

/**
*	\mainpage	Manipulator Template
*	\section	intro	Introduction
*	Template showing what needs to be done
*	in order to create a new manipulator in FiLMBOX.

LOG

10.10.2013
 - add justification and alignment processing for mouse inside function
 - add camera switcher for buttons
 - some bug fixes for using several different huds in different cameras

*/
