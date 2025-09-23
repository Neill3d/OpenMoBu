
#pragma once

// posteffectlensflare
/*
Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectsingleshader.h"

// forward
class EffectShaderLensFlare;

/// <summary>
/// effect with once shader - lens flare, output directly to effects chain dst buffer
/// </summary>
using PostEffectLensFlare = PostEffectSingleShader<EffectShaderLensFlare>;

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

	bool PrepPass(int pass, int width, int height) override;

private:
	static const int NUMBER_OF_SHADERS{ 3 };
	static constexpr const char* SHADER_NAME = "Lens Flare";
	static constexpr const char* SHADER_VERTEX = "/GLSL/simple130.glslv";
	static constexpr const char* SHADER_FRAGMENT = "/GLSL/lensFlare.fsh";
	static constexpr const char* SHADER_BUBBLE_FRAGMENT = "/GLSL/lensFlareBubble.fsh";
	static constexpr const char* SHADER_ANAMORPHIC_FRAGMENT = "/GLSL/lensFlareAnamorphic.fsh";

protected:

	[[nodiscard]] virtual const char* GetUseMaskingPropertyName() const noexcept override;
	[[nodiscard]] virtual const char* GetMaskingChannelPropertyName() const noexcept override;

	// this is a predefined effect shader, properties are defined manually
	virtual bool DoPopulatePropertiesFromUniforms() const override {
		return false;
	}

	virtual bool OnCollectUI(const IPostEffectContext* effectContext, int maskIndex) override;

private:

	ShaderProperty* mFlareSeed{ nullptr };
	ShaderProperty* mAmount{ nullptr };
	ShaderProperty* mTime{ nullptr };
	ShaderProperty* mLightPos{ nullptr }; // vec3 array

	ShaderProperty* mTint{ nullptr };
	ShaderProperty* mInner{ nullptr };
	ShaderProperty* mOuter{ nullptr };
	ShaderProperty* mFadeToBorders{ nullptr };
	ShaderProperty* mBorderWidth{ nullptr };
	ShaderProperty* mFeather{ nullptr };

	struct SubShader
	{
	public:

		SubShader() = default;
		virtual ~SubShader() = default;

		int				m_NumberOfPasses{ 1 };
		float			m_DepthAttenuation{ 1.0f };

		std::vector<FBVector3d>	m_LightPositions; // window xy and depth (for attenuation)
		std::vector<FBColor>	m_LightColors;
		std::vector<float>		m_LightAlpha;

		void Init();
		bool CollectUIValues(int shaderIndex, const IPostEffectContext* effectContext, int maskIndex);
		
	private:
		void ProcessLightObjects(PostPersistentData* pData, FBCamera* pCamera, int w, int h, double dt, FBTime systemTime, double* flarePos);
		void ProcessSingleLight(PostPersistentData* pData, FBCamera* pCamera, FBMatrix& mvp, int index, int w, int h, double dt, double* flarePos);
	};

	SubShader	subShaders[NUMBER_OF_SHADERS];
};
