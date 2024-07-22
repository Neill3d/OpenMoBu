
// posteffectdisplacement.cpp
/*
Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectdisplacement.h"
#include "postpersistentdata.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "postprocessing_helper.h"

#define SHADER_DISPLACEMENT_NAME				"Displacement"
#define SHADER_DISPLACEMENT_VERTEX				"\\GLSL\\displacement.vsh"
#define SHADER_DISPLACEMENT_FRAGMENT			"\\GLSL\\displacement.fsh"


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
const char *PostEffectDisplacement::GetVertexFname(const int shaderIndex)
{
	return SHADER_DISPLACEMENT_VERTEX;
}
const char *PostEffectDisplacement::GetFragmentFname(const int shaderIndex)
{
	return SHADER_DISPLACEMENT_FRAGMENT;
}

bool PostEffectDisplacement::PrepUniforms(const int shaderIndex)
{
	bool lSuccess = false;

	GLSLShader* shader = mShaders[shaderIndex];

	if (shader)
	{
		shader->Bind();

		GLint loc = shader->findLocation("colorSampler");
		if (loc >= 0)
			glUniform1i(loc, 0);
		loc = shader->findLocation("depthSampler");
		if (loc >= 0)
			glUniform1i(loc, 2);
		loc = shader->findLocation("texRandom");
		if (loc >= 0)
			glUniform1i(loc, 4);

		PrepareUniformLocations(shader);

		mLoc.iTime = shader->findLocation("iTime");
		mLoc.iSpeed = shader->findLocation("iSpeed");
		mLoc.useQuakeEffect = shader->findLocation("useQuakeEffect");
		mLoc.xDistMag = shader->findLocation("xDistMag");
		mLoc.yDistMag = shader->findLocation("yDistMag");

		mLoc.xSineCycles = shader->findLocation("xSineCycles");
		mLoc.ySineCycles = shader->findLocation("ySineCycles");

		shader->UnBind();

		lSuccess = true;
	}

	return lSuccess;
}

bool PostEffectDisplacement::CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera)
{
	bool lSuccess = false;

	FBTime systemTime = (pData->Disp_UsePlayTime) ? mSystem.LocalTime : mSystem.SystemTime;

	const double timerMult = pData->Disp_Speed;
	const double _timer = 0.01 * timerMult * systemTime.GetSecondDouble();

	const double xdist = pData->Disp_MagnitudeX;
	const double ydist = pData->Disp_MagnitudeY;

	const double xcycles = pData->Disp_SinCyclesX;
	const double ycycles = pData->Disp_SinCyclesY;

	if (GetShaderPtr())
	{
		GetShaderPtr()->Bind();

		UpdateUniforms(pData);

		// iTime
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

		GetShaderPtr()->UnBind();

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
