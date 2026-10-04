
#pragma once

// posteffectshader_lensflare.h
/*
Sergei <Neill3d> Solokhin 2018-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffect_shader.h"

// forward
class EffectShaderLensFlare;

/// <summary>
/// effect with once shader - lens flare, output directly to effects chain dst buffer
/// </summary>
//using PostEffectLensFlare = PostEffectSingleShader<EffectShaderLensFlare>;

/// <summary>
/// lens flare post processing effect
/// </summary>
struct EffectShaderLensFlare : public PostEffectBufferShader
{
public:
	
	EffectShaderLensFlare(FBComponent* ownerIn);
	virtual ~EffectShaderLensFlare() = default;

	int GetNumberOfVariations() const override { return NUMBER_OF_SHADERS; }
	int GetNumberOfPasses() const override;

	[[nodiscard]] const char* GetName() const noexcept override { return SHADER_NAME; }
	uint32_t GetNameHash() const override { return SHADER_NAME_HASH; }
	[[nodiscard]] const char* GetVertexFname(const int shaderIndex) const noexcept override { return SHADER_VERTEX; }
	[[nodiscard]] const char* GetFragmentFname(const int shaderIndex) const noexcept override 
	{
		switch (shaderIndex)
		{
		case 1: return SHADER_BUBBLE_FRAGMENT;
		case 2: return SHADER_ANAMORPHIC_FRAGMENT;
		default: return SHADER_FRAGMENT;
		}
	}

	void OnRenderBegin(PostEffectRenderContext& renderContextParent, PostEffectContextProxy* effectContext) override;
	bool OnRenderPassBegin(int passIndex, PostEffectRenderContext& renderContext, PostEffectContextProxy* effectContext) override;

private:
	static const int NUMBER_OF_SHADERS{ 3 };
	static constexpr const char* SHADER_NAME = "Lens Flare";
	static uint32_t SHADER_NAME_HASH;
	static constexpr const char* SHADER_VERTEX = "/GLSL/simple130.glslv";
	static constexpr const char* SHADER_FRAGMENT = "/GLSL/lensFlare.fsh";
	static constexpr const char* SHADER_BUBBLE_FRAGMENT = "/GLSL/lensFlareBubble.fsh";
	static constexpr const char* SHADER_ANAMORPHIC_FRAGMENT = "/GLSL/lensFlareAnamorphic.fsh";

protected:

	[[nodiscard]] virtual const char* GetUseMaskingPropertyName() const noexcept override;
	[[nodiscard]] virtual const char* GetMaskingChannelPropertyName() const noexcept override;

	// this is a predefined effect shader, properties are defined manually
	virtual bool DoPopulatePropertiesFromUniforms() const override { return false; }

	virtual void OnPopulateProperties(ShaderPropertyScheme* scheme) override;

	virtual bool OnCollectUI(PostEffectContextProxy* effectContext, int maskIndex) const override;

private:

	ShaderPropertyProxy mFlareSeed;
	ShaderPropertyProxy mAmount;
	ShaderPropertyProxy mTime;
	ShaderPropertyProxy mLightPos; // vec3 array

	ShaderPropertyProxy mTint;
	ShaderPropertyProxy mInner;
	ShaderPropertyProxy mOuter;
	ShaderPropertyProxy mFadeToBorders;
	ShaderPropertyProxy mBorderWidth;
	ShaderPropertyProxy mFeather;

	struct SubShader
	{
	public:

		SubShader() = default;
		virtual ~SubShader() = default;

		float			m_DepthAttenuation{ 1.0f };

		std::vector<FBVector3d>	m_LightPositions; // window xy and depth (for attenuation)
		std::vector<FBColor>	m_LightColors;
		std::vector<float>		m_LightAlpha;

		bool CollectUIValues(int shaderIndex, PostEffectContextProxy* effectContext, int numberOfPasses, int maskIndex);
		
	private:
		void ProcessLightObjects(PostEffectContextProxy* effectContext, PostPersistentData* pData, FBCamera* pCamera, int numberOfPasses, int w, int h, double dt, FBTime systemTime, double* flarePos);
		void ProcessSingleLight(PostEffectContextProxy* effectContext, PostPersistentData* pData, FBCamera* pCamera, FBMatrix& mvp, int index, int w, int h, double dt, double* flarePos);
	};

	SubShader	subShaders[NUMBER_OF_SHADERS];
	
	mutable std::atomic<int> mNumberOfPasses{ 1 };

};
