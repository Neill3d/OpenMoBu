
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: fb_shader_particles_layout.cxx
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declarations
#include "fb_shader_particles_layout.h"

//--- Registration define
#define ORSHADER_TEMPLATE__LAYOUT		GPUshader_ParticlesLayout

const char *gGenerationPropNames [ePropertyCount] = {
	
	"Emitter",			// emitter type

	"Separator 1",

	"Play Mode",
	"Use Custom Range",
	"Emit Start",
	"Emit Stop",

	"Separator 2",

	"Reset Time",
	"Reset Quantity",

	"Use Rate",
	"Particle Rate",
	"Use PreGenerated Particles",

	"Next Column",

	"Emit Direction",
	"Dir Spread Latitude",
	"Dir Spread Longitude",
	"Use Emitter Normals As Dir",

	"Emit Speed",
	"Emit Speed Spread",
	"Inherit Emitter Speed",

	"Life Time",
	"Life Time Variation"

};

const char *gDynamicPropNames [ePropDynamicCount] = {

	"Mass",
	"Damping",

	"Use Gravity",
	"Gravity",

	"Use Floor",
	"Floor Friction",
	"Floor Level",

	"Next Column 1",

	"Use Forces",
	"Forces",

	"Use Collisions",
	"Collisions",

	"Self Collisions",

	"Next Column 2",

	"Use Turbulence",
	"Noise Frequency",
	"Noise Speed",
	"Noise Amplitude"

};

const char *gShadingPropNames [ePropShadingCount] = {

	"Inherit Emitter Colors",

	"Point Smooth",
	"Point Falloff",
	"Primitive Type",

	"Instance Object",
	"Texture Object",

	"Shade Mode",
	"Affecting Lights",
	"Transparency",
	"Transparency Factor",

	"Color",
	"Color Variation (%)",
	"Use Color Curve",
	"Color Curve",

	"Size",
	"Size Variation (%)",

	"Min Point Scale",
	"Max Point Scale",
	"Point Scale Distance",

	"Use Size Curve",
	"Size Curve"
};

const char *gCommonPropNames [ePropCommonCount] = {

	"Maximum Particles",
	"Iterations",
	"Delta Time Limit",
	"Adaptive SubSteps",
	"SubSteps"

};

//--- FiLMBOX implementation and registration
FBShaderLayoutImplementation(	ORSHADER_TEMPLATE__LAYOUT		);
FBRegisterShaderLayout		(	ORSHADER_TEMPLATE__LAYOUT,
								ORSHADER_TEMPLATE__DESCSTR,
								FB_DEFAULT_SDK_ICON				);	// Icon filename (default=Open Reality icon)

/************************************************
 *	FiLMBOX constructor.
 ************************************************/
bool GPUshader_ParticlesLayout::FBCreate()
{
	

	UICreate	();
	UIConfigure	();
	UIReset		();

	OnResize.Add(this, (FBCallback) &GPUshader_ParticlesLayout::OnResizeCallback );

	return true;
}


/************************************************
 *	FiLMBOX destructor.
 ************************************************/
void GPUshader_ParticlesLayout::FBDestroy()
{
}


/************************************************
 *	Create the UI.
 ************************************************/
void GPUshader_ParticlesLayout::UICreate()
{
	int lS=10;
	int lH=25;
	int lW=140;

	AddRegion( "ButtonActive", "ButtonActive",
			lS, kFBAttachLeft,	"",	1.0,
			lS,	kFBAttachNone,	"",	1.0,
			lW,	kFBAttachNone,	"",	1.0,
			lH,	kFBAttachNone,"", 1.0 );
	SetControl( "ButtonActive", mButtonActive );

	AddRegion( "ButtonReset", "ButtonReset",
			lS*3, kFBAttachRight,	"ButtonActive",	1.0,
			lS,	kFBAttachNone,	"",	1.0,
			lW,	kFBAttachNone,	"",	1.0,
			lH,	kFBAttachNone,"", 1.0 );
	SetControl( "ButtonReset", mButtonReset );

	AddRegion( "ButtonResetAll", "ButtonResetAll",
			lS*3, kFBAttachRight,	"ButtonReset",	1.0,
			lS,	kFBAttachNone,	"",	1.0,
			lW,	kFBAttachNone,	"",	1.0,
			lH,	kFBAttachNone,"", 1.0 );
	SetControl( "ButtonResetAll", mButtonResetAll );

	AddRegion( "ButtonReload", "ButtonReload",
			lS*3, kFBAttachRight,	"ButtonResetAll",	1.0,
			lS,	kFBAttachNone,	"",	1.0,
			lW,	kFBAttachNone,	"",	1.0,
			lH,	kFBAttachNone,"", 1.0 );
	SetControl( "ButtonReload", mButtonReload );

	AddRegion( "Tabs", "Tabs",
			lS, kFBAttachNone,	"",	1.0,
			lS,	kFBAttachBottom,"ButtonActive",	1.0,
			-lS,	kFBAttachRight,	"",	1.0,
			lH,	kFBAttachNone,"", 1.0 );
	SetControl( "Tabs", mTabs);
	
	AddRegion( "LayoutProperties", "LayoutProperties",
			lS, kFBAttachLeft,	"",	1.0,
			lS,	kFBAttachBottom,"Tabs",	1.0,
			-lS,kFBAttachRight,	"",	1.0,
			-lS,kFBAttachBottom,"", 1.0 );
	SetControl( "LayoutProperties", mLayoutGeneration );
	
	AddRegion( "LayoutPin", "LayoutPin",
		0, kFBAttachWidth, "", 1.0,
		0, kFBAttachHeight, "", 1.0,
		lS, kFBAttachRight, "", 1.0,
		lS, kFBAttachBottom, "", 1.0 );
	
	UICreateProps(mLayoutGeneration, ePropertyCount, gGenerationPropNames, mEditGenerationProps);
	UICreateProps(mLayoutDynamic, ePropDynamicCount, gDynamicPropNames, mEditDynamicProps);
	UICreateProps(mLayoutShading, ePropShadingCount, gShadingPropNames, mEditShadingProps);
	UICreateProps(mLayoutCommon, ePropCommonCount, gCommonPropNames, mEditCommonProps);
}

void GPUshader_ParticlesLayout::UICreateProps(FBLayout &layout, const int count, const char *names[], FBEditProperty *props)
{
	int lL = 4;
	int lS=4;
	int lH=20;
	int lW=300;

	FBString lastItem("");
	FBAttachType lastType = kFBAttachNone;

	//int layoutX, layoutY, layoutwidth, layoutheight;
//	int layoutwidth = 400;
	int layoutheight = 400;
	
	int accH = lS;

	for (int i=0; i<count; ++i)
	{
		// go to next column
		if (accH > layoutheight)
		{
			lL += lW + 30;
			accH = lS;
		}

		layout.AddRegion( names[i], names[i],
			lL,		kFBAttachNone,	"",	1.0,
			accH,	kFBAttachNone,	"",	1.0,
			lW, kFBAttachNone,	"",	1.0,
			lH,	kFBAttachNone,	"", 1.0 );
		layout.SetControl( names[i], props[i] );

		lastItem = names[i];
		lastType = kFBAttachBottom;

		accH += lH + lS * 2;
	}
}

/************************************************
 *	Configure the UI.
 ************************************************/
void GPUshader_ParticlesLayout::UIConfigure()
{
	//mButtonTest.OnClick.Add( this, (FBCallback)&GPUshader_ParticlesLayout::OnButtonTestClick );

	GPUshader_Particles	*pShader = (GPUshader_Particles*) (FBShader*) Shader;

	mButtonActive.Property = &pShader->Enable;
	mButtonActive.Caption = "Enable";

	mButtonReset.Property = &pShader->Reset;
	mButtonReset.Caption = "Reset";
	mButtonReset.CaptionSize = 0;

	mButtonResetAll.Property = &pShader->ResetAll;
	mButtonResetAll.Caption = "Reset All";
	mButtonResetAll.CaptionSize = 0;

	mButtonReload.Property = pShader->PropertyList.Find("Reload Shader");
	mButtonReload.Caption = "Reload Shader";
	mButtonReload.CaptionSize = 0;

	mTabs.Items.SetString("Generation~Dynamic~Shading~Common");
	mTabs.Layout = &mLayoutGeneration;

	mTabs.OnChange.Add(this, (FBCallback) &GPUshader_ParticlesLayout::OnTabChange);

	UIConfigureProps(mLayoutGeneration, ePropertyCount, gGenerationPropNames, mEditGenerationProps);
	UIConfigureProps(mLayoutDynamic, ePropDynamicCount, gDynamicPropNames, mEditDynamicProps);
	UIConfigureProps(mLayoutShading, ePropShadingCount, gShadingPropNames, mEditShadingProps);
	UIConfigureProps(mLayoutCommon, ePropCommonCount, gCommonPropNames, mEditCommonProps);
}

void GPUshader_ParticlesLayout::UIConfigureProps(FBLayout &layout, const int count, const char *names[], FBEditProperty *props)
{
	GPUshader_Particles	*pShader = (GPUshader_Particles*) (FBShader*) Shader;

	for (int i=0; i<count; ++i)
	{
		//if (i==17) break;
		props[i].Property = pShader->PropertyList.Find(names[i]);
		props[i].Caption = names[i];
	}
}


/************************************************
 *	Reset the UI from the device.
 ************************************************/
void GPUshader_ParticlesLayout::UIReset()
{
	//mButtonTest.Caption = "GPU Particles (Beta)";
	UIResetGeneration(mLayoutGeneration);
}

void GPUshader_ParticlesLayout::UIResetGeneration(FBLayout &layout)
{

}

/************************************************
 *	Button clicked.
 ************************************************/
void GPUshader_ParticlesLayout::OnButtonTestClick(HISender pSender, HKEvent pEvent)
{
	//mButtonTest.Caption = "Clicked";
}

void GPUshader_ParticlesLayout::OnTabChange(HISender pSender, HKEvent pEvent)
{
	int currentTab = mTabs.ItemIndex;

	switch (currentTab)
	{
	case 1:
		SetControl( "LayoutProperties", mLayoutDynamic );
		break;
	case 2:
		SetControl( "LayoutProperties", mLayoutShading );
		break;
	case 3:
		SetControl( "LayoutProperties", mLayoutCommon );
		break;
	default:
		SetControl( "LayoutProperties", mLayoutGeneration );
	}
}

void GPUshader_ParticlesLayout::OnResizeCallback(HISender pSender, HKEvent pEvent)
{
	FBEventResize	levent(pEvent);
	
	mLayoutWidth = levent.Width;
	mLayoutHeight = levent.Height - 120;

	UIResizeProps(mLayoutGeneration, ePropertyCount, gGenerationPropNames, mEditGenerationProps );
	UIResizeProps(mLayoutDynamic, ePropDynamicCount, gDynamicPropNames, mEditDynamicProps );
	UIResizeProps(mLayoutShading, ePropShadingCount, gShadingPropNames, mEditShadingProps);
	UIResizeProps(mLayoutCommon, ePropCommonCount, gCommonPropNames, mEditCommonProps);
}


void GPUshader_ParticlesLayout::UIResizeProps(FBLayout &layout, const int count, const char *names[], FBEditProperty *props)
{
	int lL = 4;
	int lS=4;
	int lH=20;
	int lW=300;

	// compute number of columns

//	const int layoutwidth = (mLayoutWidth > 0) ? mLayoutWidth : 400;
	const int layoutheight = (mLayoutHeight > 0) ? mLayoutHeight : 400;

	int accH = lS * 2 + lH;

	for (int i=0; i<count; ++i)
	{
		// go to next column
		if (accH > layoutheight)
		{
			lL += lW + 30;
			accH = lS;
		}

		layout.MoveRegion( names[i], lL, accH );

		accH += lH + lS * 2;
	}
}