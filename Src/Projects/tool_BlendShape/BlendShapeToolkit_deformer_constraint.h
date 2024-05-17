
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BlendShapeToolkit_deformer_constraint.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>

#include "BlendShapeToolkit_brushesBase.h"
#include "BlendShapeToolkit_deformer_manager.h"

#define BLENDSHAPEDEFORMER__CLASSNAME		BlendShapeDeformerConstraint
#define BLENDSHAPEDEFORMER__CLASSSTR		"BlendShapeDeformerConstraint"


////////////////////////////////////////////////////////////////////////////////////
// FBDeformerCorrective
//  class is used in FBModels deformers to make a link pointer to the assigned deformer constraint
////////////////////////////////////////////////////////////////////////////////////
__FB_FORWARD( FBDeformerCorrective );

//! Base Model deformer class
class FBDeformerCorrective : public FBDeformer {
    FBClassDeclare( FBDeformerCorrective,FBDeformer );

public:
    /**    Constructor.
    *    \param    pName        Name of deformer.
    *    \param    pObject        For internal use only(default=NULL).
    */
    FBDeformerCorrective(const char* pName, HIObject pObject=NULL);
	
	FBPropertyListObject		Constraint;			//!< <b>Read Write Property:</b> Deformable blendshape constraint.
    FBPropertyBool				Active;             //!< <b>Read Write Property:</b> Active.
	FBPropertyInt				Channel;			//!< Channel index in the constraint
	
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//! A simple constraint class.
class BlendShapeDeformerConstraint : public FBConstraint
{
	//--- declaration
	FBConstraintDeclare( BlendShapeDeformerConstraint, FBConstraint );

public:
	//--- Creation & Destruction
	virtual bool			FBCreate() override;						//!< Constructor.
	virtual void			FBDestroy() override;						//!< Destructor.

	//--- Animation node management
	virtual void			RemoveAllAnimationNodes() override;			//!< Remove animation nodes.
	virtual void			SetupAllAnimationNodes() override;			//!< Setup animation nodes.
	virtual void			FreezeSuggested() override;					//!< Suggest 'freeze'.

	//
	void		EventSystemIdle( HISender pSender, HKEvent pEvent );

	//--- Real-Time Engine
	//! Real-time evaluation engine function.
	virtual bool			AnimationNodeNotify		( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo, FBConstraintInfo* pConstraintInfo ) override;

	//! Real-time deformation engine function.
	virtual bool			DeformerNotify(	FBModel* pModel, const FBVertex*  pSrcVertex, const FBVertex* pSrcNormal,
											int pCount,
											FBVertex*  pDstVertex,FBVertex*  pDstNormal) override;
	//--- Binding notification callback
	virtual bool ReferenceRemoveNotify( int pGroupIndex, FBModel* pModel ) override;
	virtual bool ReferenceAddNotify( int pGroupIndex, FBModel* pModel ) override;

	//--- FBX Interface
	virtual bool			FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat ) override;	//!< FBX Storage.
	virtual bool			FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat ) override;	//!< FBX Retrieval.

	FBPropertyBool			ApplyOnlyOnKeyframe;	//!< apply shapes only if some keyframe is setted up
	FBPropertyBool			ApplyTransformation;	//!< apply tm matrix to each vertex (hold sculpt result during transform)
	FBPropertyBool			ExclusiveMode;		//!< apply only one current shape under a cursor

protected:

	std::vector<FBModel*>	mAffectedModels;		//!< store list of models in our constraint (help with auto store/retrieve objects)
	std::vector<int>		mChannelList;

public:

	bool			Temp;		//!< flag if that constraint is needed just for sculpture and should be removed after

	int			AddBlendShape(	const FBTime &curTime, 
								FBModel *pModel, 
								const OperationBuffer &bufferSrc, 
								const OperationBuffer &bufferDst, 
								const bool autokeyframe, 
								const double keyframein, 
								const double keyframelen, 
								const double keyframeout, 
								const bool replaceExisting );

	void		ClearBlendShapes();
	void		RemoveBlendShape();		//!< remove using current time

	//! that's only for temp constraint !
	static int		ComputeDifference( const OperationBuffer &mBufferSrc, const OperationBuffer &mBufferDst, CDeformerShape *shape );

	//
	//

	const int GetNumberOfChannels() const;
	const char *GetChannelName(const int index) const;
	CDeformerChannel *GetChannelPtr(const int index);
	const int GetNumberOfChannelVertices(const int index) const
	{
		return mManager.GetNumberOfChannelVertices(index);
	}

	const int GetNumberOfShapes(const int channel) const;
	const char *GetShapeName(const int channel, const int shape) const;
	CDeformerShape *GetShapePtr(const int channel, const int shape)
	{
		return mManager.GetShapePtr(channel, shape);
	}
	FBPropertyAnimatableDouble *GetShapeProperty(const int channel, const int shape);

	// move all shapes to zero and put a keyframe for that
	void	ZeroAll();

	void SelectShapes(FBModel *pModel);
	void DeselectShapes();

	// put keyframe in each frame (use lower transformation level) - can be used to collapse several deformers in one
	void	Plot(CDeformerChannel *pChannel, const FBTime &startTime, const FBTime &stopTime, const FBTime &stepTime);

private:

	FBSystem		mSystem;

	//--- Local parameters
	FBAnimationNode*	mSourceTranslation;		//!< AnimationNode: INPUT  -> Source.
	FBAnimationNode*	mDeformTranslation;		//!< AnimationNode: OUTPUT -> Translation.
	FBAnimationNode*	mGhostNode;				//!< Ghost Node, needed to call AnimationNodeNotify()

	int					mGroupSource;			//!< Source groupe index.
	int					mGroupDeform;			//!< Deformation group index.

	//--- Calculations for deformation
	
	FBTVector					mPosition;		//!< Position vector.
	FBTVector					mOldPosition;	//!< Old position vector.
	FBTVector					mDisplacement;	//!< Displacement vector.
	bool						mFirstTime;		//!< First time that the constraint is called?
	bool						mResetCount;	//!< if we have different model in our constraint
	FBSVector					mScaling;		//!< Scaling vector.

	//
	//

	OperationBuffer				mBufferSrc;
	OperationBuffer				*mBufferDst;

public:

	//
	void						*mMutex;

	//
	std::vector<FBVertex>		mLastPoints;
	void					Reset();

protected:

	HdlFBPlugTemplate<FBManipulator>	mManipulator;

	CDeformerManager			mManager;

};


