
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ortexture_advanceBlend_layout.cxx
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/OpenMoBu
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declarations
#include "ortexture_advanceBlend_layout.h"

//--- Registration define
#define ORTEXTUREADVANCEBLEND__LAYOUT		ORTextureAdvanceBlendLayout

//--- FiLMBOX implementation and registration
FBTextureLayoutImplementation(	ORTEXTUREADVANCEBLEND__LAYOUT);
FBRegisterTextureLayout	  (	ORTEXTUREADVANCEBLEND__LAYOUT, ORTEXTUREADVANCEBLEND__CLASSSTR, FB_DEFAULT_SDK_ICON);

/************************************************
*	FiLMBOX constructor.
************************************************/
bool ORTextureAdvanceBlendLayout::FBCreate()
{
	mTextureCustom	= (ORTextureAdvanceBlend*) (FBTexture*) Texture;

    UICreate	();
    UIConfigure	();
    UIReset		();

    return true;
}


/************************************************
*	FiLMBOX destructor.
************************************************/
void ORTextureAdvanceBlendLayout::FBDestroy()
{

}


/************************************************
*	Create the UI.
************************************************/
void ORTextureAdvanceBlendLayout::UICreate()
{
    int lS=5;
    int lH=17;
    int lW=300;

    AddRegion("AuxLayer","",lS,kFBAttachTop,"",1,lS,kFBAttachLeft,"",1,lW,kFBAttachLeft,"",1, lH,kFBAttachNone,"", 1);
}


/************************************************
*	Configure the UI.
************************************************/
void ORTextureAdvanceBlendLayout::UIConfigure()
{
	SetControl( "AuxLayer", mEditAuxLayer );
	mEditAuxLayer.Property = &(mTextureCustom->BackgroundColor);
}

/************************************************
*	Reset the UI from the device.
************************************************/
void ORTextureAdvanceBlendLayout::UIReset()
{

}
