
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

/**	\file	filterextrapolate_filter.cxx
*	Definition of a extrapolator filter class.
*	Simple filter function definitions for the FilterExtrapolate
*	class.
*/

//--- Class declarations
#include "filterextrapolate_filter.h"

//--- Missing from headers
#define FBTIME_POS_INFINITE FBTime( kLongLong( K_LONGLONG( 0x7fffffffffffffff )))
#define FBTIME_NEG_INFINITE FBTime( kLongLong( K_LONGLONG( 0x7fffffffffffffff )))

//--- Error tables
static char* ORFilterErrorTable [FilterExtrapolate::eErrorCount] =
{
	"No key",
	"Left unchanged"
};

//--- Registration defines
#define	FILTEREXTRAPOLATE__CLASS		FilterExtrapolate
#define FILTEREXTRAPOLATE__NAME			"Extrapolate"
#define FILTEREXTRAPOLATE__LABEL		"Extrapolate"
#define FILTEREXTRAPOLATE__DESC			"Extrapolate Filter"
#define FILTEREXTRAPOLATE__TYPE			kFBFilterNumber | kFBFilterVector
#define FILTEREXTRAPOLATE__ERRTABLE		ORFilterErrorTable
#define FILTEREXTRAPOLATE__ERRCOUNT		FilterExtrapolate::eErrorCount

//--- FiLMBOX implementation and registration
FBFilterImplementation	(	FILTEREXTRAPOLATE__CLASS			);
FBRegisterFilter		(	FILTEREXTRAPOLATE__CLASS,
							FILTEREXTRAPOLATE__NAME,
							FILTEREXTRAPOLATE__LABEL,
							FILTEREXTRAPOLATE__TYPE,
							FILTEREXTRAPOLATE__ERRTABLE,
							FILTEREXTRAPOLATE__ERRCOUNT,
							FB_DEFAULT_SDK_ICON				);	// Icon filename (default=Open Reality icon)

const char * FBPropertyBaseEnum<kPredictTypeEnum>::mStrings[] = {"Const", "Simple", "Linear", "LSS", 0};

/************************************************
 *	FiLMBOX Creation function.
 ************************************************/
bool FilterExtrapolate::FBCreate()
{
	FBPropertyPublish( this, Area, "Area", NULL, NULL );
	FBPropertyPublish( this, PreEnabled, "Pre Enabled", NULL, NULL );
	FBPropertyPublish( this, PreCount, "Pre Count", NULL, NULL );
	FBPropertyPublish( this, PreType, "Pre Type", NULL, NULL );
	FBPropertyPublish( this, PostEnabled, "Post Enabled", NULL, NULL );
	FBPropertyPublish( this, PostCount, "Post Count", NULL, NULL );
	FBPropertyPublish( this, PostType, "Post Type", NULL, NULL );
	Reset		();

	return true;
}


/************************************************
 *	FiLMBOX Destruction function.
 ************************************************/
void FilterExtrapolate::FBDestroy()
{
}


/************************************************
 *	Filtering functionality (animation node).
 ************************************************/
bool FilterExtrapolate::Apply( HFBAnimationNode pNode, bool pRecursive )
{
	return FBFilter::Apply( pNode, pRecursive );
}

/************************************************
 *	Filtering functionality (FCurve).
 ************************************************/
bool FilterExtrapolate::Apply( HFBFCurve pCurve )
{
	FBTime		lStart = FBTime(Start);
	FBTime		lStop = FBTime(Stop);
	FBTime		lEvalTime;
	FBTime		lEvalStep;
	double		lValue;
	int count = pCurve->Keys.GetCount();

	PredictionBase<1> prediction;

	if (Area <= 0) return false;
	if (count == 0) return false;

	if (lStart == FBTime::MinusInfinity) lStart = pCurve->Keys[0].Time;
	if (lStop == FBTime::Infinity) lStop = pCurve->Keys[count-1].Time;

	if ( lStart > lStop ) return false;

	// pre extrapolate
	if ( (bool) PreEnabled )
	{
		prediction.Clear();

		lEvalStep.SetTime(0,0,0, (int)Area);
		lEvalTime = lStart;
		FBTime lStop2 = lStart;
		lStop2 += lEvalStep;
		if (lStop2 > lStop) lStop2 = lStop;
		lEvalStep.SetTime(0,0,0,1);

		// prepare input data
		while (lEvalTime <= lStop2) {
			lValue = pCurve->Evaluate(lEvalTime);
			prediction.PushValue( &lValue, lEvalTime);
			lEvalTime += lEvalStep;
		}

		prediction.SetType( (kPredictTypeEnum) PreType );
		prediction.Evaluate();

		// extrapolate data
		lEvalTime = lStart;
		for (int i=0; i< (int)PreCount; i++) {
			lEvalTime -= lEvalStep;
			prediction.Predict(&lValue, lEvalTime);

			pCurve->KeyAdd( lEvalTime, lValue );
		}
	}

	// post extrapolate
	if ( (bool) PostEnabled )
	{
		prediction.Clear();

		lEvalStep.SetTime(0,0,0, (int)Area);
		lEvalTime = lStop;
		lEvalTime -= lEvalStep;
		if (lEvalTime < lStart) lEvalTime = lStart;
		lEvalStep.SetTime(0,0,0,1);

		// prepare input data
		while (lEvalTime <= lStop) {
			lValue = pCurve->Evaluate(lEvalTime);
			prediction.PushValue( &lValue, lEvalTime );
			lEvalTime += lEvalStep;
		}

		prediction.SetType( (kPredictTypeEnum) PostType );
		prediction.Evaluate();

		// extrapolate data
		lEvalTime = lStop;
		for (int i=0; i< (int)PostCount; i++) {
			lEvalTime += lEvalStep;
			prediction.Predict(&lValue, lEvalTime );

			pCurve->KeyAdd( lEvalTime, lValue );
		}
	}
	
	return FBFilter::Apply( pCurve );
}


/************************************************
 *	Reset the filter parameters.
 ************************************************/
void FilterExtrapolate::Reset()
{
	FBFilter::Reset();

	Area = 10;
	PreEnabled = false;
	PreCount = 5;
	PreType = kPredictItemSimple;
	PostEnabled = true;
	PostCount = 5;
	PostType = kPredictItemSimple;
}
