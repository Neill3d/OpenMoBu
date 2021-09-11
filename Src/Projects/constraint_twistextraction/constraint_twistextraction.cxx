/**	\file	constraint_twistextraction.cxx

Mikkel Brons-Frandsen 2021

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declarations
#include "constraint_twistextraction.h"

#include <vector>

//--- Registration defines
#define	CCONSTRAINTTWISTEXTRACTION__CLASS CCONSTRAINTTWISTEXTRACTION__CLASSNAME
#define CCONSTRAINTTWISTEXTRACTION__NAME "Twist Extraction Constraint"
#define CCONSTRAINTTWISTEXTRACTION__LABEL "Twist Extraction Constraint"
#define CCONSTRAINTTWISTEXTRACTION__DESC "Twist Extraction"

//--- implementation and registration
FBConstraintImplementation(CCONSTRAINTTWISTEXTRACTION__CLASS);
FBRegisterConstraint(CCONSTRAINTTWISTEXTRACTION__NAME,
	CCONSTRAINTTWISTEXTRACTION__CLASS,
	CCONSTRAINTTWISTEXTRACTION__LABEL,
	CCONSTRAINTTWISTEXTRACTION__DESC,
	FB_DEFAULT_SDK_ICON);	// Icon filename (default=Open Reality icon)


FBRotationOrder MapRotationOrder(const FBModelRotationOrder& modelRotationOrder)
{
	switch(modelRotationOrder)
	{
		case FBModelRotationOrder::kFBEulerXYZ: // ambigous without explicit class
			return FBRotationOrder::kFBXYZ;
		case kFBEulerXZY:
			return FBRotationOrder::kFBXZY;
		case kFBEulerYZX:
			return FBRotationOrder::kFBYZX;
		case kFBEulerYXZ:
			return FBRotationOrder::kFBYXZ;
		case kFBEulerZXY:
			return FBRotationOrder::kFBZXY;
		case kFBEulerZYX:
			return FBRotationOrder::kFBZYX;
		case kFBSphericXYZ:
			return FBRotationOrder::kFBXYZ;
	}
}


void CConstraintTwistextraction::SetAxis(HIObject pObject, axis pValue)
{
	CConstraintTwistextraction* pConstraint = FBCast<CConstraintTwistextraction>(pObject);
	if (pConstraint) 
	{
		pConstraint->ForwardAxis.SetPropertyValue(pValue);
		switch (pValue)
		{
			case X:
				pConstraint->mForwardVector = vec3(nv_scalar(1), nv_scalar(0), nv_scalar(0));
				break;
			case Y:
				pConstraint->mForwardVector = vec3(nv_scalar(0), nv_scalar(1), nv_scalar(0));
				break;
			case Z:
				pConstraint->mForwardVector = vec3(nv_scalar(0), nv_scalar(0), nv_scalar(1));
				break;
		}
	}
}

bool CConstraintTwistextraction::FBCreate()
{
	Deformer = false;

	// Create reference groups
	mGroupConstrain = ReferenceGroupAdd("Constrain", 1);
	mGroupSource = ReferenceGroupAdd("Source Object", 1);

	mSourceRotation = NULL;
	mConstrainedRotation = NULL;

	FBPropertyPublish(this, ForwardAxis, "Forward Axis", nullptr, SetAxis);

	return true;
}

void CConstraintTwistextraction::FBDestroy()
{
}

void CConstraintTwistextraction::RemoveAllAnimationNodes()
{
}

void CConstraintTwistextraction::SetupAllAnimationNodes()
{
	if (ReferenceGet(mGroupSource, 0) && ReferenceGet(mGroupConstrain, 0))
	{
		mSourceRotation = AnimationNodeOutCreate(1, ReferenceGet(mGroupSource, 0), ANIMATIONNODE_TYPE_LOCAL_ROTATION);

		mConstrainedRotation = AnimationNodeInCreate(4, ReferenceGet(mGroupConstrain, 0), ANIMATIONNODE_TYPE_LOCAL_ROTATION);
	}
}

void CConstraintTwistextraction::SnapSuggested()
{
	SetSuggested(false);
}

void CConstraintTwistextraction::FreezeSuggested()
{
	SetSuggested(true);
}

bool CConstraintTwistextraction::AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo, FBConstraintInfo* pConstraintInfo)
{
	FBRVector rvector;
	mSourceRotation->ReadData(rvector, pEvaluateInfo);

	// create quat from source rotation
	FBQuaternion sourceQuat;
	FBRotationToQuaternion(sourceQuat, rvector, mSourceRotationOrder);
	quat fullQ((nv_scalar)sourceQuat[0], (nv_scalar)sourceQuat[1], (nv_scalar)sourceQuat[2], (nv_scalar)sourceQuat[3]);

	// rotate forward vector by quat
	mRotatedVector = mForwardVector;
	mRotatedVector.rotateBy(fullQ);

	// create shortest arc quat from rotated vector and forward vector
	quat shortestQ;
	ShortestArcQuat(mRotatedVector, mForwardVector, shortestQ);
	
	// subtract shortest arc quat from source quat to find the twist
	shortestQ = shortestQ.Inverse();
	fullQ = fullQ * shortestQ;

	// add twist to offset rotation
	fullQ = mOffsetRotation * fullQ;
	
	// change back into euler rotation
	sourceQuat[0] = fullQ[0]; sourceQuat[1] = fullQ[1]; sourceQuat[2] = fullQ[2]; sourceQuat[3] = fullQ[3];
	FBQuaternionToRotation(rvector, sourceQuat, mConstrainedRotationOrder);

	// write to mConstrainedRotation
	mConstrainedRotation->WriteData(rvector, pEvaluateInfo);

	return true;
}

bool CConstraintTwistextraction::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}

bool CConstraintTwistextraction::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}


void CConstraintTwistextraction::SetSuggested(bool freeze)
{
	// store the rotation orders of the source and constrained object
	// store the constrained objects rotation if 'freeze'

	FBModel* source = ReferenceGet(mGroupSource, 0);
	FBModel* constrained = ReferenceGet(mGroupConstrain, 0);

	if (source && constrained)
	{
		mSourceRotationOrder = MapRotationOrder(source->RotationOrder.GetPropertyValue());
		mConstrainedRotationOrder = MapRotationOrder(constrained->RotationOrder.GetPropertyValue());
		FBQuaternion offsetQ;
		if (freeze)
			FBRotationToQuaternion(offsetQ, constrained->Rotation, mConstrainedRotationOrder);
		else
			offsetQ[0] = 0; offsetQ[1] = 0; offsetQ[2] = 0; offsetQ[3] = 1;

		mOffsetRotation[0] = (nv_scalar)offsetQ[0]; 
		mOffsetRotation[1] = (nv_scalar)offsetQ[1]; 
		mOffsetRotation[2] = (nv_scalar)offsetQ[2]; 
		mOffsetRotation[3] = (nv_scalar)offsetQ[3];
	}
}

void CConstraintTwistextraction::ShortestArcQuat(const vec3& rotatedVector, const vec3& forwardVector, quat& outQuaternion)
{
	// magnitude = sqrt(magnitude(vectorA) **2 * magnitude(vectorB) **2)
	// w = dot(vectorA, vectorB) + magnitude
	// xyz = cross(vectorA, vectorB)
	// q = normalize(x, y, z, w)

	nv_scalar cosine = dot(forwardVector, rotatedVector) + 1;
	vec3 xyz;
	quat q;

	if (cosine > 0.00001) 
	{
		cross(xyz, forwardVector, rotatedVector);
		outQuaternion[0] = xyz[0]; outQuaternion[1] = xyz[1]; outQuaternion[2] = xyz[2]; outQuaternion[3] = cosine;
		outQuaternion.Normalize();
		
	}
	else // pointing in the opposite direction. Defaulting to a quat rotated pi around any axis orthogonal to forward
	{
		Orthogonal(forwardVector, xyz);
		xyz.normalize();
		outQuaternion[0] = xyz[0]; outQuaternion[1] = xyz[1]; outQuaternion[2] = xyz[2]; outQuaternion[3] = 0;
	}
}

void CConstraintTwistextraction::Orthogonal(const vec3& inVec, vec3& outVec)
{
	vec3 v;
	if (inVec[0] < inVec[1]) 
	{
		if (inVec[0] < inVec[2])
		{
			v[0] = 1;
		}
		else if (inVec[1] < inVec[2])
		{
			v[1] = 1;
		}
	}
	else
	{
		v[2] = 1;
	}

	//cross product of the vector and the most orthogonal basis vector
	cross(outVec, inVec, v);
}
