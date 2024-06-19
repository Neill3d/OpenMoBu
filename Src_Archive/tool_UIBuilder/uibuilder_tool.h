
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


#ifndef __TOOL_UIBUILDER_H__
#define __TOOL_UIBUILDER_H__

/**	\file	uibuilder_tool.h

	Author - Sergey Solohin (Neill)
	homepage - http://neill3d.com/tool-programming-in-mobu/ui-builder?langswitch_lang=en
*/

//--- SDK include
#include <fbsdk/fbsdk.h>
#include "ui.h"

//--- Registration define
#define ORTOOLTEMPLATE__CLASSNAME	UIBuilderTool
#define ORTOOLTEMPLATE__CLASSSTR	"UIBuilderTool"


enum SceneAction {saSelect, saRegion, saPreview};

/**	Tool template.
*/
class UIBuilderTool : public FBTool
{
	//--- FiLMBOX Tool declaration.
	FBToolDeclare( UIBuilderTool, FBTool );

public:
	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

	virtual bool FbxStore		( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );
	virtual bool FbxRetrieve	( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );

private:
	void		EventButtonFileClick( HISender pSender, HKEvent pEvent );
	void		EventButtonEditClick( HISender pSender, HKEvent pEvent );
	void		EventButtonHelpClick( HISender pSender, HKEvent pEvent );
	void		EventButtonAddClick( HISender pSender, HKEvent pEvent );
	void		EventButtonDelClick( HISender pSender, HKEvent pEvent );
	void		EventButtonRunClick( HISender pSender, HKEvent pEvent );
	void		EventButtonAssignClick( HISender pSender, HKEvent pEvent );
	void		EventToolIdle		( HISender pSender, HKEvent pEvent );
	void		EventToolShow		( HISender pSender, HKEvent pEvent );
	void		EventToolPaint		( HISender pSender, HKEvent pEvent );
	void		EventToolResize		( HISender pSender, HKEvent pEvent );
	void		EventToolInput		( HISender pSender, HKEvent pEvent );

	void		EventSelectInput		( HISender pSender, HKEvent pEvent );
	void		EventRegionInput		( HISender pSender, HKEvent pEvent );
	void		EventPreviewInput		( HISender pSender, HKEvent pEvent );

	void		EventPropertiesTabChange		( HISender pSender, HKEvent pEvent );
	void		EventToolCellChange		( HISender pSender, HKEvent pEvent );
	void		EventCompCellChange		( HISender pSender, HKEvent pEvent );
	void		EventCellChange		( HISender pSender, HKEvent pEvent );
	void		EventNavigatorSel	( HISender pSender, HKEvent pEvent );

	void		EventActionsChange		( HISender pSender, HKEvent pEvent );

	void		EventListChange		( HISender pSender, HKEvent pEvent );
	void		EventMemoChange		( HISender pSender, HKEvent pEvent );

public:
	void		RebuildNavigatorTree();
	void		RebuildAttachList();
	void		PrepareWndProc();
	void		RebuildProperties();
	void		ShowContextMenu();
	void		EnterPreviewMode();
	void		ClosePreviewMode();
	bool		ToggleSnapMode();
	void		SetSnapStep();
public:

	// code editor
	FBList		mEventsList;
	FBMemo		mEventCode;

	// layout for arranging components
	FBLayout	mFormView;
	
	// sections of navigator and properties
	FBTree		mNavigator;
	FBTabPanel	mPropertiesTab;
	FBLayout	mProperties;

	FBSpread	mToolTable;		// tool properties
	FBSpread	mPropTable;		// region properties
	FBSpread	mCompTable;		// visual component properties
	FBSpread	mEventTable;	// events list

	//! buttons for elements creation
	FBButton	mButton;

	//! common ui functionality
	uimain			mRects;
	//! use snap in movement
	int					mSnapStep;
	int					mOldSnapStep;

	
	FBTreeNode	*mLayoutRoot;

	FBButton	mButtonFile;
	FBButton	mButtonEdit;
	FBButton	mButtonHelp;
	FBButton	mButtonRun;
	FBButton  mButtonAssign;

	FBLabel		mHint;

	FBVisualContainer	mActions;
	SceneAction	mAction;
};

#endif /* __TOOL_UIBUILDER_H__ */
