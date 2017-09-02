
/**	\file	ortoolview3d_tool.cxx

	Author Sergey Solohin (Neill3d)
	 e-mail to: s@neill3d.com
	  www.neill3d.com

*/



//--- Class declaration
#include "tool_viewTwoPanes_tool.h"
#include "utils.h"

//--- Registration defines
#define ORTOOLVIEWTWOPANES__CLASS	ORTOOLVIEWTWOPANES__CLASSNAME
#define ORTOOLVIEWTWOPANES__LABEL	"View Two Panes Tool"
#define ORTOOLVIEWTWOPANES__DESC	"View Two Panes Tool"



//--- Implementation and registration
FBToolImplementation(	ORTOOLVIEWTWOPANES__CLASS	);
FBRegisterTool		(	ORTOOLVIEWTWOPANES__CLASS,
						ORTOOLVIEWTWOPANES__LABEL,
						ORTOOLVIEWTWOPANES__DESC,
						FB_DEFAULT_SDK_ICON	);	// Icon filename (default=Open Reality icon)

extern QWidget* CreateQtTestWidget( QWidget* pParent );
extern void ToggleMaximize(const bool maximized);

/************************************************
 *	Constructor.
 ************************************************/
bool ORToolViewTwoPanes::FBCreate()
{
	
	StartSize[0] = 640;
	StartSize[1] = 480;

	// Manage UI
	UICreate	();
	UIConfigure	();
	UIReset		();

	// Add tool callbacks
	OnShow.Add	( this, (FBCallback) &ORToolViewTwoPanes::EventToolShow		);
	//OnIdle.Add	( this, (FBCallback) &ORToolViewDistored::EventToolIdle		);
	OnResize.Add( this, (FBCallback) &ORToolViewTwoPanes::EventToolResize		);
	OnPaint.Add	( this, (FBCallback) &ORToolViewTwoPanes::EventToolPaint		);
	OnInput.Add	( this, (FBCallback) &ORToolViewTwoPanes::EventToolInput		);

	mQtHolder.SetCreator( CreateQtTestWidget );

	mUseView1 = false;
	mLastPaneCount = 1;

	mLastWidth = 640;
	mLastHeight = 480;

	//
	PrepViewerComponents();

	return true;
}

/************************************************
 *	Create, configure & reset UI.
 ************************************************/
void ORToolViewTwoPanes::UICreate()
{

	// Tool options
	int lS = 2;

	// Configure layout
	AddRegion( "ViewPane0", "ViewPane0",
										0,	kFBAttachLeft,	"",	1.0,
										0,	kFBAttachTop,	"",	1.0,
										100,kFBAttachNone,	"",	1.0,
										0,kFBAttachBottom,"",	1.0 );

	AddRegion( "ViewPane1", "ViewPane1",
										0,	kFBAttachRight,	"ViewPane0",	1.0,
										0,	kFBAttachTop,	"",	1.0,
										0,kFBAttachRight,	"",	1.0,
										0,kFBAttachBottom,"",	1.0 );

	AddRegion( "temp", "temp",
										-lS,	kFBAttachNone,	"",	1.0,
										-lS,	kFBAttachNone,	"",	1.0,
										lS,		kFBAttachNone,	"",	1.0,
										lS,		kFBAttachNone,"",	1.0 );

	// Assign regions
	SetView		( "ViewPane0",				mView );
	//SetView		( "ViewPane1",				mView1 );

	SetControl( "temp", mQtHolder );
}
void ORToolViewTwoPanes::UIConfigure()
{
}
void ORToolViewTwoPanes::UIReset()
{
}

/************************************************
 *	Destruction function.
 ************************************************/
void ORToolViewTwoPanes::FBDestroy()
{
	// Remove tool callbacks
	OnShow.Remove	( this, (FBCallback) &ORToolViewTwoPanes::EventToolShow	);
	OnIdle.Remove	( this, (FBCallback) &ORToolViewTwoPanes::EventToolIdle	);
	OnPaint.Remove	( this, (FBCallback) &ORToolViewTwoPanes::EventToolPaint	);
	OnInput.Remove	( this, (FBCallback) &ORToolViewTwoPanes::EventToolInput	);
	OnResize.Remove	( this, (FBCallback) &ORToolViewTwoPanes::EventToolResize	);

	// Free user allocated memory
}

/************************************************
 *	UI Idle callback.
 ************************************************/
void ORToolViewTwoPanes::EventToolIdle( HISender pSender, HKEvent pEvent )
{
	int paneCount=0;
	FBCamera *pCameraPane0=nullptr;
	FBCamera *pCameraPane1=nullptr;
	GetViewerPaneInfo(paneCount, pCameraPane0, pCameraPane1);

	if (paneCount != mLastPaneCount)
	{
		EventToolResize(nullptr, nullptr);
		mLastPaneCount = paneCount;
	}

	RefreshView();
}

void ORToolViewTwoPanes::RefreshView()
{
	mView.Refresh(false);

	if (true == mUseView1)
		mView1.Refresh(false);
}

/************************************************
 *	Handle tool activation (selection/unselection).
 ************************************************/
void ORToolViewTwoPanes::EventToolShow( HISender pSender, HKEvent pEvent )
{
	FBEventShow lEvent( pEvent );

	if( lEvent.Shown )
	{
		// Reset the UI here.
		//FBSystem::TheOne().Renderer->CloneViewAdd( &mView );
		OnIdle.Add	( this, (FBCallback) &ORToolViewTwoPanes::EventToolIdle		);
	}
	else
	{
		//FBSystem::TheOne().Renderer->CloneViewRemove( &mView );
		OnIdle.Remove ( this, (FBCallback) &ORToolViewTwoPanes::EventToolIdle		);
	}
}


/************************************************
 *	Paint callback for tool (on expose).
 ************************************************/
void ORToolViewTwoPanes::EventToolPaint( HISender pSender, HKEvent pEvent )
{
	RefreshView();
}

/************************************************
 *	Tool resize callback.
 ************************************************/
void ORToolViewTwoPanes::EventToolResize( HISender pSender, HKEvent pEvent )
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

	//
	//

	int paneCount=0;
	FBCamera *pCameraPane0=nullptr;
	FBCamera *pCameraPane1=nullptr;
	GetViewerPaneInfo(paneCount, pCameraPane0, pCameraPane1);

	if (paneCount <= 1)
	{
		ClearControl("ViewPane1");
		mUseView1 = false;
	}
	else
	{
		pW = pW / 2;
		SetView( "ViewPane1", mView1 );
		mUseView1 = true;
	}

	SizeRegion("ViewPane0", pW, 0);
	Restructure(false);

	RefreshView();
}

/************************************************
 *	Handle input into the tool.
 ************************************************/
void ORToolViewTwoPanes::EventToolInput( HISender pSender, HKEvent pEvent )
{
}

/************************************************
 *	FBX Storage.
 ************************************************/
bool ORToolViewTwoPanes::FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	return true;
}

/************************************************
 *	FBX Retrieval.
 ************************************************/
bool ORToolViewTwoPanes::FbxRetrieve( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	return true;
}

