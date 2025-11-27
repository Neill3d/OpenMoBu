
#pragma once

// posteffectshader_lineardepth
/*
Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectsingleshader.h"

// forward
class PostEffectShaderLinearDepth;

/// <summary>
/// effect with once shader - linear depth extraction
/// </summary>
using PostEffectLinearDepth = PostEffectSingleShader<PostEffectShaderLinearDepth>;


/// <summary>
/// one single fragment shader that we do one number of passes to process the input
/// </summary>
class PostEffectShaderLinearDepth : public PostEffectBufferShader
{
public:

	PostEffectShaderLinearDepth(FBComponent* uiComponent = nullptr);
	virtual ~PostEffectShaderLinearDepth() = default;

	int GetNumberOfVariations() const override { return 1; }
	int GetNumberOfPasses() const override { return 1; }

	const char* GetName() const override { return SHADER_NAME; }
	uint32_t GetNameHash() const override { return SHADER_NAME_HASH; }
	const char* GetVertexFname(const int variationIndex) const override { return VERTEX_SHADER_FILE; }
	const char* GetFragmentFname(const int variationIndex) const override { return FRAGMENT_SHADER_FILE; }

public:

	// properties
	ShaderProperty* mClipInfo;

protected:
	static constexpr const char* SHADER_NAME = "LinearDepth";
	static const uint32_t SHADER_NAME_HASH;
	static constexpr const char* VERTEX_SHADER_FILE = "/GLSL/simple130.glslv";
	static constexpr const char* FRAGMENT_SHADER_FILE = "/GLSL/depthLinearize.fsh";

	const char* GetUseMaskingPropertyName() const override { return nullptr; }
	const char* GetMaskingChannelPropertyName() const override { return nullptr; }

	// this is a predefined effect shader, properties are defined manually
	virtual bool DoPopulatePropertiesFromUniforms() const override {
		return false;
	}

	//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
	virtual bool OnCollectUI(IPostEffectContext* effectContext, int maskIndex) override;
};
