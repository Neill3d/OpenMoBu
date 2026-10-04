
// posteffectshader_motionblur.cpp
/*
Sergei <Neill3d> Solokhin 2018-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectshader_motionblur.h"
#include "postpersistentdata.h"
#include "shaderproperty_writer.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "postprocessing_helper.h"
#include <hashUtils.h>

uint32_t EffectShaderMotionBlur::SHADER_NAME_HASH = xxhash32(EffectShaderMotionBlur::SHADER_NAME);

EffectShaderMotionBlur::EffectShaderMotionBlur(FBComponent* ownerIn)
	: PostEffectBufferShader(ownerIn)
{}

const char* EffectShaderMotionBlur::GetUseMaskingPropertyName() const noexcept
{
	return PostPersistentData::MOTIONBLUR_USE_MASKING;
}
const char* EffectShaderMotionBlur::GetMaskingChannelPropertyName() const noexcept
{
	return PostPersistentData::MOTIONBLUR_MASKING_CHANNEL;
}

void EffectShaderMotionBlur::OnPopulateProperties(ShaderPropertyScheme* scheme)
{
	mDt = scheme->AddProperty("dt", "dt", EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();
}

bool EffectShaderMotionBlur::OnCollectUI(PostEffectContextProxy* effectContext, int maskIndex) const
{
	if (!effectContext->GetCamera() || !effectContext->GetPostProcessData())
		return false;

	const int localFrame = effectContext->GetLocalFrame(); 
	
	if (0 == localFrame || (localFrame != mLastLocalFrame))
	{
		ShaderPropertyWriter writer(this, effectContext);
		writer(mDt, static_cast<float>(effectContext->GetLocalTimeDT()));

		mLastLocalFrame = effectContext->GetLocalFrame();
	}

	return true;
}
