
#pragma once

// posteffectdisplacement.h
/*
Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectsingleshader.h"

// forward
class EffectShaderDisplacement;

/// <summary>
/// effect with once shader - displacement, output directly to effects chain dst buffer
/// </summary>
using PostEffectDisplacement = PostEffectSingleShader<EffectShaderDisplacement>;


class EffectShaderDisplacement : public PostEffectBufferShader
{
private:
	static constexpr const char* SHADER_NAME = "Displacement";
	static uint32_t SHADER_NAME_HASH;
	static constexpr const char* SHADER_VERTEX = "/GLSL/simple130.glslv";
	static constexpr const char* SHADER_FRAGMENT = "/GLSL/displacement.fsh";
	
public:

	EffectShaderDisplacement(FBComponent* ownerIn);
	virtual ~EffectShaderDisplacement() = default;

	int GetNumberOfVariations() const override { return 1; }

	const char* GetName() const override { return SHADER_NAME; }
	uint32_t GetNameHash() const override { return SHADER_NAME_HASH; }
	const char* GetVertexFname(const int shaderIndex) const override { return SHADER_VERTEX; }
	const char* GetFragmentFname(const int shaderIndex) const override { return SHADER_FRAGMENT; }

protected:

	ShaderProperty* mTime;
	ShaderProperty* mUseQuakeEffect;
	ShaderProperty* mXDistMag;
	ShaderProperty* mYDistMag;
	ShaderProperty* mXSineCycles;
	ShaderProperty* mYSineCycles;

	[[nodiscard]] virtual const char* GetUseMaskingPropertyName() const noexcept override;
	[[nodiscard]] virtual const char* GetMaskingChannelPropertyName() const noexcept override;

	// this is a predefined effect shader, properties are defined manually
	virtual bool DoPopulatePropertiesFromUniforms() const override {
		return false;
	}

	virtual bool OnCollectUI(IPostEffectContext* effectContext, int maskIndex) override;

};

