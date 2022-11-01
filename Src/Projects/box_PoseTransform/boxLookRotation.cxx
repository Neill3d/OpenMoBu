
/////////////////////////////////////////////////////////////////////////////////////////
//
// boxPoseTransform.cxx
//
// Sergei <Neill3d> Solokhin 2014-2020
//
// GitHub page - https://github.com/Neill3d/OpenMoBu
// Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
/////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "boxLookRotation.h"
#include "nv_math.h"
#include "math3d.h"

//--- Registration defines

#define BOXLOOK__CLASS			BOXLOOK__CLASSNAME
#define BOXLOOK__NAME			BOXLOOK__CLASSSTR
#define	BOXLOOK__LOCATION		"Neill3d"
#define BOXLOOK__LABEL			"Look Rotation"
#define	BOXLOOK__DESC			"Compute rotation to aim at the target"


//--- implementation and registration

FBBoxImplementation(BOXLOOK__CLASS);	// Box class name
FBRegisterBox(BOXLOOK__NAME,			// Unique name to register box.
    BOXLOOK__CLASS,		// Box class name
    BOXLOOK__LOCATION,		// Box location ('plugins')
    BOXLOOK__LABEL,		// Box label (name of box to display)
    BOXLOOK__DESC,			// Box long description.
    FB_DEFAULT_SDK_ICON);	// Icon filename (default=Open Reality icon)


/************************************************
 *	Creation
 ************************************************/
bool CBoxLookRotation::FBCreate()
{
    /*
    *	Create the nodes for the box.
    */
    if (FBBox::FBCreate())
    {
        
        // Create the input node.
        
        m_InTarget = AnimationNodeInCreate(1, "In Target", ANIMATIONNODE_TYPE_LOCAL_TRANSLATION);
        m_InUp = AnimationNodeInCreate(2, "In Up", ANIMATIONNODE_TYPE_LOCAL_TRANSLATION);

        // Create the output nodes
        
        m_Rotation = AnimationNodeOutCreate(3, "Rotation", ANIMATIONNODE_TYPE_LOCAL_ROTATION);

        return true;
    }
    return false;
}


/************************************************
 *	Destruction.
 ************************************************/
void CBoxLookRotation::FBDestroy()
{
    /*
    *	Free any user memory associated to box.
    */
    //	FBBox::Destroy();
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

bool CBoxLookRotation::AnimationNodeNotify(FBAnimationNode *pAnimationNode, FBEvaluateInfo *pEvaluateInfo)
{
    /*
    *	1. Read the data from the in connector
    *	2. Treat the data as required
    *	3. Write the data to the out connector
    *	4. Return the status (LIVE/DEAD) of the box.
    */

    FBVector3d		target, up;

    // Read the input nodes.
    
    m_InTarget->ReadData(target, pEvaluateInfo);
    m_InUp->ReadData(up, pEvaluateInfo);
    
    FBQuaternion q = LookRotation(target, up);

    FBVector3d rotation;
    FBQuaternionToRotation(rotation, q);

    m_Rotation->WriteGlobalData(rotation, pEvaluateInfo);

    return true;
}