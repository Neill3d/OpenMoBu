
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: colors_MAIN.h
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/OpenMoBu
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>
#include <fbsdk/fbsystem.h>
#include <fbsdk/fbscene.h>
#include <fbsdk/fbrenderer.h>

#include "colors_renderer.h"

#ifdef KARCH_ENV_WIN
	#include <windows.h>
#endif

//--- Library declaration
FBLibraryDeclare( orcustomrenderer )
{
    FBLibraryRegister( ORCustomRendererCallbackLayout );

	FBLibraryRegister( ColorsRendererCallback );
	
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()	{ return true; }
bool FBLibrary::LibOpen()	{ return true; }
bool FBLibrary::LibReady()	{ return true; }
bool FBLibrary::LibClose()	{ return true; }
bool FBLibrary::LibRelease(){ return true; }
