
#pragma once


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

#include "math3d.h"

/*!
 * Prediction
 *
 * predict trajectory of a motion point.
*/
enum kPredictTypeEnum {
	kPredictItemConst,	// output value = input value (without any evaluating)
	kPredictItemSimple,
	kPredictItemLinear, // y = kx + b
	kPredictItemLSS		// lin function
};


template <int Length>
class	PredictFrame	
{
public:
	double		mFrameTime;
	double		mValue[Length];
	//! a constructor
	PredictFrame(double frametime, double	*value)
		: mFrameTime(frametime)
	{
		memcpy(&mValue[0], value, sizeof(double)*Length);	// copy L of double
	}
};

/*
	Prediction 3 DOF

	predict 3 double values (like position)
*/
template <int Length>
class	PredictionBase
{
public:
	
	//! a constructor
	PredictionBase()
	{
		mType = kPredictItemConst;
		for (int i=0; i<Length; i++)
			c[i] = nullptr;
		mLastTime = FBTime::MinusInfinity;
		mPenultimate = FBTime::MinusInfinity;
	}
	//! a destructor
	~PredictionBase()
	{
		FreeC();
	}

	/*
		get/set prediction type (kPredictItemSimple, kPredictItemLinear, etc.)
	*/
	kPredictTypeEnum	GetType() {
		return mType;
	}
	void SetType(kPredictTypeEnum	type) {
		mType = type;
	}
	

	/* Clear
		clear input value for prediction
	*/
	void Clear() {
		frames.Clear();
	}

	/* PushValue
		put a value on a top of a stack
		\param frame - x parameter
		\param value - y parameter 
	*/
	void	PushValue(double	*value, FBTime time=FBTime::Infinity) {
		double frameTime = 0.0;
		if (time == FBTime::Infinity) {
			time = FBSystem().LocalTime;
			frameTime = time.GetSecondDouble();
		}
		else	frameTime = time.GetSecondDouble();

		frames.Add( PredictFrame<Length>(frameTime, value) );
	}

	/* F 
		function to evaluate simple lSS
	*/
	static double F(int m, double x, const double *c) {
		double y = 0.0;

		for(int i=0; i<m; i++)
			y = y + c[i]* pow(x, i);
		return y;
	}

	/* Evaluate
		use this function to calculate prediction result value
	*/
	void	Evaluate();

	/* Predict
		predict values from input pushed one
		\frame - x paramater to predict
		\result - pointer to storage array
	*/
	void Predict(double *result, FBTime time = FBTime::Infinity);

private:
	kPredictTypeEnum						mType;	// prediction type
	FBArrayTemplate< PredictFrame<Length> >	frames;

	// auto calculate direction
	FBTime					mLastTime;		//!> calculate direction (forward or backward)
	FBTime					mPenultimate;	//!> punultimate frame time

	double	*c[Length];	// polyfit coeff.

	void	FreeC() {
		for (int i=0; i<Length; i++)
			ARRAYFREE(c[i]);
	}

	void EvaluateLinear();
	void EvaluateLSS();

	void PredictConst(double frametime, double *result);
	void PredictSimple(double frametime, double *result);
	void PredictLinear(double frametime, double *result);
	void PredictLSS(double frametime, double *result);
};


typedef	PredictionBase<3>	Prediction3DOF;	// predict only pos
typedef PredictionBase<6>	Prediction6DOF;	// pos & rot

void AnimationNodeToPrediction( HFBAnimationNode translate, HFBAnimationNode rotate, Prediction6DOF &prediction );


template <int Length>
void PredictionBase<Length>::Evaluate() 
{
	switch(mType)
	{
	case kPredictItemLinear:
		EvaluateLinear();
		break;
	case kPredictItemLSS:
		EvaluateLSS();
		break;
	}
}

template <int Length>
void PredictionBase<Length>::EvaluateLinear() {
	if (frames.GetCount() < 2)	return;

	FreeC();

	// build line function
	const unsigned	maxn = frames.GetCount();
	const unsigned	maxm = 2;		// polynon 1, y = kx + b

	double	*x	= new double[maxn];
	double	*y	= new double[maxn];

	for (int i=0; i<Length; i++)
		c[i] = new double[maxm];

	for (int i=0; i<Length; i++)
	{
		for (unsigned j=0; j<maxn; j++)
		{
			x[j] = frames[j].mFrameTime;
			y[j] = frames[j].mValue[i];
		}

		apprLSS_H( maxn, x, y, maxm, c[i] );
	}
	FREE_ARRAY(x);
	FREE_ARRAY(y);
}

template <int Length>
void PredictionBase<Length>::EvaluateLSS() {
	if (frames.GetCount() < 2)	return;

	FreeC();

	// LSS
	const unsigned	maxn = frames.GetCount();
	const unsigned	maxm = 4;		// polynon

	double	*x	= new double[maxn];
	double	*y	= new double[maxn];
	// evaluation vars
	double	*a	= new double[maxm*maxm];
	double	*b	= new double[maxm];

	for (int i=0; i<Length; i++)
		c[i] = new double[maxm];

	for (int i=0; i<Length; i++)
	{
		for (unsigned j=0; j<maxn; j++)
		{
			x[j] = frames[j].mFrameTime;
			y[j] = frames[j].mValue[i];
		}

		linBuild(maxn, maxm, x, y, a, b);
		linSolve(maxm, a,b, c[i]);
	}
	FREE_ARRAY(x);
	FREE_ARRAY(y);
	FREE_ARRAY(a);
	FREE_ARRAY(b);
}

template<int Length>
void PredictionBase<Length>::Predict(double *result, FBTime time = FBTime::Infinity)
{
	double frameTime = 0.0;
	if (time == FBTime::Infinity) {
		time = FBSystem().LocalTime;
		frameTime = time.GetSecondDouble();
	}
	else	frameTime = time.GetSecondDouble();

	switch(mType)
	{
	case kPredictItemConst:
		PredictConst( frameTime, result );
		break;
	case kPredictItemSimple:
		PredictSimple( frameTime, result );
		break;
	case kPredictItemLinear:
		PredictLinear( frameTime, result );
		break;
	case kPredictItemLSS:
		PredictLSS( frameTime, result );
		break;
	}
}

template <int Length>
void PredictionBase<Length>::PredictConst(double frametime, double *result) 
{
	int count = frames.GetCount();
	if ( !count ) return;
	if (count < 2) {
		memcpy(result, frames[0].mValue, sizeof(double)*Length);
		return;
	}

	if (frametime >= frames[count-1].mFrameTime) 
		memcpy(result, frames[count-1].mValue, sizeof(double)*Length);
	else if (frametime <= frames[0].mFrameTime) 
		memcpy(result, frames[0].mValue, sizeof(double)*Length);
}

template <int Length>
void PredictionBase<Length>::PredictSimple(double frametime, double *result) 
{
	int count = frames.GetCount();
	if ( !count ) return;
	if (count < 2) {
		memcpy(result, frames[0].mValue, sizeof(double)*Length);
		return;
	}

	if (frametime >= frames[count-1].mFrameTime) 
	{
		double t = (frametime - frames[count-1].mFrameTime) / 
							(frames[count-1].mFrameTime - frames[count-2].mFrameTime);

		for (int i=0; i<Length; i++)
			result[i] = frames[count-1].mValue[i] + (frames[count-1].mValue[i] - frames[count-2].mValue[i]) * t;
	}
	else if (frametime <= frames[0].mFrameTime)
	{
		double t = (frametime - frames[0].mFrameTime) / 
							(frames[0].mFrameTime - frames[1].mFrameTime);

		for (int i=0; i<Length; i++)
			result[i] = frames[0].mValue[i] + (frames[0].mValue[i] - frames[1].mValue[i]) * t;
	}
}

template <int Length>
void PredictionBase<Length>::PredictLinear(double frametime, double *result) 
{
	if ( !frames.GetCount() ) return;
	if (frames.GetCount() < 2) {
		memcpy(result, frames[0].mValue, sizeof(double)*Length);
		return;
	}

	const unsigned	maxm = 2;		// polynon 1, y = kx + b
	for (int i=0; i<Length; i++)
	{
		// 11 is time value (on x axis)
		result[i] = F(maxm, frametime, c[i]);
	}
}

template <int Length>
void PredictionBase<Length>::PredictLSS(double frametime, double *result) 
{		
	if ( !frames.GetCount() ) return;
	if (frames.GetCount() < 2) {
		memcpy(result, frames[0].mValue, sizeof(double)*Length);
		return;
	}

	const unsigned	maxm = 4;		// polynon 1, y = kx + b
	for (int i=0; i<Length; i++)
	{
		// 11 is time value (on x axis)
		result[i] = F(maxm, frametime, c[i]);
	}
}