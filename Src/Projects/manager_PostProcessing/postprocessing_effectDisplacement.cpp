
// postprocessing_effectDisplacement.cpp
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "postprocessing_effectDisplacement.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "postprocessing_helper.h"

#define SHADER_DISPLACEMENT_NAME				"Displacement"
#define SHADER_DISPLACEMENT_VERTEX				"\\GLSL\\displacement.vsh"
#define SHADER_DISPLACEMENT_FRAGMENT			"\\GLSL\\displacement.fsh"

////////////////////////////////////////////////////////////////////////////////////
// post SSAO

//! a constructor
PostEffectDisplacement::PostEffectDisplacement()
	: PostEffectBase()
{
	for (int i = 0; i < LOCATIONS_COUNT; ++i)
		mLoc.arr[i] = -1;

}

//! a destructor
PostEffectDisplacement::~PostEffectDisplacement()
{
	
}

const char *PostEffectDisplacement::GetName()
{
	return SHADER_DISPLACEMENT_NAME;
}
const char *PostEffectDisplacement::GetVertexFname()
{
	return SHADER_DISPLACEMENT_VERTEX;
}
const char *PostEffectDisplacement::GetFragmentFname()
{
	return SHADER_DISPLACEMENT_FRAGMENT;
}

bool PostEffectDisplacement::PrepUniforms()
{
	bool lSuccess = false;

	if (nullptr != mShader)
	{
		mShader->Bind();

		GLint loc = mShader->findLocation("colorSampler");
		if (loc >= 0)
			glUniform1i(loc, 0);
		loc = mShader->findLocation("depthSampler");
		if (loc >= 0)
			glUniform1i(loc, 2);
		loc = mShader->findLocation("texRandom");
		if (loc >= 0)
			glUniform1i(loc, 4);

		mLoc.upperClip = mShader->findLocation("upperClip");
		mLoc.lowerClip = mShader->findLocation("lowerClip");

		mLoc.iTime = mShader->findLocation("iTime");
		mLoc.iSpeed = mShader->findLocation("iSpeed");
		mLoc.useQuakeEffect = mShader->findLocation("useQuakeEffect");
		mLoc.xDistMag = mShader->findLocation("xDistMag");
		mLoc.yDistMag = mShader->findLocation("yDistMag");

		mLoc.xSineCycles = mShader->findLocation("xSineCycles");
		mLoc.ySineCycles = mShader->findLocation("ySineCycles");

		mShader->UnBind();

		lSuccess = true;
	}

	return lSuccess;
}

bool PostEffectDisplacement::CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera)
{
	bool lSuccess = false;

	FBTime systemTime = (pData->Disp_UsePlayTime) ? mSystem.LocalTime : mSystem.SystemTime;

	const double upperClip = pData->UpperClip;
	const double lowerClip = pData->LowerClip;

	const double timerMult = pData->Disp_Speed;
	const double _timer = 0.01 * timerMult * systemTime.GetSecondDouble();

	const double xdist = pData->Disp_MagnitudeX;
	const double ydist = pData->Disp_MagnitudeY;

	const double xcycles = pData->Disp_SinCyclesX;
	const double ycycles = pData->Disp_SinCyclesY;

	if (nullptr != mShader)
	{
		mShader->Bind();

		// iTime
		
		if (mLoc.upperClip >= 0)
			glUniform1f(mLoc.upperClip, 0.01f * (float)upperClip);

		if (mLoc.lowerClip >= 0)
			glUniform1f(mLoc.lowerClip, 1.0f - 0.01f * (float)lowerClip);

		if (mLoc.iTime >= 0)
			glUniform1f(mLoc.iTime, (float)_timer);

		if (mLoc.iSpeed >= 0)
			glUniform1f(mLoc.iSpeed, (float)timerMult);

		if (mLoc.useQuakeEffect >= 0)
			glUniform1f(mLoc.useQuakeEffect, (pData->UseQuakeWaterEffect) ? 1.0f : 0.0f);

		if (mLoc.xDistMag >= 0)
			glUniform1f(mLoc.xDistMag, 0.0001f * (float)xdist);

		if (mLoc.yDistMag >= 0)
			glUniform1f(mLoc.yDistMag, 0.0001f * (float)ydist);

		if (mLoc.xSineCycles >= 0)
			glUniform1f(mLoc.xSineCycles, (float) xcycles);

		if (mLoc.ySineCycles >= 0)
			glUniform1f(mLoc.ySineCycles, (float)ycycles);

		mShader->UnBind();

		lSuccess = true;
	}

	return lSuccess;
}

void PostEffectDisplacement::Bind()
{
	PostEffectBase::Bind();
}

void PostEffectDisplacement::UnBind()
{
	PostEffectBase::UnBind();
}
