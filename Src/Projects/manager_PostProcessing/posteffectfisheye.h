#pragma once

// posteffectfisheye
/*
Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectsingleshader.h"

// forward

class EffectShaderFishEye;

/// <summary>
/// effect with once shader - fish eye
/// </summary>
using PostEffectFishEye = PostEffectSingleShader<EffectShaderFishEye>;

/// <summary>
/// fish eye post processing effect
/// </summary>
class EffectShaderFishEye : public PostEffectBufferShader
{
private:
	static constexpr const char* SHADER_NAME = "Fish Eye";
	static constexpr const char* SHADER_VERTEX = "/GLSL/fishEye.vsh";
	static constexpr const char* SHADER_FRAGMENT = "/GLSL/fishEye.fsh";

public:

	//! a constructor
	EffectShaderFishEye(FBComponent* ownerIn);
	virtual ~EffectShaderFishEye() = default;

	int GetNumberOfVariations() const override { return 1; }

	const char* GetName() const override { return SHADER_NAME; }
	const char* GetVertexFname(const int shaderIndex) const override { return SHADER_VERTEX; }
	const char* GetFragmentFname(const int shaderIndex) const override { return SHADER_FRAGMENT; }

protected:

	ShaderProperty* mAmount;
	ShaderProperty* mLensRadius;
	ShaderProperty* mSignCurvature;

	[[nodiscard]] virtual const char* GetUseMaskingPropertyName() const noexcept override;
	[[nodiscard]] virtual const char* GetMaskingChannelPropertyName() const noexcept override;

	// this is a predefined effect shader, properties are defined manually
	virtual bool DoPopulatePropertiesFromUniforms() const override {
		return false;
	}

	virtual bool OnCollectUI(const IPostEffectContext* effectContext, int maskIndex) override;
};



