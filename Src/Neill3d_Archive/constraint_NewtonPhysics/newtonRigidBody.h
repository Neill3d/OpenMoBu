#ifndef __NEWTON_RIGID_BODY_H__
#define __NEWTON_RIGID_BODY_H__

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

//--- SDK include
#include <fbsdk/fbsdk.h>

//-- Newton SDK
#include "Newton.h"

//--- Registration defines
#define	NEWTON_RIGIDBODY__CLASSNAME		CRigidBody
#define NEWTON_RIGIDBODY__CLASSSTR		"Newton_RigidBody"

#define	NEWTON_RIGIDJOINT__CLASSNAME	CRigidJoint
#define NEWTON_RIGIDJOINT__CLASSSTR		"Newton_RigidJoint"

//
// User data transfer
//
struct RigidBodyUserDataA
{
	double			pos[3];
	double			lastPos[3];
};


/**	rigid body class.
*/
class CRigidBody : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( CRigidBody, FBBox );

public:
	//! creation function.
	virtual bool FBCreate();

	//! destruction function.
	virtual void FBDestroy();

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(HFBAnimationNode pAnimationNode,HFBEvaluateInfo pEvaluateInfo);

	//! FBX Storage function
	virtual bool FbxStore( HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat );

	//! FBX Retrieval function
	virtual bool FbxRetrieve(HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat );

private:

	NewtonBody			*rigidBodyBox;
	double				bodyMatrix[4][4];
	RigidBodyUserDataA	rigidUserData;

	double				boxSize[3];

	double				lastTimeDouble;

	//-- connections

	HFBAnimationNode 	mIn;			//!< Input node:	Vector
	HFBAnimationNode	mScaleIn;		//!< Input node:	Vector
	HFBAnimationNode	mMassIn;		//!< Input node:	Number
	HFBAnimationNode 	mOut;			//!< Output node:	Vector
	HFBAnimationNode	mRotationOut;	//!< Output node:	Rotation
	HFBAnimationNode	mBody;			//!< Output node:	Body index (for joints)
};


/**	rigid joint (Ball & Socket) class.
*/
class CRigidJoint : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( CRigidJoint, FBBox );

public:
	//! creation function.
	virtual bool FBCreate();

	//! destruction function.
	virtual void FBDestroy();

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(HFBAnimationNode pAnimationNode,HFBEvaluateInfo pEvaluateInfo);

	virtual bool AnimationNodeDestroy( HFBAnimationNode pAnimationNode );

	//! FBX Storage function
	virtual bool FbxStore( HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat );

	//! FBX Retrieval function
	virtual bool FbxRetrieve(HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat );

private:

	NewtonJoint			*ballConstraint;
	
	int					oldStatus1, oldStatus2, oldStatus3;
	double				lastTimeDouble;

	//-- connections

	HFBAnimationNode 	mBodyChild;		//!< Input node:	Number
	HFBAnimationNode	mBodyParent;	//!< Input node:	Number
	HFBAnimationNode	mPivot;			//!< Input node:	Vector
	//HFBAnimationNode	mOut;			//!< Output node:	Number
};


#endif /* __NEWTON_RIGID_BODY_H__ */
