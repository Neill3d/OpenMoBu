
/**	\file	posteffectshader_fisheye.cxx

Sergei <Neill3d> Solokhin 2018-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectshader_fisheye.h"
#include "postpersistentdata.h"
#include "shaderproperty_writer.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "postprocessing_helper.h"
#include <hashUtils.h>

uint32_t EffectShaderFishEye::SHADER_NAME_HASH = xxhash32(EffectShaderFishEye::SHADER_NAME);

EffectShaderFishEye::EffectShaderFishEye(FBComponent* ownerIn)
	: PostEffectBufferShader(ownerIn)
{}

const char* EffectShaderFishEye::GetUseMaskingPropertyName() const noexcept
{
	return PostPersistentData::FISHEYE_USE_MASKING;
}
const char* EffectShaderFishEye::GetMaskingChannelPropertyName() const noexcept
{
	return PostPersistentData::FISHEYE_MASKING_CHANNEL;
}

void EffectShaderFishEye::OnPopulateProperties(ShaderPropertyScheme* scheme)
{
	scheme->AddProperty("color", "sampler0")
		.SetType(EPropertyType::TEXTURE)
		.SetFlag(PropertyFlag::SKIP)
		.SetDefaultValue(CommonEffect::ColorSamplerSlot);

	mAmount = scheme->AddProperty(PostPersistentData::FISHEYE_AMOUNT, "amount")
		.SetScale(0.01f)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();
	mLensRadius = scheme->AddProperty(PostPersistentData::FISHEYE_LENS_RADIUS, "lensradius")
		.SetScale(1.0f)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();
	mSignCurvature = scheme->AddProperty(PostPersistentData::FISHEYE_SIGN_CURV, "signcurvature")
		.SetScale(1.0f)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();
}

bool EffectShaderFishEye::OnCollectUI(PostEffectContextProxy* effectContext, int maskIndex) const
{
	const PostPersistentData* pData = effectContext->GetPostProcessData();
	if (!pData)
		return false;

	const double amount = pData->FishEyeAmount;
	const double lensradius = pData->FishEyeLensRadius;
	const double signcurvature = pData->FishEyeSignCurvature;

	ShaderPropertyWriter writer(this, effectContext);
	writer(mAmount, static_cast<float>(amount))
		(mLensRadius, static_cast<float>(lensradius))
		(mSignCurvature, static_cast<float>(signcurvature));
	
	return true;
}
