
/**	\file	posteffect_color.cxx

Sergei <Neill3d> Solokhin 2018-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectshader_color.h"
#include "posteffectshader_mix.h"
#include "posteffectshader_blur_lineardepth.h"
#include "posteffect_buffers.h"
#include "postpersistentdata.h"
#include "shaderproperty_writer.h"
#include "standardeffectcollection.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "postprocessing_helper.h"
#include "hashUtils.h"

uint32_t EffectShaderColor::SHADER_NAME_HASH = xxhash32(EffectShaderColor::SHADER_NAME);

EffectShaderColor::EffectShaderColor(FBComponent* ownerIn)
	: PostEffectBufferShader(ownerIn)
{}

const char* EffectShaderColor::GetUseMaskingPropertyName() const noexcept
{
	return PostPersistentData::COLOR_USE_MASKING;
}
const char* EffectShaderColor::GetMaskingChannelPropertyName() const noexcept
{
	return PostPersistentData::COLOR_MASKING_CHANNEL;
}

void EffectShaderColor::OnPopulateProperties(ShaderPropertyScheme* scheme)
{
	scheme->AddProperty("color", "sampler0")
		.SetType(EPropertyType::TEXTURE)
		.SetFlag(PropertyFlag::SKIP)
		.SetDefaultValue(CommonEffect::ColorSamplerSlot);

	mChromaticAberration = scheme->AddProperty("gCA", "gCA", EPropertyType::VEC4)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();
	mCSB = scheme->AddProperty("gCSB", "gCSB", EPropertyType::VEC4)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();
	mHue = scheme->AddProperty("gHue", "gHue", EPropertyType::VEC4)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();
}

bool EffectShaderColor::OnCollectUI(PostEffectContextProxy* effectContext, int maskIndex) const
{
	const PostPersistentData* pData = effectContext->GetPostProcessData();
	if (!pData)
		return false;

	const float chromatic_aberration = (pData->ChromaticAberration) ? 1.0f : 0.0f;
	const FBVector2d ca_dir = pData->ChromaticAberrationDirection;

	const double saturation = 1.0 + 0.01 * pData->Saturation;
	const double brightness = 1.0 + 0.01 * pData->Brightness;
	const double contrast = 1.0 + 0.01 * pData->Contrast;
	const double gamma = 0.01 * pData->Gamma;

	const float inverse = (pData->Inverse) ? 1.0f : 0.0f;
	const double hue = 0.01 * pData->Hue;
	const double hueSat = 0.01 * pData->HueSaturation;
	const double lightness = 0.01 * pData->Lightness;

	ShaderPropertyWriter writer(this, effectContext);
	writer(mChromaticAberration, static_cast<float>(ca_dir[0]), static_cast<float>(ca_dir[1]), 0.0f, chromatic_aberration)
		(mCSB, static_cast<float>(contrast), static_cast<float>(saturation), static_cast<float>(brightness), static_cast<float>(gamma))
		(mHue, static_cast<float>(hue), static_cast<float>(hueSat), static_cast<float>(lightness), inverse);

	return true;
}

bool EffectShaderColor::OnRenderPassBegin(int passIndex, PostEffectRenderContext& renderContext, PostEffectContextProxy* effectContext)
{
	const PostPersistentData* postData = effectContext->GetPostProcessData();
	if (!postData || !postData->Bloom)
		return true;

	constexpr const char* bufferName = "color_correction";
	static const uint32_t bufferNameKey = xxhash32(bufferName);

	PostEffectBuffers* buffers = renderContext.buffers;

	constexpr bool makeDownscale = false;
	const int outWidth = (makeDownscale) ? buffers->GetWidth() / 2 : buffers->GetWidth();
	const int outHeight = (makeDownscale) ? buffers->GetHeight() / 2 : buffers->GetHeight();
	constexpr int numColorAttachments = 2;

	FrameBuffer* pBuffer = buffers->RequestFramebuffer(bufferNameKey,
		outWidth, outHeight, PostEffectBuffers::GetFlagsForSingleColorBuffer(),
		numColorAttachments,
		false, [](FrameBuffer* frameBuffer) {
			PostEffectBuffers::SetParametersForMainColorBuffer(frameBuffer, false);
		});

	renderContext.targetFramebuffer = pBuffer;
	renderContext.colorAttachment = 0;
	return true;
}

void EffectShaderColor::OnRenderEnd(PostEffectRenderContext& renderContextParent, PostEffectContextProxy* effectContext)
{
	// render color into its own buffer
	constexpr const char* bufferName = "color_correction";
	static const uint32_t bufferNameKey = xxhash32(bufferName);

	const PostPersistentData* postData = effectContext->GetPostProcessData();
	PostEffectBuffers* buffers = renderContextParent.buffers;

	if (!postData || !buffers || !postData->Bloom)
		return;

	auto effectCollection = effectContext->GetEffectCollection();

	constexpr bool makeDownscale = false;
	const int outWidth = (makeDownscale) ? buffers->GetWidth() / 2 : buffers->GetWidth();
	const int outHeight = (makeDownscale) ? buffers->GetHeight() / 2 : buffers->GetHeight();
	constexpr int numColorAttachments = 2;

	FrameBuffer* pBuffer = buffers->RequestFramebuffer(bufferNameKey,
		outWidth, outHeight, PostEffectBuffers::GetFlagsForSingleColorBuffer(),
		numColorAttachments,
		false, [](FrameBuffer* frameBuffer) {
			PostEffectBuffers::SetParametersForMainColorBuffer(frameBuffer, false);
		});

	{
		auto shaderBlur = effectCollection->GetEffectBlurLinearDepth();

		PostEffectRenderContext renderContextBlur;
		renderContextBlur.buffers = buffers;
		renderContextBlur.targetFramebuffer = pBuffer;
		renderContextBlur.colorAttachment = 1;
		renderContextBlur.srcTextureId = pBuffer->GetColorObject(0); // renderContextParent.srcTextureId; //
		renderContextBlur.width = outWidth;
		renderContextBlur.height = outHeight;
		renderContextBlur.generateMips = false;

		const float color_shift = (postData->Bloom) ? static_cast<float>(0.01 * postData->BloomMinBright) : 0.0f;
		renderContextBlur.OverrideUniform(shaderBlur->GetPropertySchemePtr(), shaderBlur->mColorShift, color_shift);
		renderContextBlur.OverrideUniform(shaderBlur->GetPropertySchemePtr(), shaderBlur->mBlurSharpness, static_cast<float>(0.1 * postData->SSAO_BlurSharpness));
		renderContextBlur.OverrideUniform(shaderBlur->GetPropertySchemePtr(), shaderBlur->mInvRes, 1.0f / static_cast<float>(outWidth), 1.0f / static_cast<float>(outHeight));
		renderContextBlur.OverrideUniform(shaderBlur->GetPropertySchemePtr(), shaderBlur->mColorTexture, CommonEffect::ColorSamplerSlot);
		renderContextBlur.OverrideUniform(shaderBlur->GetPropertySchemePtr(), shaderBlur->mLinearDepthTexture, CommonEffect::LinearDepthSamplerSlot);

		shaderBlur->Render(renderContextBlur, effectContext);
	}

	// mix src texture with color corrected image
	
	const auto shaderMix = effectCollection->GetEffectMix();

	renderContextParent.OverrideUniform(shaderMix->GetPropertySchemePtr(), shaderMix->mBloom,
		static_cast<float>(0.01 * postData->BloomTone),
		static_cast<float>(0.01 * postData->BloomStretch),
		0.0f,
		1.0f);
	renderContextParent.OverrideUniform(shaderMix->GetPropertySchemePtr(), shaderMix->mColorSamplerA, CommonEffect::ColorSamplerSlot);
	renderContextParent.OverrideUniform(shaderMix->GetPropertySchemePtr(), shaderMix->mColorSamplerB, CommonEffect::UserSamplerSlot);

	glActiveTexture(GL_TEXTURE0 + CommonEffect::UserSamplerSlot);
	const uint32_t ccTextureId = pBuffer->GetColorObject(1);
	glBindTexture(GL_TEXTURE_2D, ccTextureId);
	glActiveTexture(GL_TEXTURE0);

	shaderMix->Render(renderContextParent, effectContext);

	glActiveTexture(GL_TEXTURE0 + CommonEffect::UserSamplerSlot);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
}