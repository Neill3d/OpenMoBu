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
		default:
			return FBRotationOrder::kFBXYZ;
	}
}


void CConstraintTwistextraction::SetAxis(HIObject pObject, axis pValue)
{
	using namespace nv;
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
	using namespace nv;
	FBModel* source = ReferenceGet(mGroupSource, 0);
	FBModel* constrained = ReferenceGet(mGroupConstrain, 0);

	if (source && constrained && (ForwardAxis.AsInt() != axis::None))
	{
		mSourceRotationOrder = MapRotationOrder(source->RotationOrder.GetPropertyValue());
		mConstrainedRotationOrder = MapRotationOrder(constrained->RotationOrder.GetPropertyValue());
		
		// store the rotation offset
		FBQuaternion offsetQ;
		FBRotationToQuaternion(offsetQ, constrained->Rotation, mConstrainedRotationOrder);

		mOffsetRotation.x = (nv_scalar)offsetQ[0];
		mOffsetRotation.y = (nv_scalar)offsetQ[1];
		mOffsetRotation.z = (nv_scalar)offsetQ[2];
		mOffsetRotation.w = (nv_scalar)offsetQ[3];
	}
}

void CConstraintTwistextraction::FreezeSuggested()
{
	FBConstraint::FreezeSuggested();

	if (ReferenceGet(mGroupSource, 0))
	{
		FreezeSRT((FBModel*)ReferenceGet(mGroupSource, 0), false, true, false);
	}
}

bool CConstraintTwistextraction::AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo, FBConstraintInfo* pConstraintInfo)
{
	using namespace nv;
	// zero the rotation offset
	if (pConstraintInfo->GetZeroRequested())
		SetZero();

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
	fullQ = shortestQ * fullQ;

	// add twist to offset rotation
	fullQ = mOffsetRotation * fullQ;
	
	// change back into euler rotation
	sourceQuat[0] = fullQ.x; sourceQuat[1] = fullQ.y; sourceQuat[2] = fullQ.z; sourceQuat[3] = fullQ.w;
	FBQuaternionToRotation(rvector, sourceQuat, mConstrainedRotationOrder);

	// write to mConstrainedRotation
	mConstrainedRotation->WriteData(rvector, pEvaluateInfo);

	return true;
}

bool CConstraintTwistextraction::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	// store attributes
	if (pStoreWhat & kAttributes)
	{
		pFbxObject->FieldWriteD("ForwardX", mForwardVector.x);
		pFbxObject->FieldWriteD("ForwardY", mForwardVector.y);
		pFbxObject->FieldWriteD("ForwardZ", mForwardVector.z);

		pFbxObject->FieldWriteD("OffsetX", mOffsetRotation.x);
		pFbxObject->FieldWriteD("OffsetY", mOffsetRotation.y);
		pFbxObject->FieldWriteD("OffsetZ", mOffsetRotation.z);
		pFbxObject->FieldWriteD("OffsetW", mOffsetRotation.w);
	}

	return true;
}

bool CConstraintTwistextraction::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	if (pStoreWhat & kAttributes)
	{
		mForwardVector.x = pFbxObject->FieldReadD("ForwardX");
		mForwardVector.y = pFbxObject->FieldReadD("ForwardY");
		mForwardVector.z = pFbxObject->FieldReadD("ForwardZ");

		mOffsetRotation.x = pFbxObject->FieldReadD("OffsetX");
		mOffsetRotation.y = pFbxObject->FieldReadD("OffsetY");
		mOffsetRotation.z = pFbxObject->FieldReadD("OffsetZ");
		mOffsetRotation.w = pFbxObject->FieldReadD("OffsetW");
	}
	else if (pStoreWhat & kCleanup)
	{
		// try set forward axis
		// this is technically not needed since the mForwardVector is already set but I want the property to display the correct axis
		if (mForwardVector.x > 0)
			ForwardAxis.SetPropertyValue(axis::X);
		else if (mForwardVector.y > 0)
			ForwardAxis.SetPropertyValue(axis::Y);
		else
			ForwardAxis.SetPropertyValue(axis::Z);
	}

	return true;
}


void CConstraintTwistextraction::SetZero()
{
	// set the offset rotation to identity
	mOffsetRotation.x = 0; 
	mOffsetRotation.y = 0; 
	mOffsetRotation.z = 0; 
	mOffsetRotation.w = 1;
}

void CConstraintTwistextraction::ShortestArcQuat(const nv::vec3& rotatedVector, const nv::vec3& forwardVector, nv::quat& outQuaternion)
{
	using namespace nv;
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
		outQuaternion.x = xyz.x; outQuaternion.y = xyz.y; outQuaternion.z = xyz.z; outQuaternion.w = cosine;
		outQuaternion.Normalize();
		
	}
	else // pointing in the opposite direction. Defaulting to a quat rotated pi around any axis orthogonal to forward
	{
		Orthogonal(forwardVector, xyz);
		xyz.normalize();
		outQuaternion.x = xyz.x; outQuaternion.y = xyz.y; outQuaternion.z = xyz.z; outQuaternion.w = 0;
	}
}

void CConstraintTwistextraction::Orthogonal(const nv::vec3& inVec, nv::vec3& outVec)
{
	using namespace nv;
	vec3 v;
	if (inVec.x < inVec.y) 
	{
		if (inVec.x < inVec.z)
		{
			v.x = 1;
		}
		else if (inVec.y < inVec.z)
		{
			v.y = 1;
		}
	}
	else
	{
		v.z = 1;
	}

	//cross product of the vector and the most orthogonal basis vector
	cross(outVec, inVec, v);
}
