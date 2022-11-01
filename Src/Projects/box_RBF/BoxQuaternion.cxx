
/**	\file	boxQuaternion.cxx
*	Definition of Quaternion relation boxes.
*
*	Sergei Solokhin (Neill3d) 2022
*/

//--- Class declaration
#include "BoxQuaternion.h"
#include "nv_math.h"
#include "math3d.h"

//--- Registration defines

#define BOXEULTOQUAT__CLASS		BOXEULTOQUAT__CLASSNAME
#define BOXEULTOQUAT__NAME		BOXEULTOQUAT__CLASSSTR
#define	BOXEULTOQUAT__LOCATION	"Quaternion"
#define BOXEULTOQUAT__LABEL		"Euler To Quaternion"
#define	BOXEULTOQUAT__DESC		"Euler To Quaternion"

#define BOXQUATTOEUL__CLASS		BOXQUATTOEUL__CLASSNAME
#define BOXQUATTOEUL__NAME		BOXQUATTOEUL__CLASSSTR
#define	BOXQUATTOEUL__LOCATION	"Quaternion"
#define BOXQUATTOEUL__LABEL		"Quaternion To Euler"
#define	BOXQUATTOEUL__DESC		"Quaternion To Euler"

#define BOXQUATMULT__CLASS		BOXQUATMULT__CLASSNAME
#define BOXQUATMULT__NAME		BOXQUATMULT__CLASSSTR
#define	BOXQUATMULT__LOCATION	"Quaternion"
#define BOXQUATMULT__LABEL		"Mult"
#define	BOXQUATMULT__DESC		"Quaternion Mult"

#define BOXQUATINV__CLASS		BOXQUATINV__CLASSNAME
#define BOXQUATINV__NAME		BOXQUATINV__CLASSSTR
#define	BOXQUATINV__LOCATION	"Quaternion"
#define BOXQUATINV__LABEL		"Invert"
#define	BOXQUATINV__DESC		"Quaternion Invert"

#define BOXQUATNOR__CLASS		BOXQUATNOR__CLASSNAME
#define BOXQUATNOR__NAME		BOXQUATNOR__CLASSSTR
#define	BOXQUATNOR__LOCATION	"Quaternion"
#define BOXQUATNOR__LABEL		"Normalize"
#define	BOXQUATNOR__DESC		"Quaternion Normalize"

#define BOXQUATLOOK__CLASS		BOXQUATLOOK__CLASSNAME
#define BOXQUATLOOK__NAME		BOXQUATLOOK__CLASSSTR
#define	BOXQUATLOOK__LOCATION	"Quaternion"
#define BOXQUATLOOK__LABEL		"Look Rotation"
#define	BOXQUATLOOK__DESC		"Quaternion Look Rotation"

#define BOXQUATAXIS__CLASS		BOXQUATAXIS__CLASSNAME
#define BOXQUATAXIS__NAME		BOXQUATAXIS__CLASSSTR
#define	BOXQUATAXIS__LOCATION	"Quaternion"
#define BOXQUATAXIS__LABEL		"Axis Rotation"
#define	BOXQUATAXIS__DESC		"Quaternion Axis Rotation"

#define BOXQUATLERP__CLASS		BOXQUATLERP__CLASSNAME
#define BOXQUATLERP__NAME		BOXQUATLERP__CLASSSTR
#define	BOXQUATLERP__LOCATION	"Quaternion"
#define BOXQUATLERP__LABEL		"Interpolate"
#define	BOXQUATLERP__DESC		"Quaternion Interpolate Rotation"

#define BOXVECTORROTATEBY__CLASS	BOXVECTORROTATEBY__CLASSNAME
#define BOXVECTORROTATEBY__NAME		BOXVECTORROTATEBY__CLASSSTR
#define	BOXVECTORROTATEBY__LOCATION	"Quaternion"
#define BOXVECTORROTATEBY__LABEL	"RotateBy"
#define	BOXVECTORROTATEBY__DESC		"Vector Rotate By"

//--- implementation and registration
FBBoxImplementation(BOXEULTOQUAT__CLASS);		// Box class name
FBRegisterBox(BOXEULTOQUAT__NAME,				// Unique name to register box.
    BOXEULTOQUAT__CLASS,				// Box class name
    BOXEULTOQUAT__LOCATION,			// Box location ('plugins')
    BOXEULTOQUAT__LABEL,				// Box label (name of box to display)
    BOXEULTOQUAT__DESC,			// Box long description.
    FB_DEFAULT_SDK_ICON);	// Icon filename (default=Open Reality icon)

FBBoxImplementation(BOXQUATTOEUL__CLASS);		// Box class name
FBRegisterBox(BOXQUATTOEUL__NAME,				// Unique name to register box.
    BOXQUATTOEUL__CLASS,				// Box class name
    BOXQUATTOEUL__LOCATION,			// Box location ('plugins')
    BOXQUATTOEUL__LABEL,				// Box label (name of box to display)
    BOXQUATTOEUL__DESC,			// Box long description.
    FB_DEFAULT_SDK_ICON);	// Icon filename (default=Open Reality icon)

FBBoxImplementation(BOXQUATMULT__CLASS);		// Box class name
FBRegisterBox(BOXQUATMULT__NAME,				// Unique name to register box.
    BOXQUATMULT__CLASS,				// Box class name
    BOXQUATMULT__LOCATION,			// Box location ('plugins')
    BOXQUATMULT__LABEL,				// Box label (name of box to display)
    BOXQUATMULT__DESC,			// Box long description.
    FB_DEFAULT_SDK_ICON);	// Icon filename (default=Open Reality icon)

FBBoxImplementation(BOXQUATINV__CLASS);		// Box class name
FBRegisterBox(BOXQUATINV__NAME,				// Unique name to register box.
    BOXQUATINV__CLASS,				// Box class name
    BOXQUATINV__LOCATION,			// Box location ('plugins')
    BOXQUATINV__LABEL,				// Box label (name of box to display)
    BOXQUATINV__DESC,			// Box long description.
    FB_DEFAULT_SDK_ICON);	// Icon filename (default=Open Reality icon)

FBBoxImplementation(BOXQUATNOR__CLASS);		// Box class name
FBRegisterBox(BOXQUATNOR__NAME,				// Unique name to register box.
    BOXQUATNOR__CLASS,				// Box class name
    BOXQUATNOR__LOCATION,			// Box location ('plugins')
    BOXQUATNOR__LABEL,				// Box label (name of box to display)
    BOXQUATNOR__DESC,			// Box long description.
    FB_DEFAULT_SDK_ICON);	// Icon filename (default=Open Reality icon)

FBBoxImplementation(BOXQUATLOOK__CLASS);		// Box class name
FBRegisterBox(BOXQUATLOOK__NAME,				// Unique name to register box.
    BOXQUATLOOK__CLASS,				// Box class name
    BOXQUATLOOK__LOCATION,			// Box location ('plugins')
    BOXQUATLOOK__LABEL,				// Box label (name of box to display)
    BOXQUATLOOK__DESC,			// Box long description.
    FB_DEFAULT_SDK_ICON);	// Icon filename (default=Open Reality icon)

FBBoxImplementation(BOXQUATAXIS__CLASS);		// Box class name
FBRegisterBox(BOXQUATAXIS__NAME,				// Unique name to register box.
    BOXQUATAXIS__CLASS,				// Box class name
    BOXQUATAXIS__LOCATION,			// Box location ('plugins')
    BOXQUATAXIS__LABEL,				// Box label (name of box to display)
    BOXQUATAXIS__DESC,			// Box long description.
    FB_DEFAULT_SDK_ICON);	// Icon filename (default=Open Reality icon)

FBBoxImplementation(BOXQUATLERP__CLASS);		// Box class name
FBRegisterBox(BOXQUATLERP__NAME,				// Unique name to register box.
    BOXQUATLERP__CLASS,				// Box class name
    BOXQUATLERP__LOCATION,			// Box location ('plugins')
    BOXQUATLERP__LABEL,				// Box label (name of box to display)
    BOXQUATLERP__DESC,			// Box long description.
    FB_DEFAULT_SDK_ICON);	// Icon filename (default=Open Reality icon)

FBBoxImplementation(BOXVECTORROTATEBY__CLASS);		// Box class name
FBRegisterBox(BOXVECTORROTATEBY__NAME,				// Unique name to register box.
    BOXVECTORROTATEBY__CLASS,				// Box class name
    BOXVECTORROTATEBY__LOCATION,			// Box location ('plugins')
    BOXVECTORROTATEBY__LABEL,				// Box label (name of box to display)
    BOXVECTORROTATEBY__DESC,			// Box long description.
    FB_DEFAULT_SDK_ICON);	// Icon filename (default=Open Reality icon)


//

const char * FBPropertyBaseEnum<ERotationOrder>::mStrings[] = {
    "XYZ",		//!< XYZ  
    "XZY",     //!< XZY 
    "YXZ",     //!< YXZ 
    "YZX",		//!< YZX 	
    "ZXY",		//!< ZXY 	
    "ZYX",     //!< ZYX
    0 };

//////////////////////////////////////////////////////////////////////////////////////
// Euler To Quaternion


/************************************************
 *	Creation
 ************************************************/
bool BoxEulerToQuaternion::FBCreate()
{
    // Properties

    FBPropertyPublish(this, rotationOrder, "Rotation Order", nullptr, nullptr);
    rotationOrder = ERotationOrder::eXYZ;

    // Input Node

    m_EulerRotation = AnimationNodeInCreate(0, "Euler", ANIMATIONNODE_TYPE_VECTOR);
    m_OutQuaternion = AnimationNodeOutCreate(1, "Quat", ANIMATIONNODE_TYPE_VECTOR_4);
    
    return true;
}


/************************************************
 *	Destruction.
 ************************************************/
void BoxEulerToQuaternion::FBDestroy()
{
}


/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool BoxEulerToQuaternion::AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo)
{
    FBRVector rot;
    if (!m_EulerRotation->ReadData(rot, pEvaluateInfo))
        return false;

    FBQuaternion q;
    FBRotationToQuaternion(q, rot, (FBRotationOrder) (int) rotationOrder);

    m_OutQuaternion->WriteData(q, pEvaluateInfo);
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////
// Quaternion To Euler


/************************************************
 *	Creation
 ************************************************/
bool BoxQuaternionToEuler::FBCreate()
{
    // Properties

    FBPropertyPublish(this, rotationOrder, "Rotation Order", nullptr, nullptr);
    rotationOrder = ERotationOrder::eXYZ;

    // Input Node

    m_InQuaternion = AnimationNodeInCreate(0, "Quat", ANIMATIONNODE_TYPE_VECTOR_4);
    m_EulerRotation = AnimationNodeOutCreate(1, "Euler", ANIMATIONNODE_TYPE_VECTOR);
    
    return true;
}


/************************************************
 *	Destruction.
 ************************************************/
void BoxQuaternionToEuler::FBDestroy()
{
}


/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool BoxQuaternionToEuler::AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo)
{
    FBRVector rot;
    FBQuaternion q;

    if (!m_InQuaternion->ReadData(q, pEvaluateInfo))
        return false;

    FBQuaternionToRotation(rot, q, (FBRotationOrder)(int)rotationOrder);
    m_EulerRotation->WriteData(rot, pEvaluateInfo);
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////
// Quaternion Mult


/************************************************
 *	Creation
 ************************************************/
bool BoxQuaternionMult::FBCreate()
{
    // Input Node

    m_InQuaternionQ = AnimationNodeInCreate(0, "Q", ANIMATIONNODE_TYPE_VECTOR_4);
    m_InQuaternionP = AnimationNodeInCreate(1, "P", ANIMATIONNODE_TYPE_VECTOR_4);
    m_OutQuaternion = AnimationNodeOutCreate(2, "Result", ANIMATIONNODE_TYPE_VECTOR_4);

    return true;
}


/************************************************
 *	Destruction.
 ************************************************/
void BoxQuaternionMult::FBDestroy()
{
}


/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool BoxQuaternionMult::AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo)
{
    FBQuaternion q, p;

    if (!m_InQuaternionQ->ReadData(q, pEvaluateInfo)
        || !m_InQuaternionP->ReadData(p, pEvaluateInfo))
    {
        return false;
    }
    
    FBQMult(q, q, p);
    m_OutQuaternion->WriteData(q, pEvaluateInfo);
    return true;
}



//////////////////////////////////////////////////////////////////////////////////////
// Quaternion Invert


/************************************************
 *	Creation
 ************************************************/
bool BoxQuaternionInvert::FBCreate()
{
    // Input Node

    m_InQuaternion = AnimationNodeInCreate(0, "In", ANIMATIONNODE_TYPE_VECTOR_4);
    m_OutQuaternion = AnimationNodeOutCreate(1, "Out", ANIMATIONNODE_TYPE_VECTOR_4);

    return true;
}


/************************************************
 *	Destruction.
 ************************************************/
void BoxQuaternionInvert::FBDestroy()
{
}


/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool BoxQuaternionInvert::AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo)
{
    FBQuaternion q;

    if (!m_InQuaternion->ReadData(q, pEvaluateInfo))
    {
        return false;
    }

    FBQuaternion p = FBQuaternion(-q[0], -q[1], -q[2], q[3]);
    m_OutQuaternion->WriteData(p, pEvaluateInfo);
    return true;
}



//////////////////////////////////////////////////////////////////////////////////////
// Quaternion Normalize


/************************************************
 *	Creation
 ************************************************/
bool BoxQuaternionNormalize::FBCreate()
{
    // Input Node

    m_InQuaternion = AnimationNodeInCreate(0, "In", ANIMATIONNODE_TYPE_VECTOR_4);
    m_OutQuaternion = AnimationNodeOutCreate(1, "Out", ANIMATIONNODE_TYPE_VECTOR_4);

    return true;
}


/************************************************
 *	Destruction.
 ************************************************/
void BoxQuaternionNormalize::FBDestroy()
{
}


/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool BoxQuaternionNormalize::AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo)
{
    FBQuaternion q;

    if (!m_InQuaternion->ReadData(q, pEvaluateInfo))
    {
        return false;
    }

    double len = FBQLength(q);
    if (len > 0.0)
    {
        double invLen = 1.0 / len;

        FBQuaternion p = FBQuaternion(invLen * q[0], invLen * q[1], invLen * q[2], invLen * q[3]);
        m_OutQuaternion->WriteData(p, pEvaluateInfo);
        return true;
    }

    return false;
}



//////////////////////////////////////////////////////////////////////////////////////
// Quaternion Look Rotation


/************************************************
 *	Creation
 ************************************************/
bool BoxQuaternionLookRotation::FBCreate()
{
    // Input Node

    m_InForward = AnimationNodeInCreate(0, "Forward", ANIMATIONNODE_TYPE_VECTOR);
    m_InUp = AnimationNodeInCreate(1, "Up", ANIMATIONNODE_TYPE_VECTOR);
    m_OutQuaternion = AnimationNodeOutCreate(2, "Out", ANIMATIONNODE_TYPE_VECTOR_4);

    return true;
}


/************************************************
 *	Destruction.
 ************************************************/
void BoxQuaternionLookRotation::FBDestroy()
{
}


/************************************************
 *	Real-time engine evaluation
 ************************************************/

FBQuaternion LookRotation(FBVector3d forward, FBVector3d up)
{
    FBVector3d vector(forward);
    VectorNormalize(vector);
    FBVector3d vector2 = CrossProduct(up, vector);
    VectorNormalize(vector2);
    FBVector3d vector3 = CrossProduct(vector, vector2);
    double m00 = vector2[0];
    double m01 = vector2[1];
    double m02 = vector2[2];
    double m10 = vector3[0];
    double m11 = vector3[1];
    double m12 = vector3[2];
    double m20 = vector[0];
    double m21 = vector[1];
    double m22 = vector[2];

    double num8 = (m00 + m11) + m22;
    FBQuaternion quaternion;
    if (num8 > 0.0)
    {
        double num = sqrt(num8 + 1.0);
        quaternion[3] = num * 0.5;
        num = 0.5 / num;
        quaternion[0] = (m12 - m21) * num;
        quaternion[1] = (m20 - m02) * num;
        quaternion[2] = (m01 - m10) * num;
        return quaternion;
    }
    if ((m00 >= m11) && (m00 >= m22))
    {
        double num7 = sqrt(((1.0 + m00) - m11) - m22);
        double num4 = 0.5 / num7;
        quaternion[0] = 0.5 * num7;
        quaternion[1] = (m01 + m10) * num4;
        quaternion[2] = (m02 + m20) * num4;
        quaternion[3] = (m12 - m21) * num4;
        return quaternion;
    }
    if (m11 > m22)
    {
        double num6 = sqrt(((1.0 + m11) - m00) - m22);
        double num3 = 0.5 / num6;
        quaternion[0] = (m10 + m01) * num3;
        quaternion[1] = 0.5 * num6;
        quaternion[2] = (m21 + m12) * num3;
        quaternion[3] = (m20 - m02) * num3;
        return quaternion;
    }
    double num5 = sqrt(((1.0 + m22) - m00) - m11);
    double num2 = 0.5 / num5;
    quaternion[0] = (m20 + m02) * num2;
    quaternion[1] = (m21 + m12) * num2;
    quaternion[2] = 0.5 * num5;
    quaternion[3] = (m01 - m10) * num2;
    return quaternion;
}

bool BoxQuaternionLookRotation::AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo)
{
    FBVector3d forward, up;
    FBQuaternion q;

    if (!m_InForward->ReadData(forward, pEvaluateInfo)
        || !m_InUp->ReadData(up, pEvaluateInfo))
    {

        return false;
    }

    q = LookRotation(forward, up);
    m_OutQuaternion->WriteData(q, pEvaluateInfo);
    return true;
}




//////////////////////////////////////////////////////////////////////////////////////
// Quaternion Axis Rotation


/************************************************
 *	Creation
 ************************************************/
bool BoxQuaternionAxisRotation::FBCreate()
{
    // Input Node

    m_InAxis = AnimationNodeInCreate(0, "Axis", ANIMATIONNODE_TYPE_VECTOR);
    m_InAngle = AnimationNodeInCreate(1, "Angle", ANIMATIONNODE_TYPE_NUMBER);
    m_OutQuaternion = AnimationNodeOutCreate(2, "Out", ANIMATIONNODE_TYPE_VECTOR_4);

    return true;
}


/************************************************
 *	Destruction.
 ************************************************/
void BoxQuaternionAxisRotation::FBDestroy()
{
}


/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool BoxQuaternionAxisRotation::AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo)
{
    FBVector3d axis;
    double angle;
    FBQuaternion q;

    if (!m_InAxis->ReadData(axis, pEvaluateInfo)
        || !m_InAngle->ReadData(&angle, pEvaluateInfo))
    {
        return false;
    }

    q = UnitAxisToQuaternion(axis, DEG2RAD(angle));
    m_OutQuaternion->WriteData(q, pEvaluateInfo);
    return true;
}




//////////////////////////////////////////////////////////////////////////////////////
// Quaternion Lerp


/************************************************
 *	Creation
 ************************************************/
bool BoxQuaternionLerp::FBCreate()
{
    // Input Node

    m_InQ = AnimationNodeInCreate(0, "Q", ANIMATIONNODE_TYPE_VECTOR_4);
    m_InP = AnimationNodeInCreate(1, "P", ANIMATIONNODE_TYPE_VECTOR_4);
    m_InFactor = AnimationNodeInCreate(2, "Factor", ANIMATIONNODE_TYPE_NUMBER);
    m_OutQuaternion = AnimationNodeOutCreate(3, "Out", ANIMATIONNODE_TYPE_VECTOR_4);

    return true;
}


/************************************************
 *	Destruction.
 ************************************************/
void BoxQuaternionLerp::FBDestroy()
{
}


/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool BoxQuaternionLerp::AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo)
{
    FBVector3d q, p;
    double factor;

    if (!m_InQ->ReadData(q, pEvaluateInfo)
        || !m_InP->ReadData(p, pEvaluateInfo)
        || !m_InFactor->ReadData(&factor, pEvaluateInfo))
    {
        return false;
    }

    FBInterpolateRotation(q, q, p, factor);
    m_OutQuaternion->WriteData(q, pEvaluateInfo);
    return true;
}



//////////////////////////////////////////////////////////////////////////////////////
// Vector RotateBy


/************************************************
 *	Creation
 ************************************************/
bool BoxVectorRotateBy::FBCreate()
{
    // Input Node

    m_InQ = AnimationNodeInCreate(0, "Q", ANIMATIONNODE_TYPE_VECTOR_4);
    m_InV = AnimationNodeInCreate(1, "V", ANIMATIONNODE_TYPE_VECTOR);
    m_OutV = AnimationNodeOutCreate(2, "Out", ANIMATIONNODE_TYPE_VECTOR);

    return true;
}


/************************************************
 *	Destruction.
 ************************************************/
void BoxVectorRotateBy::FBDestroy()
{
}


/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool BoxVectorRotateBy::AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo)
{
    FBQuaternion q;
    FBVector3d v;

    if (!m_InQ->ReadData(q, pEvaluateInfo)
        || !m_InV->ReadData(v, pEvaluateInfo))
    {
        return false;
    }

    vec3 dst;
    vec3 src = vec3((float)v[0], (float)v[1], (float)v[2]);

    rotateBy(dst, src, quat((float)q[0], (float)q[1], (float)q[2], (float)q[3]));
    v = FBVector3d((double)dst.x, (double)dst.y, (double)dst.z);

    m_OutV->WriteData(v, pEvaluateInfo);
    return true;
}