
/////////////////////////////////////////////////////////////////////////////////////////
//
// box_rayInteresect.cxx
//
// Sergei <Neill3d> Solokhin 2014-2018
//
// GitHub page - https://github.com/Neill3d/OpenMoBu
// Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
/////////////////////////////////////////////////////////////////////////////////////////


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
