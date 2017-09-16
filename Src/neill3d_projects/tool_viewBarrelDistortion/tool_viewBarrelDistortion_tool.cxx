
/**	\file	tool_viewStereoDistortion_tool.cxx

// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE

	GitHub repo - https://github.com/Neill3d/MoBu

	Author Sergey Solokhin (Neill3d)
	 e-mail to: s@neill3d.com
	  www.neill3d.com

*/



//--- Class declaration
#include "tool_viewBarrelDistortion_tool.h"
#include "utils.h"

//--- Registration defines
#define ORTOOLVIEWSTEREO__CLASS	ORTOOLVIEWSTEREO__CLASSNAME
#define ORTOOLVIEWSTEREO__LABEL	"View Barrel Distortion Tool"
#define ORTOOLVIEWSTEREO__DESC	"View Barrel Distortion Tool"



//--- Implementation and registration
FBToolImplementation(	ORTOOLVIEWSTEREO__CLASS	);
FBRegisterTool		(	ORTOOLVIEWSTEREO__CLASS,
						ORTOOLVIEWSTEREO__LABEL,
						ORTOOLVIEWSTEREO__DESC,
						FB_DEFAULT_SDK_ICON	);	// Icon filename (default=Open Reality icon)

extern QWidget* CreateQtTestWidget( QWidget* pParent );
extern void ToggleMaximize(const bool maximized);

/************************************************
 *	Constructor.
 ************************************************/
bool ORToolViewBarrelDistortion::FBCreate()
{
	
	StartSize[0] = 640;
	StartSize[1] = 480;

	// Manage UI
	UICreate	();
	UIConfigure	();
	UIReset		();

	// Add tool callbacks
	OnShow.Add	( this, (FBCallback) &ORToolViewBarrelDistortion::EventToolShow		);
	//OnIdle.Add	( this, (FBCallback) &ORToolViewBarrelDistortion::EventToolIdle		);
	OnResize.Add( this, (FBCallback) &ORToolViewBarrelDistortion::EventToolResize		);
	OnPaint.Add	( this, (FBCallback) &ORToolViewBarrelDistortion::EventToolPaint		);
	OnInput.Add	( this, (FBCallback) &ORToolViewBarrelDistortion::EventToolInput		);

	mQtHolder.SetCreator( CreateQtTestWidget );

	mUseView1 = false;
	mLastPaneCount = 1;
	mLastCamera = nullptr;

	mLastWidth = 640;
	mLastHeight = 480;

	//
	PrepViewerComponents();

	return true;
}

/************************************************
 *	Create, configure & reset UI.
 ************************************************/
void ORToolViewBarrelDistortion::UICreate()
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
	SetView		( "ViewPane0",				mViewLeft );
	//SetView		( "ViewPane1",				mViewRight );

	SetControl( "temp", mQtHolder );
}
void ORToolViewBarrelDistortion::UIConfigure()
{
}
void ORToolViewBarrelDistortion::UIReset()
{
}

/************************************************
 *	Destruction function.
 ************************************************/
void ORToolViewBarrelDistortion::FBDestroy()
{
	// Remove tool callbacks
	OnShow.Remove	( this, (FBCallback) &ORToolViewBarrelDistortion::EventToolShow	);
	OnIdle.Remove	( this, (FBCallback) &ORToolViewBarrelDistortion::EventToolIdle	);
	OnPaint.Remove	( this, (FBCallback) &ORToolViewBarrelDistortion::EventToolPaint	);
	OnInput.Remove	( this, (FBCallback) &ORToolViewBarrelDistortion::EventToolInput	);
	OnResize.Remove	( this, (FBCallback) &ORToolViewBarrelDistortion::EventToolResize	);

	// Free user allocated memory
}

/************************************************
 *	UI Idle callback.
 ************************************************/
void ORToolViewBarrelDistortion::EventToolIdle( HISender pSender, HKEvent pEvent )
{
	int paneCount=1;
	FBCamera *pCameraPane0 = mSystem.Renderer->CurrentCamera;
	//FBCamera *pCameraPane1=nullptr;
	//GetViewerPaneInfo(paneCount, pCameraPane0, pCameraPane1);

	if (paneCount != mLastPaneCount || mLastCamera != pCameraPane0)
	{
		EventToolResize(nullptr, nullptr);

		mLastPaneCount = paneCount;
		mLastCamera = pCameraPane0;
	}

	RefreshView();
}

void ORToolViewBarrelDistortion::RefreshView()
{
	mViewLeft.Refresh(false);

	if (true == mUseView1)
		mViewRight.Refresh(false);
}

/************************************************
 *	Handle tool activation (selection/unselection).
 ************************************************/
void ORToolViewBarrelDistortion::EventToolShow( HISender pSender, HKEvent pEvent )
{
	FBEventShow lEvent( pEvent );

	if( lEvent.Shown )
	{
		// Reset the UI here.
		//FBSystem::TheOne().Renderer->CloneViewAdd( &mView );
		OnIdle.Add	( this, (FBCallback) &ORToolViewBarrelDistortion::EventToolIdle		);
	}
	else
	{
		//FBSystem::TheOne().Renderer->CloneViewRemove( &mView );
		OnIdle.Remove ( this, (FBCallback) &ORToolViewBarrelDistortion::EventToolIdle		);
	}
}


/************************************************
 *	Paint callback for tool (on expose).
 ************************************************/
void ORToolViewBarrelDistortion::EventToolPaint( HISender pSender, HKEvent pEvent )
{
	RefreshView();
}

/************************************************
 *	Tool resize callback.
 ************************************************/
void ORToolViewBarrelDistortion::EventToolResize( HISender pSender, HKEvent pEvent )
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
	FBCamera *pCameraPane0 = mSystem.Renderer->CurrentCamera;
	FBCamera *pCameraPane1=nullptr;
	//GetViewerPaneInfo(paneCount, pCameraPane0, pCameraPane1);

	if ( nullptr != pCameraPane0 && FBIS(pCameraPane0, FBCameraStereo) )
	{
		FBCameraStereo *pStereo = (FBCameraStereo*) pCameraPane0;

		paneCount = 2;
		pCameraPane0 = pStereo->LeftCamera;
		pCameraPane1 = pStereo->RightCamera;
	}

	if (paneCount <= 1)
	{
		ClearControl("ViewPane1");
		mUseView1 = false;
	}
	else
	{
		pW = pW / 2;
		SetView( "ViewPane1", mViewRight );
		mUseView1 = true;
	}

	SizeRegion("ViewPane0", pW, 0);
	Restructure(false);

	RefreshView();
}

/************************************************
 *	Handle input into the tool.
 ************************************************/
void ORToolViewBarrelDistortion::EventToolInput( HISender pSender, HKEvent pEvent )
{
}

/************************************************
 *	FBX Storage.
 ************************************************/
bool ORToolViewBarrelDistortion::FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	return true;
}

/************************************************
 *	FBX Retrieval.
 ************************************************/
bool ORToolViewBarrelDistortion::FbxRetrieve( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	return true;
}

