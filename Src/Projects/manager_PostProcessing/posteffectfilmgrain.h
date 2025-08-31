#pragma once

// posteffectfilmgrain
/*
Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectsingleshader.h"

// forward
class EffectShaderFilmGrain;

/// <summary>
/// effect with once shader - displacement, output directly to effects chain dst buffer
/// </summary>
typedef PostEffectSingleShader<EffectShaderFilmGrain> PostEffectFilmGrain;


/// <summary>
/// film grain post processign effect
/// </summary>
class EffectShaderFilmGrain : public PostEffectBufferShader
{
private:
	static constexpr const char* SHADER_NAME = "Film Grain";
	static constexpr const char* SHADER_VERTEX = "/GLSL/simple130.glslv";
	static constexpr const char* SHADER_FRAGMENT = "/GLSL/filmGrain.fsh";

public:

	EffectShaderFilmGrain(FBComponent* ownerIn);
	virtual ~EffectShaderFilmGrain() = default;

	int GetNumberOfVariations() const override { return 1; }

	const char* GetName() const override { return SHADER_NAME; }
	const char* GetVertexFname(const int shaderIndex) const override { return SHADER_VERTEX; }
	const char* GetFragmentFname(const int shaderIndex) const override { return SHADER_FRAGMENT; }

protected:

	IEffectShaderConnections::ShaderProperty* mTextureWidth;
	IEffectShaderConnections::ShaderProperty* mTextureHeight;
	IEffectShaderConnections::ShaderProperty* mTimer;
	IEffectShaderConnections::ShaderProperty* mGrainAmount; //!< = 0.05; //grain amount
	IEffectShaderConnections::ShaderProperty* mColored; //!< = false; //colored noise?
	IEffectShaderConnections::ShaderProperty* mColorAmount; // = 0.6;
	IEffectShaderConnections::ShaderProperty* mGrainSize; // = 1.6; //grain particle size (1.5 - 2.5)
	IEffectShaderConnections::ShaderProperty* mLumAmount; // = 1.0; //

	[[nodiscard]] virtual const char* GetUseMaskingPropertyName() const noexcept override;
	[[nodiscard]] virtual const char* GetMaskingChannelPropertyName() const noexcept override;

	// this is a predefined effect shader, properties are defined manually
	virtual bool DoPopulatePropertiesFromUniforms() const override {
		return false;
	}

	virtual bool OnCollectUI(const IPostEffectContext* effectContext, int maskIndex) override;
};
