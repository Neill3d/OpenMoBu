
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

/**	\file	box_poseReader_box.cxx

 This plugin is based on the Comet's PoseReader plugin for Maya

*/

//--- Class declaration
#include "box_poseReader_box.h"
#include <math.h>

//--- Registration defines
#define ORBOXTEMPLATE__CLASS		ORBOXTEMPLATE__CLASSNAME
#define ORBOXTEMPLATE__NAME			ORBOXTEMPLATE__CLASSSTR
#define	ORBOXTEMPLATE__LOCATION		"Neill3d"
#define ORBOXTEMPLATE__LABEL		"Comet's PoseReader"
#define	ORBOXTEMPLATE__DESC			"Pose Space Angle Reader MotionBuilder Plugin"

//--- implementation and registration
FBBoxImplementation	(	ORBOXTEMPLATE__CLASS		);	// Box class name
FBRegisterBox		(	ORBOXTEMPLATE__NAME,			// Unique name to register box.
						ORBOXTEMPLATE__CLASS,			// Box class name
						ORBOXTEMPLATE__LOCATION,		// Box location ('plugins')
						ORBOXTEMPLATE__LABEL,			// Box label (name of box to display)
						ORBOXTEMPLATE__DESC,			// Box long description.
						FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)

#define VRADTODEG 57.295828
#define VDEGTORAD 0.0174533

typedef enum { eInterpLinear, eInterpSmoothStep, eInterpGaussian, eInterpCurve } EINTERPMODE ;


/************************************************
 *	Creation
 ************************************************/
bool Box_PoseReader::FBCreate()
{
	/*
	*	Create the nodes for the box.
	*/

	double min, max;

	if (FBBox::FBCreate() )
	{
		mWorldMatrixLiveIn[0] = AnimationNodeInCreate(0, "Translation live in", ANIMATIONNODE_TYPE_VECTOR );
		mWorldMatrixLiveIn[1] = AnimationNodeInCreate(1, "Rotation live in", ANIMATIONNODE_TYPE_VECTOR );

		mWorldMatrixPoseIn[0] = AnimationNodeInCreate(2, "Translation pose in", ANIMATIONNODE_TYPE_VECTOR );
		mWorldMatrixPoseIn[1] = AnimationNodeInCreate(3, "Rotation pose in", ANIMATIONNODE_TYPE_VECTOR );

		min = 0.0;
		max = 2.0;
		mReadAxis = AnimationNodeInCreate( 4, "readAxis", ANIMATIONNODE_TYPE_INTEGER, false, &min, &max );
		
		min = 0.0;
		max = 2.0;
		mInterpMode = AnimationNodeInCreate( 5, "interpMode", ANIMATIONNODE_TYPE_INTEGER, false, &min, &max );
		
		min = 0.0;
		max = 1.0;
		mAllowRotate = AnimationNodeInCreate( 6, "allowRotate", ANIMATIONNODE_TYPE_NUMBER, false, &min, &max );
		min = 0.0;
		max = 180.0;
		mMinAngle = AnimationNodeInCreate( 7, "minAngle", ANIMATIONNODE_TYPE_NUMBER, false, &min, &max );
		mMaxAngle = AnimationNodeInCreate( 8, "maxAngle", ANIMATIONNODE_TYPE_NUMBER, false, &min, &max );

		min = 0.0;
		max = 1.0;
		mAllowTwist = AnimationNodeInCreate( 9, "allowTwist", ANIMATIONNODE_TYPE_NUMBER, false, &min, &max );
		min = 0.0;
		max = 180.0;
		mMinTwist = AnimationNodeInCreate( 10, "minTwist", ANIMATIONNODE_TYPE_NUMBER, false, &min, &max );
		mMaxTwist = AnimationNodeInCreate( 11, "maxTwist", ANIMATIONNODE_TYPE_NUMBER, false, &min, &max );

		min = 0.0;
		max = 1.0;
		mAllowTranslate = AnimationNodeInCreate( 9, "allowTranslate", ANIMATIONNODE_TYPE_NUMBER, false, &min, &max );
		min = 0.0;
		max = 180.0;
		mMinTranslate = AnimationNodeInCreate( 10, "minTranslate", ANIMATIONNODE_TYPE_NUMBER, false, &min );
		mMaxTranslate = AnimationNodeInCreate( 11, "maxTranslate", ANIMATIONNODE_TYPE_NUMBER, false, &min );


		mOutWeight = AnimationNodeOutCreate( 12, "outWeight", ANIMATIONNODE_TYPE_NUMBER );

		return true;
	}

	return false;
}


/************************************************
 *	Destruction.
 ************************************************/
void Box_PoseReader::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
	FBBox::FBDestroy();
}


/************************************************
 *	Real-time engine evaluation
 ************************************************/
void VectorNormalize(FBTVector &vector)
{
	double len = FBLength(vector);
	if (len != 0.0)
	{
		FBMult( vector, vector, 1.0/len);
	}
}

// --------------------------------------------------------------------------


/*
 * poseReader::calcWtFromAngle() - Calcs weight based on a 0-180 angle.
 */
double calcWtFromAngle(const double& dAngle, const double& dMinAngle, const double& dMaxAngle)
{
	double dWt ;

	if (dAngle >= dMaxAngle)
		dWt = 0.0 ;
	else if (dAngle <= dMinAngle)
		dWt = 1.0 ;
	else
		{
		double dDelta = dMaxAngle - dMinAngle ;
		if (dDelta > 0)
			dWt = 1.0 - ((dAngle - dMinAngle) / dDelta) ;
		else
			dWt = 0.0 ;
		}

	return dWt ;
}

// ---------------------------------------------------------------------------


/*
 * poseReader::smoothStep() - Take a value from 0-1 and instead of having it be linear,
 *			make it ease out and then in from the 0 and 1 locations.
 */
double smoothStep(const double &dVal)
{
	// x^2*(3-2x)
	double dRet = dVal * dVal * (3.0 - (2.0 * dVal) );
	return dRet ;
}

// ---------------------------------------------------------------------------

/*
 * poseReader::smoothGaussian() - Take a value from 0-1 and instead of having it be linear,
 *			make it ease out and then in from the 0 and 1 locations.  This has more of a longer
 *			ease than a smoothstep and so a faster falloff in the middle.
 */
double smoothGaussian(const double &dVal)
{
	// 1.0 - exp( (-1 * x^2 ) / (2*sigma^2) )
	
	double dSigma = 1.0 ;		// Must be >0.   As drops to zero, higher value one lasts longer.

	double dRet = (1.0 - exp( -1.0 * (dVal*dVal) * 10.0  / (2.0 * dSigma*dSigma)  ) );	// RBF
	return dRet ;
}

bool Box_PoseReader::AnimationNodeNotify( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status of the connection
	*/
	FBMatrix	matLive, invmatLive, matPose, invmatPose;

	FBTVector	tLive, tPose;
	FBRVector	rLive, rPose;

	FBSVector	s(1.0, 1.0, 1.0);

	double		value;
    int			lReadAxis;
	int			lInterpMode;
	double		lAllowRotate;
	double		lMinAngle, lMaxAngle;
	double		lAllowTwist;
	double		lMinTwist, lMaxTwist;
	double		lAllowTranslate;
	double		lMinTranslate, lMaxTranslate; 

	// Read connector in values (set default values if no connection)
	mWorldMatrixLiveIn[0]->ReadData( tLive, pEvaluateInfo );
	mWorldMatrixLiveIn[1]->ReadData( rLive, pEvaluateInfo );
	
	FBTRSToMatrix(matLive, tLive, rLive, s );
#ifdef OLD_FBMATH
	FBMatrixInverse( invmatLive, matLive );
#else
	invmatLive = matLive;
	invmatLive.Inverse();
#endif
	mWorldMatrixPoseIn[0]->ReadData( tPose, pEvaluateInfo );
	mWorldMatrixPoseIn[1]->ReadData( rPose, pEvaluateInfo );

	FBTRSToMatrix(matPose, tPose, rPose, s );
#ifdef OLD_FBMATH
	FBMatrixInverse( invmatPose, matPose );
#else
	invmatPose = matPose;
	invmatPose.Inverse();
#endif
	if (mReadAxis->ReadData( &value, pEvaluateInfo ) ) lReadAxis = (int) value;
	else lReadAxis = 1;
	if (mInterpMode->ReadData( &value, pEvaluateInfo ) ) lInterpMode = (int) value;
	else lInterpMode = 1;


	if (!mAllowRotate->ReadData( &lAllowRotate, pEvaluateInfo ) ) lAllowRotate = 0.0;
	if (!mMinAngle->ReadData( &lMinAngle, pEvaluateInfo ) ) lMinAngle = 0.0;
	if (!mMaxAngle->ReadData( &lMaxAngle, pEvaluateInfo ) ) lMaxAngle = 180.0;
	
	if (lMinAngle >= lMaxAngle)
			lMinAngle = lMaxAngle-0.001 ;

	if (!mAllowTwist->ReadData( &lAllowTwist, pEvaluateInfo ) ) lAllowTwist = 1.0;
	if (!mMinTwist->ReadData( &lMinTwist, pEvaluateInfo ) ) lMinTwist = 0.0;
	if (!mMaxTwist->ReadData( &lMaxTwist, pEvaluateInfo ) ) lMaxTwist = 180.0;

	if (lMinTwist >= lMaxTwist)
			lMinTwist = lMaxTwist-0.001 ;

	if (!mAllowTranslate->ReadData( &lAllowTranslate, pEvaluateInfo ) ) lAllowTranslate = 1.0;
	if (!mMinTranslate->ReadData( &lMinTranslate, pEvaluateInfo ) ) lMinTranslate = 0.0;
	if (!mMaxTranslate->ReadData( &lMaxTranslate, pEvaluateInfo ) ) lMaxTranslate = 1.0;

	if (lMinTranslate >= lMaxTranslate)
			lMinTranslate = lMaxTranslate-0.001 ;

	// ==========================================
	// Now do real calc

	double dWt = 1.0 ;			// output weight

	double dDot = -1.0 ;
	double dAngle = 180.0;
	double dWtTwist = 1.0 ;
	double dWtTranslate = 1.0 ;

	FBMatrix matRelPose;
	FBMatrixMult( matRelPose, invmatPose, matLive );	// where is joint relative to pose node?
	//matRelPose = invmatPose * matLive;

	FBVector4d xAxis(1.0, 0.0, 0.0, 1.0);
	FBVector4d yAxis(0.0, 1.0, 0.0, 1.0);
	FBVector4d zAxis(0.0, 0.0, 1.0, 1.0);

	if (lReadAxis == 0)		// X-Axis
	{
		// Get X axis of live joint in relative space of the pose node.
		FBVector4d vAxis;
		
		FBVectorMatrixMult( vAxis, matRelPose, xAxis );
		VectorNormalize( vAxis );

		dDot = FBDot( vAxis, xAxis ); // Now in pose space, compare it's X axis to the joint
		
		if (dDot >= 1.0)		// Have to do this to handle precision errors in acos returning -1.#IND
			dAngle = 0.0 ;
		else if (dDot <= -1.0)
			dAngle = 180.0 ;
		else
			dAngle = acos(dDot) * VRADTODEG;	// Now what is actual angle in degress?

		dWt = calcWtFromAngle(dAngle, lMinAngle, lMaxAngle) ;
		dWt = ((1.0- lAllowRotate) * dWt) + (lAllowRotate * 1.0) ;

		if ( lAllowTwist != 1.0)
		{
			FBTVector vAxisTwist;
			FBVectorMatrixMult( vAxisTwist, matRelPose, yAxis );
			VectorNormalize( vAxisTwist );

			double dDotTwist = FBDot( vAxisTwist, yAxis ); 
			double dAngleTwist ;			
			if (dDotTwist >= 1.0)
				dAngleTwist = 0.0 ;
			else if (dDotTwist <= -1.0)
				dAngleTwist = 180.0 ;
			else
				dAngleTwist = acos(dDotTwist) * VRADTODEG;	// Now what is actual angle in degress?
			
			dWtTwist = calcWtFromAngle(dAngleTwist, lMinTwist, lMaxTwist) ;

			dWt = ((1.0-lAllowTwist)*(dWt * dWtTwist)) + (lAllowTwist * dWt) ;
		}
	}
	else if (lReadAxis == 1)		// Y-Axis
	{
		// Get X axis of live joint in relative space of the pose node.
		FBTVector vAxis;
		FBVectorMatrixMult( vAxis, matRelPose, yAxis );
		VectorNormalize( vAxis );

		dDot = FBDot( vAxis, yAxis ); // Now in pose space, compare it's X axis to the joint
		
		if (dDot >= 1.0)		// Have to do this to handle precision errors in acos returning -1.#IND
			dAngle = 0.0 ;
		else if (dDot <= -1.0)
			dAngle = 180.0 ;
		else
			dAngle = acos(dDot) * VRADTODEG;	// Now what is actual angle in degress?

		dWt = calcWtFromAngle(dAngle, lMinAngle, lMaxAngle) ;
		dWt = ((1.0-lAllowRotate) * dWt) + (lAllowRotate * 1.0) ;

		if ( lAllowTwist != 1.0)
		{
			FBTVector vAxisTwist;
			FBVectorMatrixMult( vAxisTwist, matRelPose, xAxis );
			VectorNormalize( vAxisTwist );

			double dDotTwist = FBDot( vAxisTwist, xAxis ); 
			double dAngleTwist ;			
			if (dDotTwist >= 1.0)
				dAngleTwist = 0.0 ;
			else if (dDotTwist <= -1.0)
				dAngleTwist = 180.0 ;
			else
				dAngleTwist = acos(dDotTwist) * VRADTODEG;	// Now what is actual angle in degress?
			
			dWtTwist = calcWtFromAngle(dAngleTwist, lMinTwist, lMaxTwist) ;

			dWt = ((1.0-lAllowTwist)*(dWt * dWtTwist)) + (lAllowTwist * dWt) ;
		}
	}
	else if (lReadAxis == 2)		// Z-Axis
	{
		// Get X axis of live joint in relative space of the pose node.
		FBTVector vAxis;
		FBVectorMatrixMult( vAxis, matRelPose, zAxis );
		VectorNormalize( vAxis );

		dDot = FBDot( vAxis, zAxis ); // Now in pose space, compare it's X axis to the joint
		
		if (dDot >= 1.0)		// Have to do this to handle precision errors in acos returning -1.#IND
			dAngle = 0.0 ;
		else if (dDot <= -1.0)
			dAngle = 180.0 ;
		else
			dAngle = acos(dDot) * VRADTODEG;	// Now what is actual angle in degress?

		dWt = calcWtFromAngle(dAngle, lMinAngle, lMaxAngle) ;
		dWt = ((1.0-lAllowRotate) * dWt) + (lAllowRotate * 1.0) ;

		if ( lAllowTwist != 1.0)
		{
			FBTVector vAxisTwist;
			FBVectorMatrixMult( vAxisTwist, matRelPose, xAxis );
			VectorNormalize( vAxisTwist );

			double dDotTwist = FBDot( vAxisTwist, xAxis ); 
			double dAngleTwist ;			
			if (dDotTwist >= 1.0)
				dAngleTwist = 0.0 ;
			else if (dDotTwist <= -1.0)
				dAngleTwist = 180.0 ;
			else
				dAngleTwist = acos(dDotTwist) * VRADTODEG;	// Now what is actual angle in degress?
			
			dWtTwist = calcWtFromAngle(dAngleTwist, lMinTwist, lMaxTwist) ;

			dWt = ((1.0-lAllowTwist)*(dWt * dWtTwist)) + (lAllowTwist * dWt) ;
		}
	}

	if (lAllowTranslate != 1.0)
	{
		FBTVector vTrans( matRelPose[3*4], matRelPose[3*4+1], matRelPose[3*4+2], 1.0 ) ;
		double dDist = FBLength( vTrans );
		if (dDist <= lMinTranslate)
			dWtTranslate = 1.0 ;
		else if (dDist >= lMaxTranslate)
			dWtTranslate = 0.0 ;
		else
		{
			dWtTranslate = 1.0 - ((dDist - lMinTranslate) / (lMaxTranslate - lMinTranslate)) ;
		}

		dWt = ((1.0-lAllowTranslate)*(dWt * dWtTranslate)) + (lAllowTranslate * dWt) ;
	}

	// Adjust actual output a bit if desired.
	if (lInterpMode == eInterpLinear)
		;
	else if (lInterpMode == eInterpSmoothStep)
		dWt = smoothStep(dWt) ;
	else if (lInterpMode == eInterpGaussian)
		dWt = smoothGaussian(dWt) ;

	// Write result out to connector.
	mOutWeight->WriteData( &dWt, pEvaluateInfo );
	return true;
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool Box_PoseReader::FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Store box parameters.
	*/
	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
bool Box_PoseReader::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Retrieve box parameters.
	*/
	return true;
}
