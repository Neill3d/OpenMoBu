
// posteffectmotionblur.cpp
/*
Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectmotionblur.h"
#include "postpersistentdata.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "postprocessing_helper.h"
#include <hashUtils.h>

uint32_t EffectShaderMotionBlur::SHADER_NAME_HASH = xxhash32(EffectShaderMotionBlur::SHADER_NAME);

EffectShaderMotionBlur::EffectShaderMotionBlur(FBComponent* ownerIn)
	: PostEffectBufferShader(ownerIn)
{
	MakeCommonProperties();

	mDt = &AddProperty(ShaderProperty("dt", "dt", EPropertyType::FLOAT))
		.SetFlag(PropertyFlag::ShouldSkip, true); // NOTE: skip of automatic reading value and let it be done manually
}

const char* EffectShaderMotionBlur::GetUseMaskingPropertyName() const noexcept
{
	return PostPersistentData::MOTIONBLUR_USE_MASKING;
}
const char* EffectShaderMotionBlur::GetMaskingChannelPropertyName() const noexcept
{
	return PostPersistentData::MOTIONBLUR_MASKING_CHANNEL;
}

bool EffectShaderMotionBlur::OnCollectUI(IPostEffectContext* effectContext, int maskIndex)
{
	if (!effectContext->GetCamera() || !effectContext->GetPostProcessData())
		return false;

	const int localFrame = effectContext->GetLocalFrame(); 
	
	if (0 == localFrame || (localFrame != mLastLocalFrame))
	{
		effectContext->GetShaderPropertyStorage()->WriteValue(GetNameHash(), *mDt, static_cast<float>(effectContext->GetLocalTimeDT()));
		//mDt->SetValue();
		mLastLocalFrame = effectContext->GetLocalFrame();
	}

	return true;
}
