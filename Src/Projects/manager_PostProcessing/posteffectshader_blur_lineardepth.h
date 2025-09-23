
#pragma once

// posteffectshader_blur_lineardepth
/*
Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectsingleshader.h"

// forward
class PostEffectShaderBlurLinearDepth;

/// <summary>
/// effect with once shader - bilateral blur
/// </summary>
using PostEffectBlurLinearDepth = PostEffectSingleShader<PostEffectShaderBlurLinearDepth>;


/// <summary>
/// one single fragment shader that we do one number of passes to process the input
/// </summary>
class PostEffectShaderBlurLinearDepth : public PostEffectBufferShader
{
public:

	PostEffectShaderBlurLinearDepth(FBComponent* uiComponent);
	virtual ~PostEffectShaderBlurLinearDepth() = default;

	int GetNumberOfVariations() const override { return 1; }
	int GetNumberOfPasses() const override { return 1; }

	//! an effect public name
	const char* GetName() const override { return SHADER_NAME; }
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

protected:
	static constexpr const char* SHADER_NAME = "Blur w/th LinearDepth";
	static constexpr const char* VERTEX_SHADER_FILE = "/GLSL/simple130.glslv";
	static constexpr const char* FRAGMENT_SHADER_FILE = "/GLSL/blur.fsh";

	const char* GetUseMaskingPropertyName() const override { return nullptr; }
	const char* GetMaskingChannelPropertyName() const override { return nullptr; }

	// this is a predefined effect shader, properties are defined manually
	bool DoPopulatePropertiesFromUniforms() const override {
		return false;
	}

	//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
	bool OnCollectUI(const IPostEffectContext* effectContext, int maskIndex) override;
};
