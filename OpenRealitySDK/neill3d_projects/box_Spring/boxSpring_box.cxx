
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://code.google.com/p/motioncodelibrary/wiki/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////

/**	\file	boxSpring_box.cxx
*/

//--- Class declaration
#include <math.h>
#include "boxSpring_box.h"

//--- Registration defines
#define BOXSPRING__CLASS		BOXSPRING__CLASSNAME
#define BOXSPRING__NAME			BOXSPRING__CLASSSTR
#define	BOXSPRING__LOCATION		"Neill3d"
#define BOXSPRING__LABEL		"Spring"
#define	BOXSPRING__DESC			"spring controller"

//--- implementation and registration
FBBoxImplementation	(	BOXSPRING__CLASS		);	// Box class name
FBRegisterBox		(	BOXSPRING__NAME,			// Unique name to register box.
						BOXSPRING__CLASS,			// Box class name
						BOXSPRING__LOCATION,		// Box location ('plugins')
						BOXSPRING__LABEL,			// Box label (name of box to display)
						BOXSPRING__DESC,			// Box long description.
						FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)

/************************************************
 *	Creation
 ************************************************/
bool CBoxSpring::FBCreate()
{
	lastLocalTimeDouble = lastSystemTimeDouble = 0.0;
	mOldPos[0] = mOldPos[1] = mOldPos[2] = 0.0;
	mCurrentPos[0] = mCurrentPos[1] = mCurrentPos[2] = 0.0;
	mVel[0] = mVel[1] = mVel[2] = 0.0;

	mDeltaTimeDouble = 0.0;
	
	mReset = false;

	//printf( "spring box here!\n" );

	if( FBBox::FBCreate() )
	{
		// Input Nodes
		mStiffnessNode	= AnimationNodeInCreate	( 0, "Stiff",		ANIMATIONNODE_TYPE_NUMBER );
		mDampingNode	= AnimationNodeInCreate	( 1, "Damp",		ANIMATIONNODE_TYPE_NUMBER );
		mFrictionNode	= AnimationNodeInCreate	( 2, "Friction",	ANIMATIONNODE_TYPE_NUMBER );
		mLengthNode		= AnimationNodeInCreate	( 3, "Length",		ANIMATIONNODE_TYPE_NUMBER );
		mMassNode		= AnimationNodeInCreate	( 4, "Mass",		ANIMATIONNODE_TYPE_NUMBER );
		mPosNode		= AnimationNodeInCreate	( 5, "Pos",			ANIMATIONNODE_TYPE_VECTOR );
	
		mTimedt			= AnimationNodeInCreate	( 6, "EvalSteps",	ANIMATIONNODE_TYPE_INTEGER );
		mZeroFrame		= AnimationNodeInCreate	( 7, "ZeroFrame",	ANIMATIONNODE_TYPE_INTEGER );

		mRealTime		= AnimationNodeInCreate	( 8, "RealTime",	ANIMATIONNODE_TYPE_BOOL );

		// Output Node
		mResultNode		= AnimationNodeOutCreate( 9, "Result",	ANIMATIONNODE_TYPE_VECTOR );

		return true;
	}
	return false;
}


/************************************************
 *	Destruction.
 ************************************************/
void CBoxSpring::FBDestroy()
{
	FBBox::FBDestroy();
}


/************************************************
 *	Real-time engine evaluation
 ************************************************/
void VectorSet( float a, float b, float c, FBVector3d &v )
{
	v[0] = a;
	v[1] = b;
	v[2] = c;
}

void VectorAdd( const FBVector3d &a, const FBVector3d &b, FBVector3d &c )
{
	c[0] = a[0] + b[0];
	c[1] = a[1] + b[1];
	c[2] = a[2] + b[2];
}

void VectorSub( const FBVector3d &a, const FBVector3d &b, FBVector3d &c )
{
	c[0] = a[0] - b[0];
	c[1] = a[1] - b[1];
	c[2] = a[2] - b[2];
}

void VectorMul( const FBVector3d &a, const FBVector3d &b, FBVector3d &c )
{
	c[0] = a[0] * b[0];
	c[1] = a[1] * b[1];
	c[2] = a[2] * b[2];
}

void VectorMul( const FBVector3d &a, float scale, FBVector3d &c )
{
	c[0] = a[0] * scale;
	c[1] = a[1] * scale;
	c[2] = a[2] * scale;
}

void VectorDiv( const FBVector3d &a, float scale, FBVector3d &c )
{
	c[0] = a[0] / scale;
	c[1] = a[1] / scale;
	c[2] = a[2] / scale;
}

float VectorLength( const FBVector3d &a )
{
	return sqrt( a[0]*a[0] + a[1]*a[1] + a[2]*a[2] );
}

void VectorNorm( FBVector3d &v )
{
	float len = VectorLength( v );

	if (len > 0)
	{
		v[0] /= len;
		v[1] /= len;
		v[2] /= len;
	}
}

float VectorDot( const FBVector3d &a, FBVector3d &b )
{
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

struct SpringSettings
{
	double length; 
	double friction;
	double mass; 
	double stiffness;
	double damping;

	SpringSettings(const double _length, const double _friction, const double _mass, const double _stiffness, const double _damping)
		: length(_length)
		, friction(_friction)
		, mass(_mass)
		, stiffness(_stiffness)
		, damping(_damping)
	{}
};

void checkDouble(const double value)
{
	int fpclass = _fpclass(value);

	if (fpclass == _FPCLASS_NINF || fpclass == _FPCLASS_QNAN || fpclass == _FPCLASS_SNAN)
		printf ("ERROR -INF\n");
}

bool CalculateSpring( const double dt, const FBVector3d &lV, const FBVector3d &mPos, const FBVector3d &mOldPos, const SpringSettings &settings, FBVector3d &mVel, FBVector3d &outPos )
{
	FBVector3d	lForce, lR;
	FBVector3d	px1, px2, pv1, pv2, dx, dv;			// position & velocity
	double		m, r, value;						// vector length

	// position
	px1 = lV;
	px2 = mOldPos;
	VectorSub( px1, px2, dx );

	// velocity
	VectorSub( lV, mOldPos, pv1 );
	pv2 = mVel;
	VectorSub( pv1, pv2, dv );
		
	// force
	r = settings.length;
	m = VectorLength( dx );
	if (m == 0.0) m = 0.0001;

	lForce = dx;
	VectorNorm( lForce );
	value = (settings.stiffness*(m-r)+settings.damping*(VectorDot(dv, dx)/m));
	//value = lS*(m-r);
	VectorMul( lForce, value, lForce );
	VectorDiv( lForce, settings.mass, lForce );

	// add friction
	lR = mVel;
	if (m > 1.0f)
		VectorMul( lR, -settings.friction, lR );
	else
		VectorMul( lR, -settings.friction, lR );
	VectorAdd( lForce, lR, lForce );

	VectorAdd( mVel, lForce, mVel );			// Change in velocity is added to the velocity.
												// The change is proportinal with the acceleration (force / m) and change in time
	
	VectorMul( mVel, dt, lR );					// Change in position is added to the position.
	VectorAdd( mOldPos, mVel, outPos );				// Change in position is velocity times the change in time
		
/*
	//
	//	damping
	//
	VectorSub( lV, mPos, lR );
	VectorMul( lR, 0.5, lR );
	double dt = (lTime - lastTimeDouble) * 10;
	VectorMul( lR, dt, lR );

	VectorAdd( mPos, lR, mPos );
	lR = mPos;
*/

	return true;
}

bool CBoxSpring::AnimationNodeNotify( FBAnimationNode *pAnimationNode, FBEvaluateInfo *pEvaluateInfo )
{
	double		lS, lD, lM, lFriction, lLength, lTimeDt, lZeroFrame, lRealTime;
	FBVector3d	lInputPos, lR, lVel, lHold, lForce;
	bool		lStatus[9];
	
	FBTime		lEvaluationTime, lLocalTime;

	
	// Read connector in values
	lStatus[0] = mStiffnessNode	->ReadData( &lS, pEvaluateInfo );
	lStatus[1] = mMassNode		->ReadData( &lM, pEvaluateInfo );
	lStatus[2] = mPosNode		->ReadData( &lInputPos[0], pEvaluateInfo );
	lStatus[3] = mFrictionNode	->ReadData( &lFriction, pEvaluateInfo );
	lStatus[4] = mLengthNode	->ReadData( &lLength, pEvaluateInfo );
	lStatus[5] = mDampingNode	->ReadData( &lD, pEvaluateInfo );
	lStatus[6] = mTimedt		->ReadData( &lTimeDt, pEvaluateInfo );
	lStatus[7] = mZeroFrame		->ReadData( &lZeroFrame, pEvaluateInfo );
	lStatus[8] = mRealTime		->ReadData( &lRealTime, pEvaluateInfo );

	// Set default values if no input connection.
	if( !lStatus[0] )
	{
		lS = 1.25;
	}
	if( !lStatus[1] )
	{
		lM = 2.0;
	}
	if( !lStatus[2] )
	{
		VectorSet( 0.0, 0.0, 0.0, lInputPos );
	}
	if( !lStatus[3] )
	{
		lFriction = 0.15;
	}
	if( !lStatus[4] )
	{
		lLength = 0.0;
	}
	if( !lStatus[5] )
	{
		lD = 0.1;
	}
	if (!lStatus[6]) lTimeDt = 30.0;
	if (!lStatus[7]) lZeroFrame = 0.0;
	if (!lStatus[8]) lRealTime = 1.0;

	
	const double resetLimit = 20.0;

	// Get the current evaluation time, indicating if recording.
#ifdef OLD_FBEVALUATE_LOCALTIME
	
	// Get the current evaluation time, indicating if recording.
    if( lRealTime > 0.0 )
    {
		lEvaluationTime = FBSystem().SystemTime;
		mDeltaTimeDouble += lEvaluationTime.GetSecondDouble() - lastSystemTimeDouble;

		if (mDeltaTimeDouble > resetLimit)
		{
			lastSystemTimeDouble = lEvaluationTime.GetSecondDouble();
			mDeltaTimeDouble = 0.0;
			mReset = true;
		}
    }
    else
    {
        lEvaluationTime = pEvaluateInfo->GetLocalStart();

		mDeltaTimeDouble += lEvaluationTime.GetSecondDouble() - lastLocalTimeDouble;

		if (mDeltaTimeDouble > resetLimit || lEvaluationTime.GetFrame() == (int)lZeroFrame)
		{
			lastLocalTimeDouble = lEvaluationTime.GetSecondDouble();
			mDeltaTimeDouble = 0.0;
			mReset = true;
		}
    }
	lLocalTime = pEvaluateInfo->GetLocalStart();

#else

	// Get the current evaluation time, indicating if recording.
    if( lRealTime > 0.0 )
    {
        lEvaluationTime = pEvaluateInfo->GetSystemTime();
		mDeltaTimeDouble += lEvaluationTime.GetSecondDouble() - lastSystemTimeDouble;

		if (mDeltaTimeDouble > resetLimit)
		{
			lastSystemTimeDouble = lEvaluationTime.GetSecondDouble();
			mDeltaTimeDouble = 0.0;
			mReset = true;
		}
    }
    else
    {
        lEvaluationTime = pEvaluateInfo->GetLocalTime();

		mDeltaTimeDouble += lEvaluationTime.GetSecondDouble() - lastLocalTimeDouble;

		if (mDeltaTimeDouble > resetLimit || lEvaluationTime.GetFrame() == (int)lZeroFrame)
		{
			lastLocalTimeDouble = lEvaluationTime.GetSecondDouble();
			mDeltaTimeDouble = 0.0;
			mReset = true;
		}
    }
	lLocalTime = pEvaluateInfo->GetLocalTime();

#endif


	if ( mReset )
	{
		VectorSet( 0.0, 0.0, 0.0, mVel );
		mCurrentPos = lInputPos;	// at start mass pos and input pos is equal

		lR = lInputPos;
		mOldPos = lInputPos;

		mReset = false;
	}
	else
	{	
		
		if (lTimeDt == 0.0) lTimeDt = 30.0;
		else
		if (lTimeDt < 0.0) lTimeDt = abs(lTimeDt);

		if (lTimeDt > 200.0) lTimeDt = 200.0;

		const double dt = 1.0 / lTimeDt;

		SpringSettings settings(lLength, lFriction, lM, lS, lD);
		lHold = lInputPos;

		//printf( "delta time double - %.2f\n", (float) mDeltaTimeDouble );
		while( mDeltaTimeDouble > dt )
		{
			//printf( "vel length - %.2f\n", (float)FBLength( FBTVector(mVel[0], mVel[1], mVel[2], 0.0) ) );
			
			CalculateSpring( dt, lInputPos, mCurrentPos, mOldPos, settings, mVel, mCurrentPos );
			mDeltaTimeDouble -= dt;
		}

		lR = mCurrentPos;
		mOldPos = mCurrentPos;
	}

	
	if( lRealTime > 0.0 )
    {
		lastSystemTimeDouble = lEvaluationTime.GetSecondDouble();
    }
    else
    {
		lastLocalTimeDouble = lEvaluationTime.GetSecondDouble();
    }

	// Write result out to connector.
	mResultNode->WriteData( &lR[0], pEvaluateInfo );
	return true;
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool CBoxSpring::FbxStore( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Store box parameters.
	*/
	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
bool CBoxSpring::FbxRetrieve(FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Retrieve box parameters.
	*/
	return true;
}