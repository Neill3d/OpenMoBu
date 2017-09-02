
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
#include "windowsSubMenu_tool.h"

//--- Registration defines
#define ORTOOLTEMPLATE__CLASS	ORTOOLTEMPLATE__CLASSNAME
#define ORTOOLTEMPLATE__LABEL	"WindowSubMenu sample"
#define ORTOOLTEMPLATE__DESC	"WindowSubMenu sample - neill3d.com"

//--- FiLMBOX implementation and registration
FBToolImplementation(	ORTOOLTEMPLATE__CLASS	);
FBRegisterTool		(	ORTOOLTEMPLATE__CLASS,
						ORTOOLTEMPLATE__LABEL,
						ORTOOLTEMPLATE__DESC,
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)




/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ORTool_Template::FBCreate()
{
	// Tool options
	StartSize[0] = 400;
	StartSize[1] = 300;

    int lB = 10;
	//int lS = 4;
	int lW = 100;
	int lH = 18;

	// Configure layout
	AddRegion( "ButtonTest", "ButtonTest",
										lB,	kFBAttachLeft,	"",	1.0	,
										lB,	kFBAttachTop,	"",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	mMenu.SetTitle( ORTOOLTEMPLATE__LABEL );
	mMenu.SetHandle( &mMenuHandle );
	
	SetControl( "ButtonTest", mMenu.GetControl() );
	mMenu.GetControl().Caption = "Test >";
	
	// Add tool callbacks
	OnShow.Add	( this, (FBCallback) &ORTool_Template::EventToolShow		);
	OnIdle.Add	( this, (FBCallback) &ORTool_Template::EventToolIdle		);
	OnResize.Add( this, (FBCallback) &ORTool_Template::EventToolResize	);
	OnPaint.Add	( this, (FBCallback) &ORTool_Template::EventToolPaint		);
	OnInput.Add	( this, (FBCallback) &ORTool_Template::EventToolInput		);

	return true;
}


/************************************************
 *	FiLMBOX Destruction function.
 ************************************************/
void ORTool_Template::FBDestroy()
{
	// Remove tool callbacks
	OnShow.Remove	( this, (FBCallback) &ORTool_Template::EventToolShow	);
	OnIdle.Remove	( this, (FBCallback) &ORTool_Template::EventToolIdle	);
	OnPaint.Remove	( this, (FBCallback) &ORTool_Template::EventToolPaint );
	OnInput.Remove	( this, (FBCallback) &ORTool_Template::EventToolInput );
	OnResize.Remove	( this, (FBCallback) &ORTool_Template::EventToolResize);

	// Free user allocated memory
}





/************************************************
 *	UI Idle callback.
 ************************************************/
void ORTool_Template::EventToolIdle( HISender pSender, HKEvent pEvent )
{
}


/************************************************
 *	Handle tool activation (selection/unselection).
 ************************************************/
void ORTool_Template::EventToolShow( HISender pSender, HKEvent pEvent )
{
	FBEventShow lEvent( pEvent );

	if( lEvent.Shown )
	{
		// Reset the UI here.
		// Re assign window handle
		mMenu.Update();
	}
	else
	{
	}
}


/************************************************
 *	Paint callback for tool (on expose).
 ************************************************/
void ORTool_Template::EventToolPaint( HISender pSender, HKEvent pEvent )
{
}


/************************************************
 *	Tool resize callback.
 ************************************************/
void ORTool_Template::EventToolResize( HISender pSender, HKEvent pEvent )
{
}


/************************************************
 *	Handle input into the tool.
 ************************************************/
void ORTool_Template::EventToolInput( HISender pSender, HKEvent pEvent )
{
}


