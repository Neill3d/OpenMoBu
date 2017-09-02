
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://code.google.com/p/motioncodelibrary/wiki/LICENSE
//
// MotionCodeLibrary collection - https://code.google.com/p/motioncodelibrary/
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////

/**	\file	box_rayIntersect.cxx
*	Library declarations.
*	Contains the basic routines to declare the DLL as a loadable
*	library.
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#ifdef KARCH_ENV_WIN
	#include <windows.h>
#endif

//--- Library declaration
FBLibraryDeclare( box_rayintersect )
{
	FBLibraryRegister( Box_RayIntersect );
	FBLibraryRegister( Box_SphereCoords );
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
*	\mainpage	Box Ray Intersect
*	\section	intro	Introduction
*	Relation box object to get U,V coords of picked node
*		using ray intersection method
*/
