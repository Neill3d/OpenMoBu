
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: MAIN.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>

#ifdef KARCH_ENV_WIN
	#include <windows.h>
#endif

#include "ResourceUtils.h"

//--- Library declaration.
FBLibraryDeclare( ortool_blendshape )
{
	FBLibraryRegister( ORTool_BlendShape );

	FBLibraryRegister( ORManip_Sculpt );
	FBLibraryRegister( ORManip_Sculpt_Tool	);
	FBLibraryRegister( KSculptBrushAssociation );

	FBLibraryRegister( BlendShapeDeformerConstraint );
	FBLibraryRegister( BlendShapeDeformerLayout );
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()		{ 
	
	InitResourceUtils();
	
	return true; }
bool FBLibrary::LibOpen()		{ return true; }
bool FBLibrary::LibReady()		{ return true; }
bool FBLibrary::LibClose()		{ return true; }
bool FBLibrary::LibRelease()	{ return true; }

