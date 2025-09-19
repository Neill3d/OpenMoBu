#pragma once

// posteffectcolor
/*
Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectsingleshader.h"

// forward
class EffectShaderColor;

/// <summary>
/// effect with once shader - displacement, output directly to effects chain dst buffer
/// </summary>
using PostEffectColor = PostEffectSingleShader<EffectShaderColor>;


/// <summary>
/// color correction post processing effect
/// </summary>
class EffectShaderColor : public PostEffectBufferShader
{
private:
	static constexpr const char* SHADER_NAME = "Color Correction";
	static constexpr const char* SHADER_VERTEX = "/GLSL/simple130.glslv";
	static constexpr const char* SHADER_FRAGMENT = "/GLSL/color.fsh";
	
public:
	
	EffectShaderColor(FBComponent* ownerIn);
	virtual ~EffectShaderColor() = default;

	int GetNumberOfVariations() const override { return 1; }

	const char* GetName() const override { return SHADER_NAME; }
	const char* GetVertexFname(const int shaderIndex) const override { return SHADER_VERTEX; }
	const char* GetFragmentFname(const int shaderIndex) const override { return SHADER_FRAGMENT; }

protected:

	ShaderProperty* mChromaticAberration;
	ShaderProperty* mCSB;
	ShaderProperty* mHue;
	
	[[nodiscard]] virtual const char* GetUseMaskingPropertyName() const noexcept override;
	[[nodiscard]] virtual const char* GetMaskingChannelPropertyName() const noexcept override;

	// this is a predefined effect shader, properties are defined manually
	virtual bool DoPopulatePropertiesFromUniforms() const override {
		return false;
	}

	virtual bool OnCollectUI(const IPostEffectContext* effectContext, int maskIndex) override;
};
