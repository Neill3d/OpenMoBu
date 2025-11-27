
#pragma once

// posteffectshader_blur_lineardepth
/*
Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectsingleshader.h"

// forward
class EffectShaderBlurLinearDepth;

/// <summary>
/// effect with once shader - bilateral blur
/// </summary>
using PostEffectBlurLinearDepth = PostEffectSingleShader<EffectShaderBlurLinearDepth>;


/// <summary>
/// one single fragment shader that we do one number of passes to process the input
/// </summary>
class EffectShaderBlurLinearDepth : public PostEffectBufferShader
{
public:

	EffectShaderBlurLinearDepth(FBComponent* uiComponent);
	virtual ~EffectShaderBlurLinearDepth() = default;

	int GetNumberOfVariations() const override { return 1; }
	int GetNumberOfPasses() const override { return 1; }

	//! an effect public name
	const char* GetName() const override { return SHADER_NAME; }
	uint32_t GetNameHash() const override { return SHADER_NAME_HASH; }
	//! get a filename of vertex shader, for this effect. returns a relative filename
	const char* GetVertexFname(const int variationIndex) const override { return VERTEX_SHADER_FILE; }
	//! get a filename of a fragment shader, for this effect, returns a relative filename
	const char* GetFragmentFname(const int variationIndex) const override { return FRAGMENT_SHADER_FILE; }

public:

	// properties
	ShaderProperty* mColorTexture{ nullptr };
	ShaderProperty* mLinearDepthTexture{ nullptr };
	ShaderProperty* mBlurSharpness{ nullptr };
	ShaderProperty* mInvRes{ nullptr };
	ShaderProperty* mColorShift{ nullptr };

protected:
	static constexpr const char* SHADER_NAME = "Blur w/th LinearDepth";
	static uint32_t SHADER_NAME_HASH;
	static constexpr const char* VERTEX_SHADER_FILE = "/GLSL/simple130.glslv";
	static constexpr const char* FRAGMENT_SHADER_FILE = "/GLSL/blur.fsh";

	const char* GetUseMaskingPropertyName() const override { return nullptr; }
	const char* GetMaskingChannelPropertyName() const override { return nullptr; }

	// this is a predefined effect shader, properties are defined manually
	bool DoPopulatePropertiesFromUniforms() const override { return false; }

	//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
	bool OnCollectUI(IPostEffectContext* effectContext, int maskIndex) override;
};
