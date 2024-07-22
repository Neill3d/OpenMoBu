
/**	\file	posteffectfilmgrain.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectfilmgrain.h"
#include "postpersistentdata.h"

#define SHADER_FILMGRAIN_NAME			"Film Grain"
#define SHADER_FILMGRAIN_VERTEX			"\\GLSL\\simple.vsh"
#define SHADER_FILMGRAIN_FRAGMENT		"\\GLSL\\filmGrain.fsh"

//
extern void LOGE(const char* pFormatString, ...);

//! a constructor
PostEffectFilmGrain::PostEffectFilmGrain()
	: PostEffectBase()
{
	for (int i = 0; i < LOCATIONS_COUNT; ++i)
		mLocations[i] = -1;
}

//! a destructor
PostEffectFilmGrain::~PostEffectFilmGrain()
{

}

const char* PostEffectFilmGrain::GetName()
{
	return SHADER_FILMGRAIN_NAME;
}
const char* PostEffectFilmGrain::GetVertexFname(const int)
{
	return SHADER_FILMGRAIN_VERTEX;
}
const char* PostEffectFilmGrain::GetFragmentFname(const int)
{
	return SHADER_FILMGRAIN_FRAGMENT;
}

bool PostEffectFilmGrain::PrepUniforms(const int shaderIndex)
{
	GLSLShader* mShader = mShaders[shaderIndex];
	if (!mShader)
		return false;

	mShader->Bind();

	GLint loc = mShader->findLocation("sampler0");
	if (loc >= 0)
		glUniform1i(loc, 0);

	PrepareUniformLocations(mShader);

	textureWidth = mShader->findLocation("textureWidth");
	textureHeight = mShader->findLocation("textureHeight");

	timer = mShader->findLocation("timer");
	grainamount = mShader->findLocation("grainamount");
	colored = mShader->findLocation("colored");
	coloramount = mShader->findLocation("coloramount");
	grainsize = mShader->findLocation("grainsize");
	lumamount = mShader->findLocation("lumamount");

	mShader->UnBind();
	return true;
}

bool PostEffectFilmGrain::CollectUIValues(PostPersistentData* pData, int w, int h, FBCamera* pCamera)
{
	FBTime systemTime = (pData->FG_UsePlayTime) ? mSystem.LocalTime : mSystem.SystemTime;

	const double timerMult = pData->FG_TimeSpeed;
	const double _timer = 0.01 * timerMult * systemTime.GetSecondDouble();

	const double _grainamount = pData->FG_GrainAmount;
	const double _colored = (pData->FG_Colored) ? 1.0 : 0.0;
	const double _coloramount = pData->FG_ColorAmount;
	const double _grainsize = pData->FG_GrainSize;
	const double _lumamount = pData->FG_LumAmount;

	GLSLShader* mShader = GetShaderPtr();
	if (!mShader)
		return false;

	mShader->Bind();
	UpdateUniforms(pData);

	if (textureWidth >= 0)
		glUniform1f(textureWidth, static_cast<float>(w));
	if (textureHeight >= 0)
		glUniform1f(textureHeight, static_cast<float>(h));

	if (timer >= 0)
		glUniform1f(timer, static_cast<float>(_timer));
	if (grainamount >= 0)
		glUniform1f(grainamount, 0.01f * static_cast<float>(_grainamount));
	if (colored >= 0)
		glUniform1f(colored, static_cast<float>(_colored));
	if (coloramount >= 0)
		glUniform1f(coloramount, 0.01f * static_cast<float>(_coloramount));
	if (grainsize >= 0)
		glUniform1f(grainsize, 0.01f * static_cast<float>(_grainsize));
	if (lumamount >= 0)
		glUniform1f(lumamount, 0.01f * static_cast<float>(_lumamount));

	mShader->UnBind();
	return true;
}
