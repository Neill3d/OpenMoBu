#pragma once

// posteffectshader_filmgrain
/*
Sergei <Neill3d> Solokhin 2018-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffect_shader.h"

// forward
class EffectShaderFilmGrain;

/// <summary>
/// effect with once shader - displacement, output directly to effects chain dst buffer
/// </summary>
//using PostEffectFilmGrain = PostEffectSingleShader<EffectShaderFilmGrain>;


/// <summary>
/// film grain post processign effect
/// </summary>
class EffectShaderFilmGrain : public PostEffectBufferShader
{
public:

	EffectShaderFilmGrain(FBComponent* ownerIn);
	virtual ~EffectShaderFilmGrain() = default;

	int GetNumberOfVariations() const override { return 1; }

	const char* GetName() const override { return SHADER_NAME; }
	uint32_t GetNameHash() const override { return SHADER_NAME_HASH; }
	const char* GetVertexFname(const int shaderIndex) const override { return SHADER_VERTEX; }
	const char* GetFragmentFname(const int shaderIndex) const override { return SHADER_FRAGMENT; }

private:
	static constexpr const char* SHADER_NAME = "Film Grain";
	static uint32_t SHADER_NAME_HASH;
	static constexpr const char* SHADER_VERTEX = "/GLSL/simple130.glslv";
	static constexpr const char* SHADER_FRAGMENT = "/GLSL/filmGrain.fsh";

private:

	ShaderPropertyProxy mTimer;
	ShaderPropertyProxy mGrainAmount; //!< = 0.05; //grain amount
	ShaderPropertyProxy mColored; //!< = false; //colored noise?
	ShaderPropertyProxy mColorAmount; // = 0.6;
	ShaderPropertyProxy mGrainSize; // = 1.6; //grain particle size (1.5 - 2.5)
	ShaderPropertyProxy mLumAmount; // = 1.0; //

protected:
	[[nodiscard]] virtual const char* GetUseMaskingPropertyName() const noexcept override;
	[[nodiscard]] virtual const char* GetMaskingChannelPropertyName() const noexcept override;

	// this is a predefined effect shader, properties are defined manually
	virtual bool DoPopulatePropertiesFromUniforms() const override { return false; }

	virtual void OnPopulateProperties(ShaderPropertyScheme* scheme) override;

	virtual bool OnCollectUI(PostEffectContextProxy* effectContext, int maskIndex) const override;
};
