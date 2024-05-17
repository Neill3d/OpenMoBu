
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


//--- SDK include
//#include <pyfbsdk/pyfbsdk.h>

#ifdef KARCH_ENV_WIN
#include <windows.h>
#endif

#include "ortexture_advanceBlend_texture.h"

/*
#include "ortexture_advanceBlend_pytexture.h"
//--- Python module initialization functions.
BOOST_PYTHON_MODULE(ortexture_advanceblend) //! Must exactly match the name of the module,usually it's the same name as $(TargetName).
{
    //! Preferred practice, invokes initialization functions defined in separated CXX files.
    ORTextureAdvanceBlend_Init();
}
*/
//! Set the target extension to ".pyd" on property page of project, Or
//! Use post-build event "copy $(TargetPath)  ..\..\..\..\bin\$(PlatformName)\plugins\$(TargetName).pyd"
//! to rename the module by file extension “.pyd” into python directory to make sure the module could be found.

//--- Library declaration.
FBLibraryDeclare( ortexture_advanceblend )
{
	FBLibraryRegisterStorable(ORTextureAdvanceBlend);
	FBLibraryRegisterShadingElement(ORTextureAdvanceBlend);
	FBLibraryRegister( ORTextureAdvanceBlendLayout );

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
