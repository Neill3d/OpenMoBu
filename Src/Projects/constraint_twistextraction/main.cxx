/**	\file	main.cxx

Mikkel Brons-Frandsen 2021

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#ifdef KARCH_ENV_WIN
#include <windows.h>
#endif

//--- Library declaration
FBLibraryDeclare(constrainttwistextraction)
{
	FBLibraryRegister(CConstraintTwistextraction);
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit() { return true; }
bool FBLibrary::LibOpen() { return true; }
bool FBLibrary::LibReady() { return true; }
bool FBLibrary::LibClose() { return true; }
bool FBLibrary::LibRelease() { return true; }
