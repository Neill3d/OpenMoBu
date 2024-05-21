
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BlendShapeToolkit_manip_layout.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declarations
#include "BlendShapeToolkit_manip_layout.h"
#include "BlendShapeToolkit_Helper.h"

//--- Registration defines
#define ORTOOLBRUSH__CLASS	ORTOOLBRUSH__CLASSNAME
#define ORTOOLBRUSH__LABEL	"Sculpt Brush Tool"
#define ORTOOLBRUSH__DESC	"Sculpt Brush Tool"

//--- FiLMBOX implementation and registration
FBToolImplementation(	ORTOOLBRUSH__CLASS	);
FBRegisterTool		(	ORTOOLBRUSH__CLASS,
						ORTOOLBRUSH__LABEL,
						ORTOOLBRUSH__DESC,
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ORManip_Sculpt_Tool::FBCreate()
{
	
	// Affect the handle on the manipulator.
	mSystem.Scene->OnChange.Add( this, (FBCallback) &ORManip_Sculpt_Tool::EventSceneChange );

	mManipulator = nullptr;
	for (int i=0; i<mSystem.Manipulators.GetCount(); ++i)
		if (mSystem.Manipulators[i]->Name == "Sculpting brush")
		{
			mManipulator = (ORManip_Sculpt *) mSystem.Manipulators[i];
		}

	if (mManipulator)
	{
		// Create & configure the UI
		UICreate	();
		UIConfigure	();
		UIReset		();

		StartSize[0] = 300;
		StartSize[1] = 610;

		OnShow.Add( this, (FBCallback) &ORManip_Sculpt_Tool::EventUIShow );
		OnIdle.Add( this, (FBCallback) &ORManip_Sculpt_Tool::EventUIIdle );

		mNeedUpdate = false;

		return true;
	}

	return false;
}


/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void ORManip_Sculpt_Tool::FBDestroy()
{
	mManipulator->OnDataUpdate.clear();
	mSystem.Scene->OnChange.Remove( this, (FBCallback) &ORManip_Sculpt_Tool::EventSceneChange );
	SetControl( "Canvas", mLayoutCommon );
}


/************************************************
 *	Create the UI.
 ************************************************/
void ORManip_Sculpt_Tool::UICreate()
{
	int lB = 10;
	int	lS = 4;
	int lH = 18;
	int lW = 64;

	// Add regions
	AddRegion( "ButtonActive",		"ButtonActive",
									lB,		kFBAttachLeft,	"",						1.0,
									lB,		kFBAttachNone,	"",						1.0,
									lW,		kFBAttachNone,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );
	AddRegion( "ButtonDeformer",		"ButtonDeformer",
									lB,		kFBAttachRight,	"ButtonActive",			1.0,
									lB,		kFBAttachNone,	"",						1.0,
									lW+10,	kFBAttachNone,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );
	AddRegion( "LabelModelName",		"LabelModelName",
									lB,		kFBAttachRight,		"ButtonDeformer",	1.0,
									lB,		kFBAttachNone,		"",					1.0,
									lW*2,	kFBAttachNone,		"",					1.0,
									lH,		kFBAttachNone,		"",					1.0 );

	AddRegion( "LabelList",		"LabelList",
									lB,		kFBAttachLeft,	"",						1.0,
									lB,		kFBAttachBottom,"ButtonActive",			1.0,
									0,		kFBAttachWidth,	"",						0.3,
									lH,		kFBAttachNone,	"",						1.0 );

	AddRegion( "ListConstraints",		"ListConstraints",
									lS,		kFBAttachRight,	"LabelList",			1.0,
									lB,		kFBAttachBottom,"ButtonActive",			1.0,
									0,		kFBAttachWidth,	"",						0.4,
									lH,		kFBAttachNone,	"",						1.0 );
	AddRegion( "ButtonSelectConstraint",	"ButtonSelectConstraint",
									lS,		kFBAttachRight,	"ListConstraints",		1.0,
									lB,		kFBAttachBottom,"ButtonActive",			1.0,
									lH,		kFBAttachNone,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );
	AddRegion( "ButtonRenameConstraint",	"ButtonRenameConstraint",
									lS,		kFBAttachRight,	"ButtonSelectConstraint",1.0,
									lB,		kFBAttachBottom,"ButtonActive",			1.0,
									lH,		kFBAttachNone,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );
	AddRegion( "ButtonRemoveConstraint",	"ButtonRemoveConstraint",
									lS,		kFBAttachRight,	"ButtonRenameConstraint",1.0,
									lB,		kFBAttachBottom,"ButtonActive",			1.0,
									lH,		kFBAttachNone,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );
	AddRegion( "EditExclusiveMode",	"EditExclusiveMode",
									lS,		kFBAttachLeft,	"",1.0,
									lB,		kFBAttachBottom,"LabelList",			1.0,
									0,		kFBAttachWidth,	"",						0.5,
									lH,		kFBAttachNone,	"",						1.0 );
	/*
	AddRegion( "ArrowFalloff",		"ArrowFalloff",
									lB,		kFBAttachLeft,		"",						1.0,
									-3*lB-lW,	kFBAttachBottom,	"",						1.0,
									0,		kFBAttachNone,		"",						1.0,
									0,		kFBAttachBottom,	"",						1.0 );
	*/
	AddRegion( "ArrowFalloff",		"ArrowFalloff",
									lB,		kFBAttachLeft,		"",						1.0,
									lB,		kFBAttachBottom,	"EditExclusiveMode",	1.0,
									0,		kFBAttachNone,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );

	//
	AddRegion( "LabelContainer",	"LabelContainer",
									lB,		kFBAttachLeft,		"",						1.0,
									lB,		kFBAttachBottom,	"ArrowFalloff",			1.0,
									lW,		kFBAttachNone,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );
	AddRegion( "Container",		"Container",
									lB,		kFBAttachLeft,		"",						1.0,
									lB,		kFBAttachBottom,	"LabelContainer",		1.0,
									lW,		kFBAttachNone,		"",						1.0,
									-lB,	kFBAttachBottom,	"",		1.0 );
	AddRegion( "TabPanel", "TabPanel",
									lB,		kFBAttachRight,		"Container",			1.0,
									lB,		kFBAttachBottom,	"ArrowFalloff",			1.0,
									-lB,	kFBAttachRight,		"",			1.0,
									lH,		kFBAttachNone,		"",			1.0 );
	AddRegion( "DeformerCanvas", "",
									0,		kFBAttachLeft,		"TabPanel",		1.0,
									-140,	kFBAttachBottom,	"",		1.0,
									-lB,	kFBAttachRight,		"",		1.0,
									-lB,	kFBAttachBottom,	"",		1.0 );
	AddRegion( "Canvas", "Canvas",
									0,		kFBAttachLeft,		"TabPanel",		1.0,
									0,		kFBAttachBottom,	"TabPanel",		1.0,
									-lB,	kFBAttachRight,		"",		1.0,
									-lB,	kFBAttachTop,		"DeformerCanvas",		1.0 );
	
	

	// Assign regions
	SetControl( "ButtonActive", mButtonActive );
	SetControl( "ButtonDeformer", mEditDeformConstraintMode );
	SetControl( "LabelModelName", mLabelModelName );
	
	SetControl( "LabelList", mLabelConstraintList );
	SetControl( "ListConstraints", mListConstraints );
	SetControl( "ButtonSelectConstraint", mButtonSelectConstraint );
	SetControl( "ButtonRenameConstraint", mButtonRenameConstraint );
	SetControl( "ButtonRemoveConstraint", mButtonRemoveConstraint );
	SetControl( "EditExclusiveMode", mEditExclusiveMode );

	SetControl( "LabelContainer", mLabelBrushes );
	SetControl( "Container", mContainerBrushes );
	SetControl( "TabPanel", mTabPanel );
	SetControl( "Canvas", mLayoutCommon );

	SetBorder( "DeformerCanvas", kFBStandardBorder, true, true, 1, 0, 90.0f, 0 );
	SetControl( "DeformerCanvas", mLayoutDeformer );

	//
	
	mLayoutFalloffs.AddRegion( "Falloffs",		"Falloffs",
									lB,		kFBAttachLeft,		"",						1.0,
									lB,		kFBAttachTop,		"",						1.0,
									-lB,	kFBAttachRight,		"",						1.0,
									-lB,	kFBAttachBottom,	"",						1.0 );

	mLayoutFalloffs.SetControl( "Falloffs", mContainerFalloffs );

	SetControl( "ArrowFalloff", mArrowFalloffs );
	mArrowFalloffs.SetContent( "Falloff Presets", &mLayoutFalloffs, 270, 30+lW );
	



	//
	//
	//
	UICreateCommon();
	UICreateOptions();
	UICreateDeformer();
}

void ORManip_Sculpt_Tool::UICreateCommon()
{
	int lB = 10;
	//int	lS = 4;
	int lH = 18;
	//int lW = 40;

	// Add regions
	mLayoutCommon.AddRegion( "EditColor",		"EditColor",
									lB,		kFBAttachLeft,		"",						1.0,
									lB,		kFBAttachTop,		"",						1.0,
									-lB,	kFBAttachRight,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );
	mLayoutCommon.AddRegion( "EditRadius",		"EditRadius",
									lB,		kFBAttachLeft,		"",						1.0,
									lB,		kFBAttachBottom,	"EditColor",			1.0,
									-lB,	kFBAttachRight,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );
	mLayoutCommon.AddRegion( "EditStrength",		"EditStrength",
									lB,		kFBAttachLeft,		"",						1.0,
									lB,		kFBAttachBottom,	"EditRadius",			1.0,
									-lB,	kFBAttachRight,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );
	
	mLayoutCommon.AddRegion( "Inverted",		"Inverted",
									lB,		kFBAttachLeft,		"",						1.0,
									lB,	kFBAttachBottom,		"EditStrength",			1.0,
									-lB,	kFBAttachRight,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );


	mLayoutCommon.AddRegion( "DirectionMode",		"DirectionMode",
									lB,		kFBAttachLeft,		"",						1.0,
									2*lB,	kFBAttachBottom,	"Inverted",				1.0,
									-lB,	kFBAttachRight,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );
	mLayoutCommon.AddRegion( "AffectMode",		"AffectMode",
									lB,		kFBAttachLeft,		"",						1.0,
									lB,		kFBAttachBottom,	"DirectionMode",		1.0,
									-lB,	kFBAttachRight,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );
	mLayoutCommon.AddRegion( "ScreenInfluence",		"ScreenInfluence",
									lB,		kFBAttachLeft,		"",						1.0,
									lB,		kFBAttachBottom,	"AffectMode",			1.0,
									-lB,	kFBAttachRight,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );
	mLayoutCommon.AddRegion( "ScreenInfluenceRadius",		"ScreenInfluenceRadius",
									lB,		kFBAttachLeft,		"",						1.0,
									lB,		kFBAttachBottom,	"ScreenInfluence",		1.0,
									-lB,	kFBAttachRight,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );

	//
	mLayoutCommon.AddRegion( "ButtonFreezeAll",		"ButtonFreezeAll",
									lB,		kFBAttachLeft,		"",						1.0,
									2*lB,	kFBAttachBottom,	"ScreenInfluenceRadius",1.0,
									-lB,	kFBAttachWidth,		"",						0.33,
									lH,		kFBAttachNone,		"",						1.0 );
	mLayoutCommon.AddRegion( "ButtonFreezeInvert",		"ButtonFreezeInvert",
									lB,		kFBAttachRight,		"ButtonFreezeAll",		1.0,
									2*lB,	kFBAttachBottom,	"ScreenInfluenceRadius",1.0,
									-lB,	kFBAttachWidth,		"",						0.33,
									lH,		kFBAttachNone,		"",						1.0 );
	mLayoutCommon.AddRegion( "ButtonFreezeNone",		"ButtonFreezeNone",
									lB,		kFBAttachRight,		"ButtonFreezeInvert",	1.0,
									2*lB,	kFBAttachBottom,	"ScreenInfluenceRadius",1.0,
									-lB,	kFBAttachRight,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );

	mLayoutCommon.AddRegion( "ButtonFill",		"ButtonFill",
									lB,		kFBAttachLeft,		"",						1.0,
									lB,		kFBAttachBottom,	"ButtonFreezeNone",		1.0,
									55,		kFBAttachNone,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );
	mLayoutCommon.AddRegion( "ButtonReset",		"ButtonReset",
									lB,		kFBAttachRight,		"ButtonFill",			1.0,
									lB,		kFBAttachBottom,	"ButtonFreezeNone",		1.0,
									55,		kFBAttachNone,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );

	mLayoutCommon.AddRegion( "ButtonZeroAll",		"ButtonZeroAll",
									lB,		kFBAttachRight,		"ButtonReset",			1.0,
									lB,		kFBAttachBottom,	"ButtonFreezeNone",		1.0,
									55,		kFBAttachNone,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );

	// Assign regions
	mLayoutCommon.SetControl( "EditColor",			mEditColor );
	mLayoutCommon.SetControl( "EditRadius",			mEditRadius );
	mLayoutCommon.SetControl( "EditStrength",		mEditStrength );
	mLayoutCommon.SetControl( "Inverted",			mEditInverted );
	
	mLayoutCommon.SetControl( "DirectionMode",		mEditBrushDirection );
	mLayoutCommon.SetControl( "AffectMode",			mEditAffectMode );
	mLayoutCommon.SetControl( "ScreenInfluence",	mEditScreenInfluence );
	mLayoutCommon.SetControl( "ScreenInfluenceRadius",	mEditScrInfRadius );
	
	mLayoutCommon.SetControl( "ButtonFreezeAll",				mButtonFreezeAll );
	mLayoutCommon.SetControl( "ButtonFreezeInvert",				mButtonFreezeInvert );
	mLayoutCommon.SetControl( "ButtonFreezeNone",				mButtonFreezeNone );
	mLayoutCommon.SetControl( "ButtonFill",						mButtonFill );
	mLayoutCommon.SetControl( "ButtonReset",					mButtonResetChanges );
	mLayoutCommon.SetControl( "ButtonZeroAll",					mButtonZeroAll );
}

void ORManip_Sculpt_Tool::UICreateOptions()
{
	const int lB = 10;
	//const int lS = 4;
	const int lH = 18;
	//const int lW = 40;

	mLayoutOptions.AddRegion( "RadiusSens",		"RadiusSens",
									lB,		kFBAttachLeft,		"",						1.0,
									lB,		kFBAttachTop,		"",						1.0,
									-lB,	kFBAttachRight,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );

	mLayoutOptions.AddRegion( "StrengthSens",		"StrengthSens",
									lB,		kFBAttachLeft,		"",						1.0,
									lB,		kFBAttachBottom,	"RadiusSens",			1.0,
									-lB,	kFBAttachRight,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );

	mLayoutOptions.AddRegion( "BrushColor",		"BrushColor",
									lB,		kFBAttachLeft,		"",						1.0,
									2*lB,	kFBAttachBottom,	"StrengthSens",			1.0,
									-lB,	kFBAttachRight,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );

	
	mLayoutOptions.AddRegion( "Device",		"Device",
									lB,		kFBAttachLeft,		"",						1.0,
									3*lB,	kFBAttachBottom,	"BrushColor",			1.0,
									-lB,	kFBAttachRight,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );

	mLayoutOptions.AddRegion( "regionDevice",		"Wacom Device:",
									-2,		kFBAttachLeft,		"Device",				1.0,
									-8,		kFBAttachTop,		"Device",				1.0,
									2,		kFBAttachRight,		"Device",				1.0,
									2,		kFBAttachBottom,	"Device",				1.0 );

	//

	mLayoutOptions.SetControl( "RadiusSens",		mEditRadiusSens );
	mLayoutOptions.SetControl( "StrengthSens",		mEditStrengthSens );
	mLayoutOptions.SetControl( "BrushColor",		mEditBrushColor );

	mLayoutOptions.SetBorder( "regionDevice", kFBStandardBorder, true, true, 1, 0, 90.0f, 0 ); 
	mLayoutOptions.SetControl( "Device",	mEditUseTablet );
}

void ORManip_Sculpt_Tool::UICreateDeformer()
{
	const int lB = 10;
	//const int lS = 4;
	const int lH = 18;
	//const int lW = 64;

	mLayoutDeformer.AddRegion( "autoBlendshape",		"autoBlendshape",
									lB,		kFBAttachLeft,		"",					1.0,
									lB,		kFBAttachNone,		"",					1.0,
									100,	kFBAttachNone,		"",					1.0,
									lH,		kFBAttachNone,		"",					1.0 );

	mLayoutDeformer.AddRegion( "setBlendshape",		"setBlendshape",
									lB,		kFBAttachRight,		"autoBlendshape",	1.0,
									lB,		kFBAttachNone,		"",					1.0,
									-lB,	kFBAttachRight,		"",					1.0,
									lH,		kFBAttachNone,		"",					1.0 );
	mLayoutDeformer.AddRegion( "overrideBlendshape",		"overrideBlendshape",
									lB,		kFBAttachLeft,		"",					1.0,
									lB,		kFBAttachBottom,	"autoBlendshape",	1.0,
									-lB,	kFBAttachRight,		"",					1.0,
									lH,		kFBAttachNone,		"",					1.0 );


	mLayoutDeformer.AddRegion( "autoKeyframe",		"autoKeyframe",
									lB,		kFBAttachLeft,		"",					1.0,
									lB,		kFBAttachBottom,	"overrideBlendshape",	1.0,
									100,	kFBAttachNone,		"",					1.0,
									lH,		kFBAttachNone,		"",					1.0 );

	mLayoutDeformer.AddRegion( "setKeyframe",		"setKeyframe",
									lB,		kFBAttachRight,		"autoKeyframe",		1.0,
									lB,		kFBAttachBottom,	"overrideBlendshape",	1.0,
									-lB,	kFBAttachRight,		"",					1.0,
									lH,		kFBAttachNone,		"",					1.0 );

	mLayoutDeformer.AddRegion( "labelKeyframeIn",		"labelKeyframeIn",
									lB+15,	kFBAttachLeft,		"",					1.0,
									lB,		kFBAttachBottom,	"autoKeyframe",		1.0,
									0,		kFBAttachWidth,		"",					0.25,
									lH,		kFBAttachNone,		"",					1.0 );
	mLayoutDeformer.AddRegion( "labelKeyframeLen",		"labelKeyframeLen",
									lB,		kFBAttachRight,		"labelKeyframeIn",	1.0,
									lB,		kFBAttachBottom,	"autoKeyframe",		1.0,
									0,		kFBAttachWidth,		"",					0.25,
									lH,		kFBAttachNone,		"",					1.0 );
	mLayoutDeformer.AddRegion( "labelKeyframeOut",		"labelKeyframeOut",
									lB,		kFBAttachRight,		"labelKeyframeLen",	1.0,
									lB,		kFBAttachBottom,	"autoKeyframe",		1.0,
									-lB,	kFBAttachRight,		"",					1.0,
									lH,		kFBAttachNone,		"",					1.0 );

	mLayoutDeformer.AddRegion( "useKeyRange",		"useKeyRange",
									5,		kFBAttachLeft,		"",					1.0,
									lB,		kFBAttachBottom,	"labelKeyframeIn",	1.0,
									15,		kFBAttachNone,		"",					1.0,
									lH,		kFBAttachNone,		"",					1.0 );
	mLayoutDeformer.AddRegion( "keyframeIn",		"keyframeIn",
									5,		kFBAttachRight,		"useKeyRange",		1.0,
									lB,		kFBAttachBottom,	"labelKeyframeIn",	1.0,
									0,		kFBAttachRight,		"labelKeyframeIn",	1.0,
									lH,		kFBAttachNone,		"",					1.0 );
	mLayoutDeformer.AddRegion( "keyframeLen",		"keyframeLen",
									lB,		kFBAttachRight,		"labelKeyframeIn",	1.0,
									lB,		kFBAttachBottom,	"labelKeyframeLen",	1.0,
									0,		kFBAttachRight,		"labelKeyframeLen",	1.0,
									lH,		kFBAttachNone,		"",					1.0 );
	mLayoutDeformer.AddRegion( "keyframeOut",		"keyframeOut",
									lB,		kFBAttachRight,		"labelKeyframeLen",	1.0,
									lB,		kFBAttachBottom,	"labelKeyframeOut",	1.0,
									0,		kFBAttachRight,		"labelKeyframeOut",	1.0,
									lH,		kFBAttachNone,		"",					1.0 );
	
	//
	//mLayoutDeformer.SetControl( "Changes", mLabelChanges );
	//mLayoutDeformer.SetControl( "ResetChanges", mButtonResetChanges );
	mLayoutDeformer.SetControl( "autoBlendshape", mEditAutoBlendShape );
	mLayoutDeformer.SetControl( "setBlendshape", mButtonSetBlendShape );
	mLayoutDeformer.SetControl( "overrideBlendshape", mButtonOverrideExisting );

	mLayoutDeformer.SetControl( "autoKeyframe", mEditAutoKeyframe );
	mLayoutDeformer.SetControl( "setKeyframe", mButtonSetKeyframe );

	mLayoutDeformer.SetControl( "useKeyRange", mButtonUseKeyRange );

	mLayoutDeformer.SetControl( "labelKeyframeIn", mLabelKeyframeIn);
	mLayoutDeformer.SetControl( "labelKeyframeLen", mLabelKeyframeLen );
	mLayoutDeformer.SetControl( "labelKeyframeOut", mLabelKeyframeOut );
	mLayoutDeformer.SetControl( "keyframeIn", mEditKeyframeIn );
	mLayoutDeformer.SetControl( "keyframeLen", mEditKeyframeLen );
	mLayoutDeformer.SetControl( "keyframeOut", mEditKeyframeOut );
}

/************************************************
 *	Configure the UI.
 ************************************************/
void ORManip_Sculpt_Tool::UIConfigure()
{
	auto brushManager = mManipulator->GetBrushManagerPtr();

	mButtonActive.Caption = "Active";
	mButtonActive.Style.SetPropertyValue(kFB2States);
	mButtonActive.State = 0;
	mButtonActive.OnClick.Add( this, (FBCallback) &ORManip_Sculpt_Tool::EventButtonActiveClick );

	mEditDeformConstraintMode.Caption = "Constraint";
	mEditDeformConstraintMode.Property = &mManipulator->UseConstraint;
	/*
	mButtonDeformer.Caption = "Constraint";
	mButtonDeformer.Style = kFB2States;
	mButtonDeformer.SetStateColor( kFBButtonState0, FBColor(1.0, 0.0, 0.0) );
	mButtonDeformer.SetStateColor( kFBButtonState1, FBColor(0.0, 1.0, 0.0) );
	mButtonDeformer.State = 0;
	mButtonDeformer.Enabled = true;
	//mButtonDeformer.OnClick.Add( this, (FBCallback) &ORManip_Sculpt_Tool::EventButtonDeformerClick );
	*/

	mLabelModelName.Caption = "None";

	mLabelConstraintList.Caption = "Choose a constraint";

	mListConstraints.Style.SetPropertyValue(kFBDropDownList);
	mListConstraints.OnChange.Add( this, (FBCallback) &ORManip_Sculpt_Tool::EventListConstraintChange );
	mEditExclusiveMode.Caption = "Exclusive Mode";
	mEditExclusiveMode.Property = nullptr;
	UpdateConstraintList();

	mButtonSelectConstraint.Caption = "S";
	mButtonSelectConstraint.Hint = "Select a constraint from the listbox";
	mButtonSelectConstraint.OnClick.Add( this, (FBCallback) &ORManip_Sculpt_Tool::EventButtonConstraintClick );
	mButtonRenameConstraint.Caption = "Rn";
	mButtonRenameConstraint.Hint = "Rename a constraint from the listbox";
	mButtonRenameConstraint.OnClick.Add( this, (FBCallback) &ORManip_Sculpt_Tool::EventButtonConstraintClick );
	mButtonRemoveConstraint.Caption = "Rm";
	mButtonRemoveConstraint.Hint = "Remove a constraint from the listbox";
	mButtonRemoveConstraint.OnClick.Add( this, (FBCallback) &ORManip_Sculpt_Tool::EventButtonConstraintClick );

	// Configure elements
	
	mContainerFalloffs.Orientation.SetPropertyValue(kFBHorizontal);
	mContainerFalloffs.ItemHeight = 60;
	mContainerFalloffs.ItemWidth = 60;
	mContainerFalloffs.IconPosition.SetPropertyValue(kFBIconTop);
	//mContainerFalloffs.Items.SetString( "Smooth~Linear~FastIn~FastOut~Top" );
	mContainerFalloffs.OnChange.Add( this, (FBCallback) &ORManip_Sculpt_Tool::EventFalloffChange );

	for (int i=0; i<brushManager->GetNumberOfFalloffs(); ++i)
	{
		auto falloff = brushManager->GetFalloffPtr(i);
		mContainerFalloffs.Items.Add( falloff->GetCaption(), (kReference) falloff );
		mContainerFalloffs.ItemIconSet( (kReference) falloff, falloff->GetPicturePath() );
	}

	mLabelBrushes.Caption = "Brushes";
	mContainerBrushes.Orientation.SetPropertyValue(kFBVertical);
	mContainerBrushes.ItemHeight = 60;
	mContainerBrushes.ItemWidth = 60;
	mContainerBrushes.IconPosition.SetPropertyValue(kFBIconTop);
	//mContainerBrushes.Items.SetString( "Move~Push~Pull~Smooth" );
	mContainerBrushes.OnChange.Add( this, (FBCallback) &ORManip_Sculpt_Tool::EventBrushChange );

	for (int i=0; i<brushManager->GetNumberOfBrushes(); ++i)
	{
		auto brush = brushManager->GetBrushPtr(i);
		mContainerBrushes.Items.Add( brush->GetCaption(), (kReference) brush );
		mContainerBrushes.ItemIconSet( (kReference) brush, brush->GetPicturePath() );
	}

	mTabPanel.Items.SetString( "Common~Brush~Options" );

	// Add callbacks
	mTabPanel.OnChange.Add( this, (FBCallback) &ORManip_Sculpt_Tool::EventTabPanelChange );

	//
	//
	//
	UIConfigureCommon();
	UIConfigureOptions();
	UIConfigDeformer();
}

void ORManip_Sculpt_Tool::UIConfigureCommon()
{
	mEditColor.Caption = "Color";
	mEditColor.Property = &mManipulator->Color;
	mEditRadius.Caption = "Radius";
	mEditRadius.Property = &mManipulator->Radius;
	mEditStrength.Caption = "Strength";
	mEditStrength.Property = &mManipulator->Strength;
	mEditInverted.Caption = "Inverted";
	mEditInverted.Property = &mManipulator->Inverted;

	mEditBrushDirection.Caption = "Direction Mode";
	mEditBrushDirection.Property = &mManipulator->DirectionMode;

	mEditAffectMode.Caption = "Affect Mode";
	mEditAffectMode.Property = &mManipulator->AffectMode;

	mEditScreenInfluence.Caption = "2D Screen Influence mode";
	mEditScreenInfluence.Property = &mManipulator->ScreenInfluence;

	mEditScrInfRadius.Caption = "Screen Influence Radius";
	mEditScrInfRadius.Property = &mManipulator->ScreenInfluenceRadius;

	mButtonFreezeAll.Caption = "Freeze All";
	mButtonFreezeAll.OnClick.Add( this, (FBCallback) &ORManip_Sculpt_Tool::EventButtonFreezeAllClick );
	mButtonFreezeInvert.Caption = "Frz Invert";
	mButtonFreezeInvert.OnClick.Add( this, (FBCallback) &ORManip_Sculpt_Tool::EventButtonFreezeInvertClick );
	mButtonFreezeNone.Caption = "Frz None";
	mButtonFreezeNone.OnClick.Add( this, (FBCallback) &ORManip_Sculpt_Tool::EventButtonFreezeNoneClick );

	mButtonFill.Caption = "Fill";
	mButtonFill.OnClick.Add( this, (FBCallback) &ORManip_Sculpt_Tool::EventButtonFillClick );

	mButtonResetChanges.Caption = "Reset";
	mButtonResetChanges.OnClick.Add( this, (FBCallback) &ORManip_Sculpt_Tool::EventButtonDeformerResetClick );
	
	mButtonZeroAll.Caption = "Zero All";
	mButtonZeroAll.OnClick.Add( this, (FBCallback) &ORManip_Sculpt_Tool::EventButtonDeformerZeroClick );
}

void ORManip_Sculpt_Tool::UIConfigureOptions()
{
	mEditRadiusSens.Caption = "Radius Sensivity";
	mEditRadiusSens.Property = &mManipulator->RadiusSens;
	mEditStrengthSens.Caption = "Strength Sensivity";
	mEditStrengthSens.Property = &mManipulator->StrengthSens;

	mEditBrushColor.Caption = "Brush Color";
	mEditBrushColor.Property = &mManipulator->BrushColor;

	mLabelDevice.Caption = "Wacom Input Device: ";
	/*
	mContainerDevice.Caption = "Input Device";
	mContainerDevice.IconPosition = kFBIconLeft;
	mContainerDevice.OnDblClick.Add( this, (FBCallback) &ORManip_Sculpt_Tool::EventContainerDblClick );
	mContainerDevice.OnDragAndDrop.Add( this, (FBCallback) &ORManip_Sculpt_Tool::EventContainerDragAndDrop );
	*/
	
	mEditUseTablet.Property = &mManipulator->UseTablet;
	mEditUseTablet.Caption = "Use Tablet";
	
	mEditUsePenPressure.Property = &mManipulator->UsePenPressure;
	mEditUsePenPressure.Caption = "Use Pen Pressure";

	mEditUsePenEraser.Property = &mManipulator->UsePenEraser;
	mEditUsePenEraser.Caption = "Use Pen Eraser";

	mEditMaxPenPressure.Property = &mManipulator->MaxPenPressure;
	mEditMaxPenPressure.Caption = "Max Pen Pressure";

	mEditDisplayPressure.Property = &mManipulator->DisplayPressure;
	mEditDisplayPressure.Caption = "Display Pressure";
}

void ORManip_Sculpt_Tool::UIConfigDeformer()
{
	mEditAutoBlendShape.Property = &mManipulator->AutoBlendshape;
	mEditAutoBlendShape.Caption = "Auto Blendshape";

	mButtonSetBlendShape.Caption = "Add Manualy";
	mButtonSetBlendShape.OnClick.Add( this, (FBCallback) &ORManip_Sculpt_Tool::EventButtonDeformerBlendshapeAddClick );

	mButtonOverrideExisting.Caption = "Override Existing";
	mButtonOverrideExisting.Style.SetPropertyValue(kFBCheckbox);
	mButtonOverrideExisting.State = 0;

	mEditAutoKeyframe.Property = &mManipulator->AutoKeyframe;
	mEditAutoKeyframe.Caption = "Auto Keyframe";
	mButtonSetKeyframe.Caption = "Set Manualy";
	mButtonSetKeyframe.Enabled = false;

	mButtonUseKeyRange.Caption = "";
	mButtonUseKeyRange.Style.SetPropertyValue(kFBCheckbox);
	mButtonUseKeyRange.State = 1;
	mButtonUseKeyRange.OnClick.Add( this, (FBCallback) &ORManip_Sculpt_Tool::EventButtonUseKeyframeRangeClick );

	mLabelKeyframeIn.Caption = "In";
	mLabelKeyframeLen.Caption = "Length";
	mLabelKeyframeOut.Caption = "Out";

	mEditKeyframeIn.Property = &mManipulator->KeyframeIn;
	mEditKeyframeIn.Caption = "";
	mEditKeyframeIn.CaptionSize = 0;
	mEditKeyframeLen.Property = &mManipulator->KeyframeLen;
	mEditKeyframeLen.Caption = "";
	mEditKeyframeLen.CaptionSize = 0;
	mEditKeyframeOut.Property = &mManipulator->KeyframeOut;
	mEditKeyframeOut.Caption = "";
	mEditKeyframeOut.CaptionSize = 0;
}

/************************************************
 *	Reset the UI from the manipulator.
 ************************************************/
void ORManip_Sculpt_Tool::UIReset()
{
	mButtonActive.State = (mManipulator && mManipulator->Active) ? 1 : 0;
	//mButtonDeformer.State = (mManipulator && mManipulator->IsModelDeformed()) ? 1 : 0;

	const char *modelName = mManipulator->GetModelName();
	mLabelModelName.Caption = (modelName!=nullptr) ? modelName : "";

	mContainerBrushes.ItemIndex = mManipulator->GetBrushManagerPtr()->GetCurrentBrush();
	mContainerFalloffs.ItemIndex = mManipulator->GetBrushManagerPtr()->GetCurrentFalloff();

	UpdateConstraintList();

	EventTabPanelChange( nullptr, nullptr );
}


/************************************************
 *	Button click callback.
 ************************************************/
void ORManip_Sculpt_Tool::EventTabPanelChange( HISender pSender, HKEvent pEvent )
{
	switch( mTabPanel.ItemIndex )
	{
	case 0:
		SetControl( "Canvas", mLayoutCommon );
		break;
	case 2:
		SetControl( "Canvas", mLayoutOptions );
		break;
	case 1:
		{
			int index = mContainerBrushes.ItemIndex;

			if (index >= 0)
			{
				BaseBrush *pBrush = (BaseBrush*) mContainerBrushes.Items.GetReferenceAt(index);

				SetControl( "Canvas", pBrush->GetLayoutPtr() );
			}
		}break;
	}
}


void ORManip_Sculpt_Tool::EventBrushChange( HISender pSender, HKEvent pEvent )
{
	int index = mContainerBrushes.ItemIndex;

	if (index >= 0)
	{
		mManipulator->SetCurrentBrush(index);

		// update ui
		EventTabPanelChange( nullptr, nullptr );
	}
}

void ORManip_Sculpt_Tool::EventFalloffChange( HISender pSender, HKEvent pEvent )
{
	int index = mContainerFalloffs.ItemIndex;

	if (index >= 0)
	{
		mManipulator->SetCurrentFalloff(index);
	}
}


void ORManip_Sculpt_Tool::EventContainerDblClick( HISender pSender, HKEvent pEvent )
{
	//mContainerDevice.Items.Clear();
	//mManipulator->SetUseTabletPenPressure( mButtonUseDevice.State == 1 );
}

void ORManip_Sculpt_Tool::EventContainerDragAndDrop( HISender pSender, HKEvent pEvent )
{
	/*
    FBEventDragAndDrop lDragAndDrop( pEvent );

    switch( lDragAndDrop.State )
    {
        case kFBDragAndDropDrag:
        {
			FBComponent *pComponent = lDragAndDrop.Get(0);
			if (pComponent && FBIS(pComponent, FBDevice) )
			{
				lDragAndDrop.Accept();
			}
        }
        break;
        case kFBDragAndDropDrop:
        {
			FBDevice *pDevice = (FBDevice*) lDragAndDrop.Get(0);
			if (pDevice)
			{
				mContainerDevice.Items.Clear();
				mContainerDevice.Items.Add( pDevice->Name, (kReference) pDevice );
				mManipulator->AssignInputDevice( pDevice );
			}
        }
        break;
    }
	*/
}

void ORManip_Sculpt_Tool::EventUIShow( HISender pSender, HKEvent pEvent )
{
	FBEventShow lEvent( pEvent );

	if( lEvent.Shown )
	{
		// try to auto assign to the selected mesh
		if (mManipulator && mManipulator->Active == false)
		{
			FBModelList		lList;
			FBGetSelectedModels(lList);

			if (lList.GetCount() > 0)
			{
				SculptMode( lList.GetAt(0), true );
			}
		}

		// Reset the UI here.
		UIReset();

		mManipulator->OnDataUpdate.Bind( this, &ORManip_Sculpt_Tool::EventManipulatorUpdate );
	}
	else
	{
		mManipulator->OnDataUpdate.clear();

		SculptMode( nullptr, false );
	}
	
}

void ORManip_Sculpt_Tool::EventButtonActiveClick( HISender pSender, HKEvent pEvent )
{
	if (mButtonActive.State)
	{
		FBModelList		lList;
		FBGetSelectedModels(lList);

		if (lList.GetCount() > 0)
		{
			SculptMode( lList.GetAt(0), true );
		}
	}
	else
	{
		SculptMode( nullptr, false );
	}

	UIReset();
}

void ORManip_Sculpt_Tool::EventButtonDeformerClick( HISender pSender, HKEvent pEvent )
{
	if (mButtonActive.State)
	{
		FBModelList		lList;
		FBGetSelectedModels(lList);

		if (lList.GetCount() > 0)
		{
			SculptMode( lList.GetAt(0), true );
		}
	}
	else
	{
		SculptMode( nullptr, false );
	}

	UIReset();
}

FBConstraint *ORManip_Sculpt_Tool::GetCurrentConstraint()
{
	FBConstraint *pConstraint = nullptr;

	if (mListConstraints.ItemIndex >= 0)
	{
		pConstraint = (FBConstraint*) mListConstraints.Items.GetReferenceAt(mListConstraints.ItemIndex);
	}

	return pConstraint;
}

void ORManip_Sculpt_Tool::EventButtonConstraintClick( HISender pSender, HKEvent pEvent )
{
	FBComponent *pComponent = FBGetFBComponent( (HIObject) pSender);

	FBConstraint *pConstraint = GetCurrentConstraint();
	if (pConstraint == nullptr)
		return;

	if (pComponent == &mButtonSelectConstraint)
	{
		//pConstraint->Selected = true;	
		pConstraint->HardSelect();
		( (BlendShapeDeformerConstraint*) pConstraint)->SelectShapes(mManipulator->GetModelPtr() );
	}
	else if (pComponent == &mButtonRenameConstraint)
	{
		char strValue[128];
		memset( strValue, 0, sizeof(char) * 128 );

		FBString name = pConstraint->Name.AsString();
		sprintf_s(strValue, 128, "%s\0", static_cast<const char*>(name) );

		if (1 == FBMessageBoxGetUserValue( "Sculpt Tool", "Enter a new constraint name:", strValue, kFBPopupString, "Ok", "Cancel" ) )
		{
			pConstraint->Name = strValue;
		}

		UIReset();
	}
	else if (pComponent == &mButtonRemoveConstraint)
	{
		mEditExclusiveMode.Property = nullptr;
		pConstraint->FBDelete();
		UIReset();
	}
}

void ORManip_Sculpt_Tool::EventButtonDeformerResetClick	( HISender pSender, HKEvent pEvent )
{
	// DONE: reInitialize zero and current buffer from the source data without making a new blendshape
	mManipulator->Reset();

	mSystem.Scene->Evaluate();
	mSystem.Scene->EvaluateDeformations();
}

void ORManip_Sculpt_Tool::EventButtonDeformerZeroClick	( HISender pSender, HKEvent pEvent )
{
	BlendShapeDeformerConstraint *pConstraint = nullptr;

	if (mListConstraints.ItemIndex >= 0)
		pConstraint = (BlendShapeDeformerConstraint*) mListConstraints.Items.GetReferenceAt( mListConstraints.ItemIndex );

	if (pConstraint)
		pConstraint->ZeroAll();

	mSystem.Scene->Evaluate();
	mSystem.Scene->EvaluateDeformations();
}

void ORManip_Sculpt_Tool::EventListConstraintChange( HISender pSender, HKEvent pEvent )
{
	BlendShapeDeformerConstraint *pConstraint = mManipulator->GetCurrentConstraint();

	mEditExclusiveMode.Property = nullptr;

	if (pConstraint)
		pConstraint->DeselectShapes();
	
	pConstraint = nullptr;

	if (mListConstraints.ItemIndex >= 0)
		pConstraint = (BlendShapeDeformerConstraint*) mListConstraints.Items.GetReferenceAt( mListConstraints.ItemIndex );

	mManipulator->SetCurrentConstraint( pConstraint );
	if (pConstraint)
	{
		pConstraint->SelectShapes(mManipulator->GetModelPtr());
		mEditExclusiveMode.Property = &pConstraint->ExclusiveMode;
	}
}

void ORManip_Sculpt_Tool::EventButtonDeformerBlendshapeAddClick ( HISender pSender, HKEvent pEvent )
{
	// DONE: add a new blendshape with all changes applied (and set keyframe if flag is setted up)

	ORManip_Sculpt::GlobalAddBlendShape( mManipulator, mButtonOverrideExisting.State == 1 );

	//
	UpdateConstraintList();
}

void ORManip_Sculpt_Tool::EventButtonUseKeyframeRangeClick ( HISender pSender, HKEvent pEvent )
{
	mManipulator->UseKeyframeRange = (mButtonUseKeyRange.State == 1);
}

void ORManip_Sculpt_Tool::EventSceneChange( HISender pSender, HKEvent pEvent )	
{
	FBEventSceneChange lEvent(pEvent);

	if (lEvent.Type.AsInt() == kFBSceneChangeAttach)
	{
		if (FBIS(lEvent.ChildComponent, FBConstraint) )
			UpdateConstraintList();
	}
	if (lEvent.Type.AsInt() == kFBSceneChangeDetach)
	{
		if (FBIS(lEvent.ChildComponent, FBConstraint) )
			UpdateConstraintList();
		/*
		for (int i=0; i<mContainerDevice.Items.GetCount(); ++i)
		{
			FBComponent *pComp = (FBComponent*) mContainerDevice.Items.GetReferenceAt(i);

			if (lEvent.ChildComponent == pComp)
			{
				mContainerDevice.Items.Clear();
				mManipulator->AssignInputDevice( nullptr );
				break;
			}
		}
		*/
	}
}

void ORManip_Sculpt_Tool::EventManipulatorUpdate()
{
	mNeedUpdate = true;
}

void ORManip_Sculpt_Tool::EventUIIdle( HISender pSender, HKEvent pEvent )	
{
	if (mNeedUpdate)
	{
		UIReset();
		mNeedUpdate = false;
	}
}

void ORManip_Sculpt_Tool::UpdateConstraintList()
{
	mEditExclusiveMode.Property = nullptr;

	mListConstraints.Items.Clear();
	mListConstraints.Items.Add( "Add a new constraint" );

	BlendShapeDeformerConstraint *pCurrentDeformer = mManipulator->GetCurrentConstraint();
	int itemIndex = 0;

	FBScene *pScene = mSystem.Scene;
	const int numberOfConstraints = pScene->Constraints.GetCount();

	FBModel *pModel = mManipulator->GetModelPtr();

	for (int i=0; i<numberOfConstraints; ++i)
		if (FBIS(pScene->Constraints[i], BlendShapeDeformerConstraint) )
		{
			BlendShapeDeformerConstraint *pDeformer = (BlendShapeDeformerConstraint*) pScene->Constraints[i];

			// check if current model is binded
			bool isBinded = false;
			for (int j=0; j<pDeformer->ReferenceGetCount(0); ++j)
				if (pDeformer->ReferenceGet(0, j) == pModel)
				{
					isBinded = true;
					break;
				}

			FBString name(pDeformer->Name);
			mListConstraints.Items.Add( (isBinded) ? FBString(name, " - [BINDED]") : name, (kReference) pDeformer );

			if (pDeformer == pCurrentDeformer)
			{
				itemIndex = mListConstraints.Items.GetCount()-1;	// last added constraint as current
				mManipulator->UseConstraint = true;
				mEditExclusiveMode.Property = &pDeformer->ExclusiveMode;
			}
		}

	mListConstraints.ItemIndex = itemIndex;
}

void ORManip_Sculpt_Tool::EventButtonFreezeAllClick( HISender pSender, HKEvent pEvent )	
{
	if (mManipulator) mManipulator->FreezeAll();
}

void ORManip_Sculpt_Tool::EventButtonFreezeInvertClick( HISender pSender, HKEvent pEvent )	
{
	if (mManipulator) mManipulator->FreezeInvert();
}

void ORManip_Sculpt_Tool::EventButtonFreezeNoneClick( HISender pSender, HKEvent pEvent )	
{
	if (mManipulator) mManipulator->FreezeNone();
}

void ORManip_Sculpt_Tool::EventButtonFillClick( HISender pSender, HKEvent pEvent )
{
	if (mManipulator) mManipulator->Fill();
}