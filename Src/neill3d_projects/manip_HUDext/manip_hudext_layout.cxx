
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


//--- Class declarations
#include "manip_hudext_layout.h"

//--- Registration define
#define ORMANIPTEMPLATE__LAYOUT		Manip_HUDext_Layout

//--- FiLMBOX implementation and registration
FBManipulatorLayoutImplementation	(	ORMANIPTEMPLATE__LAYOUT		);
FBRegisterManipulatorLayout			(	ORMANIPTEMPLATE__LAYOUT,
										ORMANIPTEMPLATE__CLASSSTR,
										FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool Manip_HUDext_Layout::FBCreate()
{
	if( FBManipulatorLayout::FBCreate() )
	{
		// Affect the handle on the manipulator.
		mManipulator = ((Manip_HUDext *)(FBManipulator *) Manipulator);

		// Create & configure the UI
		UICreate	();
		UIConfigure	();
		UIReset		();

		StartSize[0] = 300;
		StartSize[1] = 120;

		return true;
	}
	return false;
}


/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void Manip_HUDext_Layout::FBDestroy()
{
	FBManipulatorLayout::FBDestroy();
}


/************************************************
 *	Create the UI.
 ************************************************/
void Manip_HUDext_Layout::UICreate()
{
	int lB = 10;
	int	lS = 4;
	int lH = 50;

	// Add regions
	AddRegion( "LabelAbout",		"LabelAbout",
									lB,		kFBAttachLeft,		"",						1.0,
									lB,		kFBAttachTop,		"",						1.0,
									-lB,	kFBAttachRight,		"",						1.0,
									lH,		kFBAttachNone,		NULL,					1.0 );
	
	// Assign regions
	SetControl("LabelAbout",			mLabelAbout );
}


/************************************************
 *	Configure the UI.
 ************************************************/
void Manip_HUDext_Layout::UIConfigure()
{
	// Configure elements
	mLabelAbout.Caption = "Author Sergey <Neill3d> Solohin 2012\n e-mail to: s@neill3d.com\nwww.neill3d.com";
}


/************************************************
 *	Reset the UI from the manipulator.
 ************************************************/
void Manip_HUDext_Layout::UIReset()
{

}

