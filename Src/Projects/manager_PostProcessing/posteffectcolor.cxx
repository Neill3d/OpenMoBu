
/**	\file	posteffectcolor.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectcolor.h"
#include "postpersistentdata.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "postprocessing_helper.h"


//! a constructor
EffectShaderColor::EffectShaderColor(FBComponent* ownerIn)
	: PostEffectBufferShader(ownerIn)
{
	MakeCommonProperties();

	AddProperty(IEffectShaderConnections::ShaderProperty("color", "sampler0"))
		.SetType(IEffectShaderConnections::EPropertyType::TEXTURE)
		.SetValue(CommonEffect::ColorSamplerSlot);

	mResolution = &AddProperty(IEffectShaderConnections::ShaderProperty("gResolution", "gResolution", IEffectShaderConnections::EPropertyType::VEC2))
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true); // NOTE: skip of automatic reading value and let it be done manually

	mChromaticAberration = &AddProperty(IEffectShaderConnections::ShaderProperty("gCA", "gCA", IEffectShaderConnections::EPropertyType::VEC4))
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true); // NOTE: skip of automatic reading value and let it be done manually

	mCSB = &AddProperty(IEffectShaderConnections::ShaderProperty("gCSB", "gCSB", IEffectShaderConnections::EPropertyType::VEC4))
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true); // NOTE: skip of automatic reading value and let it be done manually

	mHue = &AddProperty(IEffectShaderConnections::ShaderProperty("gHue", "gHue", IEffectShaderConnections::EPropertyType::VEC4))
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true); // NOTE: skip of automatic reading value and let it be done manually

}

const char* EffectShaderColor::GetUseMaskingPropertyName() const noexcept
{
	return PostPersistentData::COLOR_USE_MASKING;
}
const char* EffectShaderColor::GetMaskingChannelPropertyName() const noexcept
{
	return PostPersistentData::COLOR_MASKING_CHANNEL;
}

bool EffectShaderColor::OnCollectUI(const IPostEffectContext* effectContext, int maskIndex)
{
	const PostPersistentData* pData = effectContext->GetPostProcessData();

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

	mResolution->SetValue(static_cast<float>(effectContext->GetViewWidth()), static_cast<float>(effectContext->GetViewHeight()));
	mChromaticAberration->SetValue(static_cast<float>(ca_dir[0]), static_cast<float>(ca_dir[1]), 0.0f, chromatic_aberration);
	mCSB->SetValue(static_cast<float>(contrast), static_cast<float>(saturation), static_cast<float>(brightness), static_cast<float>(gamma));
	mHue->SetValue(static_cast<float>(hue), static_cast<float>(hueSat), static_cast<float>(lightness), inverse);
	return true;
}