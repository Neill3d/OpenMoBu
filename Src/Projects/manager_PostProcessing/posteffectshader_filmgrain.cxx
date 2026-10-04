
/**	\file	posteffectshader_filmgrain.cxx

Sergei <Neill3d> Solokhin 2018-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectshader_filmgrain.h"
#include "postpersistentdata.h"
#include "shaderproperty_writer.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "postprocessing_helper.h"
#include <hashUtils.h>

uint32_t EffectShaderFilmGrain::SHADER_NAME_HASH = xxhash32(EffectShaderFilmGrain::SHADER_NAME);

EffectShaderFilmGrain::EffectShaderFilmGrain(FBComponent* ownerIn)
	: PostEffectBufferShader(ownerIn)
{}

const char* EffectShaderFilmGrain::GetUseMaskingPropertyName() const noexcept
{
	return PostPersistentData::GRAIN_USE_MASKING;
}
const char* EffectShaderFilmGrain::GetMaskingChannelPropertyName() const noexcept
{
	return PostPersistentData::GRAIN_MASKING_CHANNEL;
}

void EffectShaderFilmGrain::OnPopulateProperties(ShaderPropertyScheme* scheme)
{
	scheme->AddProperty("color", "sampler0")
		.SetType(EPropertyType::TEXTURE)
		.SetFlag(PropertyFlag::SKIP)
		.SetDefaultValue(CommonEffect::ColorSamplerSlot);

	mTimer = scheme->AddProperty("time", "timer", EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();
	mGrainAmount = scheme->AddProperty(PostPersistentData::GRAIN_AMOUNT, "grainamount")
		.SetScale(0.01f)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();
	mColored = scheme->AddProperty(PostPersistentData::GRAIN_COLORED, "colored")
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();
	mColorAmount = scheme->AddProperty(PostPersistentData::GRAIN_COLOR_AMOUNT, "coloramount")
		.SetScale(0.01f)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();
	mGrainSize = scheme->AddProperty(PostPersistentData::GRAIN_SIZE, "grainsize")
		.SetScale(0.01f)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();
	mLumAmount = scheme->AddProperty(PostPersistentData::GRAIN_LUMAMOUNT, "lumamount")
		.SetScale(0.01f)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();
}

bool EffectShaderFilmGrain::OnCollectUI(PostEffectContextProxy* effectContext, int maskIndex) const
{
	const PostPersistentData* pData = effectContext->GetPostProcessData();
	if (!pData)
		return false;

	const double time = (pData->FG_UsePlayTime) ? effectContext->GetLocalTime() : effectContext->GetSystemTime();

	const double timerMult = pData->FG_TimeSpeed;
	const double _timer = 0.01 * timerMult * time;

	const double _grainamount = pData->FG_GrainAmount;
	const double _colored = (pData->FG_Colored) ? 1.0 : 0.0;
	const double _coloramount = pData->FG_ColorAmount;
	const double _grainsize = pData->FG_GrainSize;
	const double _lumamount = pData->FG_LumAmount;

	ShaderPropertyWriter writer(this, effectContext);
	writer(mTimer, static_cast<float>(_timer))
		(mGrainAmount, static_cast<float>(_grainamount))
		(mColored, static_cast<float>(_colored))
		(mColorAmount, static_cast<float>(_coloramount))
		(mGrainSize, static_cast<float>(_grainsize))
		(mLumAmount, static_cast<float>(_lumamount));
	
	return true;
}