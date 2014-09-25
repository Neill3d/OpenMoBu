
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


//--- Class declaration
#include "newtonRigidBody.h"

#include "core.h"
#include "log.h"
#include <conio.h>

//--- Registration defines
#define NEWTON_RIGIDBODY__CLASS			NEWTON_RIGIDBODY__CLASSNAME
#define NEWTON_RIGIDBODY__NAME			NEWTON_RIGIDBODY__CLASSSTR
#define	NEWTON_RIGIDBODY__LOCATION		"Physics"
#define NEWTON_RIGIDBODY__LABEL			"Rigid box"
#define	NEWTON_RIGIDBODY__DESC			"Simple Newton Physics"

#define NEWTON_RIGIDJOINT__CLASS		NEWTON_RIGIDJOINT__CLASSNAME
#define NEWTON_RIGIDJOINT__NAME			NEWTON_RIGIDJOINT__CLASSSTR
#define	NEWTON_RIGIDJOINT__LOCATION		"Physics"
#define NEWTON_RIGIDJOINT__LABEL		"Ball constrant"
#define	NEWTON_RIGIDJOINT__DESC			"Newton Physics"

//--- implementation and registration
FBBoxImplementation	(	NEWTON_RIGIDBODY__CLASS		);		// Box class name
FBRegisterBox		(	NEWTON_RIGIDBODY__NAME,				// Unique name to register box.
						NEWTON_RIGIDBODY__CLASS,			// Box class name
						NEWTON_RIGIDBODY__LOCATION,			// Box location ('plugins')
						NEWTON_RIGIDBODY__LABEL,			// Box label (name of box to display)
						NEWTON_RIGIDBODY__DESC,				// Box long description.
						FB_DEFAULT_SDK_ICON			);		// Icon filename (default=Open Reality icon)

FBBoxImplementation	(	NEWTON_RIGIDJOINT__CLASS		);	// Box class name
FBRegisterBox		(	NEWTON_RIGIDJOINT__NAME,			// Unique name to register box.
					 NEWTON_RIGIDJOINT__CLASS,				// Box class name
					 NEWTON_RIGIDJOINT__LOCATION,			// Box location ('plugins')
					 NEWTON_RIGIDJOINT__LABEL,				// Box label (name of box to display)
					 NEWTON_RIGIDJOINT__DESC,				// Box long description.
					 FB_DEFAULT_SDK_ICON			);		// Icon filename (default=Open Reality icon)

// output debug information
LOG				lLog;

double			g_deltaA = 0.0;

#define			PHYSICS_UPDATE_STEP		0.03

// =============================================================================
//  PhysicsApplyForceAndTorque
// =============================================================================
void PhysicsApplyForceAndTorqueA( const NewtonBody *pBody )
{
	double mass, Ixx, Iyy, Izz;
	double force[3];

	RigidBodyUserDataA		* pData;

	NewtonBodyGetMassMatrix ( pBody, &mass, &Ixx, &Iyy, &Izz);
	pData = (RigidBodyUserDataA*) NewtonBodyGetUserData( pBody );

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
 *	Creation
 ************************************************/
bool CRigidBody::FBCreate()
{
	/*
	*	Create the nodes for the box.
	*/

	// Input Node
    mIn = AnimationNodeInCreate  ( 0, "In", ANIMATIONNODE_TYPE_VECTOR );
	mScaleIn = AnimationNodeInCreate( 0, "Scale", ANIMATIONNODE_TYPE_VECTOR );
	mMassIn = AnimationNodeInCreate  ( 0, "Mass", ANIMATIONNODE_TYPE_NUMBER );

	// Output Node
	mRotationOut = AnimationNodeOutCreate ( 0, "Rotation", ANIMATIONNODE_TYPE_VECTOR );
	mOut = AnimationNodeOutCreate  ( 0, "Out", ANIMATIONNODE_TYPE_VECTOR );
	// rigid body index (for joints)
	mBody = AnimationNodeOutCreate( 0, "Body", ANIMATIONNODE_TYPE_INTEGER );

	//
	// Physics engine init
	//
	
	for (int i = 0; i<4; i++)
		for (int j=0; j<4; j++)
			if (i==j) bodyMatrix[i][j] = 1.0;
			else bodyMatrix[i][j] = 0.0;


	NewtonCollision *boxCollision;

	boxSize[0] = 2.0;
	boxSize[1] = 2.0;
	boxSize[2] = 2.0;

	//-- create the collision shape
	boxCollision = NewtonCreateBox( GetWorld(), 2.0, 2.0, 2.0, NULL );
	// create the rigid body
	rigidBodyBox = NewtonCreateBody( GetWorld(), boxCollision );
	AddRigidBody(rigidBodyBox);
	// we don't need it any more
	NewtonReleaseCollision( GetWorld(), boxCollision );

	// set mass for body
	NewtonBodySetMassMatrix( rigidBodyBox, 1.0, 1.0, 1.0, 1.0 );

	// set position for body
	bodyMatrix[3][0] = 0.0;
	bodyMatrix[3][1] = 1.0;
	bodyMatrix[3][2] = 0.0;

	NewtonBodySetMatrix (rigidBodyBox, &bodyMatrix[0][0]);

	NewtonBodySetForceAndTorqueCallback( rigidBodyBox, &PhysicsApplyForceAndTorqueA );

	// rigid box user data define
	rigidUserData.pos[0] = rigidUserData.pos[1] = rigidUserData.pos[2] = 0.0;
	rigidUserData.lastPos[0] = rigidUserData.lastPos[1] = rigidUserData.lastPos[2] = 0.0;

	NewtonBodySetUserData( rigidBodyBox, &rigidUserData );

	// apply some force for body
	double		gravity[3];

	gravity[0] = 0.0;
	gravity[1] = 9.8;
	gravity[2] = 0.0;

	NewtonBodySetForce( rigidBodyBox, &gravity[0] );

	lastTimeDouble = 0.0;

	return true;
}


/************************************************
 *	Destruction.
 ************************************************/
void CRigidBody::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/

	RemoveRigidBody( rigidBodyBox );
}


/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool CRigidBody::AnimationNodeNotify( HFBAnimationNode pAnimationNode, HFBEvaluateInfo pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/
	 
	FBVector3d	lVector;	// Transferring vector
	FBVector3d	lScale;		// Transferring scale value
	double	lMass;		// Transferring rigid body mass
	double		lAccel[3];		// use for setting velocity and omega vectors
	int			lStatus;	// Status of input node
	double		lTime = 0.0;
	FBTime		lEvaluationTime;

	// Read the data from the input node
	lStatus = mIn->ReadData( &lVector[0], pEvaluateInfo );

	// If the read was not from a dead node.
    if( lStatus != CNT_STATUS_DEAD )
	{
		// Get the current evaluation time, indicating if recording.

		lEvaluationTime = pEvaluateInfo->GetLocalStart();

		// Calculate result.
		lTime	= lEvaluationTime.GetSecondDouble();

		if (lTime == 0.0)
		{
			lStatus = mMassIn->ReadData( &lMass, pEvaluateInfo );

			if ( lStatus != CNT_STATUS_DEAD ) 
			{
				NewtonBodySetMassMatrix( rigidBodyBox, (float) lMass, 1.0f, 1.0f, 1.0f );
			}


			lStatus = mScaleIn->ReadData( &lScale[0], pEvaluateInfo );

			if ( lStatus != CNT_STATUS_DEAD )
			{
				//-- create the collision shape
				NewtonCollision *boxCollision = NewtonCreateBox( GetWorld(), (float) lScale[0] * 2, (float) lScale[1] * 2, (float) lScale[2] * 2, NULL );
				// create the rigid body
				NewtonBodySetCollision( rigidBodyBox, boxCollision );
				// we don't need it any more
				NewtonReleaseCollision( GetWorld(), boxCollision );	
			}

			// set position for body
			bodyMatrix[3][0] = lVector[0];
			bodyMatrix[3][1] = lVector[1];
			bodyMatrix[3][2] = lVector[2];

			rigidUserData.pos[0] = lVector[0];
			rigidUserData.pos[1] = lVector[1];
			rigidUserData.pos[2] = lVector[2];

			rigidUserData.lastPos[0] = lVector[0];
			rigidUserData.lastPos[1] = lVector[1];
			rigidUserData.lastPos[2] = lVector[2];

			NewtonBodySetMatrix (rigidBodyBox, &bodyMatrix[0][0]);	

			lAccel[0] = lAccel[1] = lAccel[2] = 0.0f;
			NewtonBodySetVelocity( rigidBodyBox, &lAccel[0] );
			NewtonBodySetOmega( rigidBodyBox, &lAccel[0] );
			
			lastTimeDouble = 0.0;
			g_deltaA = 0.0;

			NewtonWorldUnfreezeBody( GetWorld(), rigidBodyBox );

			lLog.Output( "set time to start" );
		} 
		else if ( !pEvaluateInfo->IsStop() )
		{
			g_deltaA += lTime - lastTimeDouble;

			rigidUserData.pos[0] = lVector[0];
			rigidUserData.pos[1] = lVector[1];
			rigidUserData.pos[2] = lVector[2];

			while ( g_deltaA > PHYSICS_UPDATE_STEP ) {
			
				WorldUpdate( PHYSICS_UPDATE_STEP );
				g_deltaA -= PHYSICS_UPDATE_STEP;
			}

			rigidUserData.lastPos[0] = lVector[0];
			rigidUserData.lastPos[1] = lVector[1];
			rigidUserData.lastPos[2] = lVector[2];

			lastTimeDouble = lTime;
		}


		NewtonBodyGetMatrix( rigidBodyBox, &bodyMatrix[0][0] );
		lVector[0] = bodyMatrix[3][0];
		lVector[1] = bodyMatrix[3][1];
		lVector[2] = bodyMatrix[3][2];

	
		char	szText[64];

		sprintf( &szText[0], "- update world - %f", &lTime );
		lLog.Output( szText );
		


		// Write the data to the out node
		mOut->WriteData( &lVector[0], pEvaluateInfo);
		NewtonGetEulerAngle( &bodyMatrix[0][0], &lAccel[0] );

		lVector[0] = (double) lAccel[0] / 3.14 * 180;
		lVector[1] = (double) lAccel[1] / 3.14 * 180;
		lVector[2] = (double) lAccel[2] / 3.14 * 180;
		mRotationOut->WriteData( &lVector[0], pEvaluateInfo );

		// write body index
		double index = (double)GetRigidBodyIndex( rigidBodyBox );
		mBody->WriteData( &index, pEvaluateInfo );

		// Return LIVE
	    return CNT_STATUS_LIVE;
	}

	// Is DEAD, return DEAD
	return CNT_STATUS_DEAD;
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool CRigidBody::FbxStore( HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Store box parameters.
	*/
	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
bool CRigidBody::FbxRetrieve(HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Retrieve box parameters.
	*/
	return true;
}




/************************************************
*	Creation
************************************************/
bool CRigidJoint::FBCreate()
{
	/*
	*	Create the nodes for the box.
	*/

	// Input Node
	mBodyChild =	AnimationNodeInCreate	( 0, "Child body", ANIMATIONNODE_TYPE_INTEGER );
	mBodyParent =	AnimationNodeInCreate	( 0, "Parent body", ANIMATIONNODE_TYPE_INTEGER );
	//mPivot =		AnimationNodeInCreate	( 0, "Pivot", ANIMATIONNODE_TYPE_VECTOR );
	
	mPivot	=		AnimationNodeOutCreate(	0, "Pivot", ANIMATIONNODE_TYPE_VECTOR );

	ballConstraint = NULL;

	return true;
}

bool CRigidJoint::AnimationNodeDestroy( HFBAnimationNode pAnimationNode )
{
	if (ballConstraint)
		RemoveRigidJoint( ballConstraint );
	return true;
}

/************************************************
*	Destruction.
************************************************/
void CRigidJoint::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
	if (ballConstraint)
		RemoveRigidJoint( ballConstraint );
}


/************************************************
*	Real-time engine evaluation
************************************************/
bool CRigidJoint::AnimationNodeNotify( HFBAnimationNode pAnimationNode, HFBEvaluateInfo pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/
	FBVector3d	lPivot;		// Transferring pivot
	double		lBodyChild;		// Transferring child index
	double		lBodyParent;	// Transferring parent index
	int			lStatus1, lStatus2, lStatus3;	// Status of input node

	// Read the data from the input node
	lStatus1 = mBodyChild->ReadData( &lBodyChild, pEvaluateInfo );
	lStatus2 = mBodyParent->ReadData( &lBodyParent, pEvaluateInfo );
	lStatus3 = mPivot->ReadData( &lPivot[0], pEvaluateInfo );

	// If the read was not from a dead node.
	if( lStatus1 != CNT_STATUS_DEAD && lStatus2 != CNT_STATUS_DEAD && lStatus3 != CNT_STATUS_DEAD )
	{
		if (lStatus1!=oldStatus1 && lStatus2!=oldStatus2 && lStatus3!=oldStatus3)
		{
			if (pEvaluateInfo->IsStop())
			{
				if (ballConstraint) RemoveRigidJoint(ballConstraint);

				double		pivot[3];
				pivot[0] = lPivot[0];
				pivot[1] = lPivot[1];
				pivot[2] = lPivot[2];

				ballConstraint = NewtonConstraintCreateBall( GetWorld(), &pivot[0], GetRigidBodyNode((int)(lBodyChild)),
								GetRigidBodyNode((int)(lBodyParent)) );
			}
		}
		oldStatus1 = lStatus1;
		oldStatus2 = lStatus2;
		oldStatus3 = lStatus3;

//		mOut->WriteData( &lPivot, pEvaluateInfo);

		// Return LIVE
		return CNT_STATUS_LIVE;
	}

	// Is DEAD, return DEAD
	return CNT_STATUS_DEAD;
}


/************************************************
*	FBX Storage.
************************************************/
bool CRigidJoint::FbxStore( HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Store box parameters.
	*/
	return true;
}


/************************************************
*	FBX Retrieval.
************************************************/
bool CRigidJoint::FbxRetrieve(HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Retrieve box parameters.
	*/
	return true;
}
