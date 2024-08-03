
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ortexture_advanceBlend.cxx
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/OpenMoBu
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef KARCH_ENV_WIN
#include <windows.h>
#endif

#include "ortexture_advanceBlend_texture.h"
#include "mobu_logging.h"

DEFINE_LOGE;

//--- Library declaration.
FBLibraryDeclare( ortexture_advanceblend )
{
	FBLibraryRegisterStorable(ORTextureAdvanceBlend);
	FBLibraryRegisterShadingElement(ORTextureAdvanceBlend);
	
	FBLibraryRegisterStorable(ORTextureParamBlend);
	FBLibraryRegisterShadingElement(ORTextureParamBlend);
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
