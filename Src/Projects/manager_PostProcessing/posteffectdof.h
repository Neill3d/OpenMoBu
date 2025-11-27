#pragma once

// posteffectdof
/*
Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectsingleshader.h"

// forward
class EffectShaderDOF;

/// <summary>
/// Depth of field post-processing effect with single shader pipeline
/// Outputs directly to effects chain destination buffer
/// </summary>
using PostEffectDOF = PostEffectSingleShader<EffectShaderDOF>;


/// <summary>
/// Depth of field post-processing shader effect implementation
/// Provides realistic camera depth of field simulation with configurable parameters
/// </summary>
class EffectShaderDOF : public PostEffectBufferShader
{
public:

	explicit EffectShaderDOF(FBComponent* ownerIn);
	virtual ~EffectShaderDOF() = default;

	int GetNumberOfVariations() const override { return 1; }

	const char* GetName() const override { return SHADER_NAME; }
    uint32_t GetNameHash() const override { return SHADER_NAME_HASH; }
	const char* GetVertexFname(const int shaderIndex) const override { return SHADER_VERTEX; }
	const char* GetFragmentFname(const int shaderIndex) const override { return SHADER_FRAGMENT; }

protected:

    [[nodiscard]] virtual const char* GetUseMaskingPropertyName() const noexcept override;
    [[nodiscard]] virtual const char* GetMaskingChannelPropertyName() const noexcept override;
    // This is a predefined effect shader, properties are defined manually
    virtual bool DoPopulatePropertiesFromUniforms() const override {
        return false;
    }
	virtual bool OnCollectUI(IPostEffectContext* effectContext, int maskIndex) override;

private:
	static constexpr const char* SHADER_NAME = "Depth Of Field";
    static uint32_t SHADER_NAME_HASH;
	static constexpr const char* SHADER_VERTEX = "/GLSL/simple130.glslv";
	static constexpr const char* SHADER_FRAGMENT = "/GLSL/dof.fsh";

    // Core depth of field parameters
    ShaderProperty* mFocalDistance;
    ShaderProperty* mFocalRange;
    ShaderProperty* mFStop;
    ShaderProperty* mCoC;  // Circle of confusion size in mm (35mm film = 0.03mm)

    // Rendering parameters
    ShaderProperty* mSamples;
    ShaderProperty* mRings;

    // Focus control
    ShaderProperty* mAutoFocus;    // Use autofocus in shader
    ShaderProperty* mFocus;        // Autofocus point on screen (0.0,0.0 - bottom-left, 1.0,1.0 - top-right)
    ShaderProperty* mFocusPoint;
    ShaderProperty* mManualDOF;    // Manual DOF calculation toggle

    // Near and far DOF blur parameters
    ShaderProperty* mNDOFStart;    // Near DOF blur start distance
    ShaderProperty* mNDOFDist;     // Near DOF blur falloff distance
    ShaderProperty* mFDOFStart;    // Far DOF blur start distance  
    ShaderProperty* mFDOFDist;     // Far DOF blur falloff distance

    // Visual enhancement parameters
    ShaderProperty* mBlurForeground;
    ShaderProperty* mThreshold;    // Highlight threshold
    ShaderProperty* mGain;         // Highlight gain
    ShaderProperty* mBias;         // Bokeh edge bias
    ShaderProperty* mFringe;       // Bokeh chromatic aberration/fringing
    ShaderProperty* mNoise;        // Use noise instead of pattern for sample dithering

    // Experimental bokeh shape parameters
    // Note: Requires samples >= 4, rings >= 4 for good visual quality
    ShaderProperty* mPentagon;     // Use pentagon as bokeh shape
    ShaderProperty* mFeather;      // Pentagon shape feather amount

    // Debug utilities
    ShaderProperty* mDebugBlurValue;
};
