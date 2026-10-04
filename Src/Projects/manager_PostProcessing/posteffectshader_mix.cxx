
/**	\file	posteffectshader_mix.cxx

Sergei <Neill3d> Solokhin 2018-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectshader_mix.h"
#include "postpersistentdata.h"
#include "shaderproperty_writer.h"
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
{}

const char* EffectShaderMix::GetName() const
{
	if (FBIS(mUIComponent, EffectShaderMixUserObject))
	{
		EffectShaderMixUserObject* userObject = FBCast<EffectShaderMixUserObject>(mUIComponent);
		return userObject->LongName;
	}
	return SHADER_NAME;
}
uint32_t EffectShaderMix::GetNameHash() const
{
	if (FBIS(mUIComponent, EffectShaderMixUserObject))
	{
		EffectShaderMixUserObject* userObject = FBCast<EffectShaderMixUserObject>(mUIComponent);
		const char* longName = userObject->LongName;
		return xxhash32(longName);
	}
	return SHADER_NAME_HASH;
}

const char* EffectShaderMix::GetUseMaskingPropertyName() const
{
	if (FBIS(mUIComponent, EffectShaderMixUserObject))
	{
		EffectShaderMixUserObject* userObject = FBCast<EffectShaderMixUserObject>(mUIComponent);
		return userObject->UseMasking.GetName();
	}
	return nullptr;
}
const char* EffectShaderMix::GetMaskingChannelPropertyName() const
{
	if (FBIS(mUIComponent, EffectShaderMixUserObject))
	{
		EffectShaderMixUserObject* userObject = FBCast<EffectShaderMixUserObject>(mUIComponent);
		return userObject->MaskingChannel.GetName();
	}
	return nullptr;
}

void EffectShaderMix::OnPopulateProperties(ShaderPropertyScheme* scheme)
{
	if (FBIS(mUIComponent, EffectShaderMixUserObject))
	{
		EffectShaderMixUserObject* userObject = FBCast<EffectShaderMixUserObject>(mUIComponent);

		ShaderProperty texturePropertyA(EffectShaderMixUserObject::INPUT_TEXTURE_LABEL, "sampler0", EPropertyType::TEXTURE, &userObject->InputTexture);
		mColorSamplerA = scheme->AddProperty(std::move(texturePropertyA))
			.SetDefaultValue(CommonEffect::ColorSamplerSlot)
			//.SetFlag(PropertyFlag::ShouldSkip, true)
			.GetProxy();

		ShaderProperty texturePropertyB(EffectShaderMixUserObject::INPUT_TEXTURE_2_LABEL, "sampler1", EPropertyType::TEXTURE, &userObject->SecondTexture);
		mColorSamplerB = scheme->AddProperty(std::move(texturePropertyB))
			.SetDefaultValue(CommonEffect::UserSamplerSlot)
			//.SetFlag(PropertyFlag::ShouldSkip, true)
			.GetProxy();
	}
	else
	{
		mColorSamplerA = scheme->AddProperty(ShaderProperty("color0", "sampler0"))
			.SetType(EPropertyType::TEXTURE)
			.SetFlag(PropertyFlag::SKIP, true)
			.SetDefaultValue(CommonEffect::ColorSamplerSlot)
			.GetProxy();
		mColorSamplerB = scheme->AddProperty(ShaderProperty("color1", "sampler1"))
			.SetType(EPropertyType::TEXTURE)
			.SetFlag(PropertyFlag::SKIP, true)
			.SetDefaultValue(CommonEffect::UserSamplerSlot)
			.GetProxy();
	}

	mBloom = scheme->AddProperty(ShaderProperty("bloom", "gBloom"))
		.SetType(EPropertyType::VEC4)
		.SetFlag(PropertyFlag::SKIP, true)
		.GetProxy();
}

//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
bool EffectShaderMix::OnCollectUI(PostEffectContextProxy* effectContext, int maskIndex) const
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
		}
		else
		{
			writer(mBloom, 0.0f, 0.0f, 0.0f, 0.0f);
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
		}
		else
		{
			writer(mBloom, 0.0f, 0.0f, static_cast<float>(0.01 * userObject->Inverse), 0.0f);
		}
	}

	writer(mColorSamplerA, CommonEffect::ColorSamplerSlot,
		(mColorSamplerB, CommonEffect::UserSamplerSlot));
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
	FBPropertyPublish(this, Inverse, "Inverse", nullptr, nullptr);

	BloomMinBright.SetMinMax(0.0, 100.0);
	BloomTone.SetMinMax(0.0, 100.0);
	BloomStretch.SetMinMax(0.0, 100.0);
	Inverse.SetMinMax(0.0, 100.0);

	Bloom = false;
	BloomMinBright = 50.0;
	BloomTone = 100.0;
	BloomStretch = 100.0;
	Inverse = 0.0;

	VertexFile = EffectShaderMix::SHADER_VERTEX;
	VertexFile.ModifyPropertyFlag(FBPropertyFlag::kFBPropertyFlagReadOnly, true);
	FragmentFile = EffectShaderMix::SHADER_FRAGMENT;
	FragmentFile.ModifyPropertyFlag(FBPropertyFlag::kFBPropertyFlagReadOnly, true);
	NumberOfPasses.ModifyPropertyFlag(FBPropertyFlag::kFBPropertyFlagReadOnly, true);
	UniqueClassId = 73;

	return true;
}
