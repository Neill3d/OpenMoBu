
#pragma once

// posteffectdisplacement.h
/*
Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectbase.h"

// forward
class PostPersistentData;

class EffectShaderDisplacement : public PostEffectBufferShader
{
private:
	static constexpr const char* SHADER_NAME = "Displacement";
	static constexpr const char* SHADER_VERTEX = "/GLSL/simple130.glslv";
	static constexpr const char* SHADER_FRAGMENT = "/GLSL/displacement.fsh";
	static constexpr const char* ENABLE_MASKING_PROPERTY_NAME = "Disp Use Masking";
	static constexpr const char* MASKING_CHANNEL_PROPERTY_NAME = "Disp Masking Channel";

public:

	EffectShaderDisplacement(FBComponent* ownerIn);
	virtual ~EffectShaderDisplacement()
	{}

	int GetNumberOfVariations() const override { return 1; }

	const char* GetName() const override { return SHADER_NAME; }
	const char* GetVertexFname(const int shaderIndex) const override { return SHADER_VERTEX; }
	const char* GetFragmentFname(const int shaderIndex) const override { return SHADER_FRAGMENT; }

protected:

	IEffectShaderConnections::ShaderProperty* mTime;
	IEffectShaderConnections::ShaderProperty* mSpeed;
	IEffectShaderConnections::ShaderProperty* mUseQuakeEffect;
	IEffectShaderConnections::ShaderProperty* mXDistMag;
	IEffectShaderConnections::ShaderProperty* mYDistMag;
	IEffectShaderConnections::ShaderProperty* mXSineCycles;
	IEffectShaderConnections::ShaderProperty* mYSineCycles;

	
	virtual const char* GetUseMaskingPropertyName() const override {
		return ENABLE_MASKING_PROPERTY_NAME;
	}
	virtual const char* GetMaskingChannelPropertyName() const override {
		return MASKING_CHANNEL_PROPERTY_NAME;
	}

	// this is a predefined effect shader, properties are defined manually
	virtual bool DoPopulatePropertiesFromUniforms() const override {
		return false;
	}

	virtual bool OnCollectUI(const IPostEffectContext* effectContext, int maskIndex) override;
};

/// <summary>
/// effect with once shader - displacement, output directly to effects chain dst buffer
/// </summary>
typedef PostEffectSingleShader<EffectShaderDisplacement> PostEffectDisplacement;
