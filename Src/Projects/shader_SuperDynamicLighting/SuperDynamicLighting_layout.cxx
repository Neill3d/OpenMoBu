

/**	\file	DynamicLighting_layout.cxx

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declarations
#include "SuperDynamicLighting_layout.h"
#include "SuperDynamicLighting_shader.h"

//--- Registration define
#define DYNAMICLIGHTING__LAYOUT		SuperDynamicLightingLayout

//--- FiLMBOX implementation and registration
FBShaderLayoutImplementation(	DYNAMICLIGHTING__LAYOUT		);
FBRegisterShaderLayout		(	DYNAMICLIGHTING__LAYOUT,
                             DYNAMICLIGHTING__DESCSTR,
                             FB_DEFAULT_SDK_ICON				);	// Icon filename (default=Open Reality icon)

/************************************************
*	FiLMBOX constructor.
************************************************/
bool SuperDynamicLightingLayout::FBCreate()
{
    mShader	= (SuperDynamicLighting*)(FBShader*)Shader;

    UICreate	();
    UIConfigure	();
    UIReset		();

    return true;
}


/************************************************
*	FiLMBOX destructor.
************************************************/
void SuperDynamicLightingLayout::FBDestroy()
{
}


/************************************************
*	Create the UI.
************************************************/
void SuperDynamicLightingLayout::UICreate()
{
    int lB = 10;
    int lS = 4;
    int lW = 110;
    int lH = 18;

    AddRegion("TransLabel",	"TransLabel",
        lB,		kFBAttachLeft,	"",			1.00,
        lB,		kFBAttachTop,	"",			1.00,
        lW,		kFBAttachNone,	NULL,		1.00,
        lH,		kFBAttachNone,	NULL,		1.00 );
    AddRegion("Transparency","Transparency",
        lS,		kFBAttachRight,	"TransLabel",	1.00,
        0,		kFBAttachTop,	"TransLabel",	1.00,
        2*lW,	kFBAttachNone,	NULL,		1.00,
        0,		kFBAttachHeight,"TransLabel",	1.00 );

    AddRegion("TransFactorLabel",	"TransFactorLabel",
        lB,		kFBAttachLeft,	"",			1.00,
        lB,		kFBAttachBottom,"TransLabel", 1.00,
        lW,		kFBAttachNone,	NULL,		1.00,
        lH,		kFBAttachNone,	NULL,		1.00 );
    AddRegion("TransparencyFactor",	"TransparencyFactor",
        lS,		kFBAttachRight,	"TransFactorLabel",		1.00,
        0,		kFBAttachTop,   "TransFactorLabel",		1.00,
        2*lW,	kFBAttachNone,	NULL,		1.00,
        lH,		kFBAttachNone,	NULL,		1.00 );

	AddRegion("UseSceneLights", "UseSceneLights",
		lB, kFBAttachLeft, "", 1.00,
		lB, kFBAttachBottom, "TransFactorLabel", 1.00,
		lW, kFBAttachNone, NULL, 1.00,
		lH, kFBAttachNone, "", 1.00);
    
	AddRegion("AffectLabel",	"AffectLabel",
        lB,		kFBAttachLeft,	"",			1.00,
        lB,		kFBAttachBottom,"UseSceneLights", 1.00,
        lW,		kFBAttachNone,	NULL,		1.00,
        lH,		kFBAttachNone,	NULL,		1.00 );
	AddRegion("AffectingLights","AffectingLights",
        lS,		kFBAttachRight,	"AffectLabel",	1.00,
        0,		kFBAttachTop,	"AffectLabel",	1.00,
        2*lW,	kFBAttachNone,	NULL,		1.00,
        0,		kFBAttachHeight,"AffectLabel",	1.00 );

    AddRegion("CastShadows", "CastShadows",
        lB, kFBAttachLeft, "", 1.00,
        lB, kFBAttachBottom, "AffectLabel", 1.00,
        lW, kFBAttachNone, NULL, 1.00,
        lH, kFBAttachNone, "", 1.00);

    AddRegion("CastersLabel", "CastersLabel",
        lB, kFBAttachLeft, "", 1.00,
        lB, kFBAttachBottom, "CastShadows", 1.00,
        lW, kFBAttachNone, NULL, 1.00,
        lH, kFBAttachNone, NULL, 1.00);
    AddRegion("Casters", "Casters",
        lS, kFBAttachRight, "CastersLabel", 1.00,
        0, kFBAttachTop, "CastersLabel", 1.00,
        2 * lW, kFBAttachNone, NULL, 1.00,
        0, kFBAttachHeight, "CastersLabel", 1.00);

	AddRegion("UseRim", "UseRim",
		lB, kFBAttachLeft, "", 1.00,
		lB, kFBAttachBottom, "CastersLabel", 1.00,
		3*lW, kFBAttachNone, NULL, 1.00,
		lH, kFBAttachNone, "", 1.00);

	AddRegion("RimPower", "RimPower",
		lB, kFBAttachLeft, "", 1.00,
		lB, kFBAttachBottom, "UseRim", 1.00,
		3*lW, kFBAttachNone, NULL, 1.00,
		lH, kFBAttachNone, "", 1.00);
	AddRegion("RimColor", "RimColor",
		lB, kFBAttachLeft, "", 1.00,
		lB, kFBAttachBottom, "RimPower", 1.00,
		3*lW, kFBAttachNone, NULL, 1.00,
		lH, kFBAttachNone, "", 1.00);

	AddRegion("UseMatCap", "UseMatCap",
		lB, kFBAttachLeft, "", 1.00,
		lB, kFBAttachBottom, "RimColor", 1.00,
		lW, kFBAttachNone, NULL, 1.00,
		lH, kFBAttachNone, "", 1.00);

	AddRegion("MatCapLabel", "MatCapLabel",
		lB, kFBAttachLeft, "", 1.00,
		lB, kFBAttachBottom, "UseMatCap", 1.00,
		lW, kFBAttachNone, NULL, 1.00,
		lH, kFBAttachNone, NULL, 1.00);
	AddRegion("matcap", "matcap",
		lS, kFBAttachRight, "MatCapLabel", 1.00,
		0, kFBAttachTop, "MatCapLabel", 1.00,
		2 * lW, kFBAttachNone, NULL, 1.00,
		0, kFBAttachHeight, "MatCapLabel", 1.00);

    SetControl( "TransLabel",   mTransLabel );
    SetControl( "Transparency", mTransparency );

    SetControl( "TransFactorLabel", mTransFactorLabel );
    SetControl( "TransparencyFactor", mTransparencyFactor );

	SetControl("UseSceneLights", mUseSceneLights);
    SetControl( "AffectLabel",      mAffectLabel);
    SetControl( "AffectingLights",  mAffectingLights);

    SetControl("CastShadows", mCastShadows);
    SetControl("CastersLabel", mCastersLabel);
    SetControl("Casters", mShadowCasters);

	SetControl("UseRim", mUseRim);
	SetControl("RimPower", mRimPower);
	SetControl("RimColor", mRimColor);

	SetControl("UseMatCap", mUseMatCap);
	SetControl("MatCapLabel", mMatCapLabel);
	SetControl("matcap", mMatCapTexture);
}


/************************************************
*	Configure the UI.
************************************************/
void SuperDynamicLightingLayout::UIConfigure()
{
    mTransLabel.Caption = "Transparency Type";

    FBPropertyAlphaSource lEnum;
    for( int i = 0; i < 7; ++i )
    {
        mTransparency.Items.Add( lEnum.EnumList(i), i );
    }
	mTransparency.OnChange.Add(this, (FBCallback)&SuperDynamicLightingLayout::EventTransparencyChange);

    mTransFactorLabel.Caption = "Transparency Factor";
    mTransparencyFactor.Property = &mShader->TransparencyFactor;

	mUseSceneLights.Caption = "Use Scene Lights";
	mUseSceneLights.Property = &mShader->UseSceneLights;

    mAffectLabel.Caption = "Affecting Lights";
    mAffectingLights.Property = &mShader->AffectingLights;

    mCastShadows.Caption = "Cast Shadows";
    mCastShadows.Property = &mShader->Shadows;

    mCastersLabel.Caption = "Shadow Casters";
    mShadowCasters.Property = &mShader->ShadowCasters;

	mUseRim.Caption = "Use Rim";
	mUseRim.Property = &mShader->UseRim;
	mRimPower.Caption = "Rim Power";
	mRimPower.Property = &mShader->RimPower;
	mRimColor.Caption = "Rim Color";
	mRimColor.Property = &mShader->RimColor;

	mUseMatCap.Caption = "Use MatCap";
	mUseMatCap.Property = &mShader->UseMatCap;
	mMatCapLabel.Caption = "MatCap Texture";
	mMatCapTexture.Property = &mShader->MatCapTexture;
}

/************************************************
*	Reset the UI from the device.
************************************************/
void SuperDynamicLightingLayout::UIReset()
{
    mTransparency.ItemIndex = (int)mShader->GetTransparencyType();
}

void SuperDynamicLightingLayout::EventTransparencyChange(HISender /*pSender*/, HKEvent /*pEvent*/)
{
    mShader->SetTransparencyType( (FBAlphaSource)(int)mTransparency.ItemIndex );
}

