
// posteffectshader_ssao.cpp
/*
Sergei <Neill3d> Solokhin 2018-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectshader_ssao.h"
#include "posteffectshader_mix.h"
#include "posteffectshader_blur_lineardepth.h"
#include "posteffect_buffers.h"
#include "postpersistentdata.h"
#include "shaderproperty_storage.h"
#include "shaderproperty_writer.h"
#include "standardeffectcollection.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "postprocessing_helper.h"
#include "hashUtils.h"

uint32_t EffectShaderSSAO::SHADER_NAME_HASH = xxhash32(EffectShaderSSAO::SHADER_NAME);

EffectShaderSSAO::EffectShaderSSAO(FBComponent* ownerIn)
	: PostEffectBufferShader(ownerIn)
	, e2(rd())
	, dist(0, 1.0)
{
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

void EffectShaderSSAO::OnPopulateProperties(ShaderPropertyScheme* scheme)
{
	scheme->AddProperty("color", "colorSampler")
		.SetType(EPropertyType::TEXTURE)
		.SetFlag(PropertyFlag::SKIP)
		.SetDefaultValue(CommonEffect::ColorSamplerSlot);

	scheme->AddProperty("random", "texRandom")
		.SetType(EPropertyType::TEXTURE)
		.SetFlag(PropertyFlag::SKIP)
		.SetDefaultValue(CommonEffect::UserSamplerSlot);

	mProjInfo = scheme->AddProperty("projInfo", "projInfo")
		.SetType(EPropertyType::VEC4)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();
	mProjOrtho = scheme->AddProperty("projOrtho", "projOrtho")
		.SetType(EPropertyType::INT)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	mInvFullResolution = scheme->AddProperty("InvFullResolution", "InvFullResolution")
		.SetType(EPropertyType::VEC2)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	mRadiusToScreen = scheme->AddProperty("RadiusToScreen", "RadiusToScreen")
		.SetType(EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	mNegInvR2 = scheme->AddProperty("NegInvR2", "NegInvR2")
		.SetType(EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();
	mNDotVBias = scheme->AddProperty("NDotVBias", "NDotVBias")
		.SetType(EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	mAOMultiplier = scheme->AddProperty("AOMultiplier", "AOMultiplier")
		.SetType(EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	mPowExponent = scheme->AddProperty("PowExponent", "PowExponent")
		.SetType(EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	mOnlyAO = scheme->AddProperty("OnlyAO", "OnlyAO")
		.SetType(EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();
}

bool EffectShaderSSAO::OnCollectUI(PostEffectContextProxy* effectContext, int maskIndex) const
{
	FBCamera* camera = effectContext->GetCamera();
	const PostPersistentData* pData = effectContext->GetPostProcessData();
	if (!camera || !pData)
		return false;

	// calculate a diagonal fov
	// convert to mm

	const double filmWidth = 25.4 * camera->FilmSizeWidth;
	const double filmHeight = 25.4 * camera->FilmSizeHeight;

	const double diag = sqrt(filmWidth*filmWidth + filmHeight*filmHeight);
	const double focallen = camera->FocalLength;

	const float fov = 2.0 * atan(diag / (focallen * 2.0));
	
	const float onlyAO = 1.0f; // (pData->OnlyAO || pData->SSAO_Blur) ? 1.0f : 0.0f;

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
	
	ShaderPropertyWriter write(this, effectContext);

	write(mOnlyAO, onlyAO)
		(mProjInfo, projInfo[0], projInfo[1], projInfo[2], projInfo[3])
		(mProjOrtho, projOrtho)
		(mRadiusToScreen, RadiusToScreen)
		(mNegInvR2, negInvR2)
		(mPowExponent, intensity)
		(mNDotVBias, bias)
		(mAOMultiplier, aoMult)
		(mInvFullResolution,
			1.0f / float(effectContext->GetViewWidth()),
			1.0f / float(effectContext->GetViewHeight()));

	return true;
}

bool EffectShaderSSAO::Bind()
{
	if (hbaoRandomTexId == 0)
	{
		InitMisc();
	}

	// bind a random texture
	glActiveTexture(GL_TEXTURE0 + CommonEffect::UserSamplerSlot);
	glBindTexture(GL_TEXTURE_2D, hbaoRandomTexId);
	glActiveTexture(GL_TEXTURE0);

	return PostEffectBufferShader::Bind();
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

void EffectShaderSSAO::RenderPass(int passIndex, PostEffectRenderContext& renderContext, PostEffectContextProxy* effectContext)
{
	// render SSAO into its own buffer
	constexpr const char* ssaoBufferName = "ssao";
	static const uint32_t ssaoBufferNameKey = xxhash32(ssaoBufferName);

	const PostPersistentData* postData = effectContext->GetPostProcessData();
	PostEffectBuffers* buffers = renderContext.buffers;
	auto effectCollection = effectContext->GetEffectCollection();

	if (!postData->OnlyAO)
	{
		const bool doBlur = postData->SSAO_Blur;
		constexpr bool makeDownscale = true;
		const int outWidth = (makeDownscale) ? buffers->GetWidth() / 2 : buffers->GetWidth();
		const int outHeight = (makeDownscale) ? buffers->GetHeight() / 2 : buffers->GetHeight();
		constexpr int numColorAttachments = 2;

		FrameBuffer* pBufferSSAO = buffers->RequestFramebuffer(ssaoBufferNameKey,
			outWidth, outHeight, PostEffectBuffers::GetFlagsForSingleColorBuffer(),
			numColorAttachments,
			false, [](FrameBuffer* frameBuffer) {
				PostEffectBuffers::SetParametersForMainColorBuffer(frameBuffer, false);
			});

		PostEffectRenderContext renderContextSSAO;
		renderContextSSAO.buffers = buffers;
		renderContextSSAO.targetFramebuffer = pBufferSSAO;
		renderContextSSAO.colorAttachment = 0;
		renderContextSSAO.srcTextureId = renderContext.srcTextureId;
		renderContextSSAO.width = outWidth;
		renderContextSSAO.height = outHeight;
		renderContextSSAO.generateMips = false;

		PostEffectBufferShader::RenderPass(passIndex, renderContextSSAO, effectContext);

		if (doBlur)
		{
			auto shaderBlur = effectCollection->GetEffectBlurLinearDepth();

			PostEffectRenderContext renderContextBlur;
			renderContextBlur.buffers = buffers;
			renderContextBlur.targetFramebuffer = pBufferSSAO;
			renderContextBlur.colorAttachment = 1;
			renderContextBlur.srcTextureId = pBufferSSAO->GetColorObject(0);
			renderContextBlur.width = outWidth;
			renderContextBlur.height = outHeight;
			renderContextBlur.generateMips = false;

			const float color_shift = 0.0f;
			renderContextBlur.OverrideUniform(shaderBlur->GetPropertySchemePtr(), shaderBlur->mColorShift, color_shift);
			renderContextBlur.OverrideUniform(shaderBlur->GetPropertySchemePtr(), shaderBlur->mInvRes, 1.0f / static_cast<float>(outWidth), 1.0f / static_cast<float>(outHeight));

			shaderBlur->Render(renderContextBlur, effectContext);
		}

		// mix SSAO result with the original scene
		auto shaderMix = effectCollection->GetEffectMix();

		glActiveTexture(GL_TEXTURE0 + CommonEffect::UserSamplerSlot);
		const uint32_t ssaoTextureId = (doBlur) ? pBufferSSAO->GetColorObject(1) : pBufferSSAO->GetColorObject(0);
		glBindTexture(GL_TEXTURE_2D, ssaoTextureId);
		glActiveTexture(GL_TEXTURE0);

		PostEffectRenderContext renderContextMix = renderContext;
		// disable bloom in mix shader
		renderContextMix.OverrideUniform(shaderMix->GetPropertySchemePtr(), shaderMix->mBloom, 0.0f, 0.0f, 0.0f, 0.0f);

		shaderMix->Render(renderContextMix, effectContext);

		glActiveTexture(GL_TEXTURE0 + CommonEffect::UserSamplerSlot);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
	}
	else
	{
		// just render SSAO result into the output
		PostEffectBufferShader::RenderPass(passIndex, renderContext, effectContext);
	}
}