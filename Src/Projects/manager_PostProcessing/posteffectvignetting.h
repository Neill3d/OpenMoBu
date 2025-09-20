#pragma once

// posteffectvignetting
/*
Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectsingleshader.h"

// forward
class EffectShaderVignetting;

/// <summary>
/// effect with once shader - vignetting, output directly to effects chain dst buffer
/// </summary>
using PostEffectVignetting = PostEffectSingleShader<EffectShaderVignetting>;


/// <summary>
/// vignetting post processing effect
/// </summary>
class EffectShaderVignetting : public PostEffectBufferShader
{
private:
	static constexpr const char* SHADER_NAME = "Vignetting";
	static constexpr const char* SHADER_VERTEX = "/GLSL/simple130.glslv";
	static constexpr const char* SHADER_FRAGMENT = "/GLSL/vignetting.fsh";
	
public:

	//! a constructor
	EffectShaderVignetting(FBComponent* ownerIn);
	virtual ~EffectShaderVignetting() = default;

	[[nodiscard]] int GetNumberOfVariations() const noexcept override { return 1; }

	[[nodiscard]] const char* GetName() const noexcept override { return SHADER_NAME; }
	[[nodiscard]] const char* GetVertexFname(const int shaderIndex) const noexcept override { return SHADER_VERTEX; }
	[[nodiscard]] const char* GetFragmentFname(const int shaderIndex) const noexcept override { return SHADER_FRAGMENT; }

protected:

	ShaderProperty* mAmount;
	ShaderProperty* VignOut;
	ShaderProperty* VignIn;
	ShaderProperty* VignFade;

	[[nodiscard]] virtual const char* GetUseMaskingPropertyName() const noexcept override;
	[[nodiscard]] virtual const char* GetMaskingChannelPropertyName() const noexcept override;

	// this is a predefined effect shader, properties are defined manually
	virtual bool DoPopulatePropertiesFromUniforms() const override {
		return false;
	}

	virtual bool OnCollectUI(const IPostEffectContext* effectContext, int maskIndex) override;
};

