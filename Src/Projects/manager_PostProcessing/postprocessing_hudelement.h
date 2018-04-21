#ifndef __ORHUDELEMENT_TEMPLATE_HUDELEMENT_H__
#define __ORHUDELEMENT_TEMPLATE_HUDELEMENT_H__


/**	\file	postprocessing_hudelement.h

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration define
#define ORHUDELEMENTCUSTOM__CLASSNAME	ORHUDElementCustom
#define ORHUDELEMENTCUSTOM__CLASSSTR	"ORHUDElementCustom"
#define ORHUDELEMENTCUSTOM__DESCSTR	    "OR - Sample Custom HUDElement"

/**	Custom Material template.
*/
FB_FORWARD(ORHUDElementCustom);
class ORHUDElementCustom : public FBHUDElement
{
	//--- FiLMBOX Tool declaration.
	FBStorableClassDeclare( ORHUDElementCustom, FBHUDElement );

public:
	ORHUDElementCustom(const char *pName = NULL, HIObject pObject=NULL);

    FBPropertyAnimatableColorAndAlpha BackgroundColor;
    FBPropertyAnimatableColor ModelColor;

    virtual void GetIntrinsicDimension(FBViewingOptions* pViewingOptins, int& pWidth, int & pHeight) override;
    virtual void DrawElement(FBViewingOptions* pViewingOption, Rect& pRect) override;

    virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override; 
    virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;

	virtual bool FBCreate() override;		//!< FiLMBOX Creation function.
	virtual void FBDestroy() override;		//!< FiLMBOX Destruction function.


};

#endif /* __ORHUDELEMENT_TEMPLATE_HUDELEMENT_H__ */
