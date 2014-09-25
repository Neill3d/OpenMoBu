#ifndef __DRIVEN_CONSTRAINT_CONSTRAINT_H__
#define __DRIVEN_CONSTRAINT_CONSTRAINT_H__


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

#define	ORCONSTRAINT__CLASSNAME		Driven_Constraint
#define ORCONSTRAINT__CLASSSTR		"Driven_Constraint"

//! A simple constraint class.
class Driven_Constraint : public FBConstraint
{
	//--- declaration
	FBConstraintDeclare( Driven_Constraint, FBConstraint );

public:
	//--- Creation & Destruction
	virtual bool			FBCreate();							//!< Constructor.
	virtual void			FBDestroy();						//!< Destructor.

	//--- Animation node management
	virtual void			RemoveAllAnimationNodes();			//!< Remove animation nodes.
	virtual void			SetupAllAnimationNodes();			//!< Setup animation nodes.


	virtual bool			SaveState			();				//!< Save current state.
	virtual void			RestoreState		();				//!< Restore current state.
	virtual void			SnapSuggested		();				//!< Suggest 'snap'.
	virtual void			FreezeSuggested		();				//!< Suggest 'freeze'.

	//--- Constraint Status interface
	virtual bool			Disable		( FBModel *pModel );	//!< Disable the constraint on a model.
	virtual void			Snap		();						//!< 'Snap' constraint.

	//--- Real-Time Engine
	//! Real-time evaluation engine function.
	virtual bool AnimationNodeNotify( FBAnimationNode *pAnimationNode, FBEvaluateInfo *pEvaluateInfo, FBConstraintInfo *pConstraintInfo );

	//--- Deformation Interface
	//! Real-time deformation engine function.
	virtual bool			DeformerNotify(	FBModel *pModel, const FBVertex  *pSrcVertex, const FBVertex *pSrcNormal,
									int pCount, 
									FBVertex  *pDstVertex,FBVertex  *pDstNormal);
	virtual bool			DeformerBind	( FBModel *pModel );	//!< Bind a model to deformer callback.
	virtual bool			DeformerUnBind	( FBModel *pModel );	//!< Unbind a model from deformer callback.


	virtual void			FreezeSRT	( FBModel *pModel, bool pS, bool pR, bool pT );						//!< Freeze Scaling, Rotation and Translation for a model.

	virtual bool			ReferenceAddNotify		( int pGroupIndex, FBModel *pModel );					//!< Reference added: Callback.
	virtual bool			ReferenceRemoveNotify	( int pGroupIndex, FBModel *pModel );					//!< Reference removed: Callback.


	//--- FBX Interface
	virtual bool			FbxStore	( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );	//!< FBX Storage.
	virtual bool			FbxRetrieve	( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );	//!< FBX Retrieval.


	//-- constraint models
	FBString			mSrcName;
	FBString			mDstName;

	FBComponent		*mSrcModel;
	FBComponent		*mDstModel;

	//--- Local parameters
	FBString				mSrcPropName;
	FBString				mDstPropName;

	FBProperty				*mSrcProp;
	FBProperty				*mDstProp;

	int		mGroupSource;
	int		mGroupConstrain;

	FBAnimationNode			*mSourceTranslation;			//!< AnimationNode: INPUT  -> Source.
	FBAnimationNode			*mConstrainedTranslation;	//!< AnimationNode: OUTPUT -> Constraint.

//	FBPropertiesDeclare( Driven_Constraint, FBConstraint );
	FBPropertyDouble		TempValue;

private:
};

#endif	/* __DRIVEN_CONSTRAINT_CONSTRAINT_H__ */
