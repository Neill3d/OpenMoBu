
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

//--- Library declaration.
FBLibraryDeclare( UIBuilderTool )
{
	FBLibraryRegister( UIBuilderTool );
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()		{ return true; }
bool FBLibrary::LibOpen()		{ return true; }
bool FBLibrary::LibReady()		{ return true; }
bool FBLibrary::LibClose()		{ return true; }
bool FBLibrary::LibRelease()	{ return true; }

/**
*	\mainpage	Tool UIBuilder
*	\section	intro	Introduction
*	Tool for building user interface for the MotionBuilder's API like
* OpenReality SDK and Python.\n
* Features:
* - canvas for create and operate with layout regions
* - properties of the region, allow to set region connections
* - assign visual components to regions like label, button, edit box, etc.
* - adjust visual component properties using properties table
* - write events for visual component and tool
* - run tool under Python interpreter to see result of UI building
* - export ui to source code for OpenReality SDK C++ and Python
*
* Author: Sergey Solohin (Neill), e-mail to: Neill.Solow@gmail.com\n
* Project Homepage on my blog: www.neill3d.com
*/
