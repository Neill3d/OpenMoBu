
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


//--- Class declarations
#include "jointSocket.h"
#include "core.h"

//--- Registration defines
#define	ORJOINTSOCKET__CLASS		ORJOINTSOCKET__CLASSNAME
#define ORJOINTSOCKET__NAME			"Ball constraint"
#define	ORJOINTSOCKET__LABEL		"Ball constraint"
#define ORJOINTSOCKET__DESC			"joint between two rigid bodies"

//--- implementation and registration
FBConstraintImplementation	(	ORJOINTSOCKET__CLASS		);
FBRegisterConstraint		(	ORJOINTSOCKET__NAME,
							 ORJOINTSOCKET__CLASS,
							 ORJOINTSOCKET__LABEL,
							 ORJOINTSOCKET__DESC,
							 FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

/************************************************
*	Creation function.
************************************************/
bool jointSocket::FBCreate()
{
	//InitProperties();

	/*
	*	1. Create Reference group & elements
	*	2. Set constraint variables (deformer,layout,description)
	*	3. Set pointers to NULL if necessary
	*/
	Deformer	= false;
	HasLayout	= false;
	Description = "ball & socket joint";

	// Create reference groups
	mGroup	= ReferenceGroupAdd( "Nodes",	3 );

	mActive = false;

	return true;
}


/************************************************
*	Destruction function.
************************************************/
void jointSocket::FBDestroy()
{
	/*
	*	Free any user memory associated to constraint
	*/
}


/************************************************
*	Refrence added notification.
************************************************/
bool jointSocket::ReferenceAddNotify( int pGroupIndex, HFBModel pModel )
{
	/*
	*	Perform action required when a refrence is added.
	*/
	return true;
}


/************************************************
*	Reference removed notification.
************************************************/
bool jointSocket::ReferenceRemoveNotify( int pGroupIndex, HFBModel pModel )
{
	/*
	*	Perform action required when a reference is removed.
	*/

	return true;
}


/************************************************
*	Setup all of the animation nodes.
************************************************/
void jointSocket::SetupAllAnimationNodes()
{
	/*
	*	- Based on the existence of the references (ReferenceGet() function),
	*	  create the required animation nodes.
	*	- A source will use an Out node, whereas a destination will use
	*	  an In Node.
	*	- If the node is with respect to a deformation, then Bind the reference
	*	  to the deformation notification ( DeformerBind() )
	*/
	if ( ReferenceGetCount( mGroup ) == 3 )
	{
		mModelChild		= AnimationNodeOutCreate( 0, ReferenceGet( mGroup,	0), ANIMATIONNODE_TYPE_TRANSLATION );
		mModelParent	= AnimationNodeOutCreate( 1, ReferenceGet( mGroup, 1), ANIMATIONNODE_TYPE_TRANSLATION );
		mPivot			= AnimationNodeInCreate( 2, ReferenceGet(mGroup, 2), ANIMATIONNODE_TYPE_TRANSLATION );
	}
}


/************************************************
*	Removed all of the animation nodes.
************************************************/
void jointSocket::RemoveAllAnimationNodes()
{
	/*
	*	If nodes have been bound to deformations, remove the binding.
	*/

	if ( mActive && ReferenceGetCount( mGroup ) == 0 )
	{
		RemoveRigidJoint( constraint );
		mActive = false;
	}
}


/************************************************
*	Save the current state.
************************************************/
bool jointSocket::SaveState()
{
	/*
	*	Return whether or not the state of the constraint must be saved.
	*/
	if (ReferenceGet( 0,0 ))
	{
		return true;
	}
	return true;
}


/************************************************
*	Restore the state.
************************************************/
void jointSocket::RestoreState()
{
	/*
	*	Restore any custom status variables that have been stored.
	*/
}


/************************************************
*	Suggest a snap.
************************************************/
void jointSocket::SnapSuggested()
{
	/*
	*	Perform any pre-snap operations.
	*/
}


/************************************************
*	Suggest a freeze.
************************************************/
void jointSocket::FreezeSuggested()
{
	/*
	*	Perform any pre-freeze operations
	*/
	FBConstraint::FreezeSuggested();

	if( ReferenceGet( 0,0 ) )
	{
		FreezeSRT( (HFBModel)ReferenceGet( 0, 0), true, true, true );
	}
}


/************************************************
*	Disable the constraint on pModel.
************************************************/
bool jointSocket::Disable(HFBModel pModel)
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
void jointSocket::Snap()
{
	/*
	*	Perform operations for constraint snap.
	*	Must call FBConstraint::Snap()
	*/
	FBConstraint::Snap = true;
}


/************************************************
*	Real-Time Engine Evaluation.
************************************************/
bool jointSocket::AnimationNodeNotify(HFBAnimationNode pConnector,HFBEvaluateInfo pEvaluateInfo,HFBConstraintInfo pConstraintInfo)
{
	/*
	*	Constraint Evaluation
	*	1. Read data from sources
	*	2. Calculate necessary operations.
	*	3. Write output to destinations
	*	Note: Not the deformation operations !
	*/
	double		lTime = 0.0;
	FBTime		lEvaluationTime;
	FBVector3d	lAccel;

	if (ReferenceGetCount(mGroup) != 3) return false;
	ReferenceGet( mGroup, 2 )->GetVector( lAccel );

	if ( !Active ) {
		return false;
	} else
	{
		if (!mActive)
		{
			int lChildNdx=0, lParentNdx=0;

			HFBProperty lProp = ReferenceGet(mGroup, 0)->PropertyList.Find("RigidBody_Index");
			if (lProp) lChildNdx = lProp->AsInt();
			lProp = ReferenceGet(mGroup, 1)->PropertyList.Find("RigidBody_Index");
			if (lProp) lParentNdx = lProp->AsInt();

		
			constraint = NewtonConstraintCreateBall( GetWorld(), &lAccel[0],
					GetRigidBodyNode(lChildNdx), GetRigidBodyNode(lParentNdx) );

			mActive = true;
		}
	}

	// Get the current evaluation time, indicating if recording.
	lEvaluationTime = pEvaluateInfo->GetLocalStart();
	lTime	= lEvaluationTime.GetSecondDouble();

	

	// Write the data to the out node
	lAccel[0] = lAccel[1] = lAccel[2] = 0.0;
	mPivot->WriteData( &lAccel[0], pEvaluateInfo );

	return true;
}