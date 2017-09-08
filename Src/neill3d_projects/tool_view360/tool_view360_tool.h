
#pragma once

/**	\file	tool_view360_tool.h
*	Tool with 3D view
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

#include "tool_view360_view.h"

//--- Registration define
#define ORTOOLVIEW360__CLASSNAME	ORToolView360
#define ORTOOLVIEW360__CLASSSTR		"ORToolView360"

/**	Tool View 360.
*/
class ORToolView360 : public FBTool
{
	//--- Tool declaration.
	FBClassDeclare( ORToolView360, FBTool );
public: 
	//! a constructor
	ORToolView360(const char *pName=NULL)
	  : FBTool(pName) 
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
	
	ORView360		mView;				// 3D view
	
	FBWidgetHolder	mQtHolder;

	bool			mNeedUpdate;
	bool			mUseView1;	// is view1 attached (2 panes mode)
	int				mLastPaneCount;

	int				mLastWidth;
	int				mLastHeight;

};

