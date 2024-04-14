#ifndef __DRIVEN_CONSTRAINT_LAYOUT_H__
#define __DRIVEN_CONSTRAINT_LAYOUT_H__


/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: neill3d@gmail.com
// 
/////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Class declaration
#include "driven_constraint_constraint.h"

//! Simple constraint layout.
class Driven_Constraint_Layout : public FBConstraintLayout
{
	//--- constraint layout declaration.
	FBConstraintLayoutDeclare( Driven_Constraint_Layout, FBConstraintLayout );

public:
	virtual bool FBCreate();			//!< creation function.
	virtual void FBDestroy();			//!< destruction function.

	// UI Management
	void	UICreate	();
	void	UIConfigure	();
	void	UIReset		();

	// UI Callbacks
	void	EventButtonTestClick( HISender pSender, HKEvent pEvent );
	void	EventSrcContainerDragAndDrop	( HISender pSender, HKEvent pEvent );
	void	EventSrcContainerDblClick		( HISender pSender, HKEvent pEvent );
	void	EventSrcListPropertyChange		( HISender pSender, HKEvent pEvent );
	void	EventSceneChange		    ( HISender pSender, HKEvent pEvent );
	void	EventSrcValueChange		    ( HISender pSender, HKEvent pEvent );
	void	EventDstContainerDragAndDrop	( HISender pSender, HKEvent pEvent );
	void	EventDstContainerDblClick		( HISender pSender, HKEvent pEvent );
	void	EventDstListPropertyChange		( HISender pSender, HKEvent pEvent );

	void    SetupSrcPropertyList( FBComponent *pModel, FBProperty *pProperty );
	void    SetupDstPropertyList( FBComponent *pModel, FBProperty *pProperty );

private:
	Driven_Constraint*	mConstraint;		//!< Handle onto constraint.

private:
	FBScene        *mScene;
	
	FBLabel			mLabelDirections;	//!< Description label.

	FBLabel			mSrcLabel;			// !< description for the source text edit field
	FBLabel			mDstLabel;			// !< description for the dst text edit field
	
	// source property
	FBVisualContainer		mSrcContainerModel;
    FBList					mSrcListProperties;
	FBEditProperty			mSrcProperty;
	
	
	FBVisualContainer		mDstContainerModel;
    FBList					mDstListProperties;
	FBEditProperty			mDstProperty;

	FBLabel				mInfo;
};

#endif /* __DRIVEN_CONSTRAINT_LAYOUT_H__ */
