
/////////////////////////////////////////////////////////////////////////////////////////
//
// boxSpring_box.h
//
// Sergei <Neill3d> Solokhin 2014-2018
//
// GitHub page - https://github.com/Neill3d/OpenMoBu
// Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
/////////////////////////////////////////////////////////////////////////////////////////

#ifndef __BOX_SPRING_BOX_H__
#define __BOX_SPRING_BOX_H__

/**	\file	boxSpring_box.h
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration defines
#define	BOXSPRING__CLASSNAME		CBoxSpring
#define BOXSPRING__CLASSSTR		"SPRING_BOX"

/**	FBBox class for the first joint weight calculation.
*/
class CBoxSpring : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( CBoxSpring, FBBox );

public:
	//! creation function.
	virtual bool FBCreate();

	//! destruction function.
	virtual void FBDestroy();

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(FBAnimationNode *pAnimationNode, FBEvaluateInfo *pEvaluateInfo);

	//! FBX Storage function
	virtual bool FbxStore( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );

	//! FBX Retrieval function
	virtual bool FbxRetrieve(FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );

private:

	FBAnimationNode	*mStiffnessNode;		//!< Input node: Stiffness.
	FBAnimationNode	*mDampingNode;		//!< Input node: Damping
	FBAnimationNode	*mFrictionNode;		//!< Input node: Friction
	FBAnimationNode	*mLengthNode;		//!< Input node: Length
	FBAnimationNode	*mMassNode;			//!< Input node: Mass.
	
	FBAnimationNode *mOriginNode;		//!< Input node: get spring origin
	FBAnimationNode	*mPosNode;			//!< Input node: process spring end point

	FBAnimationNode *mZeroFrame;		//!< Input node: frame in which we reset our simulation
	FBAnimationNode *mTimedt;			//!< Input node: calculation time step

	FBAnimationNode *mRealTime;			//!< Input node: use system or local time ?

	FBAnimationNode	*mResultNode;		//!< Output node: Result.

	double				lastLocalTimeDouble;
	double				lastSystemTimeDouble;
	FBVector3d			mOldPos;			// motion old pos (for force calculation)
	FBVector3d			mCurrentPos;				// mass pos
	FBVector3d			mVel;				// mass velocity

	double				mDeltaTimeDouble;

	bool				mReset;
};


#endif /* __BOX_SPRING_BOX_H__ */
