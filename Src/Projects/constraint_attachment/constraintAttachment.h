#ifndef __CCONSTRAINT_ATTACHMENT_H__
#define __CCONSTRAINT_ATTACHMENT_H__


/**	\file	constraintAttachment.h

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#define CCONSTRAINTATTACHMENT__CLASSNAME	CConstraintAttachment
#define CCONSTRAINTATTACHMENT__CLASSSTR		"CConstraintAttachment"


//////////////////////////////////////////////////////////////////////////////////
//! An attachment constraint class.
class CConstraintAttachment : public FBConstraint
{
	//--- declaration
	FBConstraintDeclare(CConstraintAttachment, FBConstraint);

public:
	//--- Creation & Destruction
	virtual bool			FBCreate();							//!< Constructor.
	virtual void			FBDestroy();						//!< Destructor.

	//--- Animation node management
	virtual void			RemoveAllAnimationNodes();			//!< Remove animation nodes.
	virtual void			SetupAllAnimationNodes();			//!< Setup animation nodes.

	virtual void			SnapSuggested() override;				//!< Suggest 'snap'.
	virtual void			FreezeSuggested	() override;				//!< Suggest 'freeze'.

	//! Real-time evaluation engine function.
	virtual bool			AnimationNodeNotify		( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo, FBConstraintInfo* pConstraintInfo ) override;

	virtual bool DeformerNotify(	FBModel* pModel, const FBVertex*  pSrcVertex, const FBVertex* pSrcNormal,
									int pCount, 
									FBVertex*  pDstVertex,FBVertex*  pDstNormal) override;

	//--- FBX Interface
	virtual bool			FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );	//!< FBX Storage.
	virtual bool			FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );	//!< FBX Retrieval.

	//--- Local parameters
	FBAnimationNode		*mSourceTranslation;			//!< AnimationNode: INPUT  -> Source.
	FBAnimationNode		*mSourceRotation;
	FBAnimationNode		*mSourceScaling;

	FBAnimationNode		*mConstrainedTranslation;	//!< AnimationNode: OUTPUT -> Constraint.
	FBAnimationNode		*mConstrainedRotation;
	FBAnimationNode		*mDeformTranslation;			//!< AnimationNode: OUTPUT -> Translation.

	int					mGroupSource;		//!< Source groupe index.
	int					mGroupConstrain;	//!< Constraint group index.
	int					mGroupDeform;		//!< Deformation group index.

	FBTVector			mPosition;			//!< Position vector.
	FBTVector			mOldPosition;		//!< Old position vector.
	FBTVector			mDisplacement;		//!< Displacement vector.
	bool				mFirstTime;			//!< First time that the constraint is called?
	FBSVector			mScaling;			//!< Scaling vector.

public:

	FBPropertyBool			PutOnSurface;	// calculate an offset that the constrain object will be on a source mesh

	// constrain local matrix
	FBPropertyVector3d		OffsetTranslation;
	FBPropertyVector3d		OffsetRotation;
	FBPropertyVector3d		OffsetScaling;

	// attachment point
	FBPropertyInt			AttachmentVertA;
	FBPropertyInt			AttachmentVertB;
	FBPropertyInt			AttachmentVertC;
	FBPropertyVector3d		BaryCoords;	

	FBPropertyAction		DebugCmd;

protected:

	// precalculated from deformation
	FBMatrix			mAttachmentMatrix;

	FBVertex			mDebugA;
	FBVertex			mDebugB;
	FBVertex			mDebugC;
	FBVector3d			mDebugNormal;

	bool		CalculateClosestPoints3(FBModel *srcModel, FBModel *dstModel, FBVector3d &srcContactPoint, FBVector3d &dstContactPoint, int &outVertA, int &outVertB, int &outVertC, FBVector3d &outBaryCoords );
	
	bool		BaryCoordsToTM( FBMatrix &modelMatrix, FBMatrix &normalMatrix, int numVerts, const FBVertex *vertices, const FBNormal *normals, 
									int vertA, int vertB, int vertC, const FBVector3d &baryCoords, FBVector3d &pos, FBVector3d &nor );

	void		ComputeAttachmentMatrix(const FBMatrix &modelMatrix, const FBVertex *pSrcVertex, const int vertA, const int vertB, const int vertC, const FBVector3d &pos);

	void		DoDebug();

	static		void SetDebug (HIObject pObject, bool value);
};

#endif	/* __ORCONSTRAINT_ATTACHMENT_H__ */
