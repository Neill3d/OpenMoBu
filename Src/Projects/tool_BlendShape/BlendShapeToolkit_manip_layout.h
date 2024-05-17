#ifndef __ORMANIP_SCULPT_LAYOUT_H__
#define __ORMANIP_SCULPT_LAYOUT_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BlendShapeToolkit_manip_layout.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declaration
#include "BlendShapeToolkit_manip.h"

//--- Registration define
#define ORTOOLBRUSH__CLASSNAME	ORManip_Sculpt_Tool
#define ORTOOLBRUSH__CLASSSTR	"ORManip_Sculpt_Tool"

//! Simple constraint layout.
class ORManip_Sculpt_Tool : public FBTool
{
	//--- FiLMBOX ceclaration.
	FBToolDeclare( ORManip_Sculpt_Tool, FBTool );

public:
	//--- FiLMBOX Constructor/Destructor
	virtual bool FBCreate();				//! FiLMBOX constructor.
	virtual void FBDestroy();				//! FiLMBOX destructor.

	// UI Management
	void	UICreate	();
	void	UICreateCommon();
	void	UICreateOptions();
	void	UICreateDeformer();
	void	UIConfigure	();
	void	UIConfigureCommon();
	void	UIConfigureOptions();
	void	UIConfigDeformer();
	void	UIReset		();

	// UI Callbacks
	void	EventUIIdle						( HISender pSender, HKEvent pEvent );
	void	EventUIShow						( HISender pSender, HKEvent pEvent );
	void	EventTabPanelChange				( HISender pSender, HKEvent pEvent );

	void	EventContainerDragAndDrop		( HISender pSender, HKEvent pEvent );
	void	EventContainerDblClick			( HISender pSender, HKEvent pEvent );

	void	EventFalloffChange				( HISender pSender, HKEvent pEvent );
	void	EventBrushChange				( HISender pSender, HKEvent pEvent );

	void	EventButtonActiveClick			( HISender pSender, HKEvent pEvent );
	void	EventButtonDeformerClick		( HISender pSender, HKEvent pEvent );
	void	EventButtonConstraintClick( HISender pSender, HKEvent pEvent );
	void	EventListConstraintChange		( HISender pSender, HKEvent pEvent );

	void	EventButtonDeformerResetClick	( HISender pSender, HKEvent pEvent );
	void	EventButtonDeformerZeroClick	( HISender pSender, HKEvent pEvent );
	void	EventButtonDeformerBlendshapeAddClick ( HISender pSender, HKEvent pEvent );


	void	EventButtonFreezeAllClick		( HISender pSender, HKEvent pEvent );
	void	EventButtonFreezeInvertClick	( HISender pSender, HKEvent pEvent );
	void	EventButtonFreezeNoneClick		( HISender pSender, HKEvent pEvent );
	void	EventButtonFillClick			( HISender pSender, HKEvent pEvent );

	void	EventButtonUseKeyframeRangeClick ( HISender pSender, HKEvent pEvent );

	void	EventSceneChange				( HISender pSender, HKEvent pEvent );

	void	EventManipulatorUpdate();

private:
	
	FBButton			mButtonActive;		// if we are in sculpting process
	//FBButton			mButtonDeformer;	// choose one if our mesh is deformed by skeletal or something else
	FBEditProperty		mEditDeformConstraintMode;
	FBLabel				mLabelModelName;	//  output a name of the model that we are currently working on

	FBLabel				mLabelConstraintList;
	FBList				mListConstraints;
	FBButton			mButtonSelectConstraint;	//!< select sculpt deform constraint if assigned
	FBButton			mButtonRenameConstraint;
	FBButton			mButtonRemoveConstraint;
	FBEditProperty		mEditExclusiveMode;			//!< edit a current constraint exclusive mode

	FBLabel				mLabelBrushes;
	FBVisualContainer	mContainerBrushes;
	FBTabPanel			mTabPanel;
	
	FBLayout			mLayoutCommon;

	FBEditProperty		mEditColor;
	FBEditProperty		mEditRadius;
	FBEditProperty		mEditStrength;
	FBEditProperty		mEditInverted;
	
	FBEditProperty		mEditBrushDirection;
	FBEditProperty		mEditAffectMode;
	FBEditProperty		mEditScreenInfluence;	// calculated influences from 2d space (work in viewport 2d plane)
	FBEditProperty		mEditScrInfRadius;		// separate value for the radius in the screen influence mode

	FBButton			mButtonFreezeAll;
	FBButton			mButtonFreezeInvert;
	FBButton			mButtonFreezeNone;

	FBButton			mButtonFill;
	FBButton			mButtonResetChanges;
	FBButton			mButtonZeroAll;

	//
	//FBArrowButton		mArrowDeformer;
	FBLayout			mLayoutDeformer;

	FBEditProperty		mEditAutoBlendShape;
	FBButton			mButtonSetBlendShape;
	FBButton			mButtonOverrideExisting;	//!< override existing blendshape under current frametime (if value is 100%)

	FBEditProperty		mEditAutoKeyframe;
	FBButton			mButtonSetKeyframe;
	
	FBButton			mButtonUseKeyRange;
	FBLabel				mLabelKeyframeIn;
	FBLabel				mLabelKeyframeLen;
	FBLabel				mLabelKeyframeOut;
	FBEditProperty		mEditKeyframeIn;
	FBEditProperty		mEditKeyframeLen;
	FBEditProperty		mEditKeyframeOut;


	//
	FBLayout			mLayoutOptions;

	FBEditProperty		mEditRadiusSens;
	FBEditProperty		mEditStrengthSens;
	FBEditProperty		mEditBrushColor;

	FBLabel				mLabelDevice;
	FBEditProperty		mEditUseTablet;
	FBEditProperty		mEditUsePenPressure;
	FBEditProperty		mEditUsePenEraser;
	FBEditProperty		mEditMaxPenPressure;
	FBEditProperty		mEditDisplayPressure;
	//FBVisualContainer	mContainerDevice;		// connect input device pressure (like wacom device pressure)

	//
	FBArrowButton		mArrowFalloffs;
	FBLayout			mLayoutFalloffs;
	FBVisualContainer	mContainerFalloffs;

private:
	FBSystem			mSystem;
	ORManip_Sculpt		*mManipulator;			//!< Handle onto manipulator.

	bool				mNeedUpdate;

	FBConstraint	*GetCurrentConstraint();
	void			UpdateConstraintList();
};


#endif	/* __ORMANIP_SCULPT_LAYOUT_H__ */
