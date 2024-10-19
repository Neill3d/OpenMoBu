#ifndef __BOX_QUATERNION_H__
#define __BOX_QUATERNION_H__


/**	\file	BoxQuaternion.h
*	Declaration of a relation constraint boxes to work with quaternion.
*
*	Sergei Solokhin (Neill3d) 2022
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

// C++
#include <chrono>
#include <vector>

// Eigen
#include <Eigen/Dense>


//--- Registration defines

#define	BOXNUMTOQUAT__CLASSNAME		BoxNumberToQuaternion
#define BOXNUMTOQUAT__CLASSSTR		"BoxNumberToQuaternion"

#define	BOXQUATTONUM__CLASSNAME		BoxQuaternionToNumber
#define BOXQUATTONUM__CLASSSTR		"BoxQuaternionToNumber"

#define	BOXEULTOQUAT__CLASSNAME		BoxEulerToQuaternion
#define BOXEULTOQUAT__CLASSSTR		"BoxEulerToQuaternion"

#define	BOXQUATTOEUL__CLASSNAME		BoxQuaternionToEuler
#define BOXQUATTOEUL__CLASSSTR		"BoxQuaternionToEuler"

#define	BOXQUATMULT__CLASSNAME		BoxQuaternionMult
#define BOXQUATMULT__CLASSSTR		"BoxQuaternionMult"

#define	BOXQUATINV__CLASSNAME		BoxQuaternionInvert
#define BOXQUATINV__CLASSSTR		"BoxQuaternionInvert"

#define	BOXQUATNOR__CLASSNAME		BoxQuaternionNormalize
#define BOXQUATNOR__CLASSSTR		"BoxQuaternionNormalize"

#define	BOXQUATLOOK__CLASSNAME		BoxQuaternionLookRotation
#define BOXQUATLOOK__CLASSSTR		"BoxQuaternionLookRotation"

#define	BOXQUATAXIS__CLASSNAME		BoxQuaternionAxisRotation
#define BOXQUATAXIS__CLASSSTR		"BoxQuaternionAxisRotation"

#define	BOXQUATLERP__CLASSNAME		BoxQuaternionLerp
#define BOXQUATLERP__CLASSSTR		"BoxQuaternionLerp"

#define	BOXVECTORROTATEBY__CLASSNAME	BoxVectorRotateBy
#define BOXVECTORROTATEBY__CLASSSTR		"BoxVectorRotateBy"

enum ERotationOrder {
    eXYZ,		//!< XYZ  
    eXZY,     //!< XZY 
    eYXZ,     //!< YXZ 
    eYZX,		//!< YZX 	
    eZXY,		//!< ZXY 	
    eZYX,     //!< ZYX
};

/**	Make a quaternion from 4 component numbers X Y Z W
*	Box for a relation constraint
*/
class BoxNumberToQuaternion : public FBBox
{
    //--- box declaration.
    FBBoxDeclare(BoxNumberToQuaternion, FBBox);

public:
    virtual bool FBCreate();		//!< creation function.
    virtual void FBDestroy();		//!< destruction function.

    //! Overloaded FBBox real-time evaluation function.
    virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo);

private:

    FBAnimationNode* m_InX;
    FBAnimationNode* m_InY;
    FBAnimationNode* m_InZ;
    FBAnimationNode* m_InW;
    FBAnimationNode* m_OutQuaternion;
};

/**	Break a quaternion into 4 component numbers X Y Z W
*	Box for a relation constraint
*/
class BoxQuaternionToNumber : public FBBox
{
    //--- box declaration.
    FBBoxDeclare(BoxQuaternionToNumber, FBBox);

public:
    virtual bool FBCreate();		//!< creation function.
    virtual void FBDestroy();		//!< destruction function.

    //! Overloaded FBBox real-time evaluation function.
    virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo);

private:

    FBAnimationNode* m_OutX;
    FBAnimationNode* m_OutY;
    FBAnimationNode* m_OutZ;
    FBAnimationNode* m_OutW;
    FBAnimationNode* m_InQuaternion;
};

/**	Convert rotation vector3 euler into vector4 quaternion
*	Box for a relation constraint
*/
class BoxEulerToQuaternion : public FBBox
{
    //--- box declaration.
    FBBoxDeclare(BoxEulerToQuaternion, FBBox);

public:

    FBPropertyBaseEnum<ERotationOrder>     rotationOrder;

public:
    virtual bool FBCreate();		//!< creation function.
    virtual void FBDestroy();		//!< destruction function.

    //! Overloaded FBBox real-time evaluation function.
    virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo);

private:

    FBAnimationNode*	m_EulerRotation;
    FBAnimationNode*	m_OutQuaternion;
};


/**	Convert quaternion into vector3 euler rotation
*	Box for a relation constraint
*/
class BoxQuaternionToEuler : public FBBox
{
    //--- box declaration.
    FBBoxDeclare(BoxQuaternionToEuler, FBBox);

public:

    FBPropertyBaseEnum<ERotationOrder>     rotationOrder;

public:
    virtual bool FBCreate();		//!< creation function.
    virtual void FBDestroy();		//!< destruction function.

    //! Overloaded FBBox real-time evaluation function.
    virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo);

private:

    FBAnimationNode*	m_InQuaternion;
    FBAnimationNode*	m_EulerRotation;
};



/**	
*	Mult two quaternions
*/
class BoxQuaternionMult : public FBBox
{
    //--- box declaration.
    FBBoxDeclare(BoxQuaternionMult, FBBox);

public:
    virtual bool FBCreate();		//!< creation function.
    virtual void FBDestroy();		//!< destruction function.

    //! Overloaded FBBox real-time evaluation function.
    virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo);

private:

    FBAnimationNode*	m_InQuaternionQ;
    FBAnimationNode*	m_InQuaternionP;

    FBAnimationNode*	m_OutQuaternion;
};


/*
*	Invert a quaternion
*/
class BoxQuaternionInvert : public FBBox
{
    //--- box declaration.
    FBBoxDeclare(BoxQuaternionInvert, FBBox);

public:
    virtual bool FBCreate();		//!< creation function.
    virtual void FBDestroy();		//!< destruction function.

    //! Overloaded FBBox real-time evaluation function.
    virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo);

private:

    FBAnimationNode*	m_InQuaternion;
    FBAnimationNode*	m_OutQuaternion;
};


/*
*	Normalize a quaternion
*/
class BoxQuaternionNormalize : public FBBox
{
    //--- box declaration.
    FBBoxDeclare(BoxQuaternionNormalize, FBBox);

public:
    virtual bool FBCreate();		//!< creation function.
    virtual void FBDestroy();		//!< destruction function.

    //! Overloaded FBBox real-time evaluation function.
    virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo);

private:

    FBAnimationNode*	m_InQuaternion;
    FBAnimationNode*	m_OutQuaternion;
};

/*
*	Look Rotation
*/
class BoxQuaternionLookRotation : public FBBox
{
    //--- box declaration.
    FBBoxDeclare(BoxQuaternionLookRotation, FBBox);

public:
    virtual bool FBCreate();		//!< creation function.
    virtual void FBDestroy();		//!< destruction function.

    //! Overloaded FBBox real-time evaluation function.
    virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo);

private:

    FBAnimationNode*	m_InForward;
    FBAnimationNode*	m_InUp;
    FBAnimationNode*	m_OutQuaternion;
};

/*
*	Axis Rotation
*/
class BoxQuaternionAxisRotation : public FBBox
{
    //--- box declaration.
    FBBoxDeclare(BoxQuaternionAxisRotation, FBBox);

public:
    virtual bool FBCreate();		//!< creation function.
    virtual void FBDestroy();		//!< destruction function.

    //! Overloaded FBBox real-time evaluation function.
    virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo);

private:

    FBAnimationNode*	m_InAxis;
    FBAnimationNode*	m_InAngle;
    FBAnimationNode*	m_OutQuaternion;
};



/*
*	Quaternion Lerp
*/
class BoxQuaternionLerp : public FBBox
{
    //--- box declaration.
    FBBoxDeclare(BoxQuaternionLerp, FBBox);

public:
    virtual bool FBCreate();		//!< creation function.
    virtual void FBDestroy();		//!< destruction function.

    //! Overloaded FBBox real-time evaluation function.
    virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo);

private:

    FBAnimationNode*	m_InQ;
    FBAnimationNode*	m_InP;
    FBAnimationNode*	m_InFactor;
    FBAnimationNode*	m_OutQuaternion;
};


/*
*	Vector3 RotateBy
*/
class BoxVectorRotateBy : public FBBox
{
    //--- box declaration.
    FBBoxDeclare(BoxVectorRotateBy, FBBox);

public:
    virtual bool FBCreate();		//!< creation function.
    virtual void FBDestroy();		//!< destruction function.

    //! Overloaded FBBox real-time evaluation function.
    virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo);

private:

    FBAnimationNode*	m_InV;
    FBAnimationNode*	m_InQ;
    FBAnimationNode*	m_OutV;
};

#endif /* __BOX_QUATERNION_H__ */
