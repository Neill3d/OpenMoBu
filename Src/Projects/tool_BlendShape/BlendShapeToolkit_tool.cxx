
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BlendShapeToolkit_tool.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "BlendShapeToolkit_tool.h"
#include "BlendShapeToolkit_manip.h"
#include "BlendShapeToolkit_Helper.h"
#include "BlendShapeToolkit_applymanagerrule.h"

// MCL
#include "math3d.h"
#include "ClusterAdvance.h"
#include "GeometryUtils.h"

#include "tinyxml.h"

#include "resource.h"
#include "ResourceUtils.h"

//--- Registration defines
#define ORTOOLTEMPLATE__CLASS	ORTOOLTEMPLATE__CLASSNAME
#define ORTOOLTEMPLATE__LABEL	"BlendShape Manager"
#define ORTOOLTEMPLATE__DESC	"Tool for managing models blendshapes"

#define UI_BLENDSHAPE_CAPTION_SIZE		150

//--- FiLMBOX implementation and registration
FBToolImplementation(	ORTOOLTEMPLATE__CLASS	);
FBRegisterTool		(	ORTOOLTEMPLATE__CLASS,
						ORTOOLTEMPLATE__LABEL,
						ORTOOLTEMPLATE__DESC,
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ORTool_BlendShape::FBCreate()
{
	// Tool options
	StartSize[0] = 600;
	StartSize[1] = 830;

    UICreate();
	UICreateScrollBox();
	UICreatePanel();
	UIConfig();
	UIConfigScrollBox();
	UIConfigPanel();

	// Add tool callbacks
	OnShow.Add	( this, (FBCallback) &ORTool_BlendShape::EventToolShow		);
	OnIdle.Add	( this, (FBCallback) &ORTool_BlendShape::EventToolIdle		);
	OnResize.Add( this, (FBCallback) &ORTool_BlendShape::EventToolResize	);
	OnPaint.Add	( this, (FBCallback) &ORTool_BlendShape::EventToolPaint		);
	OnInput.Add	( this, (FBCallback) &ORTool_BlendShape::EventToolInput		);

	FBSystem::TheOne().Scene->OnChange.Add( this, (FBCallback) &ORTool_BlendShape::EventConnectionStateNotify );

	mState=0;
	mShapesCount = 0;

	return true;
}

void ORTool_BlendShape::EventConnectionStateNotify( HISender pSender, HKEvent pEvent )
{
	
	FBEventSceneChange lEvent(pEvent);

	if (lEvent.Type == kFBSceneChangeDetach)
	{

		for (int i=0; i<mContainerObjects.Items.GetCount(); ++i)
		{
			FBModel *pModel = (FBModel*) mContainerObjects.Items.GetReferenceAt(i);

			if (lEvent.ChildComponent == pModel)
			{
				mContainerObjects.Items.RemoveAt(i);
				UpdateBlendShapesView(true);
				break;
			}
		}
	}
}

void ORTool_BlendShape::UICreate()
{
	int lB = 10;
	//int lS = 4;
	int lW = 120;
	int lH = 25;

	// Configure layout
	AddRegion( "LabelObject", "LabelObject",
										lB,	kFBAttachLeft,	"",	1.0	,
										lB,	kFBAttachTop,	"",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	
	AddRegion( "ButtonSave", "ButtonSave",
										-lB-80,	kFBAttachRight,	"",	1.0	,
										lB,	kFBAttachTop,	"",	1.0,
										80,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	AddRegion( "ButtonLoad", "ButtonLoad",
										-lB-80,	kFBAttachLeft,	"ButtonSave",	1.0	,
										lB,	kFBAttachTop,	"",	1.0,
										80,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	
	AddRegion( "ButtonAssign", "ButtonAssign",
										lB,	kFBAttachRight,	"LabelObject",	1.0	,
										lB,	kFBAttachTop,	"",	1.0,
										-lB,kFBAttachLeft,	"ButtonLoad",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	

	/*
	AddRegion( "ButtonAssignAuto", "ButtonAssignAuto",
										lB,	kFBAttachRight,	"ButtonAssign",	1.0	,
										lB,	kFBAttachTop,	"",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
										*/
	
	// scrollbox
	AddRegion( "ScrollBox", "ScrollBox",
										lB,	kFBAttachLeft,	"",	1.0	,
										lB,	kFBAttachBottom,	"LabelObject",	1.0,
										-lB*3-lW,kFBAttachRight,	"",	1.0,
										-lB-lB-lH,kFBAttachBottom,	"",	1.0 );

	// bottom panel with buttons
	AddRegion( "LabelCheck", "LabelCheck",
										lB,	kFBAttachLeft,	"",	1.0	,
										-lB-lH,	kFBAttachBottom,	"",	1.0,
										-lB,	kFBAttachWidth,	"ScrollBox",	0.25,
										lH,	kFBAttachNone,	"",	1.0 );
	AddRegion( "ButtonCheckAll", "ButtonCheckAll",
										lB,	kFBAttachRight,	"LabelCheck",	1.0	,
										-lB-lH,	kFBAttachBottom,	"",	1.0,
										-lB,	kFBAttachWidth,	"ScrollBox",	0.25,
										lH,	kFBAttachNone,	"",	1.0 );
	AddRegion( "ButtonCheckNone", "ButtonCheckNone",
										lB,	kFBAttachRight,	"ButtonCheckAll",	1.0	,
										-lB-lH,	kFBAttachBottom,	"",	1.0,
										-lB,	kFBAttachWidth,	"ScrollBox",	0.25,
										lH,	kFBAttachNone,	"",	1.0 );
	AddRegion( "ButtonCheckInvert", "ButtonCheckInvert",
										lB,	kFBAttachRight,	"ButtonCheckNone",	1.0	,
										-lB-lH,	kFBAttachBottom,	"",	1.0,
										-lB,	kFBAttachWidth,	"ScrollBox",	0.25,
										lH,	kFBAttachNone,	"",	1.0 );
	
	AddRegion( "Panel", "Panel",
										-2*lB-lW,	kFBAttachRight,	"",	1.0	,
										2*lB+lH,	kFBAttachTop,	"",	1.0,
										0,			kFBAttachRight,	"",	1.0,
										0,			kFBAttachBottom,"",	1.0 );

	//
	//

	SetControl( "LabelObject", mLabelObject );
	SetControl( "ButtonAssign", mContainerObjects );
	//SetControl( "ButtonAssignAuto", mButtonAutoAssign );
	SetControl( "ButtonLoad", mButtonLoad );
	SetControl( "ButtonSave", mButtonSave );

	SetControl( "LabelCheck", mLabelCheck );
	SetControl( "ButtonCheckAll", mButtonCheckAll );
	SetControl( "ButtonCheckNone", mButtonCheckNone );
	SetControl( "ButtonCheckInvert", mButtonCheckInvert );

	SetControl( "ScrollBox", mScrollBox);	

	SetControl( "Panel", mLayoutPanel );
}

void ORTool_BlendShape::UICreateScrollBox()
{
	FBLayout *pLayout = mScrollBox.GetContent();

	const int size=128;
	char text[size];
	char szPrevText[size] = "";

	bool isFirst = true;

	int lB = 10;
	//int lS = 4;
	int lW = 300;
	int lH = 25;

	for (int i=0; i<MAX_PROPERTIES; ++i)
	{
		sprintf_s( text, size, "Region%d", i );

		FBString boxRegion( "Box", text );

		pLayout->AddRegion( boxRegion, boxRegion,
					lB,		kFBAttachNone,	"",	1.0	,
					lB,		(isFirst) ? kFBAttachNone : kFBAttachBottom,szPrevText,	1.0,
					lH,		kFBAttachNone,	"",	1.0,
					lH,		kFBAttachNone,	"",	1.0 );

		pLayout->AddRegion( text, text,
					lB,		kFBAttachRight,	boxRegion,	1.0	,
					lB,		(isFirst) ? kFBAttachNone : kFBAttachBottom,szPrevText,	1.0,
					lW,		kFBAttachNone,	"",	1.0,
					lH,		kFBAttachNone,	"",	1.0 );

		strcpy_s( szPrevText, size, text );

		isFirst = false;
	}
}

void ORTool_BlendShape::UICreatePanel()
{
	int lB = 10;
	//int lS = 4;
	int lW = 120;
	int lH = 25;


	FBArrowButton *arrowButtons[4] = { &mArrowBlendShapes, &mArrowOperations, &mArrowSculpt, &mArrowInfo };
	FBLayout *arrowContent[4] = { &mLayoutBlendShapes, &mLayoutOperations, &mLayoutSculpt, &mLayoutInfo };
	const char *parentNames[5] = { "", "arrowBlendShapes", "arrowOperations", "arrowSculpt", "arrowInfo" };
	const char *arrowTitles[4] = { "BlendShapes", "Mesh Operations", "Sculpt Brush", "Info" };
	const int arrowHeights[4] = { 270, 290, 50, 50 };

	for (int i=0; i<4; ++i)
	{
		arrowContent[i]->AddRegion( "internalBorder", "internalBorder", 
										2,		kFBAttachLeft,	"",	1.0	,
										2,		kFBAttachTop,	"",	1.0,
										-2,		kFBAttachRight,	"",	1.0,
										-2,		kFBAttachBottom,"",	1.0 );
		arrowContent[i]->SetBorder( "internalBorder", kFBStandardBorder, false, true, 1, 0, 90.0f, 0 );

		//

		mLayoutPanel.AddRegion( parentNames[i+1], parentNames[i+1], 
										0,		kFBAttachLeft,	"",	1.0	,
										0,		(i == 0) ? kFBAttachTop : kFBAttachBottom, parentNames[i], 1.0,
										0,		kFBAttachNone,	"",	1.0,
										0,		kFBAttachNone,	"",	1.0 );
		mLayoutPanel.SetControl( parentNames[i+1], *arrowButtons[i] );

		// important : we set the content AFTER having added the button arrow
        // to its parent layout

		arrowButtons[i]->SetContent( arrowTitles[i], arrowContent[i], lW+2*lB, arrowHeights[i] );
	}

	

	// buttons on the right
	mLayoutBlendShapes.AddRegion( "ButtonAdd", "ButtonAdd",
										lB,			kFBAttachLeft,	"",	1.0	,
										lB*2,		kFBAttachTop,	"",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	mLayoutBlendShapes.AddRegion( "ButtonRemove", "ButtonRemove",
										0,	kFBAttachLeft,	"ButtonAdd",	1.0	,
										lB,	kFBAttachBottom,"ButtonAdd",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	mLayoutBlendShapes.AddRegion( "ButtonClear", "ButtonClear",
										0,	kFBAttachLeft,	"ButtonAdd",	1.0	,
										lB,	kFBAttachBottom,"ButtonRemove",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	mLayoutBlendShapes.AddRegion( "ButtonDublicate", "ButtonDublicate",
										0,	kFBAttachLeft,	"ButtonAdd",	1.0	,
										lB,	kFBAttachBottom,"ButtonClear",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	mLayoutBlendShapes.AddRegion( "ButtonRename", "ButtonRename",
										0,	kFBAttachLeft,	"ButtonAdd",	1.0	,
										lB,	kFBAttachBottom,"ButtonDublicate",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	mLayoutBlendShapes.AddRegion( "ButtonMoveUp", "ButtonMoveUp",
										0,	kFBAttachLeft,	"ButtonAdd",	1.0	,
										lB*2,	kFBAttachBottom,"ButtonRename",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	mLayoutBlendShapes.AddRegion( "ButtonMoveDown", "ButtonMoveDown",
										0,	kFBAttachLeft,	"ButtonAdd",	1.0	,
										lB,	kFBAttachBottom,"ButtonMoveUp",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	//

	mLayoutOperations.AddRegion( "ButtonSnapshot", "ButtonSnapshot",
										lB,			kFBAttachLeft,	"",	1.0	,
										lB*2,		kFBAttachTop,	"",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	mLayoutOperations.AddRegion( "ListDelta", "ListDelta",
										0,	kFBAttachLeft,	"ButtonSnapshot",	1.0	,
										lB,	kFBAttachBottom,"ButtonSnapshot",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	mLayoutOperations.AddRegion( "ButtonDelta", "ButtonDelta",
										0,	kFBAttachLeft,	"ButtonSnapshot",	1.0	,
										lB,	kFBAttachBottom,"ListDelta",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	mLayoutOperations.AddRegion( "ButtonCombineDeleteSource", "ButtonCombineDeleteSource",
										0,	kFBAttachLeft,	"ButtonSnapshot",	1.0	,
										lB,	kFBAttachBottom,"ButtonDelta",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	mLayoutOperations.AddRegion( "ButtonCombine", "ButtonCombine",
										0,	kFBAttachLeft,	"ButtonSnapshot",	1.0	,
										lB,	kFBAttachBottom,"ButtonCombineDeleteSource",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	mLayoutOperations.AddRegion( "ButtonCenterPivot", "ButtonCenterPivot",
										0,	kFBAttachLeft,	"ButtonSnapshot",	1.0	,
										lB,	kFBAttachBottom,"ButtonCombine",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	mLayoutOperations.AddRegion( "ButtonOptimizeSkin", "ButtonOptimizeSkin",
										0,	kFBAttachLeft,	"ButtonSnapshot",	1.0	,
										lB,	kFBAttachBottom,"ButtonCenterPivot",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	mLayoutOperations.AddRegion( "ButtonReComputeNormals", "ButtonReComputeNormals",
										0,	kFBAttachLeft,	"ButtonSnapshot",	1.0	,
										lB,	kFBAttachBottom,"ButtonOptimizeSkin",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	mLayoutOperations.AddRegion( "ButtonInvertNormals", "ButtonInvertNormals",
										0,	kFBAttachLeft,	"ButtonSnapshot",	1.0	,
										lB,	kFBAttachBottom,"ButtonReComputeNormals",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );



	//
	
	/*
	AddRegion( "ButtonSculptEnter", "ButtonSculptEnter",
										0,	kFBAttachLeft,	"LabelOperations",	1.0	,
										lB*2,	kFBAttachBottom,"LabelSculpt",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	AddRegion( "ButtonSculptLeave", "ButtonSculptLeave",
										0,	kFBAttachLeft,	"LabelOperations",	1.0	,
										lB,	kFBAttachBottom,"ButtonSculptEnter",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
										*/
	mLayoutSculpt.AddRegion( "ButtonSculptTool", "ButtonSculptTool",
										lB,			kFBAttachLeft,	"",	1.0	,
										lB*2,		kFBAttachTop,	"",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );



	//
	
	mLayoutInfo.AddRegion( "ButtonAbout", "ButtonAbout",
										lB,			kFBAttachLeft,	"",	1.0	,
										lB*2,		kFBAttachTop,	"",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );


	//
	//

	mLayoutBlendShapes.SetControl( "ButtonAdd", mButtonAdd );
	mLayoutBlendShapes.SetControl( "ButtonRemove", mButtonRemove );
	mLayoutBlendShapes.SetControl( "ButtonClear", mButtonClear );
	mLayoutBlendShapes.SetControl( "ButtonDublicate", mButtonDublicate );
	mLayoutBlendShapes.SetControl( "ButtonRename", mButtonRename );
	//
	mLayoutBlendShapes.SetControl( "ButtonMoveUp", mButtonMoveUp );
	mLayoutBlendShapes.SetControl( "ButtonMoveDown", mButtonMoveDown );
	//
	mLayoutOperations.SetControl( "ButtonSnapshot", mButtonSnapshot );
	mLayoutOperations.SetControl( "ListDelta", mListCalcDeltaMode );
	mLayoutOperations.SetControl( "ButtonDelta", mButtonCalcDelta );
	mLayoutOperations.SetControl( "ButtonCombineDeleteSource", mButtonCombineDeleteSource );
	mLayoutOperations.SetControl( "ButtonCombine", mButtonCombine );
	mLayoutOperations.SetControl( "ButtonCenterPivot", mButtonCenterPivot );
	mLayoutOperations.SetControl( "ButtonOptimizeSkin", mButtonOptimizeSkin );
	mLayoutOperations.SetControl( "ButtonReComputeNormals", mButtonReComputeNormals );
	mLayoutOperations.SetControl( "ButtonInvertNormals", mButtonInvertNormals );
	//
	//SetControl( "ButtonSculptEnter", mButtonSculptEnter );
	//SetControl( "ButtonSculptLeave", mButtonSculptLeave );
	mLayoutSculpt.SetControl( "ButtonSculptTool", mButtonSculptTool );
	//
	mLayoutInfo.SetControl( "ButtonAbout", mButtonAbout );
}

void ORTool_BlendShape::UIConfig()
{
	// Configure button
	mLabelObject.Caption = "Objects container: ";
		
	mContainerObjects.IconPosition = kFBIconLeft;
	mContainerObjects.OnDblClick.Add( this, (FBCallback) &ORTool_BlendShape::EventContainerDblClick );
	mContainerObjects.OnDragAndDrop.Add( this, (FBCallback) &ORTool_BlendShape::EventContainerDragAndDrop );

	mScrollBox.SetContentHeight(1000);
	mScrollBox.SetContentWidth(1000);

	mButtonLoad.Caption = "Load";
	mButtonLoad.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonLoadClick );

	mButtonSave.Caption = "Save";
	mButtonSave.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonSaveClick );

	mButtonAutoAssign.Caption = "Auto assign";
	mButtonAutoAssign.Style = kFBCheckbox;
	//mButtonAutoAssign.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonAutoAssignClick );
		
	mLabelCheck.Caption = "Check state: ";

	mButtonCheckAll.Caption = "All";
	mButtonCheckAll.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonCheckAllClick );

	mButtonCheckNone.Caption = "None";
	mButtonCheckNone.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonCheckNoneClick );

	mButtonCheckInvert.Caption = "Invert";
	mButtonCheckInvert.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonCheckInvertClick );
}

void ORTool_BlendShape::UIConfigScrollBox()
{
	for (int i=0; i<MAX_PROPERTIES; ++i)
	{
		mButtonShapes[i].Style = kFBCheckbox;
	}
}

void ORTool_BlendShape::UIConfigPanel()
{
	//
	//
	//

	mArrowBlendShapes.Caption = "BlendShape actions";

	mButtonAdd.Caption = "Add";
	mButtonAdd.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonAddClick );
	
	mButtonRemove.Caption = "Remove";
	mButtonRemove.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonRemoveClick );

	mButtonClear.Caption = "Clear";
	mButtonClear.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonClearClick );

	mButtonDublicate.Caption = "Dublicate";
	mButtonDublicate.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonDublicateClick );

	mButtonRename.Caption = "Rename";
	mButtonRename.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonRenameClick );

	mButtonMoveUp.Caption = "Move Up";
	mButtonMoveUp.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonMoveUpClick );

	mButtonMoveDown.Caption = "Move Down";
	mButtonMoveDown.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonMoveDownClick );

	//
	//

	mArrowOperations.Caption = "Mesh Operations";

	mButtonSnapshot.Caption = "Snapshot";
	mButtonSnapshot.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonSnapshotClick );

	mListCalcDeltaMode.Style = kFBDropDownList;
	mListCalcDeltaMode.Items.SetString( "All~Only Pos. X~Only Neg. X" );
	mListCalcDeltaMode.ItemIndex = 0;

	mButtonCalcDelta.Caption = "Calculate Delta";
	mButtonCalcDelta.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonCalcDeltaClick );

	mButtonCombineDeleteSource.Caption = "Delete source models";
	mButtonCombineDeleteSource.Style = kFBCheckbox;
	mButtonCombineDeleteSource.State = 1;
	
	mButtonCombine.Caption = "Combine models";
	mButtonCombine.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonCombineClick );

	mButtonCenterPivot.Caption = "Center Pivot";
	mButtonCenterPivot.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonCenterPivotClick );

	mButtonOptimizeSkin.Caption = "Optimize skin";
	mButtonOptimizeSkin.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonOptimizeSkinClick );

	mButtonReComputeNormals.Caption = "ReCompute Normals";
	mButtonReComputeNormals.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonReComputeNormalsClick );

	mButtonInvertNormals.Caption = "Invert Normals";
	mButtonInvertNormals.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonInvertNormalsClick );

	//
	//

	mArrowSculpt.Caption = "Sculpting Brush";

	mButtonSculptEnter.Caption = "Enter sculpting";
	mButtonSculptEnter.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonSculptEnterClick );

	mButtonSculptLeave.Caption = "Leave sculpting";
	mButtonSculptLeave.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonSculptLeaveClick );

	mButtonSculptTool.Caption = "Sculpt Tool";
	mButtonSculptTool.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonSculptToolClick );

	//
	//

	mArrowSculpt.Caption = "Info";

	mButtonAbout.Caption = "About";
	mButtonAbout.OnClick.Add( this, (FBCallback) &ORTool_BlendShape::EventButtonAboutClick );
}

void ORTool_BlendShape::UIReset()
{
}

void ORTool_BlendShape::UIResetPanel()
{
}

void ORTool_BlendShape::UIResetScrollBox()
{
	FBLayout *pLayout = mScrollBox.GetContent();

	const int size = 128;
	char text[size];
	
	for (int i=0; i<MAX_PROPERTIES; ++i)
	{
		sprintf_s( text, size, "Region%d", i );
		FBString boxRegion( "Box", text );

		pLayout->ClearControl( boxRegion );
		pLayout->ClearControl( text );

		mBlendshapes[i].Property = nullptr;

		mUIModels[i] = nullptr;
	}
}

/************************************************
 *	FiLMBOX Destruction function.
 ************************************************/
void ORTool_BlendShape::FBDestroy()
{
	// Remove tool callbacks
	OnShow.Remove	( this, (FBCallback) &ORTool_BlendShape::EventToolShow	);
	OnIdle.Remove	( this, (FBCallback) &ORTool_BlendShape::EventToolIdle	);
	OnPaint.Remove	( this, (FBCallback) &ORTool_BlendShape::EventToolPaint );
	OnInput.Remove	( this, (FBCallback) &ORTool_BlendShape::EventToolInput );
	OnResize.Remove	( this, (FBCallback) &ORTool_BlendShape::EventToolResize);

	// Free user allocated memory
	FBSystem::TheOne().Scene->OnChange.Remove( this, (FBCallback) &ORTool_BlendShape::EventConnectionStateNotify );
}


/************************************************
 *	Button click callback.
 ************************************************/
/*
void ORTool_BlendShape::EventButtonTestClick( HISender pSender, HKEvent pEvent )
{
	
	switch( mState )
	{
		case 0:
		{
			// Create a new marker
			mHdlModel = new FBModelPlane("Tool_Template Model");
			mButtonTest.Caption = "Make visible";
		}
		break;
		case 1:
		{
			// Make the model visible
			if(mHdlModel.Ok())
			{
				mHdlModel->Show = true;
			}
			mButtonTest.Caption = "Remove model";
		}
		break;
		case 2:
		{
			// Remove model from visible set
			if( mHdlModel.Ok() )
			{
				mHdlModel->Show = false;
			}
			mButtonTest.Caption = "Delete model";
		}
		break;
		case 3:
		{
			// Delete model
			if( mHdlModel.Ok() )
			{
				mHdlModel->FBDelete();
			}

			mButtonTest.Caption = "Create model";
		}
		break;
	}
	mState = (mState+1) % 4;
	
}
*/

void ORTool_BlendShape::EventContainerDragAndDrop( HISender pSender, HKEvent pEvent )
{
    FBEventDragAndDrop lDragAndDrop( pEvent );

    switch( lDragAndDrop.State )
    {
        case kFBDragAndDropDrag:
        {
            lDragAndDrop.Accept();
        }
        break;
        case kFBDragAndDropDrop:
        {
            SetupBlendShapes( (FBModel*)lDragAndDrop.Get(0) );
        }
        break;
    }
}

void ORTool_BlendShape::SetupBlendShapes( FBModel *pModel )
{
	//mContainerObjects.Items.Clear();
    //mListProperties.Items.Clear();
	/*
    mEditProperty.Property          = 0;
    mEditPropertyModern.Property    = 0;
    mButtonPreviousProperty.Enabled = false;
    mButtonNextProperty.Enabled     = false;
	*/
    if( pModel )
    {
        mContainerObjects.Items.Add( pModel->Name, (kReference)pModel );
        /*
		int lIdx = 0;
        int lNbProps = lModel->PropertyList.GetCount();

        mListProperties.Items.Add( "<Select Property>", (kReference)0 );
        mListProperties.ItemIndex = 0;
        for( lIdx = 0; lIdx < lNbProps; ++lIdx )
        {
            FBProperty* lProp = lModel->PropertyList[lIdx];
            if( lProp->IsInternal() && !lProp->GetPropertyFlag( kFBPropertyFlagHideProperty ))
            {
                mListProperties.Items.Add( lProp->GetName(), (kReference)lProp );
            }
        }
        mButtonPreviousProperty.Enabled = true;
        mButtonNextProperty.Enabled = true;
		*/
		UpdateBlendShapesView(true);
    }
}

FBString GetShapeRegionName(int idx)
{
	const int text_size = 128;

	char text[text_size];
	sprintf_s( text, text_size, "Region%d", idx );

	return FBString(text);
}

void ORTool_BlendShape::UpdateBlendShapesView(const bool resetCheckState)
{
	FBLayout *pLayout = mScrollBox.GetContent();

	mScrollBox.SetContentWidth( 320 );

	FBString regionText, boxRegionText;
	int idx = 0;

	UIResetScrollBox();

	for (int i=0; i<mContainerObjects.Items.GetCount(); ++i)
	{
		FBModel *pModel = (FBModel*) mContainerObjects.Items.GetReferenceAt(i);

		regionText = GetShapeRegionName(idx);
		pLayout->SetControl( regionText, mLabelShapes[idx] );
		mLabelShapes[idx].Caption = FBString(pModel->LongName, " - BlendShapes List");
		idx++;
		
		FBGeometry *pGeometry = pModel->Geometry;
		if (pGeometry == nullptr)
			continue;
		int shapesCount = pGeometry->ShapeGetCount();

		for (int j=0; j<shapesCount; ++j)
		{
			FBString shapeName = pGeometry->ShapeGetName(j);
			
			regionText = GetShapeRegionName(idx);
			boxRegionText = FBString( "Box", regionText );

			FBProperty *pProperty = pModel->PropertyList.Find( shapeName );

			if (pProperty != nullptr)
			{

				mBlendshapes[idx].Property = pProperty;

				pLayout->SetControl( boxRegionText, mButtonShapes[idx] );
				pLayout->SetControl( regionText, mBlendshapes[idx] );

				if (resetCheckState) 	
				{
					mButtonShapes[idx].State = 0;
				}
				mUIModels[idx] = pModel;

				mBlendshapes[idx].Caption = shapeName;
				mBlendshapes[idx].CaptionSize = UI_BLENDSHAPE_CAPTION_SIZE;
				//mBlendshapes[idx].Property = pModel->PropertyList.Find( shapeName );

				idx++;
			}
		}
	}

	mShapesCount = idx;

	mScrollBox.SetContentHeight( idx * 35 + 35 );
}

void ORTool_BlendShape::EventContainerDblClick( HISender pSender, HKEvent pEvent )
{
    //SetupBlendShapes( nullptr );
	mContainerObjects.Items.Clear();

	UpdateBlendShapesView(true);
}


void ORTool_BlendShape::EventButtonAddClick( HISender pSender, HKEvent pEvent )
{
	
	FBModelList pList;
	FBGetSelectedModels( pList );

	if (pList.GetCount() == 0)
	{
		const char* szTitle = LoadStringFromResource1(IDS_TITLE);
		const char* szMsg = LoadStringFromResource2(IDS_ADD_NOTE);
		FBMessageBox( szTitle, szMsg, "Ok" );
		return;
	}

	// try to find a same geometry as a selected blendshapes

	//bool result = false;

	for (int i=0; i<mContainerObjects.Items.GetCount(); ++i)
	{
		FBModel *pBaseModel = (FBModel*) mContainerObjects.Items.GetReferenceAt(i);
		FBString baseModelLongName = pBaseModel->LongName.AsString();
		FBGeometry *pBaseGeometry = pBaseModel->Geometry;
		FBMesh *pBaseMesh = (FBMesh*) pBaseGeometry;

		const int numberOfVertices = pBaseGeometry->VertexCount();
		const int numberOfPolys = pBaseMesh->PolygonCount();

		for (int j=0; j<pList.GetCount(); ++j)
		{
			FBModel *pModel = pList[j];

			FBGeometry *pGeometry = pModel->Geometry;
			FBMesh *pMesh = (FBMesh*) pGeometry;

			// if we have property with specified name, skip operation if name is not equel
			FBProperty *pProp = pModel->PropertyList.Find( "BaseModel" );
			if (pProp)
			{
				FBString modelName = pProp->AsString();
				if (baseModelLongName != modelName)
				{
					continue;
				}
			}

			// check geometry equal
			if (pGeometry)
			{
				const int lverts = pGeometry->VertexCount();
				const int lpolys = pMesh->PolygonCount();

				if (lverts == numberOfVertices && lpolys == numberOfPolys)
				{
					AddBlendShape( pBaseModel, pModel );
				}
			}
		}
	}

	// update UI
	UpdateBlendShapesView(true);
}

// OUTPUT checkList with filled boxes
bool ORTool_BlendShape::PrepareCheckList( FBModel *pModel, FBArrayTemplate<bool>	&checkList )
{
	FBGeometry *pGeometry = pModel->Geometry;
	if (pGeometry == nullptr) return false;

	int lcount = pGeometry->ShapeGetCount();
	checkList.SetCount(lcount);

	for (int j=0; j<lcount; ++j)
	{
		FBString geomShapeName( pGeometry->ShapeGetName(j) );
		checkList[j] = false;

		for (int i=0; i<mShapesCount; ++i)
		{
			if ( (pModel == mUIModels[i]) && (mButtonShapes[i].State == 1) )
			{
				FBString caption(mBlendshapes[i].Caption);

				if (geomShapeName == caption)
				{
					checkList[j] = true;
					break;
				}
			}
		}
	}

	return true;
}

bool ORTool_BlendShape::UpdateCheckList( FBModel *pModel, FBArrayTemplate<bool> &checkList )
{
	FBGeometry *pGeometry = pModel->Geometry;
	if (pGeometry == nullptr) return false;

	const int lcount = pGeometry->ShapeGetCount();

	int idx = -1;

	for (int i=0; i<mShapesCount; ++i)
		if (pModel == mUIModels[i])
		{
			idx = i;
			break;
		}

	if (idx >= 0)
	{
		for (int i=0; i<lcount; ++i)
			mButtonShapes[i+idx].State = checkList[i];
	}

	return true;
}


void ORTool_BlendShape::EventButtonRemoveClick( HISender pSender, HKEvent pEvent )
{
	const char* szTitle = LoadStringFromResource1(IDS_TITLE);
	const char* szMsg = LoadStringFromResource2(IDS_QUESTION_REMOVE);

	if (FBMessageBox( szTitle, szMsg, "Ok", "Cancel" ) == 1)
	{
		FBArrayTemplate<bool>		checkList;

		for (int iModel=0; iModel<mContainerObjects.Items.GetCount(); ++iModel)
		{
			FBModel *pModel = (FBModel*) mContainerObjects.Items.GetReferenceAt(iModel);
			FBGeometry *pGeometry = pModel->Geometry;

			PrepareCheckList( pModel, checkList );
			/*
			for(int i=0; i<shapesCount; ++i)
			{
				if (removeList[i])
				{
					FBProperty *pProperty = pModel->PropertyList.Find( pGeometry->ShapeGetName(i) );
					if (pProperty)
						pModel->PropertyRemove(pProperty);
				}
			}
			*/

			ModifyGeometryShapes( pGeometry, checkList.GetArray(), kFBShapeRemove );

			/*
			int propIndex = pModel->PropertyList.GetCount() - 1;

			while(propIndex >= 0)
			{
				FBProperty *pProperty = pModel->PropertyList[propIndex];

				const char *typeName = pProperty->GetPropertyTypeName();
				if ( strstr(typeName, "shape") != nullptr )
				{
					pModel->PropertyRemove(pProperty);
				}

				propIndex -= 1;
			}
			*/
			pModel->SetupPropertiesForShapes();
		}

		

		UpdateBlendShapesView(true);
	}
}

void ORTool_BlendShape::EventButtonClearClick( HISender pSender, HKEvent pEvent )
{
	const char* szTitle = LoadStringFromResource1(IDS_TITLE);
	const char* szMsg = LoadStringFromResource2(IDS_QUESTION_CLEAR);

	if (FBMessageBox( szTitle, szMsg, "Ok", "Cancel" ) == 1)
	{
		for (int iModel=0; iModel<mContainerObjects.Items.GetCount(); ++iModel)
		{
			FBModel *pModel = (FBModel*) mContainerObjects.Items.GetReferenceAt(iModel);
			FBGeometry *pGeometry = pModel->Geometry;

			if (pGeometry)
			{
				pGeometry->ShapeClearAll();
				pModel->SetupPropertiesForShapes();

				UpdateBlendShapesView(true);
			}
		}
	}
}

void ORTool_BlendShape::EventButtonDublicateClick( HISender pSender, HKEvent pEvent )
{
	const char* szTitle = LoadStringFromResource1(IDS_TITLE);
	const char* szMsg = LoadStringFromResource2(IDS_QUESTION_DUBLICATE);

	if (FBMessageBox( szTitle, szMsg, "Ok", "Cancel" ) == 1)
	{
		FBArrayTemplate<bool>		checkList;

		for (int iModel=0; iModel<mContainerObjects.Items.GetCount(); ++iModel)
		{
			FBModel *pModel = (FBModel*) mContainerObjects.Items.GetReferenceAt(iModel);
			FBGeometry *pGeometry = pModel->Geometry;

			PrepareCheckList( pModel, checkList );
			ModifyGeometryShapes( pGeometry, checkList.GetArray(), kFBShapeDublicate );

			pModel->SetupPropertiesForShapes();
		}

		UpdateBlendShapesView(true);
	}
}

void ORTool_BlendShape::EventButtonRenameClick( HISender pSender, HKEvent pEvent )
{
	const char* szTitle = LoadStringFromResource1(IDS_TITLE);
	const char* szMsg = LoadStringFromResource2(IDS_QUESTION_RENAME);

	if (FBMessageBox( szTitle, szMsg, "Ok", "Cancel" ) == 1)
	{
		FBArrayTemplate<bool>		checkList;

		for (int iModel=0; iModel<mContainerObjects.Items.GetCount(); ++iModel)
		{
			FBModel *pModel = (FBModel*) mContainerObjects.Items.GetReferenceAt(iModel);
			FBGeometry *pGeometry = pModel->Geometry;

			PrepareCheckList( pModel, checkList );
			ModifyGeometryShapes( pGeometry, checkList.GetArray(), kFBShapeRename );

			pModel->SetupPropertiesForShapes();
		}

		UpdateBlendShapesView(true);
	}
}


void PrintCheckList(const int count, const bool *checkList)
{
	printf( "checkList - " );

	for (int i=0; i<count; ++i)
	{
		printf( "[%d]=%d; ", i, (checkList[i])?1:0 );
	}

	printf( "\n" );
}

void ORTool_BlendShape::EventButtonMoveUpClick( HISender pSender, HKEvent pEvent )
{
	FBArrayTemplate<bool>		checkList;

	for (int iModel=0; iModel<mContainerObjects.Items.GetCount(); ++iModel)
	{
		FBModel *pModel = (FBModel*) mContainerObjects.Items.GetReferenceAt(iModel);
		FBGeometry *pGeometry = pModel->Geometry;

		PrepareCheckList( pModel, checkList );
		//PrintCheckList( checkList.GetCount(), checkList.GetArray() );
		ModifyGeometryShapes( pGeometry, checkList.GetArray(), kFBShapeMoveUp );
		UpdateCheckList( pModel, checkList );
		//PrintCheckList( checkList.GetCount(), checkList.GetArray() );

		pModel->SetupPropertiesForShapes();
	}

	UpdateBlendShapesView(false);
}

void ORTool_BlendShape::EventButtonMoveDownClick( HISender pSender, HKEvent pEvent )
{
	FBArrayTemplate<bool>		checkList;

	for (int iModel=0; iModel<mContainerObjects.Items.GetCount(); ++iModel)
	{
		FBModel *pModel = (FBModel*) mContainerObjects.Items.GetReferenceAt(iModel);
		FBGeometry *pGeometry = pModel->Geometry;

		PrepareCheckList( pModel, checkList );
		//PrintCheckList( checkList.GetCount(), checkList.GetArray() );
		ModifyGeometryShapes( pGeometry, checkList.GetArray(), kFBShapeMoveDown );
		UpdateCheckList( pModel, checkList );
		//PrintCheckList( checkList.GetCount(), checkList.GetArray() );

		pModel->SetupPropertiesForShapes();
	}

	UpdateBlendShapesView(false);
}

void ORTool_BlendShape::EventButtonSnapshotClick( HISender pSender, HKEvent pEvent )
{
	FBModelList		llist;
	FBStringList	newList;

	FBGetSelectedModels(llist);

	if (llist.GetCount() == 0)
	{
		const char* szTitle = LoadStringFromResource1(IDS_TITLE);
		const char* szMsg = LoadStringFromResource2(IDS_SNAPSHOT_NOTE);
		FBMessageBox( szTitle, szMsg, "Ok" );
		return;
	}

	FBConfigFile	lFile( "@BlendShapeTookit.txt", true );
	
	FBString strDefault( "false" );
	const char *szDefault = strDefault;
	lFile.GetOrSet( "Mesh Operations", "ResetXForm", szDefault, "Do you need to reset transform for a snapshot (true/false)" );

	bool DoResetXForm = false;
	if ( strstr(szDefault, "true") != nullptr ) DoResetXForm = true;

	strDefault = "true";
	szDefault = strDefault;
	lFile.GetOrSet( "Mesh Operations", "CopyShaders", szDefault, "Do you need to copy shaders for a snapshot (true/false)" );
	bool DoCopyShaders = false;
	if ( strstr(szDefault, "true") != nullptr ) DoCopyShaders = true;

	int numberOfCopies = 1;
	// query number of snapshots per model needed
	const char *title = LoadStringFromResource1( IDS_TITLE );
	const char *question = LoadStringFromResource2( IDS_QUESTION_SNAPSHOT );
	if (1 == FBMessageBoxGetUserValue( title, question, &numberOfCopies, kFBPopupInt, "Ok", "Cancel" ) )
	{
		// clear all scene selection
		FBScene *pScene = FBSystem::TheOne().Scene;
		for (int i=0; i<pScene->Components.GetCount(); ++i)
		{
			pScene->Components[i]->Selected = false;
		}

		// snapshots
		for (int i=0; i<llist.GetCount(); ++i)
		{
			FBModel *pModel = llist[i];

			for (int j=0; j<numberOfCopies; ++j)
			{
				FBModel *pNewModel = MakeSnapshot2( pModel, DoResetXForm, DoCopyShaders );
				if (pNewModel) newList.Add(pNewModel->LongName);
			}
		}

		// select result meshes

		for (int i=0; i<newList.GetCount(); ++i)
		{
			FBModel *pModel = FBFindModelByLabelName( newList[i] );
			if (pModel) pModel->Selected = true;
		}
	}
}




void ORTool_BlendShape::EventButtonCalcDeltaClick( HISender pSender, HKEvent pEvent )
{
	// BASE* - source animated mesh
	// EDIT* - user sculpting mesh from snapshot
	// DELTA* - write difference here

	FBModelList pList;
	FBGetSelectedModels(pList);

	try
	{
		FBString modelNameBASE("base");

		FBModel *pModelEDIT = nullptr;
		FBModel *pModelBASE = nullptr;
		FBGeometry *pGeometryBASE = nullptr;

		FBTime lTime(0);

		FBMatrix InvTM;
		InvTM.Identity();

		if (pList.GetCount() == 2 && (1 == FBMessageBox("Calculate Delta", "Did you follow selection order: corrective model, then base ?", "Yes", "Cancel")) )
		{

			FBUndoManager	mUndoManager;
			mUndoManager.Undo();

			FBModelList		mSecondList;
			FBGetSelectedModels(mSecondList);

			if (mSecondList.GetCount() > 0)
				pModelEDIT = mSecondList.GetAt(0);

			if (pModelEDIT == nullptr)
				throw ("Empty model");

			pModelBASE = (pList[0]==pModelEDIT) ? pList[1] : pList[0];
			if (pModelBASE != nullptr)
			{
				pGeometryBASE = pModelBASE->Geometry;
				modelNameBASE = pModelBASE->Name;
			}

			lTime = FBSystem::TheOne().LocalTime;
		}
		else
		if (pList.GetCount() != 1)
		{
			throw ("Please select a one snapshot model to calculate delta");
		}
		else
		{
			pModelEDIT = pList.GetAt(0);
			FBProperty *lProp = pModelEDIT->PropertyList.Find( "SnapshotTime" );
			FBProperty *lNameProp = pModelEDIT->PropertyList.Find( "BaseModel" );

			if (lProp == nullptr || lNameProp == nullptr)
				throw ("Selected mesh has no snapshot properties");

			
			double value;
			lProp->GetData(&value, sizeof(double) );
			lTime.SetSecondDouble(value);

			modelNameBASE = lNameProp->AsString();

			pModelBASE = FBFindModelByLabelName(modelNameBASE);
			if (pModelBASE == nullptr)
				throw ( "Base model is not found" );
			pGeometryBASE = pModelBASE->Geometry;

			//
			// snapshot matrix (if exist)
			FBProperty *lPropPos = pModelEDIT->PropertyList.Find( "SnapshotPosition" );
			FBProperty *lPropRot = pModelEDIT->PropertyList.Find( "SnapshotRotation" );
			FBProperty *lPropScale = pModelEDIT->PropertyList.Find( "SnapshotScale" );

			if (lPropPos && lPropRot && lPropScale)
			{
				FBTVector pos;
				FBRVector rot;
				FBSVector scale;

				lPropPos->GetData( pos, sizeof(double) * 4 );
				lPropRot->GetData( rot, sizeof(double) * 3 );
				lPropScale->GetData( scale, sizeof(double) * 3 );

				FBTRSToMatrix( InvTM, pos, rot, scale );
				//FBMatrixInverse( InvTM, InvTM );

				//FBMatrix TM;
				//pModelEDIT->GetMatrix(TM);

				//FBMatrixMult( InvTM, TM, InvTM );
			}
		}
		
		
		//
		// Calculate difference between BaseModel and Model. Store difference in the third model DeltaModel
		//

		if (false == HasModelCluster(pModelEDIT) && false == HasModelBlendshapes(pModelEDIT) )
		{
			// calculate in simple way
		}
		else
		{
			// calculate cluster and blendshapes back transform
		}

		FBPlayerControl::TheOne().Goto( lTime );
		FBSystem::TheOne().Scene->Evaluate();

		// delta mesh and positions
		FBGeometry *pGeometryEDIT = pModelEDIT->Geometry;
		FBVertex *positionsEDIT = pGeometryEDIT->GetVertexes();

		/// !!!!! Replace this !!!
		FBString strDELTA( modelNameBASE, "_delta" );
		FBModel *pModelDELTA = new FBModel( strDELTA );

		FBProperty *lProp = pModelDELTA->PropertyCreate( "BaseModel", kFBPT_charptr, ANIMATIONNODE_TYPE_STRING, false, true );
		lProp->SetString( pModelBASE->LongName );

		FBModelVertexData *pVertexDataBASE = pModelBASE->ModelVertexData;
		pVertexDataBASE->VertexArrayMappingRequest();
		
		//int vertCountBASE = pVertexDataBASE->GetVertexCount();
		int vertCountEDIT = pGeometryEDIT->VertexCount();
		//int *indicesBASE = pVertexDataBASE->GetIndexArray();
		

		ClusterAdvance	clusterAdvance( nullptr );

		bool UseCluster = clusterAdvance.Init(pModelBASE);

		//unsigned int dubCount = 0;
		//const int *dubIndices = pVertexDataBASE->GetVertexArrayDuplicationMap( dubCount );

		FBVertex *positionsT = (FBVertex*) pVertexDataBASE->GetVertexArray( kFBGeometryArrayID_Point, false ); // before deform
		//FBNormal *normalsT = (FBNormal*) pVertexDataBASE->GetVertexArray( kFBGeometryArrayID_Normal, false );
		
		//FBVertex *positionsBASE = (FBVertex*) pVertexDataBASE->GetVertexArray( kFBGeometryArrayID_Point, true ); // after deform
		//FBNormal *normalsBASE = (FBNormal*) pVertexDataBASE->GetVertexArray( kFBGeometryArrayID_Normal, true );
		
		pVertexDataBASE->VertexArrayMappingRelease();

		
		//FBTRSToMatrix( scaleM, FBTVector(), FBRVector(), FBSVector(0.5, 0.5, 0.5) );

		FBMesh *pMeshDELTA = new FBMesh( FBString( strDELTA, "_mesh" ) );
		pModelDELTA->Geometry = pMeshDELTA;

		pMeshDELTA->GeometryBegin();

		pMeshDELTA->VertexArrayInit( vertCountEDIT, false );

		int calcMode = mListCalcDeltaMode.ItemIndex;

		for (int i=0; i<vertCountEDIT; ++i)
		{
			FBVertex v, vBASE;
			FBMatrix m;
			m.Identity();

			if (UseCluster) 
			{
				// TRANSFORM EDIT POSITION
				FBVertexMatrixMult( v, InvTM, positionsEDIT[i] );

				FBMatrixInverse( m, clusterAdvance.CalculateDeformedPositionMatrix(i) );
			}

			

			// UNTRANSFORM SKELETAL ANIMATION

			//FBVertexMatrixMult( v, m, positionsEDIT[i] );
			FBVertexMatrixMult( v, m, v );

			// UNTRANSFORM BLEND SHAPES

			UnTransformBlendShapes( v, i, pModelBASE );


			switch(calcMode)
			{
			case 1:
				if (positionsT[i][0] < 0.0)
					v = positionsT[i];
				break;
			case 2:
				if (positionsT[i][0] > 0.0)
					v = positionsT[i];
				break;
			}

			pMeshDELTA->VertexSet( v, i );
			//pMeshDELTA->VertexNormalSet( normalsBASE[i], i );
		}

		//
		// Polygons
		//
		FBMesh *lMeshEDIT = (FBMesh*) (FBGeometry*) pModelEDIT->Geometry;
		int polyCountEDIT = lMeshEDIT->PolygonCount();
			
		for (int i=0; i<polyCountEDIT; ++i)
		{
			int polyVertCount = lMeshEDIT->PolygonVertexCount(i);

			pMeshDELTA->PolygonBegin();
			for (int j=0; j<polyVertCount; ++j)
			{
				pMeshDELTA->PolygonVertexAdd( lMeshEDIT->PolygonVertexIndex(i,j) );
			}
			pMeshDELTA->PolygonEnd();
		}


		pMeshDELTA->GeometryEnd();

		pMeshDELTA->ComputeVertexNormals();

		// Step 3: finalize, prepare to display
		pModelDELTA->Show = true;
		pModelDELTA->Selected = true;

		// Adjust the shading mode.
		pModelDELTA->ShadingMode  = kFBModelShadingTexture;

		for (int i=0; i<pList.GetCount(); ++i)
			pList[i]->Selected = false;

		
		FBMatrix	lTM;
		pModelBASE->GetMatrix(lTM);
		pModelDELTA->SetMatrix(lTM);
		
	}
	catch( const char *message )
	{
		FBMessageBox( "BlendShape Toolkit", message, "Ok" );
	}
}

void ORTool_BlendShape::EventButtonBrushToolClick( HISender pSender, HKEvent pEvent )
{
	//FBPopNormalTool( ORMANIPASSOCIATION__LABEL );

	if (mContainerObjects.Items.GetCount() == 0) return;
	//FBModel *pBaseModel = (FBModel*) mContainerObjects.Items.GetReferenceAt(0);

	FBModelList pList;
	FBGetSelectedModels( pList );

	if (pList.GetCount() > 0)
	{
		//FBModel *pModel = pList[0];

		//BlendShapeIterativeFit( pBaseModel, pModel );
	}
	// assign test blend shape and fit the shape to edit model

}

void ORTool_BlendShape::EventButtonAboutClick( HISender pSender, HKEvent pEvent )
{
	/*
	FBModelList pList;
	FBGetSelectedModels(pList);

	try
	{
		if (pList.GetCount() != 1)
			throw ("Please select a one snapshot model to calculate delta");

		FBModel *pModel = pList.GetAt(0);

		CalculateDeformedMesh(pModel);

	}
	catch( const char *message )
	{
		FBMessageBox( "BlendShape Toolkit", message, "Ok" );
	}
	*/
	
	const char* szTitle = LoadStringFromResource1(IDS_TITLE);
	const char* szVersion = LoadVersionFromResource();
	const char* szMsg = LoadStringFromResource2(IDS_ABOUT);

	FBMessageBox( FBString(szTitle, szVersion), szMsg, "Ok" );
		
}

void ORTool_BlendShape::EventButtonCheckAllClick( HISender pSender, HKEvent pEvent )
{
	for (int i=0; i<MAX_PROPERTIES; ++i)
		mButtonShapes[i].State = 1;
}

void ORTool_BlendShape::EventButtonCheckNoneClick( HISender pSender, HKEvent pEvent )
{
	for (int i=0; i<MAX_PROPERTIES; ++i)
		mButtonShapes[i].State = 0;
}

void ORTool_BlendShape::EventButtonCheckInvertClick( HISender pSender, HKEvent pEvent )
{
	for (int i=0; i<MAX_PROPERTIES; ++i)
		mButtonShapes[i].State = !mButtonShapes[i].State;
}

/************************************************
 *	UI Idle callback.
 ************************************************/
void ORTool_BlendShape::EventToolIdle( HISender pSender, HKEvent pEvent )
{
}


/************************************************
 *	Handle tool activation (selection/unselection).
 ************************************************/
void ORTool_BlendShape::EventToolShow( HISender pSender, HKEvent pEvent )
{
	FBEventShow lEvent( pEvent );

	if( lEvent.Shown )
	{
		// Reset the UI here.
	}
	else
	{
	}
}


/************************************************
 *	Paint callback for tool (on expose).
 ************************************************/
void ORTool_BlendShape::EventToolPaint( HISender pSender, HKEvent pEvent )
{
}


/************************************************
 *	Tool resize callback.
 ************************************************/
void ORTool_BlendShape::EventToolResize( HISender pSender, HKEvent pEvent )
{
}


/************************************************
 *	Handle input into the tool.
 ************************************************/
void ORTool_BlendShape::EventToolInput( HISender pSender, HKEvent pEvent )
{
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool ORTool_BlendShape::FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	
	return true;
}

/************************************************
 *	FBX Retrieval.
 ************************************************/
bool ORTool_BlendShape::FbxRetrieve( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	
	return true;
}


void ORTool_BlendShape::EventButtonSculptEnterClick( HISender pSender, HKEvent pEvent )
{
	FBModelList		pList;
	FBGetSelectedModels( pList );

	if (pList.GetCount() > 0)
	{
		SculptMode( pList.GetAt(0), true );
	}
}

void ORTool_BlendShape::EventButtonSculptLeaveClick( HISender pSender, HKEvent pEvent )
{
	SculptMode( nullptr, false );
}

void ORTool_BlendShape::EventButtonSculptToolClick( HISender pSender, HKEvent pEvent )
{
	FBPopNormalTool( "Sculpt Brush Tool" );
}

void ORTool_BlendShape::EventButtonCenterPivotClick( HISender pSender, HKEvent pEvent )
{
	FBModelList		llist;
	FBStringList	newList;

	FBGetSelectedModels(llist);

	if (llist.GetCount() < 1)
	{
		const char* szTitle = LoadStringFromResource1(IDS_TITLE);
		const char* szMsg = LoadStringFromResource2(IDS_CENTER_PIVOT_NOTE);
		FBMessageBox( szTitle, szMsg, "Ok" );
		return;
	}

	for (int i=0; i<llist.GetCount(); ++i)
	{
		CenterPivot( llist[i] );
	}
}

void ORTool_BlendShape::EventButtonReComputeNormalsClick( HISender pSender, HKEvent pEvent )
{
	FBModelList		llist;
	FBStringList	newList;

	FBGetSelectedModels(llist);

	if (llist.GetCount() < 1)
	{
		const char* szTitle = LoadStringFromResource1(IDS_TITLE);
		const char* szMsg = LoadStringFromResource2(IDS_RECOMPUTE_NORMALS_NOTE);
		FBMessageBox( szTitle, szMsg, "Ok" );
		return;
	}

	for (int i=0; i<llist.GetCount(); ++i)
	{
		ReComputeNormals( llist[i] );
	}
}

void ORTool_BlendShape::EventButtonInvertNormalsClick( HISender pSender, HKEvent pEvent )
{
	FBModelList		llist;
	FBStringList	newList;

	FBGetSelectedModels(llist);

	if (llist.GetCount() < 1)
	{
		const char* szTitle = LoadStringFromResource1(IDS_TITLE);
		const char* szMsg = LoadStringFromResource2(IDS_RECOMPUTE_NORMALS_NOTE);
		FBMessageBox( szTitle, szMsg, "Ok" );
		return;
	}

	for (int i=0; i<llist.GetCount(); ++i)
	{
		InvertNormals( llist[i] );
	}
}

void ORTool_BlendShape::EventButtonCombineClick( HISender pSender, HKEvent pEvent )
{
	FBModelList		llist;
	FBStringList	newList;

	FBGetSelectedModels(llist);

	if (llist.GetCount() < 1)
	{
		const char* szTitle = LoadStringFromResource1(IDS_TITLE);
		const char* szMsg = LoadStringFromResource2(IDS_COMBINE_NOTE);
		FBMessageBox( szTitle, szMsg, "Ok" );
		return;
	}

	//bool DoResetXForm = false;

	// clear all scene selection
	FBScene *pScene = FBSystem::TheOne().Scene;
	for (int i=0; i<pScene->Components.GetCount(); ++i)
	{
		pScene->Components[i]->Selected = false;
	}

	CombineModels( llist );

	//
	if ( mButtonCombineDeleteSource.State == 1 )
	{
		for (int i=0; i<llist.GetCount(); ++i)
			llist[i]->FBDelete();
	}

	//
	const char* szTitle = LoadStringFromResource1(IDS_TITLE);
	const char* szMsg = LoadStringFromResource2(IDS_COMBINE_COMPLETED);
	FBMessageBox( szTitle, szMsg, "Ok" );
}

void ORTool_BlendShape::EventButtonOptimizeSkinClick( HISender pSender, HKEvent pEvent )
{
	FBModelList		llist;
	FBGetSelectedModels(llist);

	if (llist.GetCount() < 1)
	{
		const char* szTitle = LoadStringFromResource1(IDS_TITLE);
		const char* szMsg = LoadStringFromResource2(IDS_SKIN_OPTIMIZE);
		FBMessageBox( szTitle, szMsg, "Ok" );
		return;
	}

	int linksBefore, linksAfter;
	double threshold = 0.01;

	const char* szTitle = LoadStringFromResource1(IDS_TITLE);
	const char* szMsg = LoadStringFromResource2(IDS_SKIN_THRESHOLD);

	if ( 1 == FBMessageBoxGetUserValue( szTitle, szMsg, &threshold, kFBPopupDouble, "Ok", "Cancel" ) )
	{
		SkinCleanup( llist.GetAt(0), threshold, linksBefore, linksAfter );

		const int text_size = 128;
		char text[text_size];

		sprintf_s( text, text_size, "Links before - %d; links after - %d", linksBefore, linksAfter );

		FBMessageBox( szTitle, text, "Ok" );
	}
}

void ORTool_BlendShape::EventButtonLoadClick( HISender pSender, HKEvent pEvent )
{
	//
	const char* szTitle = LoadStringFromResource1(IDS_TITLE);
	const char* szMsg = LoadStringFromResource2(IDS_LOAD_MODE);
	int mode = FBMessageBox( szTitle, szMsg, "Load", "Merge", "Append" );

	FBFilePopup	lPopup;
	lPopup.Caption = "Choose a file for loading blendshapes";
	lPopup.FileName = "*.xml";
	lPopup.Style = kFBFilePopupOpen;

	if (lPopup.Execute() )
	{
		FBString fullFileName ( lPopup.FullFilename );

		// load xml
		FBModelList modelList;
		for (int iModel=0; iModel<mContainerObjects.Items.GetCount(); ++iModel)
		{
			modelList.Add( (FBModel*) mContainerObjects.Items.GetReferenceAt(iModel) );
		}

		if (modelList.GetCount() )
		{
			Blendshapes_LoadXML( modelList, fullFileName, (FBBlendShapeLoadMode) mode );
			UpdateBlendShapesView(true);
		}
		else
		{
			FBMessageBox( szTitle, "Container is empty", "Ok" );
		}
	}
}

void ORTool_BlendShape::EventButtonSaveClick( HISender pSender, HKEvent pEvent )
{
	const char* szTitle = LoadStringFromResource1(IDS_TITLE);

	//
	FBFilePopup	lPopup;
	lPopup.Caption = "Choose a file for saving blendshapes";
	lPopup.FileName = "*.xml";
	lPopup.Style = kFBFilePopupSave;

	if (lPopup.Execute() )
	{
		FBString fullFileName ( lPopup.FullFilename );

		// save xml
		FBModelList modelList;
		for (int iModel=0; iModel<mContainerObjects.Items.GetCount(); ++iModel)
		{
			modelList.Add( (FBModel*) mContainerObjects.Items.GetReferenceAt(iModel) );
		}

		if (modelList.GetCount() )
		{
			Blendshapes_SaveXML( modelList, fullFileName );
		}
		else
		{
			FBMessageBox( szTitle, "Container is empty", "Ok" );
		}
	}
}
