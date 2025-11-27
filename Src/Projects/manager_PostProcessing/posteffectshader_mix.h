
#pragma once

// posteffectshader_mix
/*
Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectsingleshader.h"
#include "posteffect_shader_userobject.h"

// forward
class EffectShaderMix;

/// <summary>
/// effect with once shader - bilateral blur
/// </summary>
using PostEffectMix = PostEffectSingleShader<EffectShaderMix>;


/// <summary>
/// blend together 2 images with optional bloom effect
/// </summary>
class EffectShaderMix : public PostEffectBufferShader
{
public:

	EffectShaderMix(FBComponent* uiComponent = nullptr);
	virtual ~EffectShaderMix() = default;

	int GetNumberOfVariations() const override { return 1; }
	int GetNumberOfPasses() const override { return 1; }	

	const char* GetName() const override { return SHADER_NAME; }
	uint32_t GetNameHash() const override { return SHADER_NAME_HASH; }
	const char* GetVertexFname(const int variationIndex) const override { return SHADER_VERTEX; }
	const char* GetFragmentFname(const int variationIndex) const override { return SHADER_FRAGMENT; }

public:

	// properties
	ShaderProperty* mColorSamplerA{ nullptr };
	ShaderProperty* mColorSamplerB{ nullptr };
	ShaderProperty* mBloom{ nullptr };

protected:
	
	const char* GetUseMaskingPropertyName() const override { return nullptr; }
	const char* GetMaskingChannelPropertyName() const override { return nullptr; }

	// this is a predefined effect shader, properties are defined manually
	virtual bool DoPopulatePropertiesFromUniforms() const override {
		return false;
	}

	//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
	virtual bool OnCollectUI(IPostEffectContext* effectContext, int maskIndex) override;

private:
	static constexpr const char* SHADER_NAME = "Mix";
	static uint32_t SHADER_NAME_HASH;
	static constexpr const char* SHADER_VERTEX = "/GLSL/simple130.glslv";
	static constexpr const char* SHADER_FRAGMENT = "/GLSL/mix.fsh";

	FBComponent* mUIComponent{ nullptr };

	friend class EffectShaderMixUserObject;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// A user object for one shader user object
///  that is designed to be connected to post processing effect
/// </summary>
class EffectShaderMixUserObject : public EffectShaderUserObject
{
	//--- FiLMBOX declaration.
	FBClassDeclare(EffectShaderMixUserObject, EffectShaderUserObject)
	FBDeclareUserObject(EffectShaderMixUserObject)

public:
	static constexpr const char* INPUT_TEXTURE_LABEL = "Input Texture";
	static constexpr const char* INPUT_TEXTURE_2_LABEL = "Second Texture";
	
public:
	//! a constructor
	EffectShaderMixUserObject(const char* pName = nullptr, HIObject pObject = nullptr);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate() override;        //!< FiLMBOX Creation function.

public: // PROPERTIES

	FBPropertyListObject	InputTexture;
	FBPropertyListObject	SecondTexture;

	FBPropertyBool					Bloom;
	FBPropertyAnimatableDouble		BloomMinBright;
	FBPropertyAnimatableDouble		BloomTone;
	FBPropertyAnimatableDouble		BloomStretch;

protected:

	virtual PostEffectBufferShader* MakeANewClassInstance() override {
		return new EffectShaderMix(this);
	}

};
