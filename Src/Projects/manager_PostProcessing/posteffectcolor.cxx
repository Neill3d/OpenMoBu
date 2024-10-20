
/**	\file	posteffectcolor.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectcolor.h"
#include "postpersistentdata.h"

#define SHADER_COLOR_NAME				"Color Correction"
#define SHADER_COLOR_VERTEX				"\\GLSL\\simple.vsh"
#define SHADER_COLOR_FRAGMENT			"\\GLSL\\color.fsh"

//
extern void LOGE(const char* pFormatString, ...);

//! a constructor
PostEffectColor::PostEffectColor()
	: PostEffectBase()
{
	for (int i = 0; i < LOCATIONS_COUNT; ++i)
		mLocations[i] = -1;
}

//! a destructor
PostEffectColor::~PostEffectColor()
{

}

const char* PostEffectColor::GetName() const
{
	return SHADER_COLOR_NAME;
}

const char* PostEffectColor::GetVertexFname(const int) const
{
	return SHADER_COLOR_VERTEX;
}

const char* PostEffectColor::GetFragmentFname(const int) const
{
	return SHADER_COLOR_FRAGMENT;
}

bool PostEffectColor::PrepUniforms(const int shaderIndex)
{
	GLSLShader* shader = mShaders[shaderIndex];
	if (!shader)
		return false;

	shader->Bind();

	GLint loc = shader->findLocation("sampler0");
	if (loc >= 0)
		glUniform1i(loc, 0);

	mResolution = shader->findLocation("gResolution");
	mChromaticAberration = shader->findLocation("gCA");

	PrepareUniformLocations(shader);

	mLocCSB = shader->findLocation("gCSB");
	mLocHue = shader->findLocation("gHue");

	shader->UnBind();
	return true;

}

bool PostEffectColor::CollectUIValues(PostPersistentData* pData, PostEffectContext& effectContext)
{
	const float chromatic_aberration = (pData->ChromaticAberration) ? 1.0f : 0.0f;
	const FBVector2d ca_dir = pData->ChromaticAberrationDirection;

	double saturation = 1.0 + 0.01 * pData->Saturation;
	double brightness = 1.0 + 0.01 * pData->Brightness;
	double contrast = 1.0 + 0.01 * pData->Contrast;
	double gamma = 0.01 * pData->Gamma;

	const float inverse = (pData->Inverse) ? 1.0f : 0.0f;
	double hue = 0.01 * pData->Hue;
	double hueSat = 0.01 * pData->HueSaturation;
	double lightness = 0.01 * pData->Lightness;

	GLSLShader* mShader = GetShaderPtr();
	if (!mShader)
		return false;

	mShader->Bind();

	if (mResolution >= 0)
	{
		glUniform2f(mResolution, static_cast<float>(effectContext.w), static_cast<float>(effectContext.h));
	}

	if (mChromaticAberration >= 0)
	{
		glUniform4f(mChromaticAberration, static_cast<float>(ca_dir[0]), static_cast<float>(ca_dir[1]), 0.0f, chromatic_aberration);
	}

	UpdateUniforms(pData);

	if (mLocCSB >= 0)
		glUniform4f(mLocCSB, (float)contrast, (float)saturation, (float)brightness, (float)gamma);

	if (mLocHue >= 0)
		glUniform4f(mLocHue, (float)hue, (float)hueSat, (float)lightness, inverse);

	mShader->UnBind();
	return true;
}