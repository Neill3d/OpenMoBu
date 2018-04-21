#ifndef postprocessing_applymanagerrule_h__
#define postprocessing_applymanagerrule_h__

/**	\file	postprocessing_association.h

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration define
#define POSTASSOCIATION__CLASSNAME	    PostProcessingAssociation
#define POSTASSOCIATION__CLASSSTR		"PostProcessingAssociation"

#include <fbsdk/fbapplymanagerrule.h>

/////////////////////////////////////////////
//

/**	KAMRManipulatorAssociation class.
*	Provides custom context menu rules for ORManip_Association and ORModelItem.
*/
class PostProcessingAssociation : public FBApplyManagerRule
{
	FBApplyManagerRuleDeclare(PostProcessingAssociation, FBApplyManagerRule);

public:
	/** Return true if pSrc is the type of custom object which this rule to be defined for. 
    *
    *    @warning you should only return true if pSrc is the type of custom object you defined.
    */
    virtual bool IsValidSrc( FBComponent* /*pSrc*/);

    /** Return true if pDst is the type of custom or MB object which your custom object 
    *    pSrc will be applied on. it's possible to return proper destination and return it by pDst. 
    *    For example, pSrc is RagDoll Property, pDst is a one of IK/FK bone, then you can find
    *    associated Character and replace.
    *
    *    \param    pSrc    the custom type of object you drag it to attach another object.
    *    \retval    pDst    one of the selected destinations, it could be modified and return proper destination.
    *    \retval    pAllowMultiple    return true if allow multiple objects of same type as pSrc to be connected
    *                   to pDst.
    *    \return            return true if connection between pSrc and pDst is allowed.
    */
    virtual bool IsValidConnection( FBComponent* pSrc, FBComponent*& pDst, bool& pAllowMultiple);

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
	int		mDuplicateId;
	FBSystem mSystem;
};

#endif // postprocessing_applymanagerrule_h__
