
/**	\file	posteffectvignetting.cxx

Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectvignetting.h"
#include "postpersistentdata.h"

#include "postprocessing_helper.h"

//! a constructor
EffectShaderVignetting::EffectShaderVignetting(FBComponent* ownerIn)
	: PostEffectBufferShader(ownerIn)
{
	MakeCommonProperties();
	
	AddProperty(IEffectShaderConnections::ShaderProperty("color", "colorSampler"))
		.SetType(IEffectShaderConnections::EPropertyType::TEXTURE)
		.SetValue(CommonEffect::ColorSamplerSlot);
	
	mAmount = &AddProperty(IEffectShaderConnections::ShaderProperty(PostPersistentData::VIGN_AMOUNT, "amount", nullptr))
		.SetScale(0.01f);
	VignOut = &AddProperty(IEffectShaderConnections::ShaderProperty(PostPersistentData::VIGN_OUT, "vignout", nullptr))
		.SetScale(0.01f);
	VignIn = &AddProperty(IEffectShaderConnections::ShaderProperty(PostPersistentData::VIGN_IN, "vignin", nullptr))
		.SetScale(0.01f);
	VignFade = &AddProperty(IEffectShaderConnections::ShaderProperty(PostPersistentData::VIGN_FADE, "vignfade", nullptr));
}

const char* EffectShaderVignetting::GetUseMaskingPropertyName() const noexcept
{
	return PostPersistentData::VIGN_USE_MASKING;
}
const char* EffectShaderVignetting::GetMaskingChannelPropertyName() const noexcept
{
	return PostPersistentData::VIGN_MASKING_CHANNEL;
}

bool EffectShaderVignetting::OnCollectUI(const IPostEffectContext* effectContext, int maskIndex)
{
	const PostPersistentData* data = effectContext->GetPostProcessData();

	const double amount = data->VignAmount;
	const double vignout = data->VignOut;
	const double vignin = data->VignIn;
	const double vignfade = data->VignFade;

	mAmount->SetValue(static_cast<float>(amount));
	VignOut->SetValue(static_cast<float>(vignout));
	VignIn->SetValue(static_cast<float>(vignin));
	VignFade->SetValue(static_cast<float>(vignfade));

	return true;
}
