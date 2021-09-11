#ifndef __CCONSTRAINT_TWISTEXTRACTION_H__
#define __CCONSTRAINT_TWISTEXTRACTION_H__


/**	\file	constraint_twistextraction.h

Mikkel Brons-Frandsen 2021

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include

#include <fbsdk/fbsdk.h>
#include "nv_math.h"

enum axis
{
	None,
	X,
	Y,
	Z,
};

const char* FBPropertyBaseEnum<axis>::mStrings[] = { "Choose Forward Axis...", "X", "Y", "Z", 0 };

#define CCONSTRAINTTWISTEXTRACTION__CLASSNAME	CConstraintTwistextraction
#define CCONSTRAINTTWISTEXTRACTION__CLASSSTR	"CConstraintTwistextraction"

//////////////////////////////////////////////////////////////////////////////////
//! An attachment constraint class.
class CConstraintTwistextraction : public FBConstraint
{
	//--- declaration
	FBConstraintDeclare(CConstraintTwistextraction, FBConstraint);

public:
	//--- Creation & Destruction
	virtual bool FBCreate();							//!< Constructor.
	virtual void FBDestroy();						//!< Destructor.

	//--- Animation node management
	virtual void RemoveAllAnimationNodes();			//!< Remove animation nodes.
	virtual void SetupAllAnimationNodes();			//!< Setup animation nodes.

	virtual void SnapSuggested() override;				//!< Suggest 'snap'.
	virtual void FreezeSuggested() override;				//!< Suggest 'freeze'.

	//! Real-time evaluation engine function.
	virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo, FBConstraintInfo* pConstraintInfo) override;

	//--- FBX Interface
	virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);	//!< FBX Storage.
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);	//!< FBX Retrieval.

	//--- Local parameters
	FBAnimationNode* mSourceRotation; //!< AnimationNode: INPUT  -> Source.

	FBAnimationNode* mConstrainedRotation; //!< AnimationNode: OUTPUT -> Constraint.

	int mGroupSource;		//!< Source groupe index.
	int	mGroupConstrain;	//!< Constraint group index.

	FBPropertyBaseEnum<axis> ForwardAxis;

	FBPropertyVector3d OffsetRotation;

protected:

	quat mOffsetRotation;

	vec3 mForwardVector;
	vec3 mRotatedVector;

	FBRotationOrder mSourceRotationOrder;
	FBRotationOrder mConstrainedRotationOrder;

	void SetZero();

	static void ShortestArcQuat(const vec3& rotatedVector, const vec3& forwardVector, quat& outQuaternion);
	static void Orthogonal(const vec3& inVec, vec3& outVec);

	static void SetAxis(HIObject pObject, axis pValue);
};

#endif	/* __ORCONSTRAINT_TWISTEXTRACTION_H__ */
