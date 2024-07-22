
/**	\file	posteffectfisheye.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectfisheye.h"
#include "postpersistentdata.h"

#define SHADER_FISH_EYE_NAME			"Fish Eye"
#define SHADER_FISH_EYE_VERTEX			"\\GLSL\\fishEye.vsh"
#define SHADER_FISH_EYE_FRAGMENT		"\\GLSL\\fishEye.fsh"

//
extern void LOGE(const char* pFormatString, ...);


//! a constructor
PostEffectFishEye::PostEffectFishEye()
	: PostEffectBase()
{
	for (int i = 0; i < LOCATIONS_COUNT; ++i)
		mLocations[i] = -1;
}

//! a destructor
PostEffectFishEye::~PostEffectFishEye()
{

}

const char* PostEffectFishEye::GetName()
{
	return SHADER_FISH_EYE_NAME;
}

const char* PostEffectFishEye::GetVertexFname(const int shaderIndex)
{
	return SHADER_FISH_EYE_VERTEX;
}

const char* PostEffectFishEye::GetFragmentFname(const int shaderIndex)
{
	return SHADER_FISH_EYE_FRAGMENT;
}

bool PostEffectFishEye::PrepUniforms(const int shaderIndex)
{
	GLSLShader* shader = mShaders[shaderIndex];
	if (!shader)
		return false;

	shader->Bind();

	GLint loc = shader->findLocation("sampler0");
	if (loc >= 0)
		glUniform1i(loc, 0);
	PrepareUniformLocations(shader);

	mLocAmount = shader->findLocation("amount");
	mLocLensRadius = shader->findLocation("lensradius");
	mLocSignCurvature = shader->findLocation("signcurvature");

	shader->UnBind();
	return true;
}

bool PostEffectFishEye::CollectUIValues(PostPersistentData* pData, int w, int h, FBCamera* pCamera)
{
	const double amount = pData->FishEyeAmount;
	const double lensradius = pData->FishEyeLensRadius;
	const double signcurvature = pData->FishEyeSignCurvature;

	GLSLShader* shader = GetShaderPtr();
	if (!shader)
		return false;

	shader->Bind();
	UpdateUniforms(pData);

	if (mLocAmount >= 0)
		glUniform1f(mLocAmount, 0.01f * static_cast<float>(amount));
	if (mLocLensRadius >= 0)
		glUniform1f(mLocLensRadius, static_cast<float>(lensradius));
	if (mLocSignCurvature >= 0)
		glUniform1f(mLocSignCurvature, static_cast<float>(signcurvature));

	shader->UnBind();
	return true;
}
