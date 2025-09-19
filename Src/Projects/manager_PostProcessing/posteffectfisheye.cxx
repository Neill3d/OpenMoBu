
/**	\file	posteffectfisheye.cxx

Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectfisheye.h"
#include "postpersistentdata.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "postprocessing_helper.h"

//! a constructor
EffectShaderFishEye::EffectShaderFishEye(FBComponent* ownerIn)
	: PostEffectBufferShader(ownerIn)
{
	MakeCommonProperties();

	AddProperty(ShaderProperty("color", "sampler0"))
		.SetType(EPropertyType::TEXTURE)
		.SetValue(CommonEffect::ColorSamplerSlot);

	mAmount = &AddProperty(ShaderProperty(PostPersistentData::FISHEYE_AMOUNT, "amount", nullptr))
		.SetScale(0.01f);
	mLensRadius = &AddProperty(ShaderProperty(PostPersistentData::FISHEYE_LENS_RADIUS, "lensradius", nullptr))
		.SetScale(1.0f);
	mSignCurvature = &AddProperty(ShaderProperty(PostPersistentData::FISHEYE_SIGN_CURV, "signcurvature", nullptr))
		.SetScale(1.0f);
}

const char* EffectShaderFishEye::GetUseMaskingPropertyName() const noexcept
{
	return PostPersistentData::FISHEYE_USE_MASKING;
}
const char* EffectShaderFishEye::GetMaskingChannelPropertyName() const noexcept
{
	return PostPersistentData::FISHEYE_MASKING_CHANNEL;
}

bool EffectShaderFishEye::OnCollectUI(const IPostEffectContext* effectContext, int maskIndex)
{
	const PostPersistentData* pData = effectContext->GetPostProcessData();

	const double amount = pData->FishEyeAmount;
	const double lensradius = pData->FishEyeLensRadius;
	const double signcurvature = pData->FishEyeSignCurvature;

	mAmount->SetValue(static_cast<float>(amount));
	mLensRadius->SetValue(static_cast<float>(lensradius));
	mSignCurvature->SetValue(static_cast<float>(signcurvature));
	
	return true;
}
