#ifndef __ORTOOL_BLENDSHAPE_TOOL_H__
#define __ORTOOL_BLENDSHAPE_TOOL_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BlendShapeTookit_tool.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#define		MAX_PROPERTIES		100

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration define
#define ORTOOLTEMPLATE__CLASSNAME	ORTool_BlendShape
#define ORTOOLTEMPLATE__CLASSSTR	"ORTool_BlendShape"

/**	Tool template.
*/
class ORTool_BlendShape : public FBTool
{
	//--- FiLMBOX Tool declaration.
	FBToolDeclare( ORTool_BlendShape, FBTool );

public:
	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

	virtual bool FbxStore		( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );
	virtual bool FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );

private:
	void		EventButtonLoadClick( HISender pSender, HKEvent pEvent );
	void		EventButtonSaveClick( HISender pSender, HKEvent pEvent );
	//void		EventButtonAutoAssignClick( HISender pSender, HKEvent pEvent );
	void		EventButtonAddClick( HISender pSender, HKEvent pEvent );
	void		EventButtonRemoveClick( HISender pSender, HKEvent pEvent );
	void		EventButtonClearClick( HISender pSender, HKEvent pEvent );
	void		EventButtonDublicateClick( HISender pSender, HKEvent pEvent );
	void		EventButtonRenameClick( HISender pSender, HKEvent pEvent );

	void		EventButtonMoveUpClick( HISender pSender, HKEvent pEvent );
	void		EventButtonMoveDownClick( HISender pSender, HKEvent pEvent );
	
	void		EventButtonSnapshotClick( HISender pSender, HKEvent pEvent );
	void		EventButtonCalcDeltaClick( HISender pSender, HKEvent pEvent );
	void		EventButtonCombineClick( HISender pSender, HKEvent pEvent );
	void		EventButtonCenterPivotClick( HISender pSender, HKEvent pEvent );
	void		EventButtonOptimizeSkinClick( HISender pSender, HKEvent pEvent );
	void		EventButtonReComputeNormalsClick( HISender pSender, HKEvent pEvent );
	void		EventButtonInvertNormalsClick( HISender pSender, HKEvent pEvent );
	void		EventButtonBrushToolClick( HISender pSender, HKEvent pEvent );
	void		EventButtonAboutClick( HISender pSender, HKEvent pEvent );

	void		EventButtonSculptEnterClick( HISender pSender, HKEvent pEvent );
	void		EventButtonSculptLeaveClick( HISender pSender, HKEvent pEvent );
	void		EventButtonSculptToolClick( HISender pSender, HKEvent pEvent );

	void		EventConnectionStateNotify( HISender pSender, HKEvent pEvent );

	void		EventButtonCheckAllClick( HISender pSender, HKEvent pEvent );
	void		EventButtonCheckNoneClick( HISender pSender, HKEvent pEvent );
	void		EventButtonCheckInvertClick( HISender pSender, HKEvent pEvent );

	void	EventContainerDragAndDrop	( HISender pSender, HKEvent pEvent );
	void	EventContainerDblClick		( HISender pSender, HKEvent pEvent );

	void		EventToolIdle		( HISender pSender, HKEvent pEvent );
	void		EventToolShow		( HISender pSender, HKEvent pEvent );
	void		EventToolPaint		( HISender pSender, HKEvent pEvent );
	void		EventToolResize		( HISender pSender, HKEvent pEvent );
	void		EventToolInput		( HISender pSender, HKEvent pEvent );

private:

	FBLabel				mLabelObject;
	FBVisualContainer	mContainerObjects;		// with with these objects blendshapes
	FBButton			mButtonAutoAssign;		// auto assign selected models into the container

	FBButton			mButtonSave;
	FBButton			mButtonLoad;			// combine functions of load, merge, append

	FBScrollBox			mScrollBox;

	FBLabel				mLabelCheck;
	FBButton			mButtonCheckAll;
	FBButton			mButtonCheckNone;
	FBButton			mButtonCheckInvert;

	int					mShapesCount;

	FBLabel				mLabelShapes[MAX_PROPERTIES];	// label for each group of blendshapes
	FBButton			mButtonShapes[MAX_PROPERTIES];	// blend shapes checkboxes
	FBEditProperty		mBlendshapes[MAX_PROPERTIES];
	FBModel				*mUIModels[MAX_PROPERTIES];		// link ui blendshape list to model pointer

	//
	// side control panel

	FBLayout			mLayoutPanel;

	FBArrowButton		mArrowBlendShapes;
	FBLayout			mLayoutBlendShapes;

	FBButton			mButtonAdd;
	FBButton			mButtonRemove;
	FBButton			mButtonRename;
	FBButton			mButtonClear;
	FBButton			mButtonDublicate;
	
	FBButton			mButtonMoveUp;
	FBButton			mButtonMoveDown;

	// remove nearest keyframe
	//FBButton			mButtonRemoveKeyframe;

	//
	FBArrowButton		mArrowOperations;
	FBLayout			mLayoutOperations;

	FBButton			mButtonSnapshot;
	FBList				mListCalcDeltaMode;
	FBButton			mButtonCalcDelta;	// calculate base mesh with only point difference
	FBButton			mButtonCombine;		// combine meshes together including textures, materials and ! clusters
	FBButton			mButtonCombineDeleteSource; // delete all source models
	FBButton			mButtonCenterPivot;	// explore model into separete models (1 model per material)
	FBButton			mButtonOptimizeSkin;	// remove small or empty influences
	FBButton			mButtonReComputeNormals;	// compute smooth normals for a selected meshes
	FBButton			mButtonInvertNormals;

	FBArrowButton		mArrowSculpt;
	FBLayout			mLayoutSculpt;

	FBButton			mButtonSculptEnter;	// enter sculpt mode
	FBButton			mButtonSculptLeave;
	FBButton			mButtonSculptTool;	// launch sculpt tool

	FBArrowButton		mArrowInfo;
	FBLayout			mLayoutInfo;

	FBButton			mButtonBrushTool;
	FBButton			mButtonAbout;

	int					mState;
	HdlFBPlugTemplate<FBModel>	mHdlModel;

	void		UICreate();
	void		UIConfig();
	void		UIReset();

	void		UICreateScrollBox();
	void		UIConfigScrollBox();
	void		UIResetScrollBox();

	void		UICreatePanel();
	void		UIConfigPanel();
	void		UIResetPanel();

	void		SetupBlendShapes( FBModel *pModel );

	//
	//	\param resetCheckState read-only	do we need to reset all checkboxes on this update
	//
	void		UpdateBlendShapesView(const bool resetCheckState);

	bool		PrepareCheckList( FBModel *pModel, FBArrayTemplate<bool>	&checkList );
	bool		UpdateCheckList( FBModel *pModel, FBArrayTemplate<bool> &checkList );
};

#endif /* __ORTOOL_BLENDSHAPE_TOOL_H__ */
