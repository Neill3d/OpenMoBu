
/////////////////////////////////////////////////////////////////////////////////////////
//
// box_poseReader.cxx
//
// Sergei <Neill3d> Solokhin 2014-2018
//
// GitHub page - https://github.com/Neill3d/OpenMoBu
// Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
/////////////////////////////////////////////////////////////////////////////////////////

/**	\file	box_poseReader.cxx
*	Library declarations.
*	Contains the basic routines to declare the DLL as a loadable
*	library.
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#ifdef KARCH_ENV_WIN
	#include <windows.h>
#endif

#include "resourceUtils.h"

//--- Library declaration
FBLibraryDeclare( box_posereader )
{
	FBLibraryRegister( Box_PoseReader );
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()		{ return true; }
bool FBLibrary::LibOpen()		{ return true; }
bool FBLibrary::LibReady()		
{ 
	InitResourceUtils();
	return true; 
}
bool FBLibrary::LibClose()		{ return true; }
bool FBLibrary::LibRelease()	{ return true; }

/**
*	\mainpage	Box Template
*	\section	intro	Introduction
*	Template showing what needs to be done
*	in order to create a new box/operator.
*/
