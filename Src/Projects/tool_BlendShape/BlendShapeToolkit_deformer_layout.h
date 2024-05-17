
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BlendShapeToolkit_deformer_layout.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Class declaration
#include "blendshapetoolkit_deformer_constraint.h"

//! Simple constraint layout.
class BlendShapeDeformerLayout : public FBConstraintLayout
{
	//--- constraint layout declaration.
	FBConstraintLayoutDeclare( BlendShapeDeformerLayout, FBConstraintLayout );

public:
	//--- constructor/destructor
	virtual bool FBCreate();			//!< creation function.
	virtual void FBDestroy();			//!< destruction function.

	//--- UI Creation/Configuration
	void UICreate	();
	void UIConfigure();
	void UIReset ();

	//--- UI Callback
	void EventSpreadChange( HISender pSender, HKEvent pEvent );
	void EventChannelSelect( HISender pSender, HKEvent pEvent );
	void EventShapeSelect( HISender pSender, HKEvent pEvent );

	void EventButtonPlotClick( HISender pSender, HKEvent pEvent );

	void EventButtonShapesUpdate( HISender pSender, HKEvent pEvent );

private:
	BlendShapeDeformerConstraint*	mConstraint;			//!< Handle onto constraint.

	FBLabel							mLabelModels;
	FBSpread						mSpreadModels;			// output vertices count


	FBLabel							mLabelChannels;
	FBList							mListChannels;			// output vertices count

	FBButton						mButtonChannelRename;
	FBButton						mButtonChannelRemove;
	FBButton						mButtonChannelPlot;		// bake time range transform into the new single frame keyframes (exclusive)

	FBLabel							mLabelShapes;
	FBList							mListShapes;

	FBButton						mButtonShapesTimeline;	// go to time when selected shape is 100 %
	FBButton						mButtonShapesUpdate;		// update each shape percentage at current time

	FBButton						mButtonShapesRemoveAll;
	FBButton						mButtonShapesRemoveSelected;

	FBEditProperty					mButtonShapesProperty;	// show and edit selected property value
};
