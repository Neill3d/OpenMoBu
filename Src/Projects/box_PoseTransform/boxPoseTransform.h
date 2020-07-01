
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

#define	BOXPOSETRANSFORM__CLASSNAME		CBoxPoseTransform
#define BOXPOSETRANSFORM__CLASSSTR		"CBoxPoseTransform"

#define NUMBER_OF_POSES	6

////////////////////////////////////////////////////////////////////////////////////////////////////////
// CBoxPoseTransform

class CBoxPoseTransform : public FBBox
{
	//--- box declaration.
	FBBoxDeclare(CBoxPoseTransform, FBBox );

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
	virtual bool FbxRetrieve( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );

	void OnSystemIdle(HISender pSender, HKEvent pEvent);

public:

	FBPropertyListObject		m_Object;
	FBPropertyListObject		m_Poses[NUMBER_OF_POSES];	//!> assign pose to get transform from

private:

	FBSystem		m_System;

	FBAnimationNode		*m_InTranslation{ nullptr };
	FBAnimationNode		*m_InRotation{ nullptr };
	FBAnimationNode		*m_Factor[NUMBER_OF_POSES]{ nullptr };		//!> input - interpolate factor between input transform and output pose
	
	FBAnimationNode		*m_Translation{ nullptr };	//!> output - pose translation for the object
	FBAnimationNode		*m_Rotation{ nullptr };		//!> output - pose rotation for the object

	// retrieve
	FBString	m_ModelName;
	FBString	m_PoseNames[NUMBER_OF_POSES];
};
