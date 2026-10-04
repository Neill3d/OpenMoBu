
#pragma once

// posteffectshader_ssao
/*
Sergei <Neill3d> Solokhin 2018-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffect_shader.h"
#include <random>

/// <summary>
/// screen space ambient occlusion post processing effect
/// </summary>
struct EffectShaderSSAO : public PostEffectBufferShader
{
public:

	EffectShaderSSAO(FBComponent* ownerIn);
	virtual ~EffectShaderSSAO();

	[[nodiscard]] int GetNumberOfVariations() const noexcept override { return 1; }

	[[nodiscard]] const char* GetName() const noexcept override { return SHADER_NAME; }
	[[nodiscard]] uint32_t GetNameHash() const noexcept override { return SHADER_NAME_HASH; }
	[[nodiscard]] const char* GetVertexFname(const int shaderIndex) const noexcept override { return SHADER_VERTEX; }
	[[nodiscard]] const char* GetFragmentFname(const int shaderIndex) const noexcept override { return SHADER_FRAGMENT; }

	// does shader uses the scene linear depth sampler (part of a system input)
	virtual bool IsLinearDepthSamplerUsed() const override { return true; }

private:
	static constexpr const char* SHADER_NAME = "SSAO";
	static uint32_t SHADER_NAME_HASH;
	static constexpr const char* SHADER_VERTEX = "/GLSL/simple130.glslv";
	static constexpr const char* SHADER_FRAGMENT = "/GLSL/ssao.fsh";

protected:

	[[nodiscard]] virtual const char* GetUseMaskingPropertyName() const noexcept override;
	[[nodiscard]] virtual const char* GetMaskingChannelPropertyName() const noexcept override;

	// this is a predefined effect shader, properties are defined manually
	virtual bool DoPopulatePropertiesFromUniforms() const override { return false; }

	virtual void OnPopulateProperties(ShaderPropertyScheme* scheme) override;

	virtual bool OnCollectUI(PostEffectContextProxy* effectContext, int maskIndex) const override;

	virtual void RenderPass(int passIndex, PostEffectRenderContext& renderContext, PostEffectContextProxy* effectContext) override;

	//! bind effect shader program
	bool Bind() override;
	//! unbind effect shader program
	void UnBind() override;

private:

	ShaderPropertyProxy mProjInfo;
	ShaderPropertyProxy mProjOrtho;
	ShaderPropertyProxy mInvFullResolution;
	ShaderPropertyProxy mRadiusToScreen;
	
	ShaderPropertyProxy mNegInvR2;
	ShaderPropertyProxy mNDotVBias;

	ShaderPropertyProxy mAOMultiplier;
	ShaderPropertyProxy mPowExponent;

	ShaderPropertyProxy mOnlyAO;
	
	// texture Id
	GLuint	hbaoRandomTexId{ 0 };

	float		mRandom[4]{ 0.0f };
	

	std::random_device					rd;
	std::mt19937						e2;		// engine
	std::uniform_real_distribution<>	dist;	// distribution

	bool InitMisc();
	void DeleteTextures();
};