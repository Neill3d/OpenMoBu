
/**	\file	ortoolview360_tool.cxx

	Author Sergey Solokhin (Neill3d)
	 e-mail to: s@neill3d.com
	  www.neill3d.com

*/



//--- Class declaration
#include "tool_view360_tool.h"

//--- Registration defines
#define ORTOOLVIEW360__CLASS	ORTOOLVIEW360__CLASSNAME
#define ORTOOLVIEW360__LABEL	"View 360 Tool"
#define ORTOOLVIEW360__DESC		"View 360 Tool"



//--- Implementation and registration
FBToolImplementation(	ORTOOLVIEW360__CLASS	);
FBRegisterTool		(	ORTOOLVIEW360__CLASS,
						ORTOOLVIEW360__LABEL,
						ORTOOLVIEW360__DESC,
						FB_DEFAULT_SDK_ICON	);	// Icon filename (default=Open Reality icon)

extern QWidget* CreateQtTestWidget( QWidget* pParent );
extern void ToggleMaximize(const bool maximized);

/************************************************
 *	Constructor.
 ************************************************/
bool ORToolView360::FBCreate()
{
	
	StartSize[0] = 640;
	StartSize[1] = 480;

	// Manage UI
	UICreate	();
	UIConfigure	();
	UIReset		();

	// Add tool callbacks
	OnShow.Add	( this, (FBCallback) &ORToolView360::EventToolShow		);
	//OnIdle.Add	( this, (FBCallback) &ORToolView360::EventToolIdle		);
	OnResize.Add( this, (FBCallback) &ORToolView360::EventToolResize		);
	OnPaint.Add	( this, (FBCallback) &ORToolView360::EventToolPaint		);
	OnInput.Add	( this, (FBCallback) &ORToolView360::EventToolInput		);

	mQtHolder.SetCreator( CreateQtTestWidget );

	mUseView1 = false;
	mLastPaneCount = 1;

	mLastWidth = 640;
	mLastHeight = 480;

	return true;
}

/************************************************
 *	Create, configure & reset UI.
 ************************************************/
void ORToolView360::UICreate()
{

	// Tool options
	int lS = 2;

	// Configure layout
	AddRegion( "ViewPane0", "ViewPane0",
										0,	kFBAttachLeft,	"",	1.0,
										0,	kFBAttachTop,	"",	1.0,
										0,	kFBAttachRight,	"",	1.0,
										0,	kFBAttachBottom,"",	1.0 );

	AddRegion( "temp", "temp",
										-lS,	kFBAttachNone,	"",	1.0,
										-lS,	kFBAttachNone,	"",	1.0,
										lS,		kFBAttachNone,	"",	1.0,
										lS,		kFBAttachNone,"",	1.0 );

	// Assign regions
	SetView		( "ViewPane0",				mView );
	SetControl( "temp", mQtHolder );
}
void ORToolView360::UIConfigure()
{
}
void ORToolView360::UIReset()
{
}

/************************************************
 *	Destruction function.
 ************************************************/
void ORToolView360::FBDestroy()
{
	// Remove tool callbacks
	OnShow.Remove	( this, (FBCallback) &ORToolView360::EventToolShow	);
	OnIdle.Remove	( this, (FBCallback) &ORToolView360::EventToolIdle	);
	OnPaint.Remove	( this, (FBCallback) &ORToolView360::EventToolPaint	);
	OnInput.Remove	( this, (FBCallback) &ORToolView360::EventToolInput	);
	OnResize.Remove	( this, (FBCallback) &ORToolView360::EventToolResize	);

	// Free user allocated memory
}

/************************************************
 *	UI Idle callback.
 ************************************************/
void ORToolView360::EventToolIdle( HISender pSender, HKEvent pEvent )
{
	RefreshView();
}

void ORToolView360::RefreshView()
{
	mView.Refresh(false);

	//if (true == mUseView1)
	//	mView1.Refresh(false);
}

/************************************************
 *	Handle tool activation (selection/unselection).
 ************************************************/
void ORToolView360::EventToolShow( HISender pSender, HKEvent pEvent )
{
	FBEventShow lEvent( pEvent );

	if( lEvent.Shown )
	{
		// Reset the UI here.
		//FBSystem::TheOne().Renderer->CloneViewAdd( &mView );
		OnIdle.Add	( this, (FBCallback) &ORToolView360::EventToolIdle		);
	}
	else
	{
		//FBSystem::TheOne().Renderer->CloneViewRemove( &mView );
		OnIdle.Remove ( this, (FBCallback) &ORToolView360::EventToolIdle		);
	}
}


/************************************************
 *	Paint callback for tool (on expose).
 ************************************************/
void ORToolView360::EventToolPaint( HISender pSender, HKEvent pEvent )
{
	RefreshView();
}

/************************************************
 *	Tool resize callback.
 ************************************************/
void ORToolView360::EventToolResize( HISender pSender, HKEvent pEvent )
{
	int pW = mLastWidth;
	int pH = mLastHeight;

	if (nullptr != pEvent)
	{
		FBEventResize	lEvent(pEvent);

		if (lEvent.Width >= 1 && lEvent.Height >= 1)
		{
			pW = lEvent.Width;
			pH = lEvent.Height;
		}
	}

	//
	mLastWidth = pW;
	mLastHeight = pH;
}

/************************************************
 *	Handle input into the tool.
 ************************************************/
void ORToolView360::EventToolInput( HISender pSender, HKEvent pEvent )
{
}

/************************************************
 *	FBX Storage.
 ************************************************/
bool ORToolView360::FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	return true;
}

/************************************************
 *	FBX Retrieval.
 ************************************************/
bool ORToolView360::FbxRetrieve( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	return true;
}

