#ifndef __ORTEXTURE_ADVANCEBLEND_PYTEXTURE_H__
#define __ORTEXTURE_ADVANCEBLEND_PYTEXTURE_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ortexture_advanceBlend_pytexture.h
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/OpenMoBu
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "ortexture_advanceBlend_texture.h"

// =======================================================================================
// ORLayeredTextureCustom
// =======================================================================================
void ORTextureAdvanceBlend_Init();

inline ORTextureAdvanceBlend* CreateORLayeredTextureAdvanceBlend(const char* pName)
{
    ORTextureAdvanceBlend* lTexture = new ORTextureAdvanceBlend(pName);
    lTexture->FBCreate();
    return lTexture;
}

//--- Wrapper class for SDK Sample class ORLayeredTextureCustom
class ORTextureAdvanceBlend_Wrapper : public FBLayeredTexture_Wrapper 
{
protected:
	ORTextureAdvanceBlend			*mORTextureAdvanceBlend;

public:
    /** For the FBComponent* constructor.
    *   Used internally by the wrapper factory method to construct python wrapper from an existing C++ instance.
    */
	ORTextureAdvanceBlend_Wrapper( FBComponent* pFBComponent )
    :  FBLayeredTexture_Wrapper( pFBComponent ) 
    { 
        mORTextureAdvanceBlend = (ORTextureAdvanceBlend*)pFBComponent; 
    }

	/** Normal char* constructor.
    *   Create a new instance of C++ instance and wrap it to python. 
    */
    ORTextureAdvanceBlend_Wrapper( const char* pName )
    : FBLayeredTexture_Wrapper( CreateORLayeredTextureAdvanceBlend(pName) ) 
    { 
        mORTextureAdvanceBlend = (ORTextureAdvanceBlend*)mFBComponent; 
    }

	virtual ~ORTextureAdvanceBlend_Wrapper( ) 
    {
    }

    //--- Expose user defined properties to Python.
    DECLARE_DEFINE_ORSDK_PROPERTY_PYTHON_CUSTOM_TYPE_ACCESS( ORTextureAdvanceBlend, AuxLayer, FBColorAndAlpha );
};

#endif /* __ORTEXTURE_ADVANCEBLEND_PYTEXTURE_H__ */
