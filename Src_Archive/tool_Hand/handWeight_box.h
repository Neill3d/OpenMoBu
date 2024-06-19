#ifndef __HAND_WEIGHT_BOX_H__
#define __HAND_WEIGHT_BOX_H__


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


/**	\file	handWeight_box.h
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration defines
#define	HANDWEIGHT1__CLASSNAME		CHandWeight1
#define HANDWEIGHT1__CLASSSTR		"HANDWEIGHT1_BOX"

#define	HANDWEIGHT2__CLASSNAME		CHandWeight2
#define HANDWEIGHT2__CLASSSTR		"HANDWEIGHT2_BOX"

#define	HANDWEIGHT3__CLASSNAME		CHandWeight3
#define HANDWEIGHT3__CLASSSTR		"HANDWEIGHT3_BOX"

/**	FBBox class for the first joint weight calculation.
*/
class CHandWeight1 : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( CHandWeight1, FBBox );

public:
	//! creation function.
	virtual bool FBCreate();

	//! destruction function.
	virtual void FBDestroy();

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(FBAnimationNode *pAnimationNode,FBEvaluateInfo *pEvaluateInfo);

	//! FBX Storage function
	virtual bool FbxStore( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );

	//! FBX Retrieval function
	virtual bool FbxRetrieve(FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );

private:

	FBAnimationNode			*mWeight;		//!< Input node: Weight.
	FBAnimationNode			*mInherit;		//!< Input node: Inherit.
	FBAnimationNode			*mResult;		//!< Output node: Result.
};


/**	FBBox class for the second joint weight calculation.
*/
class CHandWeight2 : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( CHandWeight2, FBBox );

public:
	//! creation function.
	virtual bool FBCreate();

	//! destruction function.
	virtual void FBDestroy();

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(FBAnimationNode *pAnimationNode,FBEvaluateInfo *pEvaluateInfo);

	//! FBX Storage function
	virtual bool FbxStore( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );

	//! FBX Retrieval function
	virtual bool FbxRetrieve(FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );

private:

	FBAnimationNode			*mWeight;		//!< Input node: Weight.
	FBAnimationNode			*mInherit;		//!< Input node: Inherit.
	FBAnimationNode			*mResult;		//!< Output node: Result.
};


/**	FBBox class for the third joint weight calculation.
*/
class CHandWeight3 : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( CHandWeight3, FBBox );

public:
	//! creation function.
	virtual bool FBCreate();

	//! destruction function.
	virtual void FBDestroy();

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(FBAnimationNode *pAnimationNode,FBEvaluateInfo *pEvaluateInfo);

	//! FBX Storage function
	virtual bool FbxStore( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );

	//! FBX Retrieval function
	virtual bool FbxRetrieve(FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );

private:

	FBAnimationNode			*mWeight;		//!< Input node: Weight.
	FBAnimationNode			*mInherit;		//!< Input node: Inherit.
	FBAnimationNode			*mResult;		//!< Output node: Result.
};

#endif /* __ORBOX_TEMPLATE_BOX_H__ */
