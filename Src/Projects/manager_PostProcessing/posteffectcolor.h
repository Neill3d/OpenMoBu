#pragma once

// posteffectcolor
/*
Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectsingleshader.h"

// forward
class EffectShaderColor;
class EffectShaderMix;
class EffectShaderBlurLinearDepth;

/// <summary>
/// effect with 3 shaders - Color Correction (with optional Bloom effect), output directly to effects chain dst buffer
/// </summary>
class PostEffectColor : public PostEffectBase
{
public:
	PostEffectColor();
	virtual ~PostEffectColor() = default;

	virtual bool IsActive() const override;
	virtual const char* GetName() const override;

	virtual int GetNumberOfBufferShaders() const override { return 1; }
	virtual PostEffectBufferShader* GetBufferShaderPtr(const int bufferShaderIndex) override;
	virtual const PostEffectBufferShader* GetBufferShaderPtr(const int bufferShaderIndex) const override;

	EffectShaderColor* GetBufferShaderTypedPtr();
	const EffectShaderColor* GetBufferShaderTypedPtr() const;

	virtual bool Load(const char* shaderLocation) override;

	virtual bool CollectUIValues(const IPostEffectContext* effectContext) override;

	virtual void Process(const RenderEffectContext& renderContext, const IPostEffectContext* effectContext) override;

protected:

	std::unique_ptr<EffectShaderColor>		mShaderColor;
	std::unique_ptr<EffectShaderMix>		mShaderMix;
	std::unique_ptr<EffectShaderBlurLinearDepth> mShaderBlur;
};


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
	const char* GetVertexFname(const int shaderIndex) const override { return SHADER_VERTEX; }
	const char* GetFragmentFname(const int shaderIndex) const override { return SHADER_FRAGMENT; }

private:
	static constexpr const char* SHADER_NAME = "Color Correction";
	static constexpr const char* SHADER_VERTEX = "/GLSL/simple130.glslv";
	static constexpr const char* SHADER_FRAGMENT = "/GLSL/color.fsh";

protected:

	ShaderProperty* mChromaticAberration{ nullptr };
	ShaderProperty* mCSB{ nullptr };
	ShaderProperty* mHue{ nullptr };
	
	[[nodiscard]] virtual const char* GetUseMaskingPropertyName() const noexcept override;
	[[nodiscard]] virtual const char* GetMaskingChannelPropertyName() const noexcept override;

	// this is a predefined effect shader, properties are defined manually
	bool DoPopulatePropertiesFromUniforms() const override { return false;  }

	virtual bool OnCollectUI(const IPostEffectContext* effectContext, int maskIndex) override;
};
