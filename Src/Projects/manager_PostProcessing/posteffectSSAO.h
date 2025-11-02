
#pragma once

// posteffectssao
/*
Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectsingleshader.h"
#include <random>

// forward
class EffectShaderSSAO;

/// <summary>
/// effect with once shader - Screen Space Ambient Occlusion, output directly to effects chain dst buffer
/// </summary>
using PostEffectSSAO = PostEffectSingleShader<EffectShaderSSAO>;

/// <summary>
/// screen space ambient occlusion post processing effect
/// </summary>
struct EffectShaderSSAO : public PostEffectBufferShader
{
private:
	static constexpr const char* SHADER_NAME = "SSAO";
	static constexpr const char* SHADER_VERTEX = "/GLSL/simple130.glslv";
	static constexpr const char* SHADER_FRAGMENT = "/GLSL/ssao.fsh";
	
public:

	EffectShaderSSAO(FBComponent* ownerIn);
	virtual ~EffectShaderSSAO();

	[[nodiscard]] int GetNumberOfVariations() const noexcept override { return 1; }

	[[nodiscard]] const char* GetName() const noexcept override { return SHADER_NAME; }
	[[nodiscard]] const char* GetVertexFname(const int shaderIndex) const noexcept override { return SHADER_VERTEX; }
	[[nodiscard]] const char* GetFragmentFname(const int shaderIndex) const noexcept override { return SHADER_FRAGMENT; }

	//virtual bool PrepUniforms(const int shaderIndex) override;
	//virtual bool CollectUIValues(PostPersistentData* pData, PostEffectContext& effectContext) override;

	//static GLint GetColorSamplerSlot() { return 0; }
	//static GLint GetDepthSamplerSlot() { return 2; }
	//static GLint GetRandomSamplerSlot() { return 5; }

protected:

	[[nodiscard]] virtual const char* GetUseMaskingPropertyName() const noexcept override;
	[[nodiscard]] virtual const char* GetMaskingChannelPropertyName() const noexcept override;

	// this is a predefined effect shader, properties are defined manually
	virtual bool DoPopulatePropertiesFromUniforms() const override {
		return false;
	}

	virtual bool OnCollectUI(const IPostEffectContext* effectContext, int maskIndex) override;

	//! bind effect shader program
	void Bind() override;
	//! unbind effect shader program
	void UnBind() override;

private:

	//ShaderProperty* mClipInfo;
	ShaderProperty* mProjInfo;
	ShaderProperty* mProjOrtho;
	//ShaderProperty* mInvQuarterResolution;
	ShaderProperty* mInvFullResolution;
	ShaderProperty* mRadiusToScreen;
	//ShaderProperty* mR2;
	ShaderProperty* mNegInvR2;
	ShaderProperty* mNDotVBias;

	ShaderProperty* mAOMultiplier;
	ShaderProperty* mPowExponent;

	ShaderProperty* mOnlyAO;
	//ShaderProperty* mHbaoRandom;

	// texture Id
	GLuint	hbaoRandomTexId;

	float		mRandom[4]{ 0.0f };
	

	std::random_device					rd;
	std::mt19937						e2;		// engine
	std::uniform_real_distribution<>	dist;	// distribution

	bool InitMisc();
	void DeleteTextures();
};