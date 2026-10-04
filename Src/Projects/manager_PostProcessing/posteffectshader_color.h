#pragma once

// posteffect_color
/*
Sergei <Neill3d> Solokhin 2018-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffect_shader.h"

/// <summary>
/// color correction post processing effect
/// </summary>
class EffectShaderColor : public PostEffectBufferShader
{
public:
	
	EffectShaderColor(FBComponent* ownerIn);
	virtual ~EffectShaderColor() = default;

	int GetNumberOfVariations() const override { return 1; }
	int GetNumberOfPasses() const override { return 1; }

	const char* GetName() const override { return SHADER_NAME; }
	uint32_t GetNameHash() const override { return SHADER_NAME_HASH; }
	const char* GetVertexFname(const int shaderIndex) const override { return SHADER_VERTEX; }
	const char* GetFragmentFname(const int shaderIndex) const override { return SHADER_FRAGMENT; }

	// does shader uses the scene linear depth sampler (part of a system input)
	virtual bool IsLinearDepthSamplerUsed() const override { return true; }

private:
	static constexpr const char* SHADER_NAME = "Color Correction";
	static uint32_t SHADER_NAME_HASH;
	static constexpr const char* SHADER_VERTEX = "/GLSL/simple130.glslv";
	static constexpr const char* SHADER_FRAGMENT = "/GLSL/color.fsh";

protected:

	ShaderPropertyProxy mChromaticAberration;
	ShaderPropertyProxy mCSB;
	ShaderPropertyProxy mHue;
	
	[[nodiscard]] virtual const char* GetUseMaskingPropertyName() const noexcept override;
	[[nodiscard]] virtual const char* GetMaskingChannelPropertyName() const noexcept override;

	// this is a predefined effect shader, properties are defined manually
	bool DoPopulatePropertiesFromUniforms() const override { return false;  }

	virtual void OnPopulateProperties(ShaderPropertyScheme* scheme) override;

	virtual bool OnCollectUI(PostEffectContextProxy* effectContextProxy, int maskIndex) const override;

	virtual bool OnRenderPassBegin(int passIndex, PostEffectRenderContext& renderContext, PostEffectContextProxy* effectContext) override;

	// additional render passes in case of using bloom
	virtual void OnRenderEnd(PostEffectRenderContext& renderContextParent, PostEffectContextProxy* effectContext) override;
};
