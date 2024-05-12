
// postprocessing_effectSSAO.cpp
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "postprocessing_effectLensFlare.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "postprocessing_helper.h"

#define SHADER_LENSFLARE_NAME			"Lens Flare"
#define SHADER_LENSFLARE_VERTEX			"\\GLSL\\simple.vsh"
#define SHADER_LENSFLARE_FRAGMENT		"\\GLSL\\lensFlare.fsh" 
#define SHADER_LENSFLARE_BUBBLE_FRAGMENT			"\\GLSL\\lensFlareBubble.fsh"
#define SHADER_LENSFLARE_ANAMORPHIC_FRAGMENT		"\\GLSL\\lensFlareAnamorphic.fsh"


////////////////////////////////////////////////////////////////////////////////////
// post lens flare

//! a constructor
PostEffectLensFlare::PostEffectLensFlare()
	: PostEffectBase()
{
	for (int nShader = 0; nShader < NUMBER_OF_SHADERS; ++nShader)
	{
		subShaders[nShader].Init();
	}
}

void PostEffectLensFlare::SubShader::Init()
{
	m_NumberOfPasses = 1;
	for (int i = 0; i < LOCATIONS_COUNT; ++i)
		mLocations[i] = -1;
}

//! a destructor
PostEffectLensFlare::~PostEffectLensFlare()
{

}

const char *PostEffectLensFlare::GetName()
{
	return SHADER_LENSFLARE_NAME;
}
const char *PostEffectLensFlare::GetVertexFname(const int)
{
	return SHADER_LENSFLARE_VERTEX;
}
const char *PostEffectLensFlare::GetFragmentFname(const int shaderIndex)
{
	switch (shaderIndex)
	{
	case 1: return SHADER_LENSFLARE_BUBBLE_FRAGMENT;
	case 2: return SHADER_LENSFLARE_ANAMORPHIC_FRAGMENT;
	default: return SHADER_LENSFLARE_FRAGMENT;
	}
}

bool PostEffectLensFlare::PrepUniforms(const int shaderIndex)
{
	GLSLShader* mShader = mShaders[shaderIndex];
	return subShaders[shaderIndex].PrepUniforms(mShader);	
}

bool PostEffectLensFlare::SubShader::PrepUniforms(GLSLShader* mShader)
{
	bool lSuccess = false;

	if (nullptr != mShader)
	{
		mShader->Bind();

		GLint loc = mShader->findLocation("sampler0");
		if (loc >= 0)
			glUniform1i(loc, 0);
		//Louis
		seed = mShader->findLocation("flareSeed");

		upperClip = mShader->findLocation("upperClip");
		lowerClip = mShader->findLocation("lowerClip");

		amount = mShader->findLocation("amount");

		textureWidth = mShader->findLocation("textureWidth");
		textureHeight = mShader->findLocation("textureHeight");

		timer = mShader->findLocation("iTime");

		light_pos = mShader->findLocation("light_pos");

		tint = mShader->findLocation("tint");
		inner = mShader->findLocation("inner");
		outer = mShader->findLocation("outer");

		fadeToBorders = mShader->findLocation("fadeToBorders");
		borderWidth = mShader->findLocation("borderWidth");
		feather = mShader->findLocation("feather");

		mShader->UnBind();

		lSuccess = true;
	}

	return lSuccess;
}

bool PostEffectLensFlare::CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera)
{
	mCurrentShader = pData->FlareType.AsInt();

	if (mCurrentShader < 0 || mCurrentShader >= NUMBER_OF_SHADERS)
	{
		mCurrentShader = 0;
		const EFlareType newFlareType{ EFlareType::flare1 };
		pData->FlareType.SetData((void*) &newFlareType);
	}
	
	return subShaders[mCurrentShader].CollectUIValues(mCurrentShader, GetShaderPtr(), pData, w, h, pCamera);
}

bool PostEffectLensFlare::SubShader::CollectUIValues(const int shaderIndex, GLSLShader* mShader, PostPersistentData *pData, int w, int h, FBCamera *pCamera)
{
	m_NumberOfPasses = 1;
	bool lSuccess = false;
	
	
	double seedValue = 30.0;
	if (shaderIndex > 0 && pData->IsLazyLoadReady())
	{
		pData->FlareSeed.GetData(&seedValue, sizeof(double));
	}
	
	const double _upperClip = pData->UpperClip;
	const double _lowerClip = pData->LowerClip;

	FBTime systemTime = (pData->FlareUsePlayTime) ? mSystem.LocalTime : mSystem.SystemTime;

	double _amount = pData->FlareAmount;

	FBColor _tint = pData->FlareTint;
	double _inner = pData->FlareInner;
	double _outer = pData->FlareOuter;

	double timerMult = pData->FlareTimeSpeed;
	double _timer = 0.01 * timerMult * systemTime.GetSecondDouble();

	double _pos[3] = { 0.01 * pData->FlarePosX, 0.01 * pData->FlarePosY, 1.0 };

	const float _fadeToBorders = (pData->FlareFadeToBorders) ? 1.0f : 0.0f;
	double _borderWidth = pData->FlareBorderWidth;
	double _feather = pData->FlareBorderFeather;

	m_DepthAttenuation = (pData->FlareDepthAttenuation) ? 1.0f : 0.0f;

	// TODO: track light position in screen space
	if (pData->UseFlareLightObject && pData->FlareLight.GetCount() > 0)
	{
		m_NumberOfPasses = pData->FlareLight.GetCount();
		m_LightPositions.resize(m_NumberOfPasses);
		m_LightColors.resize(m_NumberOfPasses);

		for (int i = 0; i < m_NumberOfPasses; ++i)
		{
			FBLight *pLight = static_cast<FBLight*>(pData->FlareLight.GetAt(i));

			FBVector3d v;
			pLight->GetVector(v);

			FBMatrix mvp;
			pCamera->GetCameraMatrix(mvp, kFBModelViewProj);

			FBVector4d v4;
			FBVectorMatrixMult(v4, mvp, FBVector4d(v[0], v[1], v[2], 1.0));

			v4[0] = w * 0.5 * (v4[0] + 1.0);
			v4[1] = h * 0.5 * (v4[1] + 1.0);


			_pos[0] = v4[0] / w;
			_pos[1] = v4[1] / h;
			_pos[2] = v4[2];

			m_LightPositions[i].Set(_pos);
			const FBColor color(pLight->DiffuseColor);
			m_LightColors[i].Set(color);
		}

		// relative coords to a screen size
		pData->FlarePosX = 100.0 * _pos[0]; // / w;
		pData->FlarePosY = 100.0 * _pos[1]; // _pos[1] / h;
	}
	else
	{
		m_LightPositions.clear();
		m_LightColors.clear();
	}

	if (nullptr != mShader)
	{
		mShader->Bind();

		if (seed >= 0)
		{
			glUniform1f(seed, static_cast<float>(seedValue));
		}
		if (upperClip >= 0)
		{
			glUniform1f(upperClip, 0.01f * (float)_upperClip);
		}

		if (lowerClip >= 0)
			glUniform1f(lowerClip, 1.0f - 0.01f * (float)_lowerClip);

		if (amount >= 0)
			glUniform1f(amount, 0.01f * (float)_amount);

		if (textureWidth >= 0)
			glUniform1f(textureWidth, (float)w);

		if (textureHeight >= 0)
			glUniform1f(textureHeight, (float)h);

		if (timer >= 0)
			glUniform1f(timer, (float)_timer);

		if (light_pos >= 0)
		{
			glUniform4f(light_pos, static_cast<float>(_pos[0]), static_cast<float>(_pos[1]), static_cast<float>(_pos[2]), 0.0f);
		}

		if (tint >= 0)
			glUniform4f(tint, (float)_tint[0], (float)_tint[1], (float)_tint[2], 1.0f);

		if (inner >= 0)
			glUniform1f(inner, 0.01f * (float)_inner);

		if (outer >= 0)
			glUniform1f(outer, 0.01f * (float)_outer);

		if (fadeToBorders >= 0)
			glUniform1f(fadeToBorders, _fadeToBorders);

		if (borderWidth >= 0)
			glUniform1f(borderWidth, (float)_borderWidth);

		if (feather >= 0)
			glUniform1f(feather, 0.01f * (float)_feather);

		mShader->UnBind();

		lSuccess = true;
	}

	return lSuccess;
}

const int PostEffectLensFlare::GetNumberOfPasses() const
{
	return subShaders[mCurrentShader].m_NumberOfPasses;
}

bool PostEffectLensFlare::PrepPass(const int pass)
{
	return subShaders[mCurrentShader].PrepPass(GetShaderPtr(), pass);
}

bool PostEffectLensFlare::SubShader::PrepPass(GLSLShader* mShader, const int pass)
{
	// shader must be binded
	if (light_pos >= 0 && pass < static_cast<int>(m_LightPositions.size()))
	{
		const FBVector3d pos(m_LightPositions[pass]);
		glUniform4f(light_pos, static_cast<float>(pos[0]), static_cast<float>(pos[1]), static_cast<float>(pos[2]), m_DepthAttenuation);

		const FBColor _tint(m_LightColors[pass]);
		glUniform4f(tint, (float)_tint[0], (float)_tint[1], (float)_tint[2], 1.0f);
		return true;
	}
	return false;
}