
/**	\file	posteffectvignetting.cxx

Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectvignetting.h"
#include "postpersistentdata.h"
#include "shaderpropertywriter.h"
#include "postprocessing_helper.h"
#include <hashUtils.h>

uint32_t EffectShaderVignetting::SHADER_NAME_HASH = xxhash32(EffectShaderVignetting::SHADER_NAME);

EffectShaderVignetting::EffectShaderVignetting(FBComponent* ownerIn)
	: PostEffectBufferShader(ownerIn)
{
	MakeCommonProperties();
	
	AddProperty(ShaderProperty("color", "colorSampler"))
		.SetType(EPropertyType::TEXTURE)
		.SetFlag(PropertyFlag::ShouldSkip, true)
		.SetDefaultValue(CommonEffect::ColorSamplerSlot);
	
	mAmount = &AddProperty(ShaderProperty(PostPersistentData::VIGN_AMOUNT, "amount", nullptr))
		.SetFlag(PropertyFlag::ShouldSkip, true)
		.SetScale(0.01f);
	VignOut = &AddProperty(ShaderProperty(PostPersistentData::VIGN_OUT, "vignout", nullptr))
		.SetFlag(PropertyFlag::ShouldSkip, true)
		.SetScale(0.01f);
	VignIn = &AddProperty(ShaderProperty(PostPersistentData::VIGN_IN, "vignin", nullptr))
		.SetFlag(PropertyFlag::ShouldSkip, true)
		.SetScale(0.01f);
	VignFade = &AddProperty(ShaderProperty(PostPersistentData::VIGN_FADE, "vignfade", nullptr))
		.SetFlag(PropertyFlag::ShouldSkip, true)
		.SetScale(-0.1f);
}

const char* EffectShaderVignetting::GetUseMaskingPropertyName() const noexcept
{
	return PostPersistentData::VIGN_USE_MASKING;
}
const char* EffectShaderVignetting::GetMaskingChannelPropertyName() const noexcept
{
	return PostPersistentData::VIGN_MASKING_CHANNEL;
}

bool EffectShaderVignetting::OnCollectUI(IPostEffectContext* effectContext, int maskIndex)
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
	/*
	mAmount->SetValue(static_cast<float>(amount));
	VignOut->SetValue(static_cast<float>(vignout));
	VignIn->SetValue(static_cast<float>(vignin));
	VignFade->SetValue(static_cast<float>(vignfade));
	*/
	return true;
}
