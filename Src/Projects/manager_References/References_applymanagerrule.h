#ifndef references_applymanagerrule_h__
#define references_applymanagerrule_h__


/**	\file	references_applymanagerrule.h

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration define
#define REFERENCE_ASSOCIATION__CLASSNAME	ReferenceAssociation
#define REFERENCE_ASSOCIATION__CLASSSTR		"ReferenceAssociation"

#include <fbsdk/fbapplymanagerrule.h>

/**	KAMRManipulatorAssociation class.
*	Provides custom context menu rules for ORManip_Association and ORModelItem.
*/
class ReferenceAssociation : public FBApplyManagerRule
{
	FBApplyManagerRuleDeclare(ReferenceAssociation, FBApplyManagerRule);

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
	int mSaveEditsId;		//!< The menu itme id for edit by manipulator association.
	int mRestoreEditsId;	//!< The menu itme id for close manipulator association.
	int mBakeEditsId;
	int mChangePathId;
	int mReloadId;
	int mDeleteId;
	
	FBSystem mSystem;
	FBApplication mApp;
};

#endif // references_applymanagerrule_h__
