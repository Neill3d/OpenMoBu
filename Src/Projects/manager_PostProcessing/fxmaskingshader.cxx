
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: fxmaskingshader.cxx
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// Class declaration
#include "fxmaskingshader.h"

//--- Registration defines
#define FX_MASKING__CLASS	FX_MASKING__CLASSNAME
#define FX_MASKING__DESC	"FX Masking" // This is what shows up in the shader window ...

//--- FiLMBOX Registration & Implementation.
FBShaderImplementation(	FX_MASKING__CLASS	);
FBRegisterShader	(FX_MASKING__DESCSTR,	    // Unique name
                     FX_MASKING__CLASS,		// Class
                     FX_MASKING__DESCSTR,	    // Short description
                     FX_MASKING__DESC,		    // Long description
                     FB_DEFAULT_SDK_ICON	);		// Icon filename (default=Open Reality icon)

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool FXMaskingShader::FBCreate()
{
	FBPropertyPublish(this, CompositeMaskA, "Mask A", nullptr, nullptr);
	FBPropertyPublish(this, CompositeMaskB, "Mask B", nullptr, nullptr);
	FBPropertyPublish(this, CompositeMaskC, "Mask C", nullptr, nullptr);
	FBPropertyPublish(this, CompositeMaskD, "Mask D", nullptr, nullptr);

    CompositeMaskA = false;
    CompositeMaskB = false;
    CompositeMaskC = false;
    CompositeMaskD = false;

    return true;
}


/************************************************
*	FiLMBOX Destructor.
************************************************/
void FXMaskingShader::FBDestroy()
{
}


