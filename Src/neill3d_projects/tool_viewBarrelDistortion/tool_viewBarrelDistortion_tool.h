
#pragma once

/**	\file	tool_viewStereoDistortion_tool.h
*	Tool with 3D viewer and one/two panes support.
*/

/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solokhin (Neill3d) 2014-2017
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>

#include "tool_viewBarrelDistortion_view.h"

//--- Registration define
#define ORTOOLVIEWSTEREO__CLASSNAME	ORToolViewBarrelDistortion
#define ORTOOLVIEWSTEREO__CLASSSTR	"ORToolViewBarrelDistortion"

/**	Tool View Two Panes.
*/
class ORToolViewBarrelDistortion : public FBTool
{
	//--- Tool declaration.
	FBClassDeclare( ORToolViewBarrelDistortion, FBTool );
public: 
	//! a constructor
	ORToolViewBarrelDistortion(const char *pName=NULL)
	  :FBTool(pName) 
	  ,mViewLeft(0)
	  ,mViewRight(1)
	{ 
		FBClassInit; 
	} 

public:
	//--- Construction/Destruction,
	virtual bool FBCreate();		//!< Creation function.
	virtual void FBDestroy();		//!< Destruction function.

	// FBX store/retrieve
	virtual bool FbxStore		( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );
	virtual bool FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );

private:
	// UI Management
	void	UICreate	();
	void	UIConfigure	();
	void	UIReset		();

	// UI callbacks
	void	EventButtonTestClick( HISender pSender, HKEvent pEvent );
	void	EventToolIdle		( HISender pSender, HKEvent pEvent );
	void	EventToolShow		( HISender pSender, HKEvent pEvent );
	void	EventToolPaint		( HISender pSender, HKEvent pEvent );
	void	EventToolResize		( HISender pSender, HKEvent pEvent );
	void	EventToolInput		( HISender pSender, HKEvent pEvent );

	// Refresh the view
	void	RefreshView			();

private:
	
	FBSystem		mSystem;

	ORViewPaneWithDistortion		mViewLeft;				// 3D view for a left eye
	ORViewPaneWithDistortion		mViewRight;				// 3d view for a right eye (for VR)

	FBWidgetHolder	mQtHolder;

	bool			mNeedUpdate;
	bool			mUseView1;	// is view1 attached (2 panes mode)
	
	FBCamera		*mLastCamera;
	int				mLastPaneCount;

	int				mLastWidth;
	int				mLastHeight;

};

