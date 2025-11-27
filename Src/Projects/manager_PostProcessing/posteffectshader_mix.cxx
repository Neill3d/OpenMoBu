
/**	\file	posteffectshader_mix.cxx

Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectshader_mix.h"
#include "postpersistentdata.h"
#include "shaderpropertywriter.h"
#include "mobu_logging.h"
#include <hashUtils.h>

//--- FiLMBOX Registration & Implementation.
FBClassImplementation(EffectShaderMixUserObject);
FBUserObjectImplement(EffectShaderMixUserObject,
	"Mix two images with optional bloom effect",
	"cam_switcher_toggle.png");                                          //Register UserObject class
PostEffectFBElementClassImplementation(EffectShaderMixUserObject, "Mix Shader", "cam_switcher_toggle.png");                  //Register to the asset system


/////////////////////////////////////////////////////////////////////////
// PostEffectShaderMix

uint32_t EffectShaderMix::SHADER_NAME_HASH = xxhash32(EffectShaderMix::SHADER_NAME);

EffectShaderMix::EffectShaderMix(FBComponent* uiComponent)
	: PostEffectBufferShader(uiComponent)
	, mUIComponent(uiComponent)
{
	if (FBIS(uiComponent, EffectShaderMixUserObject))
	{
		EffectShaderMixUserObject* userObject = FBCast<EffectShaderMixUserObject>(uiComponent);

		ShaderProperty texturePropertyA(EffectShaderMixUserObject::INPUT_TEXTURE_LABEL, "sampler0", EPropertyType::TEXTURE, &userObject->InputTexture);
		mColorSamplerA = &AddProperty(std::move(texturePropertyA))
			.SetDefaultValue(CommonEffect::ColorSamplerSlot)
			.SetFlag(PropertyFlag::ShouldSkip, true);

		ShaderProperty texturePropertyB(EffectShaderMixUserObject::INPUT_TEXTURE_2_LABEL, "sampler1", EPropertyType::TEXTURE, &userObject->SecondTexture);
		mColorSamplerB = &AddProperty(std::move(texturePropertyB))
			.SetDefaultValue(CommonEffect::UserSamplerSlot)
			.SetFlag(PropertyFlag::ShouldSkip, true);
	}
	else
	{
		mColorSamplerA = &AddProperty(ShaderProperty("color", "sampler0"))
			.SetType(EPropertyType::TEXTURE)
			.SetFlag(PropertyFlag::ShouldSkip, true)
			.SetDefaultValue(CommonEffect::ColorSamplerSlot);
		mColorSamplerB = &AddProperty(ShaderProperty("color", "sampler1"))
			.SetType(EPropertyType::TEXTURE)
			.SetFlag(PropertyFlag::ShouldSkip, true)
			.SetDefaultValue(CommonEffect::UserSamplerSlot);
	}

	mBloom = &AddProperty(ShaderProperty("bloom", "gBloom"))
		.SetType(EPropertyType::VEC4)
		.SetFlag(PropertyFlag::ShouldSkip, true);
}

//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
bool EffectShaderMix::OnCollectUI(IPostEffectContext* effectContext, int maskIndex)
{
	ShaderPropertyWriter writer(this, effectContext);

	if (!mUIComponent)
	{
		// collect from the main post process user object

		PostPersistentData* data = effectContext->GetPostProcessData();
		if (data && data->Bloom)
		{
			writer(mBloom,
				static_cast<float>(0.01 * data->BloomTone),
				static_cast<float>(0.01 * data->BloomStretch),
				0.0f,
				1.0f);
			//mBloom->SetValue(static_cast<float>(0.01 * data->BloomTone),
			//	static_cast<float>(0.01 * data->BloomStretch),
			//	0.0f, 
			//	1.0f);
		}
		else
		{
			writer(mBloom, 0.0f, 0.0f, 0.0f, 0.0f);
			//mBloom->SetValue(0.0f, 0.0f, 0.0f, 0.0f);
		}
	}
	else
	{
		// collect from the specific effect shader user object
		EffectShaderMixUserObject* userObject = FBCast<EffectShaderMixUserObject>(mUIComponent);
		if (userObject && userObject->Bloom)
		{
			writer(mBloom,
				static_cast<float>(0.01 * userObject->BloomTone),
				static_cast<float>(0.01 * userObject->BloomStretch),
				0.0f,
				1.0f);
			//mBloom->SetValue(static_cast<float>(0.01 * userObject->BloomTone),
			//	static_cast<float>(0.01 * userObject->BloomStretch),
			//	0.0f,
			//	1.0f);
		}
		else
		{
			writer(mBloom, 0.0f, 0.0f, 0.0f, 0.0f);
			//mBloom->SetValue(0.0f, 0.0f, 0.0f, 0.0f);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// EffectShaderMixUserObject

EffectShaderMixUserObject::EffectShaderMixUserObject(const char* pName, HIObject pObject)
	: ParentClass(pName, pObject)
{
	FBClassInit;
}

/************************************************
 *  FiLMBOX Constructor.
 ************************************************/
bool EffectShaderMixUserObject::FBCreate()
{
	ParentClass::FBCreate();

	FBPropertyPublish(this, InputTexture, INPUT_TEXTURE_LABEL, nullptr, nullptr);
	FBPropertyPublish(this, SecondTexture, INPUT_TEXTURE_2_LABEL, nullptr, nullptr);
	
	FBPropertyPublish(this, Bloom, PostPersistentData::BLOOM, nullptr, nullptr);
	FBPropertyPublish(this, BloomMinBright, PostPersistentData::BLOOM_MIN_BRIGHT, nullptr, nullptr);
	FBPropertyPublish(this, BloomTone, PostPersistentData::BLOOM_TONE, nullptr, nullptr);
	FBPropertyPublish(this, BloomStretch, PostPersistentData::BLOOM_STRETCH, nullptr, nullptr);

	BloomMinBright.SetMinMax(0.0, 100.0);
	BloomTone.SetMinMax(0.0, 100.0);
	BloomStretch.SetMinMax(0.0, 100.0);

	Bloom = false;
	BloomMinBright = 50.0;
	BloomTone = 100.0;
	BloomStretch = 100.0;

	ShaderFile = EffectShaderMix::SHADER_FRAGMENT;
	ShaderFile.ModifyPropertyFlag(FBPropertyFlag::kFBPropertyFlagReadOnly, true);
	NumberOfPasses.ModifyPropertyFlag(FBPropertyFlag::kFBPropertyFlagReadOnly, true);
	UniqueClassId = 73;

	return true;
}
