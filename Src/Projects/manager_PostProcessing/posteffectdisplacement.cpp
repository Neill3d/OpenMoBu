
// posteffectdisplacement.cpp
/*
Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectdisplacement.h"
#include "postpersistentdata.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "postprocessing_helper.h"


//! a constructor
EffectShaderDisplacement::EffectShaderDisplacement(FBComponent* ownerIn)
	: PostEffectBufferShader(ownerIn)
{
	MakeCommonProperties();
	// publish input connection of the effect
	//  input connections we can use to - look for locations, to read values from a given input data component, bind values from values into shader uniforms

	AddProperty(ShaderProperty("color", "inputSampler"))
		.SetType(EPropertyType::TEXTURE)
		.SetFlag(PropertyFlag::ShouldSkip, true)
		.SetValue(CommonEffect::ColorSamplerSlot);
	mTime = &AddProperty(ShaderProperty("time", "iTime", nullptr))
		.SetFlag(PropertyFlag::ShouldSkip, true); // NOTE: skip of automatic reading value and let it be done manually
	mSpeed = &AddProperty(ShaderProperty("speed", "iSpeed", nullptr))
		.SetFlag(PropertyFlag::ShouldSkip, true);
	mUseQuakeEffect = &AddProperty(ShaderProperty(PostPersistentData::DISP_USE_QUAKE_EFFECT, "useQuakeEffect", nullptr))
		.SetFlag(PropertyFlag::IsFlag, true);

	mXDistMag = &AddProperty(ShaderProperty(PostPersistentData::DISP_MAGNITUDE_X, "xDistMag", nullptr))
		.SetScale(0.0001f);
	mYDistMag = &AddProperty(ShaderProperty(PostPersistentData::DISP_MAGNITUDE_Y, "yDistMag", nullptr))
		.SetScale(0.0001f);
	mXSineCycles = &AddProperty(ShaderProperty(PostPersistentData::DISP_SIN_CYCLES_X, "xSineCycles", nullptr));
	mYSineCycles = &AddProperty(ShaderProperty(PostPersistentData::DISP_SIN_CYCLES_Y, "ySineCycles", nullptr));
}

const char* EffectShaderDisplacement::GetUseMaskingPropertyName() const noexcept
{
	return PostPersistentData::DISP_USE_MASKING;
}
const char* EffectShaderDisplacement::GetMaskingChannelPropertyName() const noexcept
{
	return PostPersistentData::DISP_MASKING_CHANNEL;
}

bool EffectShaderDisplacement::OnCollectUI(const IPostEffectContext* effectContext, int maskIndex)
{
	const PostPersistentData* postProcess = effectContext->GetPostProcessData();
	if (!postProcess)
		return false;

	// this is a custom logic of updating uniform values

	const double time = (postProcess->Disp_UsePlayTime) ? effectContext->GetLocalTime() : effectContext->GetSystemTime();
	const double timerMult = postProcess->Disp_Speed;
	const double _timer = 0.01 * timerMult * time;

	mTime->SetValue(static_cast<float>(_timer));
	mSpeed->SetValue(static_cast<float>(timerMult));

	return true;
}