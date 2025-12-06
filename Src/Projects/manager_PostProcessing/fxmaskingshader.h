
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: fxmaskingshader.h
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>
//#include "..\Common\Common_shader.h"
//#include "fxmaskobject.h"


//--- Registration define

#define FX_MASKING__CLASSSTR		"FXMaskingShader"
#define FX_MASKING__DESCSTR			"FX Post Processing Masking"

#define FX_MASKING__CLASSNAME	FXMaskingShader

/// <summary>
/// assign mask channel to objects which are attached to the given shader
/// </summary>
class FXMaskingShader : public FBShader
{
    // Declaration.
    FBShaderDeclare(FXMaskingShader, FBShader );

public:
	
    // Open Reality constructor and destructor. Do not use C++ constructor/destructor.
    virtual bool FBCreate();
    virtual void FBDestroy();

public:
    
	FBPropertyBool									CompositeMaskA;
	FBPropertyBool									CompositeMaskB;
	FBPropertyBool									CompositeMaskC;
	FBPropertyBool									CompositeMaskD;

	// the flag defines which color object will be draw, white or black
	// that is useful for making mask occlusion with certain objects
	FBPropertyBool									OcclusionRender;
};
