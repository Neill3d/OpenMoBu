
/**	\file	Main.cxx
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
FBLibraryDeclare( boxrbf )
{
	FBLibraryRegister( BoxRBF3 );
	FBLibraryRegister( BoxRBF4 );

    // work with quaternion

    FBLibraryRegister(BoxEulerToQuaternion);
    FBLibraryRegister(BoxQuaternionToEuler);
    FBLibraryRegister(BoxQuaternionMult);
    FBLibraryRegister(BoxQuaternionInvert);
    FBLibraryRegister(BoxQuaternionNormalize);
    FBLibraryRegister(BoxQuaternionLookRotation);
    FBLibraryRegister(BoxQuaternionAxisRotation);
    FBLibraryRegister(BoxQuaternionLerp);
    FBLibraryRegister(BoxVectorRotateBy);
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
