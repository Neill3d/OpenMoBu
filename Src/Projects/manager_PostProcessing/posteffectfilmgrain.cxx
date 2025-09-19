
/**	\file	posteffectfilmgrain.cxx

Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectfilmgrain.h"
#include "postpersistentdata.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "postprocessing_helper.h"

//! a constructor
EffectShaderFilmGrain::EffectShaderFilmGrain(FBComponent* ownerIn)
	: PostEffectBufferShader(ownerIn)
{
	MakeCommonProperties();

	AddProperty(ShaderProperty("color", "sampler0"))
		.SetType(EPropertyType::TEXTURE)
		.SetValue(CommonEffect::ColorSamplerSlot);

	mTimer = &AddProperty(ShaderProperty("time", "timer", EPropertyType::FLOAT))
		.SetFlag(PropertyFlag::ShouldSkip, true); // NOTE: skip of automatic reading value and let it be done manually

	mGrainAmount = &AddProperty(ShaderProperty(PostPersistentData::GRAIN_AMOUNT, "grainamount", nullptr))
		.SetScale(0.01f);
	mColored = &AddProperty(ShaderProperty(PostPersistentData::GRAIN_COLORED, "colored", nullptr));
	mColorAmount = &AddProperty(ShaderProperty(PostPersistentData::GRAIN_COLOR_AMOUNT, "coloramount", nullptr))
		.SetScale(0.01f);
	mGrainSize = &AddProperty(ShaderProperty(PostPersistentData::GRAIN_SIZE, "grainsize", nullptr))
		.SetScale(0.01f);
	mLumAmount = &AddProperty(ShaderProperty(PostPersistentData::GRAIN_LUMAMOUNT, "lumamount", nullptr))
		.SetScale(0.01f);
}

const char* EffectShaderFilmGrain::GetUseMaskingPropertyName() const noexcept
{
	return PostPersistentData::GRAIN_USE_MASKING;
}
const char* EffectShaderFilmGrain::GetMaskingChannelPropertyName() const noexcept
{
	return PostPersistentData::GRAIN_MASKING_CHANNEL;
}

bool EffectShaderFilmGrain::OnCollectUI(const IPostEffectContext* effectContext, int maskIndex)
{
	const PostPersistentData* pData = effectContext->GetPostProcessData();

	const double time = (pData->FG_UsePlayTime) ? effectContext->GetLocalTime() : effectContext->GetSystemTime();

	const double timerMult = pData->FG_TimeSpeed;
	const double _timer = 0.01 * timerMult * time;

	const double _grainamount = pData->FG_GrainAmount;
	const double _colored = (pData->FG_Colored) ? 1.0 : 0.0;
	const double _coloramount = pData->FG_ColorAmount;
	const double _grainsize = pData->FG_GrainSize;
	const double _lumamount = pData->FG_LumAmount;

	mTimer->SetValue(static_cast<float>(_timer));
	mGrainAmount->SetValue(static_cast<float>(_grainamount));
	mColored->SetValue(static_cast<float>(_colored));
	mColorAmount->SetValue(static_cast<float>(_coloramount));
	mGrainSize->SetValue(static_cast<float>(_grainsize));
	mLumAmount->SetValue(static_cast<float>(_lumamount));
	return true;
}