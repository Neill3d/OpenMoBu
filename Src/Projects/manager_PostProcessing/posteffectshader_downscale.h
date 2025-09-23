
#pragma once

// posteffectshader_downscale
/*
Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectsingleshader.h"

// forward
class PostEffectShaderDownscale;

/// <summary>
/// effect with once shader - bilateral blur
/// </summary>
using PostEffectDownscale = PostEffectSingleShader<PostEffectShaderDownscale>;


/// <summary>
/// one single fragment shader that we do one number of passes to process the input
/// </summary>
class PostEffectShaderDownscale : public PostEffectBufferShader
{
public:

	PostEffectShaderDownscale(FBComponent* uiComponent = nullptr);
	virtual ~PostEffectShaderDownscale() = default;

	int GetNumberOfVariations() const override { return 1; }
	int GetNumberOfPasses() const override { return 1; }	

	const char* GetName() const override { return SHADER_NAME; }
	const char* GetVertexFname(const int variationIndex) const override { return SHADER_VERTEX; }
	const char* GetFragmentFname(const int variationIndex) const override { return SHADER_FRAGMENT; }

public:

	// properties
	ShaderProperty* mColorSampler;
	ShaderProperty* mTexelSize;

protected:
	
	const char* GetUseMaskingPropertyName() const override { return nullptr; }
	const char* GetMaskingChannelPropertyName() const override { return nullptr; }

	// this is a predefined effect shader, properties are defined manually
	virtual bool DoPopulatePropertiesFromUniforms() const override {
		return false;
	}

	//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
	virtual bool OnCollectUI(const IPostEffectContext* effectContext, int maskIndex) override;

private:
	static constexpr const char* SHADER_NAME = "Downscale";
	static constexpr const char* SHADER_VERTEX = "/GLSL/downscale.vsh";
	static constexpr const char* SHADER_FRAGMENT = "/GLSL/downscale.fsh";
};
