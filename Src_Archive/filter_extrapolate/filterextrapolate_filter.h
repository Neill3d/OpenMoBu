
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

#ifndef __ORFILTER_RESAMPLE_FILTER_H__
#define __ORFILTER_RESAMPLE_FILTER_H__

/**	\file	filterextrapolate_filter.h
*	Declaration of a extrapolator filter class.
*	Simple filter class declaration (FBResampleFilter).
*/

//--- SDK include
#include <fbsdk/fbsdk.h>
#include "prediction.h"

//! A simple filter class.
class FilterExtrapolate : public FBFilter
{
	//--- FiLMBOX declaration
	FBFilterDeclare( FilterExtrapolate, FBFilter );

public:
	/** Error messages
	*
	*/
	enum ORFilterError
	{
		eNoKey,
		eLeftUnchanged,
		eErrorCount
	};

	//--- Creation & Destruction
	virtual bool FBCreate();					//!< FiLMBOX Constructor.
	virtual void FBDestroy();					//!< FiLMBOX Destructor.

	//--- Apply filter.
	virtual bool Apply( FBAnimationNode *pNode, bool pRecursive = true ) override;
	virtual bool Apply( FBFCurve *pCurve ) override;

	//--- Reset Filter values
	virtual void Reset();

public:

	FBPropertyInt							Area;	// number frames that will be used as input data for extrapolator
	FBPropertyBool							PostEnabled;
	FBPropertyInt							PostCount;
	FBPropertyBaseEnum<kPredictTypeEnum>	PostType;	// type of extrapolator (const, linear, lss)
	FBPropertyBool							PreEnabled;
	FBPropertyInt							PreCount;
	FBPropertyBaseEnum<kPredictTypeEnum>	PreType;	// type of extrapolator (const, linear, lss)
};

#endif	/* __ORFILTER_RESAMPLE_FILTER_H__ */
