
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: neill3d@gmail.com
// 
/////////////////////////////////////////////////////////////////////////////////////////

//--- Class declarations
#include "driven_constraint_constraint.h"
#include "driven_constraint_layout.h"

//--- Registration defines
#define	ORCONSTRAINT__CLASS		ORCONSTRAINT__CLASSNAME
#define ORCONSTRAINT__NAME		"Driven constraint"
#define	ORCONSTRAINT__LABEL		"Driven property constraint"
#define ORCONSTRAINT__DESC		"Let drive src objects's property by dst object's property"

//--- implementation and registration
FBConstraintImplementation	(	ORCONSTRAINT__CLASS		);
FBRegisterConstraint		(	ORCONSTRAINT__NAME,
								ORCONSTRAINT__CLASS,
								ORCONSTRAINT__LABEL,
								ORCONSTRAINT__DESC,
								FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

static double gTempValue = 0;
static double	Driven_ConstraintGetTempValue( Driven_Constraint* pObject )					{ return gTempValue; }
static void		Driven_ConstraintSetTempValue( Driven_Constraint* pObject, double pValue )	{ gTempValue=pValue; }

/************************************************
 *	Creation function.
 ************************************************/
bool Driven_Constraint::FBCreate()
{
	//InitProperties();
	FBPropertyInit( this, double, TempValue, Driven_ConstraintGetTempValue, Driven_ConstraintSetTempValue );

	/*
	*	1. Create Reference group & elements
	*	2. Set constraint variables (deformer,layout,description)
	*	3. Set pointers to NULL if necessary
	*/
	Deformer	= false;
	HasLayout	= true;
	Description = "Driven property constraint";

	// Create reference groups
	mGroupSource	= ReferenceGroupAdd( "Source Object",	1 );
	mGroupConstrain	= ReferenceGroupAdd( "Constrain",		1 );

	mSrcModel = nullptr;
	mSrcProp = nullptr;
	mDstModel = nullptr;
	mDstProp = nullptr;

	return true;
}

/************************************************
 *	Destruction function.
 ************************************************/
void Driven_Constraint::FBDestroy()
{
	/*
	*	Free any user memory associated to constraint
	*/
}


/************************************************
 *	Refrence added notification.
 ************************************************/
bool Driven_Constraint::ReferenceAddNotify( int pGroupIndex, FBModel *pModel )
{
	/*
	*	Perform action required when a refrence is added.
	*/
	return true;
}


/************************************************
 *	Reference removed notification.
 ************************************************/
bool Driven_Constraint::ReferenceRemoveNotify( int pGroupIndex, FBModel *pModel )
{
	/*
	*	Perform action required when a refrence is removed.
	*/
	return true;
}


/************************************************
 *	Setup all of the animation nodes.
 ************************************************/
void Driven_Constraint::SetupAllAnimationNodes()
{
	/*
	*	- Based on the existence of the references (ReferenceGet() function),
	*	  create the required animation nodes.
	*	- A source will use an Out node, whereas a destination will use
	*	  an In Node.
	*	- If the node is with respect to a deformation, then Bind the reference
	*	  to the deformation notification ( DeformerBind() )
	*/
	if (ReferenceGet( mGroupSource, 0 ) && ReferenceGet( mGroupConstrain, 0 ) )
	{
		mSourceTranslation		= AnimationNodeOutCreate( 0, ReferenceGet( mGroupSource,	0 ), ANIMATIONNODE_TYPE_TRANSLATION );
		mConstrainedTranslation = AnimationNodeInCreate	( 1, ReferenceGet( mGroupConstrain, 0 ), ANIMATIONNODE_TYPE_TRANSLATION );
	}
}


/************************************************
 *	Removed all of the animation nodes.
 ************************************************/
void Driven_Constraint::RemoveAllAnimationNodes()
{
	/*
	*	If nodes have been bound to deformations, remove the binding.
	*/
}


/************************************************
 *	Save the current state.
 ************************************************/
bool Driven_Constraint::SaveState()
{
	/*
	*	Return whether or not the state of the constraint must be saved.
	*/
	return true;
}


/************************************************
 *	Restore the state.
 ************************************************/
void Driven_Constraint::RestoreState()
{
	/*
	*	Restore any custom status variables that have been stored.
	*/
}


/************************************************
 *	Suggest a snap.
 ************************************************/
void Driven_Constraint::SnapSuggested()
{
	/*
	*	Perform any pre-snap operations.
	*/
}


/************************************************
 *	Suggest a freeze.
 ************************************************/
void Driven_Constraint::FreezeSuggested()
{
	/*
	*	Perform any pre-freeze operations
	*/
}


/************************************************
 *	Disable the constraint on pModel.
 ************************************************/
bool Driven_Constraint::Disable(FBModel *pModel)
{
	/*
	*	Perform any operations to disable model
	*	Must call FBConstraint::Disable()
	*/
	return FBConstraint::Disable( pModel );
}


/************************************************
 *	Snap the constraint.
 ************************************************/
void Driven_Constraint::Snap()
{
	/*
	*	Perform operations for constraint snap.
	*	Must call FBConstraint::Snap()
	*/
	FBConstraint::Snap();
}


/************************************************
 *	Real-Time Engine Evaluation.
 ************************************************/
bool Driven_Constraint::AnimationNodeNotify(FBAnimationNode *pConnector,FBEvaluateInfo *pEvaluateInfo,FBConstraintInfo *pConstraintInfo)
{
	/*
	*	Constraint Evaluation
	*	1. Read data from sources
	*	2. Calculate necessary operations.
	*	3. Write output to destinations
	*	Note: Not the deformation operations !
	*/
	
	if (Active)
	{
		if (mSrcProp != NULL && mDstProp != NULL)
		{
			if (mDstProp->GetPropertyType() == kFBPT_double)
			{
				if (mSrcProp->GetPropertyType() == kFBPT_double)
				{
					// get value from the dst prop
					double temp;
					mSrcProp->GetData( &temp, sizeof(double) );
					// and set up for src prop
					mDstProp->SetData( &temp );
				}
				else if (mSrcProp->GetPropertyType() == kFBPT_Vector3D)
				{
					// get vector from the dst prop
					FBVector3d	temp( 100.0, 100.0, 100.0 );
					mSrcProp->GetData( &temp, sizeof(FBVector3d) );

					// set only x for src prop
					mDstProp->SetData( &temp[0] );
				}
			}
			else if (mDstProp->GetPropertyType() == kFBPT_Vector3D)
			{
				//
			}
		}
	}

	return true;
}


/************************************************
 *	Real-Time Deformer Evaluation.
 ************************************************/
bool Driven_Constraint::DeformerNotify(FBModel *pModel, const FBVertex  *pSrcVertex, const FBVertex *pSrcNormal,
									int pCount, 
									FBVertex  *pDstVertex,FBVertex  *pDstNormal)
{
	/*
	*	1. Get the data from the source vertices
	*	2. Calculate the necessary operations
	*	3. Write the output to the destination vertices.
	*	Note: Not the non-deformation operations !
	*/
	return true;
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool Driven_Constraint::FbxStore( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	FBX Storage of constraint parameters.
	*/
	if (pStoreWhat & kAttributes)
	{
		FBString name = (mSrcModel) ? mSrcModel->Name : "Empty"; 
		pFbxObject->FieldWriteC("Src", name );
		pFbxObject->FieldWriteC("SrcProp", (mSrcProp) ? mSrcProp->GetName() : "Empty" );
		pFbxObject->FieldWriteC("Dst", (mDstModel) ? mDstModel->Name : "Empty" );
		pFbxObject->FieldWriteC("DstProp", (mDstProp) ? mDstProp->GetName() : "Empty" );
		
	}

	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
FBComponent *FindComponentInScene(FBString &name)
{
	// try in components
	for (int i=0; i<FBSystem().Scene->Components.GetCount(); ++i)
	{
		FBString compname = FBSystem().Scene->Components[i]->Name.AsString(); 
		if (strstr( (char*)name, (char*)compname) != nullptr)
			return FBSystem().Scene->Components[i];
	}
	// try in constraints
	for (int i=0; i<FBSystem().Scene->Constraints.GetCount(); ++i)
	{
		FBConstraint *constraint = FBSystem().Scene->Constraints[i];
		FBString compname = constraint->Name.AsString(); 
		if (strstr( (char*)name, (char*)compname) != nullptr)
			return constraint;
	}
	return nullptr;
}

bool Driven_Constraint::FbxRetrieve( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	FBX Retrieval of constraint parameters.
	*/
	if (pStoreWhat & kAttributes)
	{
			mSrcName = "";
			mDstName = "";
			mSrcPropName = "";
			mDstPropName = "";
			

			mSrcName = pFbxObject->FieldReadC("Src");
			mSrcPropName = pFbxObject->FieldReadC("SrcProp");
			mDstName = pFbxObject->FieldReadC("Dst");
			mDstPropName = pFbxObject->FieldReadC("DstProp");		
	}
	else
	if (pStoreWhat & kCleanup)
	{
		mSrcModel = nullptr;
		mSrcProp = nullptr;
		mDstModel = nullptr;
		mDstProp = nullptr;

		// try to assign models and property pointers according loaded names
		
		mSrcModel = FBFindModelByLabelName(mSrcName);
		if (!mSrcModel) mSrcModel = FindComponentInScene(mSrcName);
		if (mSrcModel) {
			if (mSrcPropName != "" && mSrcPropName != "Empty") mSrcProp = mSrcModel->PropertyList.Find(mSrcPropName);
		}

		mDstModel = FBFindModelByLabelName(mDstName);
		if (!mDstModel) mDstModel = FindComponentInScene(mDstName);
		if (mDstModel) {
			if (mDstPropName != "" && mDstPropName != "Empty") mDstProp = mDstModel->PropertyList.Find(mDstPropName);
		}
	}

	return true;
}


/************************************************
 *	Deformer Binding.
 ************************************************/
bool Driven_Constraint::DeformerBind( FBModel *pModel )
{
	/*
	*	Perform the operations for the binding of a model to be deformed
	*	Must call FBConstraint::DeformerBind()
	*/
	return FBConstraint::DeformerBind( pModel );
}


/************************************************
 *	Deformer Unbinding.
 ************************************************/
bool Driven_Constraint::DeformerUnBind( FBModel *pModel )
{
	/*
	*	Perform the operations for the unbinding of a model being deformed
	*	Must call FBConstraint::DeformerUnBind()
	*/
	return FBConstraint::DeformerUnBind( pModel );
}


/************************************************
 *	Freeze SRT for the constraint.
 ************************************************/
void Driven_Constraint::FreezeSRT	( FBModel *pModel, bool pS, bool pR, bool pT )
{
	/*
	*	Freeze the model's SRT parameters (if true,true,true)
	*	Must call FBConstraint::FreezeSRT
	*/
	FBConstraint::FreezeSRT( pModel, pS, pR, pT );
}

