
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
class EffectShaderMix;
class EffectShaderBlurLinearDepth;

/// <summary>
/// effect with 3 shaders - Screen Space Ambient Occlusion, output directly to effects chain dst buffer
/// </summary>
class PostEffectSSAO : public PostEffectBase
{
public:
	PostEffectSSAO();
	virtual ~PostEffectSSAO() = default;

	virtual bool IsActive() const override;
	virtual const char* GetName() const override;

	virtual int GetNumberOfBufferShaders() const override { return 1; }
	virtual PostEffectBufferShader* GetBufferShaderPtr(const int bufferShaderIndex) override;
	virtual const PostEffectBufferShader* GetBufferShaderPtr(const int bufferShaderIndex) const override;

	EffectShaderSSAO* GetBufferShaderTypedPtr();
	const EffectShaderSSAO* GetBufferShaderTypedPtr() const;

	virtual bool Load(const char* shaderLocation) override;

	virtual bool CollectUIValues(IPostEffectContext* effectContext) override;

	virtual void Process(PostEffectRenderContext& renderContext, IPostEffectContext* effectContext) override;

protected:

	std::unique_ptr<EffectShaderSSAO>		mShaderSSAO;
	std::unique_ptr<EffectShaderMix>		mShaderMix;
	std::unique_ptr<EffectShaderBlurLinearDepth> mShaderBlur;
};

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

private:
	static constexpr const char* SHADER_NAME = "SSAO";
	static uint32_t SHADER_NAME_HASH;
	static constexpr const char* SHADER_VERTEX = "/GLSL/simple130.glslv";
	static constexpr const char* SHADER_FRAGMENT = "/GLSL/ssao.fsh";

protected:

	[[nodiscard]] virtual const char* GetUseMaskingPropertyName() const noexcept override;
	[[nodiscard]] virtual const char* GetMaskingChannelPropertyName() const noexcept override;

	// this is a predefined effect shader, properties are defined manually
	virtual bool DoPopulatePropertiesFromUniforms() const override {
		return false;
	}

	virtual bool OnCollectUI(IPostEffectContext* effectContext, int maskIndex) override;

	//! bind effect shader program
	bool Bind() override;
	//! unbind effect shader program
	void UnBind() override;

private:

	ShaderProperty* mProjInfo;
	ShaderProperty* mProjOrtho;
	ShaderProperty* mInvFullResolution;
	ShaderProperty* mRadiusToScreen;
	
	ShaderProperty* mNegInvR2;
	ShaderProperty* mNDotVBias;

	ShaderProperty* mAOMultiplier;
	ShaderProperty* mPowExponent;

	ShaderProperty* mOnlyAO;
	
	// texture Id
	GLuint	hbaoRandomTexId;

	float		mRandom[4]{ 0.0f };
	

	std::random_device					rd;
	std::mt19937						e2;		// engine
	std::uniform_real_distribution<>	dist;	// distribution

	bool InitMisc();
	void DeleteTextures();
};