
// posteffectssao.cpp
/*
Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectssao.h"
#include "posteffectshader_mix.h"
#include "posteffectshader_blur_lineardepth.h"
#include "posteffectbuffers.h"
#include "postpersistentdata.h"
#include "shaderpropertystorage.h"
#include "shaderpropertywriter.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "postprocessing_helper.h"
#include "hashUtils.h"

////////////////////////////////////////////////////////////////////////////////
// PostEffectSSAO

PostEffectSSAO::PostEffectSSAO()
	: PostEffectBase()
	, mShaderSSAO(std::make_unique<EffectShaderSSAO>(nullptr)) // making it without an owner component
	, mShaderMix(std::make_unique<EffectShaderMix>(nullptr))
	, mShaderBlur(std::make_unique<EffectShaderBlurLinearDepth>(nullptr))
{
}

bool PostEffectSSAO::IsActive() const
{
	return true;
}

const char* PostEffectSSAO::GetName() const
{
	return mShaderSSAO->GetName();
}

PostEffectBufferShader* PostEffectSSAO::GetBufferShaderPtr(const int bufferShaderIndex) 
{ 
	return static_cast<PostEffectBufferShader*>(mShaderSSAO.get());
}
const PostEffectBufferShader* PostEffectSSAO::GetBufferShaderPtr(const int bufferShaderIndex) const
{ 
	return static_cast<const PostEffectBufferShader*>(mShaderSSAO.get());
}

EffectShaderSSAO* PostEffectSSAO::GetBufferShaderTypedPtr() 
{ 
	return mShaderSSAO.get(); 
}
const EffectShaderSSAO* PostEffectSSAO::GetBufferShaderTypedPtr() const 
{ 
	return mShaderSSAO.get(); 
}

bool PostEffectSSAO::Load(const char* shaderLocation)
{
	if (!mShaderMix->Load(shaderLocation))
		return false;
	if (!mShaderBlur->Load(shaderLocation))
		return false;

	return PostEffectBase::Load(shaderLocation);
}

bool PostEffectSSAO::CollectUIValues(IPostEffectContext* effectContext)
{
	mShaderMix->CollectUIValues(effectContext, 0);
	//mShaderMix->mBloom->SetValue(0.0f, 0.0f, 0.0f, 0.0f); // disable bloom in mix shader
	mShaderBlur->CollectUIValues(effectContext, 0);
	
	return PostEffectBase::CollectUIValues(effectContext);
}

void PostEffectSSAO::Process(PostEffectRenderContext& renderContext, IPostEffectContext* effectContext)
{
	// render SSAO into its own buffer
	constexpr const char* ssaoBufferName = "ssao";
	static const uint32_t ssaoBufferNameKey = xxhash32(ssaoBufferName);

	const PostPersistentData* postData = effectContext->GetPostProcessData();
	PostEffectBuffers* buffers = renderContext.buffers;

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

		mShaderSSAO->Render(renderContextSSAO, effectContext);

		if (doBlur)
		{
			PostEffectRenderContext renderContextBlur;
			renderContextBlur.buffers = buffers;
			renderContextBlur.targetFramebuffer = pBufferSSAO;
			renderContextBlur.colorAttachment = 1;
			renderContextBlur.srcTextureId = pBufferSSAO->GetColorObject(0);
			renderContextBlur.width = outWidth;
			renderContextBlur.height = outHeight;
			renderContextBlur.generateMips = false;

			const float color_shift = 0.0f;
			renderContextBlur.OverrideUniform(*mShaderBlur->mColorShift, color_shift);
			renderContextBlur.OverrideUniform(*mShaderBlur->mInvRes, 1.0f / static_cast<float>(outWidth), 1.0f / static_cast<float>(outHeight));			

			mShaderBlur->Render(renderContextBlur, effectContext);
		}
	
		// mix SSAO result with the original scene
		glActiveTexture(GL_TEXTURE0 + CommonEffect::UserSamplerSlot);
		const uint32_t ssaoTextureId = (doBlur) ? pBufferSSAO->GetColorObject(1) : pBufferSSAO->GetColorObject(0);
		glBindTexture(GL_TEXTURE_2D, ssaoTextureId);
		glActiveTexture(GL_TEXTURE0);

		PostEffectRenderContext renderContextMix = renderContext;
		// disable bloom in mix shader
		renderContextMix.OverrideUniform(*mShaderMix->mBloom, 0.0f, 0.0f, 0.0f, 0.0f);

		mShaderMix->Render(renderContextMix, effectContext);

		glActiveTexture(GL_TEXTURE0 + CommonEffect::UserSamplerSlot);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);

		buffers->ReleaseFramebuffer(ssaoBufferNameKey);
	}
	else
	{
		// just render SSAO result into the output
		mShaderSSAO->Render(renderContext, effectContext);
	}
}


////////////////////////////////////////////////////////////////////////////////
// EffectShaderSSAO

uint32_t EffectShaderSSAO::SHADER_NAME_HASH = xxhash32(EffectShaderSSAO::SHADER_NAME);

EffectShaderSSAO::EffectShaderSSAO(FBComponent* ownerIn)
	: PostEffectBufferShader(ownerIn)
	, e2(rd())
	, dist(0, 1.0)
{
	MakeCommonProperties();

	AddProperty(ShaderProperty("color", "colorSampler"))
		.SetType(EPropertyType::TEXTURE)
		.SetFlag(PropertyFlag::ShouldSkip, true)
		.SetDefaultValue(CommonEffect::ColorSamplerSlot);

	AddProperty(ShaderProperty("random", "texRandom"))
		.SetType(EPropertyType::TEXTURE)
		.SetFlag(PropertyFlag::ShouldSkip, true)
		.SetDefaultValue(CommonEffect::UserSamplerSlot);

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

bool EffectShaderSSAO::OnCollectUI(IPostEffectContext* effectContext, int maskIndex)
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

	//mOnlyAO->SetValue(onlyAO);
	//mProjInfo->SetValue(projInfo[0], projInfo[1], projInfo[2], projInfo[3]);
	//mProjOrtho->SetValue(projOrtho);
	//mRadiusToScreen->SetValue(RadiusToScreen);
	//mNegInvR2->SetValue(negInvR2);
	//mPowExponent->SetValue(intensity);
	//mNDotVBias->SetValue(bias);
	//mAOMultiplier->SetValue(aoMult);
	//mInvFullResolution->SetValue(1.0f / float(effectContext->GetViewWidth()), 1.0f / float(effectContext->GetViewHeight()));
	
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