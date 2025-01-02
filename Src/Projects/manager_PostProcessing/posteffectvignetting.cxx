
/**	\file	posteffectvignetting.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectvignetting.h"
#include "postpersistentdata.h"

#define SHADER_VIGNETTE_NAME			"Vignetting"
#define SHADER_VIGNETTE_VERTEX			"\\GLSL\\simple.vsh"
#define SHADER_VIGNETTE_FRAGMENT		"\\GLSL\\vignetting.fsh"

//
extern void LOGE(const char* pFormatString, ...);

//! a constructor
PostEffectVignetting::PostEffectVignetting()
	: PostEffectBase()
{
	for (int i = 0; i < LOCATIONS_COUNT; ++i)
		mLocations[i] = -1;
}

//! a destructor
PostEffectVignetting::~PostEffectVignetting()
{

}

const char* PostEffectVignetting::GetName() const
{
	return SHADER_VIGNETTE_NAME;
}
const char* PostEffectVignetting::GetVertexFname(const int) const
{
	return SHADER_VIGNETTE_VERTEX;
}
const char* PostEffectVignetting::GetFragmentFname(const int) const
{
	return SHADER_VIGNETTE_FRAGMENT;
}

bool PostEffectVignetting::PrepUniforms(const int shaderIndex)
{
	GLSLShaderProgram* mShader = mShaders[shaderIndex];
	if (!mShader)
		return false;

	mShader->Bind();

	GLint loc = mShader->findLocation("sampler0");
	if (loc >= 0)
		glUniform1i(loc, 0);

	PrepareCommonLocations(mShader);

	mLocAmount = mShader->findLocation("amount");
	mLocVignOut = mShader->findLocation("vignout");
	mLocVignIn = mShader->findLocation("vignin");
	mLocVignFade = mShader->findLocation("vignfade");

	mShader->UnBind();
	return true;
}

bool PostEffectVignetting::CollectUIValues(PostPersistentData* pData, PostEffectContext& effectContext)
{
	const double amount = pData->VignAmount;
	const double vignout = pData->VignOut;
	const double vignin = pData->VignIn;
	const double vignfade = pData->VignFade;

	GLSLShaderProgram* mShader = GetShaderPtr();

	if (!mShader)
		return false;

	mShader->Bind();
	CollectCommonData(pData);

	if (mLocAmount >= 0)
		glUniform1f(mLocAmount, 0.01f * static_cast<float>(amount));
	if (mLocVignOut >= 0)
		glUniform1f(mLocVignOut, 0.01f * static_cast<float>(vignout));
	if (mLocVignIn >= 0)
		glUniform1f(mLocVignIn, 0.01f * static_cast<float>(vignin));
	if (mLocVignFade >= 0)
		glUniform1f(mLocVignFade, static_cast<float>(vignfade));

	mShader->UnBind();
	return true;
}
