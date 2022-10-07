
/////////////////////////////////////////////////////////////////////////////////////////
//
// boxPoseTransform.h
//
// Sergei <Neill3d> Solokhin 2014-2020
//
// GitHub page - https://github.com/Neill3d/OpenMoBu
// Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
/////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration defines

#define	BOXLOOK__CLASSNAME		CBoxLookRotation
#define BOXLOOK__CLASSSTR		"CBoxLookRotation"

////////////////////////////////////////////////////////////////////////////////////////////////////////
// CBoxLookRotation

class CBoxLookRotation : public FBBox
{
    //--- box declaration.
    FBBoxDeclare(CBoxLookRotation, FBBox);

public:
    //! creation function.
    virtual bool FBCreate();

    //! destruction function.
    virtual void FBDestroy();

    //! Overloaded FBBox real-time evaluation function.
    virtual bool AnimationNodeNotify(FBAnimationNode *pAnimationNode, FBEvaluateInfo *pEvaluateInfo);

private:

    FBAnimationNode		*m_InTarget{ nullptr };
    FBAnimationNode		*m_InUp{ nullptr };
    
    FBAnimationNode		*m_Rotation{ nullptr };		//!> output - pose rotation for the object

};