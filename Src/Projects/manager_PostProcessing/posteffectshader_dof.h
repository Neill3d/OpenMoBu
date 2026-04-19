#pragma once

// posteffectshader_dof.h
/*
Sergei <Neill3d> Solokhin 2018-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffect_shader.h"
#include <random>

// forward
class EffectShaderDOF;

/// <summary>
/// Depth of field post-processing effect with single shader pipeline
/// Outputs directly to effects chain destination buffer
/// </summary>
//using PostEffectDOF = PostEffectSingleShader<EffectShaderDOF>;


/// <summary>
/// Depth of field post-processing shader effect implementation
/// Provides realistic camera depth of field simulation with configurable parameters
/// </summary>
class EffectShaderDOF : public PostEffectBufferShader
{
public:

	explicit EffectShaderDOF(FBComponent* ownerIn);
    virtual ~EffectShaderDOF();

	int GetNumberOfVariations() const override { return 1; }

	const char* GetName() const override { return SHADER_NAME; }
    uint32_t GetNameHash() const override { return SHADER_NAME_HASH; }
	const char* GetVertexFname(const int shaderIndex) const override { return SHADER_VERTEX; }
	const char* GetFragmentFname(const int shaderIndex) const override { return SHADER_FRAGMENT; }

protected:

    [[nodiscard]] virtual const char* GetUseMaskingPropertyName() const noexcept override;
    [[nodiscard]] virtual const char* GetMaskingChannelPropertyName() const noexcept override;
    // This is a predefined effect shader, properties are defined manually
    virtual bool DoPopulatePropertiesFromUniforms() const override { return false; }

    virtual void OnPopulateProperties(ShaderPropertyScheme* scheme) override;

	virtual bool OnCollectUI(PostEffectContextProxy* effectContext, int maskIndex) const override;

    //! bind effect shader program
    bool Bind() override;
    //! unbind effect shader program
    void UnBind() override;

private:
	static constexpr const char* SHADER_NAME = "Depth Of Field";
    static uint32_t SHADER_NAME_HASH;
	static constexpr const char* SHADER_VERTEX = "/GLSL/simple130.glslv";
	static constexpr const char* SHADER_FRAGMENT = "/GLSL/dof.fsh";

    // Core depth of field parameters
    ShaderPropertyProxy mFocalDistance;
    ShaderPropertyProxy mFocalRange;
    ShaderPropertyProxy mFStop;
    ShaderPropertyProxy mCoC;  // Circle of confusion size in mm (35mm film = 0.03mm)

    // Rendering parameters
    ShaderPropertyProxy mSamples;
    ShaderPropertyProxy mRings;

    // Focus control
    ShaderPropertyProxy mFocusPoint;   // vec4 with x,y point in screen and w as flag (0.0 - 1.0)
    
    // Visual enhancement parameters
    ShaderPropertyProxy mBlurForeground;
    ShaderPropertyProxy mThreshold;    // Highlight threshold
    ShaderPropertyProxy mGain;         // Highlight gain
    ShaderPropertyProxy mBias;         // Bokeh edge bias
    ShaderPropertyProxy mFringe;       // Bokeh chromatic aberration/fringing
    ShaderPropertyProxy mNoise;        // Use noise instead of pattern for sample dithering

    // Experimental bokeh shape parameters
    // Note: Requires samples >= 4, rings >= 4 for good visual quality
    ShaderPropertyProxy mPentagon;     // Use pentagon as bokeh shape
    ShaderPropertyProxy mFeather;      // Pentagon shape feather amount

    // Debug utilities
    ShaderPropertyProxy mDebugBlurValue;
    ShaderPropertyProxy mDebugShowFocus;

    std::random_device					rd;
    std::mt19937						e2;		// engine
    std::uniform_real_distribution<>	dist;	// distribution

    // texture Id for random noise jitter sample in the shader
    GLuint	randomTexId{ 0 };

    void InitTexture();
	void DeleteTexture();
};
