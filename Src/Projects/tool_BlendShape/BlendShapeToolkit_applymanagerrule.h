#ifndef BlendShapeToolkit_applymanagerrule_h__
#define BlendShapeToolkit_applymanagerrule_h__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BlendShapeToolkit_applymanagerrule.h
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

//--- Registration define
#define ORAMRMANIPULATORASSOCIATION__CLASSNAME	    SculptBrushAssociation
#define ORAMRMANIPULATORASSOCIATION__CLASSSTR		"SculptBrushAssociation"

#include <fbsdk/fbapplymanagerrule.h>

/////////////////////////////////////////////
//
/* SculptMode

	param pFocusedObject - enter sculpt mode with that object
	param pConstraint - use specified constraint or create a new one ?
	enter - do we enter or leave sculpting mode ?
*/
//void SculptMode( FBModel *pFocusedObject, FBConstraint *pConstraint, bool enter );

/**	KAMRManipulatorAssociation class.
*	Provides custom context menu rules for ORManip_Association and ORModelItem.
*/
class KSculptBrushAssociation : public FBApplyManagerRule
{
	FBApplyManagerRuleDeclare(KSculptBrushAssociation, FBApplyManagerRule);

public:
	/** Build context menu for KAMRManipulatorAssociation.
	*
	*	\param	pAMMenu	the menu to add menu option on.
	*	\param	pFocusedObject	the ORModelItem type of object you right-click on.
	*	\return	return true if menu item has been added.
	*/
	virtual bool MenuBuild( FBAMMenu* pAMMenu, FBComponent* pFocusedObject);

	/** Perform action if the added menu item is clicked.
	*
	*	\param	pMenuId	the menu ID right-click on.
	*	\param	pFocusedObject	the ORModelItem type of object you right-click on.
	*	\return	return true if action has been performed.
	*/
	virtual bool MenuAction( int pMenuId, FBComponent* pFocusedObject);

protected:
	int mEditManipulatorId;		//!< The menu itme id for edit by manipulator association.
	int mCloseManipulatorId;	//!< The menu itme id for close manipulator association.
	FBSystem mSystem;
};

#endif // BlendShapeToolkit_applymanagerrule_h__
