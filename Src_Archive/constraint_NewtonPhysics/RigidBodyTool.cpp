
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include <windows.h>
#include "rigidBodyTool.h"

//--- Registration defines
#define RIGIDBODY_TOOL__CLASS	RIGIDBODY_TOOL__CLASSNAME
#define RIGIDBODY_TOOL__LABEL	"Rigid body tool"
#define RIGIDBODY_TOOL__DESC	"Modify rigid body properties"

//--- FiLMBOX implementation and registration
FBToolImplementation(	RIGIDBODY_TOOL__CLASS	);
FBRegisterTool		(	RIGIDBODY_TOOL__CLASS,
					 RIGIDBODY_TOOL__LABEL,
					 RIGIDBODY_TOOL__DESC,
					 FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool RigidBodyTool::FBCreate()
{
	// Tool options
	StartSize[0] = 235;
	StartSize[1] = 200;

	int lB = 10;
	//int lS = 4;
	int lW = 100;
	int lH = 24;

	// Configure layout
	AddRegion( "RigidBodyMassLabel", "RigidBodyMassLabel",
		lB,	kFBAttachLeft,	"",	1.0	,
		lB,	kFBAttachTop,	"",	1.0,
		lW,	kFBAttachNone,	"",	1.0,
		lH,	kFBAttachNone,	"",	1.0 );
	AddRegion( "RigidBodyMass", "RigidBodyMass",
		lB,	kFBAttachRight,	"RigidBodyMassLabel",	1.0	,
		lB,	kFBAttachTop,	"",	1.0,
		lW,	kFBAttachNone,	"",	1.0,
		lH,	kFBAttachNone,	"",	1.0 );
	// control buttons
	AddRegion( "ButtonApply", "ButtonApply",
		lB,	kFBAttachLeft,	"",				1.0	,
		-lB-lH,	kFBAttachBottom,"",	1.0,
		lW,	kFBAttachNone,	"",				1.0,
		lH,	kFBAttachNone,	"",				1.0 );
	AddRegion( "ButtonClear", "ButtonClear",
		4,	kFBAttachRight,	"ButtonApply",	1.0	,
		-lB-lH,	kFBAttachBottom,"",	1.0,
		lW,	kFBAttachNone,	"",				1.0,
		lH,	kFBAttachNone,	"",				1.0 );

	SetControl( "RigidBodyMassLabel", mLabelMass );
	SetControl( "RigidBodyMass", mEditMass );

	SetControl( "ButtonApply", mButtonApply );
	SetControl( "ButtonClear", mButtonClear );
	
	mLabelMass.Caption = "Mass:";

	// Configure button's
	mButtonApply.Caption = "Apply";
	mButtonApply.PropertyList.Find( "Hint" )->SetString( "apply rigid body properties to selection" );
	mButtonClear.Caption = "Clear";
	mButtonClear.PropertyList.Find( "Hint" )->SetString( "clear rigid body properties from selection" );
	
	mButtonApply.OnClick.Add( this, (FBCallback) &RigidBodyTool::EventButtonApplyClick );
	mButtonClear.OnClick.Add( this, (FBCallback) &RigidBodyTool::EventButtonClearClick );

	// Add tool callbacks
	OnShow.Add	( this, (FBCallback) &RigidBodyTool::EventToolShow		);
	OnIdle.Add	( this, (FBCallback) &RigidBodyTool::EventToolIdle		);
	OnResize.Add( this, (FBCallback) &RigidBodyTool::EventToolResize	);
	OnPaint.Add	( this, (FBCallback) &RigidBodyTool::EventToolPaint		);
	OnInput.Add	( this, (FBCallback) &RigidBodyTool::EventToolInput		);

	return true;
}


/************************************************
*	FiLMBOX Destruction function.
************************************************/
void RigidBodyTool::FBDestroy()
{
	mButtonApply.OnClick.Remove( this, (FBCallback) &RigidBodyTool::EventButtonApplyClick );
	mButtonClear.OnClick.Remove( this, (FBCallback) &RigidBodyTool::EventButtonClearClick );

	// Remove tool callbacks
	OnShow.Remove	( this, (FBCallback) &RigidBodyTool::EventToolShow	);
	OnIdle.Remove	( this, (FBCallback) &RigidBodyTool::EventToolIdle	);
	OnPaint.Remove	( this, (FBCallback) &RigidBodyTool::EventToolPaint );
	OnInput.Remove	( this, (FBCallback) &RigidBodyTool::EventToolInput );
	OnResize.Remove	( this, (FBCallback) &RigidBodyTool::EventToolResize);

	// Free user allocated memory
}

/************************************************
*	Save Button click callback.
************************************************/
void RigidBodyTool::EventButtonApplyClick( HISender pSender, HKEvent pEvent )
{
	FBModelList		lModelList;
	FBGetSelectedModels( lModelList );

	for (int i=0; i<lModelList.GetCount(); ++i)
	{
		HFBModel lModel = lModelList[i];

		// mass property
		HFBProperty lProp = lModel->PropertyList.Find( "RigidBody_Mass" );
		if (lProp)
		{
			double temp = mEditMass.Value;

			lProp->SetData( &temp );
		} else {
			lProp = lModel->PropertyCreate( "RigidBody_Mass", kFBPT_double, ANIMATIONNODE_TYPE_NUMBER,
										false, true, NULL );
			double temp = mEditMass.Value;
			lProp->SetData( &temp );
		}
		// index property
		lProp = lModel->PropertyList.Find( "RigidBody_Index" );
		if (lProp)
		{
			lProp->SetInt(0);
		} else {
			lProp = lModel->PropertyCreate( "RigidBody_Index", kFBPT_int, ANIMATIONNODE_TYPE_INTEGER,
				false, true, NULL );
			lProp->SetInt(0);
		}
	}
}

/************************************************
*	Button click callback.
************************************************/
void RigidBodyTool::EventButtonClearClick( HISender pSender, HKEvent pEvent )
{
	FBModelList		lModelList;
	FBGetSelectedModels( lModelList );

	for (int i=0; i<lModelList.GetCount(); ++i)
	{
		HFBModel lModel = lModelList[i];

		// remove mass property
		HFBProperty lProp = lModel->PropertyList.Find( "RigidBody_Mass" );
		if (lProp){
			lModel->PropertyRemove(lProp);
			lProp = NULL;
		}
		// remove index property
		lProp = lModel->PropertyList.Find( "RigidBody_Index" );
		if (lProp){
			lModel->PropertyRemove(lProp);
			lProp = NULL;
		}
	}		
}


/************************************************
*	UI Idle callback.
************************************************/
void RigidBodyTool::EventToolIdle( HISender pSender, HKEvent pEvent )
{
}


/************************************************
*	Handle tool activation (selection/unselection).
************************************************/
void RigidBodyTool::EventToolShow( HISender pSender, HKEvent pEvent )
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
void RigidBodyTool::EventToolPaint( HISender pSender, HKEvent pEvent )
{
}


/************************************************
*	Tool resize callback.
************************************************/
void RigidBodyTool::EventToolResize( HISender pSender, HKEvent pEvent )
{
}


/************************************************
*	Handle input into the tool.
************************************************/
void RigidBodyTool::EventToolInput( HISender pSender, HKEvent pEvent )
{
}


/************************************************
*	FBX Storage.
************************************************/
bool RigidBodyTool::FbxStore	( HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	pFbxObject->FieldWriteBegin( "RigidBodyTool_Section" );
	{
		//		pFbxObject->FieldWriteC( mButtonTest.Caption );
	}
	pFbxObject->FieldWriteEnd();
	*/

	return true;
}

/************************************************
*	FBX Retrieval.
************************************************/
bool RigidBodyTool::FbxRetrieve( HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	pFbxObject->FieldReadBegin( "RigidBodyTool_Section" );
	{
		//		mButtonTest.Caption = pFbxObject->FieldReadC();
	}
	pFbxObject->FieldReadEnd();
	*/

	return true;
}