
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


//! a constructor
EffectShaderDisplacement::EffectShaderDisplacement()
	: PostEffectBufferShader()
{
	for (int i = 0; i < LOCATIONS_COUNT; ++i)
		mLoc.arr[i] = -1;

	memset(&mData, 0, sizeof(ShaderData));
}

//! a destructor
EffectShaderDisplacement::~EffectShaderDisplacement()
{
	
}

const char * EffectShaderDisplacement::GetName() const
{
	return SHADER_NAME;
}
const char * EffectShaderDisplacement::GetVertexFname(const int shaderIndex) const
{
	return SHADER_VERTEX;
}
const char * EffectShaderDisplacement::GetFragmentFname(const int shaderIndex) const
{
	return SHADER_FRAGMENT;
}

bool EffectShaderDisplacement::OnPrepareUniforms(const int shaderIndex)
{
	if (GLSLShaderProgram* shader = mShaders[shaderIndex].get())
	{
		shader->Bind();

		const GLint loc = shader->findLocation("colorSampler");
		if (loc >= 0)
			glUniform1i(loc, CommonEffectUniforms::GetColorSamplerSlot());
		
		PrepareCommonLocations(shader);

		mLoc.iTime = shader->findLocation("iTime");
		mLoc.iSpeed = shader->findLocation("iSpeed");
		mLoc.useQuakeEffect = shader->findLocation("useQuakeEffect");
		mLoc.xDistMag = shader->findLocation("xDistMag");
		mLoc.yDistMag = shader->findLocation("yDistMag");

		mLoc.xSineCycles = shader->findLocation("xSineCycles");
		mLoc.ySineCycles = shader->findLocation("ySineCycles");

		shader->UnBind();

		return true;
	}

	return false;
}

bool EffectShaderDisplacement::OnCollectUI(const IPostEffectContext* effectContext, int maskIndex)
{
	CollectCommonData(nullptr, ENABLE_MASKING_PROPERTY_NAME);

	double time = (pData->Disp_UsePlayTime) ? effectContext->GetLocalTime() : effectContext->GetSystemTime();

	const double timerMult = pData->Disp_Speed;
	const double _timer = 0.01 * timerMult * time;

	const double xdist = pData->Disp_MagnitudeX;
	const double ydist = pData->Disp_MagnitudeY;

	const double xcycles = pData->Disp_SinCyclesX;
	const double ycycles = pData->Disp_SinCyclesY;

	mData.iTime = static_cast<float>(_timer);
	mData.iSpeed = static_cast<float>(timerMult);
	mData.useQuakeEffect = (pData->UseQuakeWaterEffect) ? 1.0f : 0.0f;
	mData.xDistMag = 0.0001f * static_cast<float>(xdist);
	mData.yDistMag = 0.0001f * static_cast<float>(ydist);
	mData.xSineCycles = static_cast<float>(xcycles);
	mData.ySineCycles = static_cast<float>(ycycles);

	return true;
}

void EffectShaderDisplacement::OnUploadUniforms(PostEffectBuffers* buffers, FrameBuffer* dstBuffer, int colorAttachment, const GLuint inputTextureId, int w, int h, bool generateMips)
{
	UploadCommonData();

	// iTime
	if (mLoc.iTime >= 0)
		glUniform1f(mLoc.iTime, mData.iTime);

	if (mLoc.iSpeed >= 0)
		glUniform1f(mLoc.iSpeed, mData.iSpeed);

	if (mLoc.useQuakeEffect >= 0)
		glUniform1f(mLoc.useQuakeEffect, mData.useQuakeEffect);

	if (mLoc.xDistMag >= 0)
		glUniform1f(mLoc.xDistMag, mData.xDistMag);

	if (mLoc.yDistMag >= 0)
		glUniform1f(mLoc.yDistMag, mData.yDistMag);

	if (mLoc.xSineCycles >= 0)
		glUniform1f(mLoc.xSineCycles, mData.xSineCycles);

	if (mLoc.ySineCycles >= 0)
		glUniform1f(mLoc.ySineCycles, mData.ySineCycles);
}

