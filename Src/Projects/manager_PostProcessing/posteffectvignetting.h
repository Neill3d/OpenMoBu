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
public:

	EffectShaderVignetting(FBComponent* ownerIn);
	virtual ~EffectShaderVignetting() = default;

	[[nodiscard]] int GetNumberOfVariations() const noexcept override { return 1; }

	[[nodiscard]] const char* GetName() const noexcept override { return SHADER_NAME; }
	uint32_t GetNameHash() const override { return SHADER_NAME_HASH; }
	[[nodiscard]] const char* GetVertexFname(const int shaderIndex) const noexcept override { return SHADER_VERTEX; }
	[[nodiscard]] const char* GetFragmentFname(const int shaderIndex) const noexcept override { return SHADER_FRAGMENT; }

private:
	static constexpr const char* SHADER_NAME = "Vignetting";
	static uint32_t SHADER_NAME_HASH;
	static constexpr const char* SHADER_VERTEX = "/GLSL/simple130.glslv";
	static constexpr const char* SHADER_FRAGMENT = "/GLSL/vignetting.fsh";

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

	virtual bool OnCollectUI(IPostEffectContext* effectContext, int maskIndex) override;
};

