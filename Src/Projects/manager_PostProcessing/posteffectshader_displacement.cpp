
// posteffectshader_displacement.cpp
/*
Sergei <Neill3d> Solokhin 2018-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectshader_displacement.h"
#include "postpersistentdata.h"
#include "shaderproperty_writer.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "postprocessing_helper.h"
#include <hashUtils.h>

uint32_t EffectShaderDisplacement::SHADER_NAME_HASH = xxhash32(EffectShaderDisplacement::SHADER_NAME);

EffectShaderDisplacement::EffectShaderDisplacement(FBComponent* ownerIn)
	: PostEffectBufferShader(ownerIn)
{}

const char* EffectShaderDisplacement::GetUseMaskingPropertyName() const noexcept
{
	return PostPersistentData::DISP_USE_MASKING;
}
const char* EffectShaderDisplacement::GetMaskingChannelPropertyName() const noexcept
{
	return PostPersistentData::DISP_MASKING_CHANNEL;
}

void EffectShaderDisplacement::OnPopulateProperties(ShaderPropertyScheme* scheme)
{
	// publish input connection of the effect
	//  input connections we can use to - look for locations, to read values from a given input data component, bind values from values into shader uniforms

	scheme->AddProperty("color", "inputSampler")
		.SetType(EPropertyType::TEXTURE)
		.SetFlag(PropertyFlag::SKIP)
		.SetDefaultValue(CommonEffect::ColorSamplerSlot);
	mTime = scheme->AddProperty("time", "iTime")
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();
	mUseQuakeEffect = scheme->AddProperty(PostPersistentData::DISP_USE_QUAKE_EFFECT, "useQuakeEffect")
		.SetFlag(PropertyFlag::IsFlag)
		.GetProxy();

	mXDistMag = scheme->AddProperty(PostPersistentData::DISP_MAGNITUDE_X, "xDistMag")
		.SetScale(0.0001f)
		.GetProxy();
	mYDistMag = scheme->AddProperty(PostPersistentData::DISP_MAGNITUDE_Y, "yDistMag")
		.SetScale(0.0001f)
		.GetProxy();
	mXSineCycles = scheme->AddProperty(PostPersistentData::DISP_SIN_CYCLES_X, "xSineCycles").GetProxy();
	mYSineCycles = scheme->AddProperty(PostPersistentData::DISP_SIN_CYCLES_Y, "ySineCycles").GetProxy();
}

bool EffectShaderDisplacement::OnCollectUI(PostEffectContextProxy* effectContext, int maskIndex) const
{
	const PostPersistentData* postProcess = effectContext->GetPostProcessData();
	if (!postProcess)
		return false;

	// this is a custom logic of updating uniform values

	const double time = (postProcess->Disp_UsePlayTime) ? effectContext->GetLocalTime() : effectContext->GetSystemTime();
	const double timerMult = postProcess->Disp_Speed;
	const double _timer = 0.01 * timerMult * time;

	ShaderPropertyWriter writer(this, effectContext);
	writer(mTime, static_cast<float>(_timer));
	
	return true;
}