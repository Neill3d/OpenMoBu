
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ortexture_advanceBlend_pytexture.cxx
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/OpenMoBu
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <pyfbsdk/pyfbsdk.h>

#ifdef KARCH_ENV_WIN
#include <windows.h>
#endif

#include "ortexture_advanceBlend_texture.h"
#include "ortexture_advanceBlend_pytexture.h"

void ORTextureAdvanceBlend_Init()
{
    REGISTER_FBWRAPPER_FORWARD( ORTextureAdvanceBlend )
	class_<ORTextureAdvanceBlend_Wrapper,bases<FBTexture_Wrapper>, Wrapper_Container_1<ORTextureAdvanceBlend_Wrapper, const char*>, boost::noncopyable >("ORTextureAdvanceBlend",init<const char*>())
		//! class properties define
        ADD_ORSDK_PROPERTY_PYTHON_CUSTOM_TYPE_ACCESS( ORTextureAdvanceBlend, AuxLayer )
		;

	//! Register for Wrapper Factory, shared pointer and Type information.
	REGISTER_FBWRAPPER( ORTextureAdvanceBlend );
}