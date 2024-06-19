
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
#include "rigidbodies.h"
#include "core.h"

//--- Registration defines
#define	ORRIGIDBODIES__CLASS		ORRIGIDBODIES__CLASSNAME
#define ORRIGIDBODIES__NAME			"Rigid bodies"
#define	ORRIGIDBODIES__LABEL		"Rigid bodies collection"
#define ORRIGIDBODIES__DESC			"Active list of the rigid bodies in simulation"

//--- implementation and registration
FBConstraintImplementation	(	ORRIGIDBODIES__CLASS		);
FBRegisterConstraint		(	ORRIGIDBODIES__NAME,
							 ORRIGIDBODIES__CLASS,
							 ORRIGIDBODIES__LABEL,
							 ORRIGIDBODIES__DESC,
							 FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

double			g_delta = 0.0;
#define			PHYSICS_UPDATE_STEP		0.03

// =============================================================================
//  PhysicsApplyForceAndTorque
// =============================================================================
void PhysicsApplyForceAndTorque( const NewtonBody *pBody )
{
	double mass, Ixx, Iyy, Izz;
	double force[3];

	NewtonBodyGetMassMatrix ( pBody, &mass, &Ixx, &Iyy, &Izz);

	force[0] = 0.0 ;
	force[1] = -mass * 9.8;
	force[2] = 0.0 ;
	NewtonBodySetForce ( pBody, &force[0] );
	/*
	if (pData) 
	{
	force[0] = (pData->pos[0] - pData->lastPos[0]) * 10;
	force[1] = (pData->pos[1] - pData->lastPos[1]) * 10;
	force[2] = (pData->pos[2] - pData->lastPos[2]) * 10;

	NewtonBodyAddForce( pBody, &force[0] );
	}
	*/
}

/************************************************
*	Creation function.
************************************************/
bool RigidBodies::FBCreate()
{
//	InitProperties();


	/*
	*	1. Create Reference group & elements
	*	2. Set constraint variables (deformer,layout,description)
	*	3. Set pointers to NULL if necessary
	*/
	Deformer	= false;
	HasLayout	= false;
	Description = "Rigid bodies collection";

	// Create reference groups
	mGroupSource	= ReferenceGroupAdd( "Source Objects",	100 );
	mGroupConstrain	= ReferenceGroupAdd( "Special",		1 );

	mFirstTime = true;
	mSetupTime = true;

	return true;
}


HFBModel RigidBodies::GetSourceModel(int index)
{
	return ReferenceGet( mGroupSource, index );
}

HFBModel RigidBodies::GetConstraintModel()
{
	return ReferenceGet( mGroupConstrain, 0 );
}

/************************************************
*	Destruction function.
************************************************/
void RigidBodies::FBDestroy()
{
	/*
	*	Free any user memory associated to constraint
	*/

	//RemoveAllRigidBodies();
}


/************************************************
*	Refrence added notification.
************************************************/
bool RigidBodies::ReferenceAddNotify( int pGroupIndex, HFBModel pModel )
{
	/*
	*	Perform action required when a refrence is added.
	*/

	if (pGroupIndex == mGroupSource)
	{
		HFBProperty lProp = pModel->PropertyList.Find("RigidBody_Mass");
		if (lProp) {
			return true;
		}

		return false;
	}

	return true;
}


/************************************************
*	Reference removed notification.
************************************************/
bool RigidBodies::ReferenceRemoveNotify( int pGroupIndex, HFBModel pModel )
{
	/*
	*	Perform action required when a reference is removed.
	*/
	if ((pGroupIndex == mGroupSource) && !mSetupTime) {

		HFBProperty lProp = pModel->PropertyList.Find("RigidBody_Index");
		NewtonBody *lBody = GetRigidBodyNode( lProp->AsInt() );
	}

	return true;
}



/************************************************
*	Setup all of the animation nodes.
************************************************/
void RigidBodies::SetupAllAnimationNodes()
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
		mSourceTranslation		= AnimationNodeOutCreate( 0, ReferenceGet( mGroupConstrain,	0 ), ANIMATIONNODE_TYPE_TRANSLATION );
		mConstrainedTranslation = AnimationNodeInCreate	( 1, ReferenceGet( mGroupConstrain, 0 ), ANIMATIONNODE_TYPE_TRANSLATION );
	}
}


/************************************************
*	Removed all of the animation nodes.
************************************************/
void RigidBodies::RemoveAllAnimationNodes()
{
	/*
	*	If nodes have been bound to deformations, remove the binding.
	*/
/*
	if (!mSetupTime) {
		RemoveAllRigidBodies();
		mSetupTime = true;
	}
	*/
}


/************************************************
*	Save the current state.
************************************************/
bool RigidBodies::SaveState()
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
void RigidBodies::RestoreState()
{
	/*
	*	Restore any custom status variables that have been stored.
	*/
	return;
}

void RigidBodies::PrepareBodies()
{
	//-- create newton rigid body's from source objects
	int count = ReferenceGetCount( mGroupSource );

	for (int i=0; i<count; ++i)
	{
		mSetupTime = false;

		HFBModel lModel = ReferenceGet( mGroupSource, i );

		HFBProperty lProp = lModel->PropertyList.Find("RigidBody_Index" );
		if (lProp) {
			lProp->SetInt(i);
		}

		// make NewTon rigid body
		lProp = lModel->PropertyList.Find( "RigidBody_Mass" );
		if (lProp){
			double mass;
			lProp->GetData( &mass, sizeof(double) );

			//
			// Physics engine init
			//

			lModel->GetMatrix( bodyMatrix, kModelTransformation );
			lModel->GetVector( boxSize, kModelScaling, false );

			NewtonCollision *boxCollision;
			/*
			boxSize[0] = bodyMatrix(3,0);
			boxSize[1] = bodyMatrix(3,1);
			boxSize[2] = bodyMatrix(3,2);
			*/

			//-- create the collision shape
			boxCollision = NewtonCreateBox( GetWorld(), boxSize[0]*2, boxSize[1]*2, boxSize[2]*2, NULL );
			// create the rigid body
			NewtonBody *rigidBodyBox = NewtonCreateBody( GetWorld(), boxCollision );
			AddRigidBody(rigidBodyBox);
			// we don't need it any more
			NewtonReleaseCollision( GetWorld(), boxCollision );

			// set mass for body
			double Ixx = 0.7 * mass * (boxSize[1]*boxSize[1] + boxSize[2]*boxSize[2])/12;
			double Iyy = 0.7 * mass * (boxSize[0]*boxSize[0] + boxSize[2]*boxSize[2])/12;
			double Izz = 0.7 * mass * (boxSize[0]*boxSize[0] + boxSize[1]*boxSize[1])/12;
			NewtonBodySetMassMatrix( rigidBodyBox, mass, Ixx, Iyy, Izz );

			// user data (initial body matrix)
			userData = new RigidBodyUserData;
			userData->initialMatrix = bodyMatrix;
			userData->initialScale.Set( &boxSize[0] );

			//--> create animation for position output
			//userData->modelPosition = AnimationNodeInCreate	( i+2, lModel, ANIMATIONNODE_TYPE_TRANSLATION );
			//userData->modelRotation = AnimationNodeInCreate ( i+2, lModel, ANIMATIONNODE_TYPE_ROTATION );

			NewtonBodySetUserData( rigidBodyBox, userData );
			userData = NULL;

			// set position for body
			NewtonBodySetMatrix (rigidBodyBox, bodyMatrix);
			NewtonBodySetForceAndTorqueCallback( rigidBodyBox, &PhysicsApplyForceAndTorque );

			lastTimeDouble = 0.0;
		} else {
			FBMessageBox( "Physics simulations ERROR", "missing rigid body mass", "Ok" );
		}
	}
}

/************************************************
*	Suggest a snap.
************************************************/
void RigidBodies::SnapSuggested()
{
	/*
	*	Perform any pre-snap operations.
	*/
	FBConstraint::SnapSuggested();

	PrepareBodies();
}


/************************************************
*	Suggest a freeze.
************************************************/
void RigidBodies::FreezeSuggested()
{
	/*
	*	Perform any pre-freeze operations
	*/
	FBConstraint::FreezeSuggested();

	if( ReferenceGet( 0,0 ) )
	{
		FreezeSRT( (HFBModel)ReferenceGet( 0, 0), true, true, true );
	}

	RemoveAllRigidBodies();
}


/************************************************
*	Disable the constraint on pModel.
************************************************/
bool RigidBodies::Disable(HFBModel pModel)
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
void RigidBodies::Snap()
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
void matrixMult(FBMatrix &a, FBMatrix &b, FBMatrix &res)
{
	FBMatrix c(res);
	for (int i=0; i<3; i++)
		for (int j=0; j<3; j++)
		{
			float sum = 0.0f;
			for (int k=0; k<3; k++)
				sum += a(i,k) * b(k,j);
			c(i,j) = sum;
		}
	res = c;
}

bool RigidBodies::AnimationNodeNotify(HFBAnimationNode pConnector,HFBEvaluateInfo pEvaluateInfo,HFBConstraintInfo pConstraintInfo)
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

	if ( !Active || pConnector!=mConstrainedTranslation ) return false;
	
	// Get the current evaluation time, indicating if recording.
	lEvaluationTime = pEvaluateInfo->GetLocalStart();
	lTime	= lEvaluationTime.GetSecondDouble();

	if (lTime == 0.0 && mFirstTime)
	{
		mFirstTime = false;

		lastTimeDouble = 0.0;
		g_delta = 0.0;

		for (int i=0; i<ReferenceGetCount( mGroupSource ); ++i)
		{
			NewtonBody *lBody = GetRigidBodyNode(i);
			userData = (RigidBodyUserData*)NewtonBodyGetUserData( lBody );
			FBMatrix	initTM = userData->initialMatrix;
			NewtonBodySetMatrix (lBody, initTM);	

			lAccel[0] = lAccel[1] = lAccel[2] = 0.0f;
			NewtonBodySetVelocity( lBody, &lAccel[0] );
			NewtonBodySetOmega( lBody, &lAccel[0] );

			NewtonWorldUnfreezeBody( GetWorld(), lBody );

			// position source object
			initTM = userData->initialMatrix;
			HFBModel lModel = ReferenceGet( mGroupSource, i );
			lModel->SetMatrix( userData->initialMatrix );
			//lModel->SetVector( userData->initialScale, kModelScaling, true );

			/*
			//-- set position using animation node
			lAccel[0] = userData->initialMatrix(3,0);
			lAccel[1] = userData->initialMatrix(3,1);
			lAccel[2] = userData->initialMatrix(3,2);
			userData->modelPosition->WriteData( &lAccel[0], pEvaluateInfo );
			// rotation animation node
			NewtonGetEulerAngle( userData->initialMatrix, &lAccel[0] );
			lAccel[0] = lAccel[0] / 3.14 * 180;
			lAccel[1] = lAccel[1] / 3.14 * 180;
			lAccel[2] = lAccel[2] / 3.14 * 180;
			userData->modelRotation->WriteData( &lAccel[0], pEvaluateInfo );
			*/
		}
	} 
	else if ( !pEvaluateInfo->IsStop() )
	{
		mFirstTime = true;

		g_delta += lTime - lastTimeDouble;

		while ( g_delta > PHYSICS_UPDATE_STEP ) {

			WorldUpdate( PHYSICS_UPDATE_STEP );
			g_delta -= PHYSICS_UPDATE_STEP;
		}

		//-- update each object transformation!
		for (int i=0; i<ReferenceGetCount( mGroupSource ); ++i)
		{
			NewtonBody *lBody = GetRigidBodyNode(i);
			NewtonBodyGetMatrix( lBody, bodyMatrix );
			userData = (RigidBodyUserData*)NewtonBodyGetUserData( lBody );
			FBMatrix	scaleTM;
			scaleTM.Identity();
			scaleTM(0,0) = userData->initialScale[0];
			scaleTM(1,1) = userData->initialScale[1];
			scaleTM(2,2) = userData->initialScale[2];
			matrixMult(bodyMatrix, scaleTM, bodyMatrix);
			// position source object
			HFBModel lModel = ReferenceGet( mGroupSource, i );
			lModel->SetMatrix( bodyMatrix );
			//lModel->SetVector( userData->initialScale, kModelScaling, true );

			/*
			//-- set position using animation node
			lAccel[0] = bodyMatrix(3,0);
			lAccel[1] = bodyMatrix(3,1);
			lAccel[2] = bodyMatrix(3,2);
			userData->modelPosition->WriteData( &lAccel[0], pEvaluateInfo );
			NewtonGetEulerAngle( bodyMatrix, &lAccel[0] );
			lAccel[0] = lAccel[0] / 3.14 * 180;
			lAccel[1] = lAccel[1] / 3.14 * 180;
			lAccel[2] = lAccel[2] / 3.14 * 180;
			userData->modelRotation->WriteData( &lAccel, pEvaluateInfo );
			*/
		}

		lastTimeDouble = lTime;
	}

	// Write the data to the out node
	//lAccel[0] = lAccel[1] = lAccel[2] = 0.0;
	mSourceTranslation->ReadData( &lAccel[0], pEvaluateInfo);
	mConstrainedTranslation->WriteData( &lAccel[0], pEvaluateInfo );

	return true;
}

