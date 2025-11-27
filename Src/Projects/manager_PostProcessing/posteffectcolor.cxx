
/**	\file	posteffectcolor.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectcolor.h"
#include "posteffectshader_mix.h"
#include "posteffectshader_blur_lineardepth.h"
#include "posteffectbuffers.h"
#include "postpersistentdata.h"
#include "shaderpropertywriter.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "postprocessing_helper.h"
#include "hashUtils.h"

////////////////////////////////////////////////////////////////////////////////
// PostEffectColor

PostEffectColor::PostEffectColor()
	: PostEffectBase()
	, mShaderColor(std::make_unique<EffectShaderColor>(nullptr)) // making it without an owner component
	, mShaderMix(std::make_unique<EffectShaderMix>(nullptr))
	, mShaderBlur(std::make_unique<EffectShaderBlurLinearDepth>(nullptr))
{
}

bool PostEffectColor::IsActive() const
{
	return true;
}

const char* PostEffectColor::GetName() const
{
	return mShaderColor->GetName();
}

PostEffectBufferShader* PostEffectColor::GetBufferShaderPtr(const int bufferShaderIndex)
{
	return static_cast<PostEffectBufferShader*>(mShaderColor.get());
}
const PostEffectBufferShader* PostEffectColor::GetBufferShaderPtr(const int bufferShaderIndex) const
{
	return static_cast<const PostEffectBufferShader*>(mShaderColor.get());
}

EffectShaderColor* PostEffectColor::GetBufferShaderTypedPtr()
{
	return mShaderColor.get();
}
const EffectShaderColor* PostEffectColor::GetBufferShaderTypedPtr() const
{
	return mShaderColor.get();
}

bool PostEffectColor::Load(const char* shaderLocation)
{
	if (!mShaderMix->Load(shaderLocation))
		return false;
	if (!mShaderBlur->Load(shaderLocation))
		return false;

	return PostEffectBase::Load(shaderLocation);
}

bool PostEffectColor::CollectUIValues(IPostEffectContext* effectContext)
{
	mShaderMix->CollectUIValues(effectContext, 0);
	mShaderBlur->CollectUIValues(effectContext, 0);

	return PostEffectBase::CollectUIValues(effectContext);
}

void PostEffectColor::Process(const PostEffectRenderContext& renderContext, const IPostEffectContext* effectContext)
{
	// render SSAO into its own buffer
	constexpr const char* bufferName = "color_correction";
	static const uint32_t bufferNameKey = xxhash32(bufferName);

	const PostPersistentData* postData = effectContext->GetPostProcessData();
	PostEffectBuffers* buffers = renderContext.buffers;

	if (postData && buffers && postData->Bloom)
	{
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

		PostEffectRenderContext renderContextColor;
		renderContextColor.buffers = buffers;
		renderContextColor.targetFramebuffer = pBuffer;
		renderContextColor.colorAttachment = 0;
		renderContextColor.srcTextureId = renderContext.srcTextureId;
		renderContextColor.width = outWidth;
		renderContextColor.height = outHeight;
		renderContextColor.generateMips = false;

		mShaderColor->Render(renderContextColor, effectContext);

		{
			PostEffectRenderContext renderContextBlur;
			renderContextBlur.buffers = buffers;
			renderContextBlur.targetFramebuffer = pBuffer;
			renderContextBlur.colorAttachment = 1;
			renderContextBlur.srcTextureId = pBuffer->GetColorObject(0);
			renderContextBlur.width = outWidth;
			renderContextBlur.height = outHeight;
			renderContextBlur.generateMips = false;

			const float color_shift = (postData->Bloom) ? static_cast<float>(0.01 * postData->BloomMinBright) : 0.0f;
			renderContextBlur.OverrideUniform(*mShaderBlur->mColorShift, color_shift);
			renderContextBlur.OverrideUniform(*mShaderBlur->mInvRes, 1.0f / static_cast<float>(outWidth), 1.0f / static_cast<float>(outHeight));

			mShaderBlur->Render(renderContextBlur, effectContext);
		}

		// mix src texture with color corrected image
		glActiveTexture(GL_TEXTURE0 + CommonEffect::UserSamplerSlot);
		const uint32_t ccTextureId = pBuffer->GetColorObject(1);
		glBindTexture(GL_TEXTURE_2D, ccTextureId);
		glActiveTexture(GL_TEXTURE0);

		mShaderMix->Render(renderContext, effectContext);

		glActiveTexture(GL_TEXTURE0 + CommonEffect::UserSamplerSlot);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);

		buffers->ReleaseFramebuffer(bufferNameKey);
	}
	else
	{
		PostEffectBase::Process(renderContext, effectContext);
	}
}

////////////////////////////////////////////////////////////////////////////////
// EffectShaderColor

uint32_t EffectShaderColor::SHADER_NAME_HASH = xxhash32(EffectShaderColor::SHADER_NAME);

EffectShaderColor::EffectShaderColor(FBComponent* ownerIn)
	: PostEffectBufferShader(ownerIn)
{
	MakeCommonProperties();

	AddProperty(ShaderProperty("color", "sampler0"))
		.SetType(EPropertyType::TEXTURE)
		.SetFlag(PropertyFlag::ShouldSkip, true)
		.SetDefaultValue(CommonEffect::ColorSamplerSlot);

	mChromaticAberration = &AddProperty(ShaderProperty("gCA", "gCA", EPropertyType::VEC4))
		.SetFlag(PropertyFlag::ShouldSkip, true); // NOTE: skip of automatic reading value and let it be done manually

	mCSB = &AddProperty(ShaderProperty("gCSB", "gCSB", EPropertyType::VEC4))
		.SetFlag(PropertyFlag::ShouldSkip, true); // NOTE: skip of automatic reading value and let it be done manually

	mHue = &AddProperty(ShaderProperty("gHue", "gHue", EPropertyType::VEC4))
		.SetFlag(PropertyFlag::ShouldSkip, true); // NOTE: skip of automatic reading value and let it be done manually

}

const char* EffectShaderColor::GetUseMaskingPropertyName() const noexcept
{
	return PostPersistentData::COLOR_USE_MASKING;
}
const char* EffectShaderColor::GetMaskingChannelPropertyName() const noexcept
{
	return PostPersistentData::COLOR_MASKING_CHANNEL;
}

bool EffectShaderColor::OnCollectUI(IPostEffectContext* effectContext, int maskIndex)
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

	//mChromaticAberration->SetValue(static_cast<float>(ca_dir[0]), static_cast<float>(ca_dir[1]), 0.0f, chromatic_aberration);
	//mCSB->SetValue(static_cast<float>(contrast), static_cast<float>(saturation), static_cast<float>(brightness), static_cast<float>(gamma));
	//mHue->SetValue(static_cast<float>(hue), static_cast<float>(hueSat), static_cast<float>(lightness), inverse);
	return true;
}