
/**	\file	posteffectshader_vignetting.cxx

Sergei <Neill3d> Solokhin 2018-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectshader_vignetting.h"
#include "postpersistentdata.h"
#include "shaderproperty_writer.h"
#include "postprocessing_helper.h"
#include <hashUtils.h>

uint32_t EffectShaderVignetting::SHADER_NAME_HASH = xxhash32(EffectShaderVignetting::SHADER_NAME);

EffectShaderVignetting::EffectShaderVignetting(FBComponent* ownerIn)
	: PostEffectBufferShader(ownerIn)
{}

const char* EffectShaderVignetting::GetUseMaskingPropertyName() const noexcept
{
	return PostPersistentData::VIGN_USE_MASKING;
}
const char* EffectShaderVignetting::GetMaskingChannelPropertyName() const noexcept
{
	return PostPersistentData::VIGN_MASKING_CHANNEL;
}

void EffectShaderVignetting::OnPopulateProperties(ShaderPropertyScheme* scheme)
{
	scheme->AddProperty("color", "colorSampler")
		.SetType(EPropertyType::TEXTURE)
		.SetFlag(PropertyFlag::SKIP)
		.SetDefaultValue(CommonEffect::ColorSamplerSlot);
	 
	mAmount = scheme->AddProperty(PostPersistentData::VIGN_AMOUNT, "amount")
		.SetFlag(PropertyFlag::SKIP)
		.SetScale(0.01f)
		.GetProxy();
	VignOut = scheme->AddProperty(PostPersistentData::VIGN_OUT, "vignout")
		.SetFlag(PropertyFlag::SKIP)
		.SetScale(0.01f)
		.GetProxy();
	VignIn = scheme->AddProperty(PostPersistentData::VIGN_IN, "vignin")
		.SetFlag(PropertyFlag::SKIP)
		.SetScale(0.01f)
		.GetProxy();
	VignFade = scheme->AddProperty(PostPersistentData::VIGN_FADE, "vignfade")
		.SetFlag(PropertyFlag::SKIP)
		.SetScale(-0.1f)
		.GetProxy();
}

bool EffectShaderVignetting::OnCollectUI(PostEffectContextProxy* effectContext, int maskIndex) const
{
	const PostPersistentData* data = effectContext->GetPostProcessData();
	if (!data)
		return false;

	const double amount = data->VignAmount;
	const double vignout = data->VignOut;
	const double vignin = data->VignIn;
	const double vignfade = data->VignFade;

	ShaderPropertyWriter writer(this, effectContext);

	writer(mAmount, static_cast<float>(amount))
		(VignOut, static_cast<float>(vignout))
		(VignIn, static_cast<float>(vignin))
		(VignFade, static_cast<float>(vignfade));
	return true;
}
