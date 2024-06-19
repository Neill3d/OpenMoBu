

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
#include "splitstyle_tool.h"

//--- Registration defines
#define TOOLSPLITSTYLE__CLASS	TOOLSPLITSTYLE__CLASSNAME
#define TOOLSPLITSTYLE__LABEL	"SplitStyle Example"
#define TOOLSPLITSTYLE__DESC	"MCL - How to use SetSplitStyle"

//--- FiLMBOX implementation and registration
FBToolImplementation(	TOOLSPLITSTYLE__CLASS	);
FBRegisterTool		(	TOOLSPLITSTYLE__CLASS,
						TOOLSPLITSTYLE__LABEL,
						TOOLSPLITSTYLE__DESC,
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool Tool_SplitStyle::FBCreate()
{
	// Tool options
	StartSize[0] = 400;
	StartSize[1] = 400;

    int lB = 10;
	//int lS = 4;
	int lW = 100;
	int lH = 300;

	// Configure layout
	AddRegion( "Layout0", "Layout0",
										lB,	kFBAttachLeft,	"",	1.0	,
										lB,	kFBAttachTop,	"",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	AddRegion( "Layout1", "Layout1",
										lB,	kFBAttachRight,	"Layout0",	1.0	,
										lB,	kFBAttachTop,	"",	1.0,
										-lB,	kFBAttachRight,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	
	SetControl( "Layout0", mLayout0 );
	//SetBorder( "Layout0", kFBHighlightBorder, true, true, 4, 3, 90.0f, 0 );
	SetSplitStyle( "Layout0", kFBVSplit );
	SetControl( "Layout1", mLayout1 );
	//SetBorder( "Layout1", kFBHighlightBorder, true, true, 4, 3, 90.0f, 0 );


	mLayout0.AddRegion( "Region", "Layout 0",
										lB,	kFBAttachLeft,	"",	1.0	,
										lB,	kFBAttachTop,	"",	1.0,
										-lB,	kFBAttachRight,	"",	1.0,
										-lB,	kFBAttachBottom,	"",	1.0 );
	mLayout0.SetControl( "Region", mRegion0 );
	mLayout0.SetBorder( "Region", kFBHighlightBorder, true, true, 4, 3, 90.0f, 0 );

	mLayout1.AddRegion( "Region", "Layout 1",
										lB,	kFBAttachLeft,	"",	1.0	,
										lB,	kFBAttachTop,	"",	1.0,
										-lB,	kFBAttachRight,	"",	1.0,
										-lB,	kFBAttachBottom,	"",	1.0 );
	mLayout1.SetControl( "Region", mRegion1 );
	mLayout1.SetBorder( "Region", kFBHighlightBorder, true, true, 4, 3, 90.0f, 0 );
	
	// Add tool callbacks
	OnShow.Add	( this, (FBCallback) &Tool_SplitStyle::EventToolShow		);
	OnIdle.Add	( this, (FBCallback) &Tool_SplitStyle::EventToolIdle		);
	OnResize.Add( this, (FBCallback) &Tool_SplitStyle::EventToolResize	);
	OnPaint.Add	( this, (FBCallback) &Tool_SplitStyle::EventToolPaint		);
	OnInput.Add	( this, (FBCallback) &Tool_SplitStyle::EventToolInput		);

	return true;
}


/************************************************
 *	FiLMBOX Destruction function.
 ************************************************/
void Tool_SplitStyle::FBDestroy()
{
	// Remove tool callbacks
	OnShow.Remove	( this, (FBCallback) &Tool_SplitStyle::EventToolShow	);
	OnIdle.Remove	( this, (FBCallback) &Tool_SplitStyle::EventToolIdle	);
	OnPaint.Remove	( this, (FBCallback) &Tool_SplitStyle::EventToolPaint );
	OnInput.Remove	( this, (FBCallback) &Tool_SplitStyle::EventToolInput );
	OnResize.Remove	( this, (FBCallback) &Tool_SplitStyle::EventToolResize);

	// Free user allocated memory
}

/************************************************
 *	UI Idle callback.
 ************************************************/
void Tool_SplitStyle::EventToolIdle( HISender pSender, HKEvent pEvent )
{
}


/************************************************
 *	Handle tool activation (selection/unselection).
 ************************************************/
void Tool_SplitStyle::EventToolShow( HISender pSender, HKEvent pEvent )
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
void Tool_SplitStyle::EventToolPaint( HISender pSender, HKEvent pEvent )
{
}


/************************************************
 *	Tool resize callback.
 ************************************************/
void Tool_SplitStyle::EventToolResize( HISender pSender, HKEvent pEvent )
{
}


/************************************************
 *	Handle input into the tool.
 ************************************************/
void Tool_SplitStyle::EventToolInput( HISender pSender, HKEvent pEvent )
{
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool Tool_SplitStyle::FbxStore	( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
{
	return true;
}

/************************************************
 *	FBX Retrieval.
 ************************************************/
bool Tool_SplitStyle::FbxRetrieve( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
{
	return true;
}


