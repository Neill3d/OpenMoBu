

/**	\file	manip_skelet_layout.cxx
*	Definition of the layout class for a simple manipulator.
*	Contains the definition of the layout class ORManip_Template_Layout.
*/

//--- Class declarations
#include "manip_skelet_layout.h"

//--- Registration define
#define MANIPSKELET__LAYOUT		Manip_Skelet_Layout

//--- FiLMBOX implementation and registration
FBManipulatorLayoutImplementation	(	MANIPSKELET__LAYOUT		);
FBRegisterManipulatorLayout			(	MANIPSKELET__LAYOUT,
										MANIPSKELET__CLASSSTR,
										FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool Manip_Skelet_Layout::FBCreate()
{
	if( FBManipulatorLayout::FBCreate() )
	{
		// Affect the handle on the manipulator.
		mManipulator = ((Manip_Skelet *)(FBManipulator *) Manipulator);

		// Create & configure the UI
		UICreate	();
		UIConfigure	();
		UIReset		();

		return true;
	}
	return false;
}


/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void Manip_Skelet_Layout::FBDestroy()
{
	FBManipulatorLayout::FBDestroy();
}


/************************************************
 *	Create the UI.
 ************************************************/
void Manip_Skelet_Layout::UICreate()
{
	int lB = 10;
	int	lS = 4;
	int lH = 18;

	// Add regions
	AddRegion( "ButtonTest",		"ButtonTest",
									lB,		kFBAttachLeft,		"",						1.0,
									lB,		kFBAttachTop,		"",						1.0,
									-lB,	kFBAttachRight,		"",						1.0,
									lH,		kFBAttachNone,		NULL,					1.0 );
	AddRegion( "ButtonModelSpace", "ButtonModelSpace",
									0,		kFBAttachLeft,		"ButtonTest",			1.0,
									lS,		kFBAttachBottom,	"ButtonTest",			1.0,
									0,		kFBAttachWidth,		"ButtonTest",			1.0,
									0,		kFBAttachHeight,	"ButtonTest",			1.0 );
	AddRegion( "ButtonDrawSquare", "ButtonDrawSquare",
									0,		kFBAttachLeft,		"ButtonModelSpace",		1.0,
									lS,		kFBAttachBottom,	"ButtonModelSpace",		1.0,
									0,		kFBAttachWidth,		"ButtonModelSpace",		1.0,
									0,		kFBAttachHeight,	"ButtonModelSpace",		1.0 );
	AddRegion( "ButtonDefaultBehavior", "ButtonDefaultBehavior",
									0,		kFBAttachLeft,		"ButtonDrawSquare",		1.0,
									lS,		kFBAttachBottom,	"ButtonDrawSquare",		1.0,
									0,		kFBAttachWidth,		"ButtonDrawSquare",		1.0,
									0,		kFBAttachHeight,	"ButtonDrawSquare",		1.0 );
	AddRegion( "ButtonAlwaysActive", "ButtonAlwaysActive",
									0,		kFBAttachLeft,		"ButtonDefaultBehavior",1.0,
									lS,		kFBAttachBottom,	"ButtonDefaultBehavior",1.0,
									0,		kFBAttachWidth,		"ButtonDefaultBehavior",1.0,
									0,		kFBAttachHeight,	"ButtonDefaultBehavior",1.0 );

	// Assign regions
	SetControl("ButtonTest",			mButtonTest				);
	SetControl("ButtonModelSpace",		mButtonModelSpace		);
	SetControl("ButtonDrawSquare",		mButtonDrawSquare		);
	SetControl("ButtonDefaultBehavior",	mButtonDefaultBehavior	);
	SetControl("ButtonAlwaysActive",	mButtonAlwaysActive		);
}


/************************************************
 *	Configure the UI.
 ************************************************/
void Manip_Skelet_Layout::UIConfigure()
{
	// Configure elements
	mButtonTest.Caption				= "Create model";
	mButtonModelSpace.Caption		= "Model Space";
	mButtonModelSpace.Style			= kFB2States;
	mButtonDrawSquare.Caption		= "Draw Square";
	mButtonDrawSquare.Style			= kFB2States;
	mButtonDefaultBehavior.Caption	= "Default Behavior";
	mButtonDefaultBehavior.Style	= kFB2States;
	mButtonAlwaysActive.Caption		= "Always Active";
	mButtonAlwaysActive.Style		= kFB2States;

	// Add callbacks
	mButtonTest.OnClick.Add				( this, (FBCallback) &Manip_Skelet_Layout::EventButtonTestClick				);
	mButtonModelSpace.OnClick.Add		( this, (FBCallback) &Manip_Skelet_Layout::EventButtonModelSpaceClick			);
	mButtonDrawSquare.OnClick.Add		( this, (FBCallback) &Manip_Skelet_Layout::EventButtonDrawSquareClick			);
	mButtonDefaultBehavior.OnClick.Add	( this, (FBCallback) &Manip_Skelet_Layout::EventButtonDefaultBehaviorClick	);
	mButtonAlwaysActive.OnClick.Add		( this, (FBCallback) &Manip_Skelet_Layout::EventButtonAlwaysActiveClick		);
}


/************************************************
 *	Reset the UI from the manipulator.
 ************************************************/
void Manip_Skelet_Layout::UIReset()
{
	mButtonModelSpace.State			= mManipulator->mRenderInModelSpace	?	1:0;
	mButtonDrawSquare.State			= mManipulator->mDrawSquare			?	1:0;
	mButtonDefaultBehavior.State	= mManipulator->DefaultBehavior		?	1:0;
	mButtonAlwaysActive.State		= mManipulator->AlwaysActive		?	1:0;
}


/************************************************
 *	Button click callback.
 ************************************************/
void Manip_Skelet_Layout::EventButtonTestClick( HISender pSender, HKEvent pEvent )
{
	switch( mManipulator->mTestState )
	{
		case 0:
		{
			// Create a new marker
			mManipulator->mModel = new FBModelMarker("ORManipulator_Template Model");
			mButtonTest.Caption = "Make visible";
		}
		break;
		case 1:
		{
			// Make the model visible
			if(mManipulator->mModel)
			{
				mManipulator->mModel->Show = true;
			}
			mButtonTest.Caption = "Remove model";
		}
		break;
		case 2:
		{
			// Remove model from visible set
			if( mManipulator->mModel )
			{
				mManipulator->mModel->Show = false;
			}
			mButtonTest.Caption = "Delete model";
		}
		break;
		case 3:
		{
			// Delete model
			if( mManipulator->mModel )
			{
				mManipulator->mModel->FBDelete();
				mManipulator->mModel = NULL;
			}
			mButtonTest.Caption = "Create model";
			mManipulator->mTestState = -1;
		}
		break;
	}
	mManipulator->mTestState ++;
}


/************************************************
 *	Model Space for square button callback.
 ************************************************/
void Manip_Skelet_Layout::EventButtonModelSpaceClick( HISender pSender, HKEvent pEvent )
{
	mManipulator->mRenderInModelSpace = mButtonModelSpace.State != 0;
	UIReset();
}


/************************************************
 *	Draw Square button callback.
 ************************************************/
void Manip_Skelet_Layout::EventButtonDrawSquareClick( HISender pSender, HKEvent pEvent )
{
	mManipulator->mDrawSquare = mButtonDrawSquare.State != 0;
	UIReset();
}


/************************************************
 *	Default Behavior button callback.
 ************************************************/
void Manip_Skelet_Layout::EventButtonDefaultBehaviorClick( HISender pSender, HKEvent pEvent )
{
	mManipulator->DefaultBehavior = mButtonDefaultBehavior.State != 0;
	UIReset();
}


/************************************************
 *	Always active button callback.
 ************************************************/
void Manip_Skelet_Layout::EventButtonAlwaysActiveClick( HISender pSender, HKEvent pEvent )
{
	mManipulator->AlwaysActive = mButtonAlwaysActive.State != 0;
	UIReset();
}

