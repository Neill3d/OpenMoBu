
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://code.google.com/p/motioncodelibrary/wiki/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////

#ifndef __BOX_POSE_READER_BOX_H__
#define __BOX_POSE_READER_BOX_H__

/**	\file	box_PoseReader_box.h

 This plugin is based on the Comet's PoseReader plugin for Maya

*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration defines
#define	ORBOXTEMPLATE__CLASSNAME	Box_PoseReader
#define ORBOXTEMPLATE__CLASSSTR		"Box_PoseReader"

/**	Template for FBBox class.
*/
class Box_PoseReader : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( Box_PoseReader, FBBox );

public:
	//! creation function.
	virtual bool FBCreate();

	//! destruction function.
	virtual void FBDestroy();

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode,FBEvaluateInfo* pEvaluateInfo);

	//! FBX Storage function
	virtual bool FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );

	//! FBX Retrieval function
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );

private:

	FBAnimationNode		*mWorldMatrixLiveIn[2];		//!< Input data: mat for real jnt
	FBAnimationNode		*mWorldMatrixPoseIn[2];		//!< Input data: mat for pose loc
	FBAnimationNode		*mReadAxis;					//!< Input data: enum of waht axis, x,y,z,all to read ?
	FBAnimationNode		*mInterpMode;				//!< Input data: how to iterpolate from 0-180...
	FBAnimationNode		*mAllowRotate;				//!< Input data: 0-1 allowing to rotate or not.
	FBAnimationNode		*mMinAngle;					//!< Input data: anything is here is one default 0
	FBAnimationNode		*mMaxAngle;					//!< Input data: up to here... default 180
	FBAnimationNode		*mAllowTwist;				//!< Input data: 0-1 allowing of twist or not
	FBAnimationNode		*mMinTwist;					//!< Input data: Min Twist allowed
	FBAnimationNode		*mMaxTwist;					//!< Input data: Max Twist allowed
	FBAnimationNode		*mAllowTranslate;			//!< Input data: If on, translate anywhere is allowed.
	FBAnimationNode		*mMinTranslate;				//!< Input data: Min Translate allowed
	FBAnimationNode		*mMaxTranslate;				//!< Input data: Max Translate allowed
	
	FBAnimationNode		*mOutWeight;				//!< Output data: output array of weight
};

#endif /* __BOX_POSE_READER_BOX_H__ */
