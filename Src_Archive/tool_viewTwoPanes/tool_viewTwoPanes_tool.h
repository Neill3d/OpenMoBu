
#pragma once

/**	\file	tool_viewTwoPanes_tool.h
*	Tool with 3D viewer and one/two panes support.
*/

/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014-2017
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>

#include "tool_viewTwoPanes_view.h"

//--- Registration define
#define ORTOOLVIEWTWOPANES__CLASSNAME	ORToolViewTwoPanes
#define ORTOOLVIEWTWOPANES__CLASSSTR	"ORToolViewTwoPanes"

/**	Tool View Two Panes.
*/
class ORToolViewTwoPanes : public FBTool
{
	//--- Tool declaration.
	FBClassDeclare( ORToolViewTwoPanes, FBTool );
public: 
	//! a constructor
	ORToolViewTwoPanes(const char *pName=NULL)
	  :FBTool(pName) 
	  ,mView(0)
	  ,mView1(1)
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
	
	ORViewPane		mView;				// 3D view
	ORViewPane		mView1;				// pane1 3d view (for VR)

	FBWidgetHolder	mQtHolder;

	bool			mNeedUpdate;
	bool			mUseView1;	// is view1 attached (2 panes mode)
	int				mLastPaneCount;

	int				mLastWidth;
	int				mLastHeight;

};

