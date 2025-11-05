
// posteffectssao.cpp
/*
Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectssao.h"
#include "postpersistentdata.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "postprocessing_helper.h"

//! a constructor
EffectShaderSSAO::EffectShaderSSAO(FBComponent* ownerIn)
	: PostEffectBufferShader(ownerIn)
	, e2(rd())
	, dist(0, 1.0)
{
	MakeCommonProperties();

	AddProperty(ShaderProperty("color", "colorSampler"))
		.SetType(EPropertyType::TEXTURE)
		.SetValue(CommonEffect::ColorSamplerSlot);

	AddProperty(ShaderProperty("random", "texRandom"))
		.SetType(EPropertyType::TEXTURE)
		.SetValue(CommonEffect::UserSamplerSlot);

	mProjInfo = &AddProperty(ShaderProperty("projInfo", "projInfo", nullptr))
		.SetType(EPropertyType::VEC4)
		.SetFlag(PropertyFlag::ShouldSkip, true);
	mProjOrtho = &AddProperty(ShaderProperty("projOrtho", "projOrtho", nullptr))
		.SetType(EPropertyType::INT)
		.SetFlag(PropertyFlag::ShouldSkip, true);

	mInvFullResolution = &AddProperty(ShaderProperty("InvFullResolution", "InvFullResolution", nullptr))
		.SetType(EPropertyType::VEC2)
		.SetFlag(PropertyFlag::ShouldSkip, true);

	mRadiusToScreen = &AddProperty(ShaderProperty("RadiusToScreen", "RadiusToScreen", nullptr))
		.SetType(EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::ShouldSkip, true);
	
	mNegInvR2 = &AddProperty(ShaderProperty("NegInvR2", "NegInvR2", nullptr))
		.SetType(EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::ShouldSkip, true);
	mNDotVBias = &AddProperty(ShaderProperty("NDotVBias", "NDotVBias", nullptr))
		.SetType(EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::ShouldSkip, true);

	mAOMultiplier = &AddProperty(ShaderProperty("AOMultiplier", "AOMultiplier", nullptr))
		.SetType(EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::ShouldSkip, true);

	mPowExponent = &AddProperty(ShaderProperty("PowExponent", "PowExponent", nullptr))
		.SetType(EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::ShouldSkip, true);

	mOnlyAO = &AddProperty(ShaderProperty("OnlyAO", "OnlyAO", nullptr))
		.SetType(EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::ShouldSkip, true);
	
	// lazy initialize of random texture on first render
	hbaoRandomTexId = 0;
}

//! a destructor
EffectShaderSSAO::~EffectShaderSSAO()
{
	DeleteTextures();
}

const char* EffectShaderSSAO::GetUseMaskingPropertyName() const noexcept
{
	return PostPersistentData::SSAO_USE_MASKING;
}
const char* EffectShaderSSAO::GetMaskingChannelPropertyName() const noexcept
{
	return PostPersistentData::SSAO_MASKING_CHANNEL;
}

void EffectShaderSSAO::DeleteTextures()
{
	if (hbaoRandomTexId > 0)
	{
		glDeleteTextures(1, &hbaoRandomTexId);
		hbaoRandomTexId = 0;
	}
}

bool EffectShaderSSAO::OnCollectUI(const IPostEffectContext* effectContext, int maskIndex)
{
	FBCamera* camera = effectContext->GetCamera();
	const PostPersistentData* pData = effectContext->GetPostProcessData();

	// calculate a diagonal fov
	// convert to mm

	const double filmWidth = 25.4 * camera->FilmSizeWidth;
	const double filmHeight = 25.4 * camera->FilmSizeHeight;

	const double diag = sqrt(filmWidth*filmWidth + filmHeight*filmHeight);
	const double focallen = camera->FocalLength;

	const float fov = 2.0 * atan(diag / (focallen * 2.0));
	
	const float onlyAO = (pData->OnlyAO || pData->SSAO_Blur) ? 1.0f : 0.0f;

	const float* P = effectContext->GetProjectionMatrixF();

	const float projInfoPerspective[4] = {
		2.0f / (P[4 * 0 + 0]),       // (x) * (R - L)/N
		2.0f / (P[4 * 1 + 1]),       // (y) * (T - B)/N
		-(1.0f - P[4 * 2 + 0]) / P[4 * 0 + 0], // L/N
		-(1.0f + P[4 * 2 + 1]) / P[4 * 1 + 1], // B/N
	};

	const float projInfoOrtho[4] = {
		2.0f / (P[4 * 0 + 0]),      // ((x) * R - L)
		2.0f / (P[4 * 1 + 1]),      // ((y) * T - B)
		-(1.0f + P[4 * 3 + 0]) / P[4 * 0 + 0], // L
		-(1.0f - P[4 * 3 + 1]) / P[4 * 1 + 1], // B
	};

	const int projOrtho = (effectContext->IsCameraOrthogonal()) ? 1 : 0;
	const float* projInfo = projOrtho ? projInfoOrtho : projInfoPerspective;
	
	float projScale;
	if (projOrtho){
		projScale = float(effectContext->GetViewHeight()) / (projInfoOrtho[1]);
	}
	else {
		projScale = float(effectContext->GetViewHeight()) / (tanf(fov * 0.5f) * 2.0f);
	}

	// radius

	float meters2viewspace = 1.0f;
	float R = (float)pData->SSAO_Radius * meters2viewspace;
	float R2 = R * R;
	float negInvR2 = -1.0f / R2;
	float RadiusToScreen = R * 0.5f * projScale;

	// ao
	float intensity = 0.01f * (float) pData->SSAO_Intensity;
	if (intensity < 0.0f)
		intensity = 0.0f;
	
	float bias = 0.01f * (float)pData->SSAO_Bias;
	if (bias < 0.0f)
		bias = 0.0f;
	else if (bias > 1.0f)
		bias = 1.0f;

	const float aoMult = 1.0f / (1.0f - bias);

	
	mOnlyAO->SetValue(onlyAO);
	mProjInfo->SetValue(projInfo[0], projInfo[1], projInfo[2], projInfo[3]);
	mProjOrtho->SetValue(projOrtho);
	mRadiusToScreen->SetValue(RadiusToScreen);
	mNegInvR2->SetValue(negInvR2);
	mPowExponent->SetValue(intensity);
	mNDotVBias->SetValue(bias);
	mAOMultiplier->SetValue(aoMult);
	mInvFullResolution->SetValue(1.0f / float(effectContext->GetViewWidth()), 1.0f / float(effectContext->GetViewHeight()));
	
	return true;
}

void EffectShaderSSAO::Bind()
{
	if (hbaoRandomTexId == 0)
	{
		InitMisc();
	}

	// bind a random texture
	glActiveTexture(GL_TEXTURE0 + CommonEffect::UserSamplerSlot);
	glBindTexture(GL_TEXTURE_2D, hbaoRandomTexId);
	glActiveTexture(GL_TEXTURE0);

	PostEffectBufferShader::Bind();
}

void EffectShaderSSAO::UnBind()
{
	// bind a random texture
	glActiveTexture(GL_TEXTURE0 + CommonEffect::UserSamplerSlot);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);

	PostEffectBufferShader::UnBind();
}

bool EffectShaderSSAO::InitMisc()
{
	constexpr int HBAO_RANDOM_SIZE{ 4 };
	float	hbaoRandom[HBAO_RANDOM_SIZE][HBAO_RANDOM_SIZE][4];

	constexpr float numDir{ 8.0f }; // keep in sync to glsl

	float Rand1 = (float) dist(e2);
	float Rand2 = (float) dist(e2);
	
	// Use random rotation angles in [0,2PI/NUM_DIRECTIONS)
	float Angle = 2.f * M_PI * Rand1 / numDir;
	mRandom[0] = cosf(Angle);
	mRandom[1] = sinf(Angle);
	mRandom[2] = Rand2;
	mRandom[3] = 0;

	for (int i = 0; i < HBAO_RANDOM_SIZE; i++)
	{
		for (int j = 0; j < HBAO_RANDOM_SIZE; ++j)
		{
			Rand1 = (float)dist(e2);
			Rand2 = (float)dist(e2);

			// Use random rotation angles in [0,2PI/NUM_DIRECTIONS)
			Angle = 2.f * M_PI * Rand1 / numDir;
			hbaoRandom[i][j][0] = cosf(Angle);
			hbaoRandom[i][j][1] = sinf(Angle);
			hbaoRandom[i][j][2] = Rand2;
			hbaoRandom[i][j][3] = 0;
		}
	}
	
	DeleteTextures();
	glGenTextures(1, &hbaoRandomTexId);

	glBindTexture(GL_TEXTURE_2D, hbaoRandomTexId);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, HBAO_RANDOM_SIZE, HBAO_RANDOM_SIZE, 0, GL_RGBA, GL_FLOAT, hbaoRandom);

	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}