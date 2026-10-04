
#pragma once

// posteffect_rendercontext.h
/*
Sergei <Neill3d> Solokhin 2025-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "shaderproperty.h"
#include "shaderproperty_scheme.h"
#include "shaderproperty_storage.h"
#include <unordered_map>

// forward
class FrameBuffer;
class PostEffectBuffers;
class PostPersistentData;
class ScopedEffectBind;
class EffectShaderUserObject;
class ShaderPropertyStorage;


struct PostEffectRenderContext
{
	PostEffectBuffers* buffers{ nullptr };

	// override uniforms - pre-render layer
	ShaderPropertyStorage::PropertyValueMap overrideUniforms;

	// INPUT: input in the effects chain for this effect
	GLuint srcTextureId{ 0 };
	GLuint depthTextureId{ 0 };

	int width{ 1 };
	int height{ 1 };

	// OUTPUT: write an effect composition to a given frame buffer
	FrameBuffer* targetFramebuffer{ nullptr };
	int colorAttachment{ 0 }; //!< a way to define a color attachment in the dstFrameBuffer where we should render into

	GLint userTextureSlot{ 0 }; //!< slot to bind next free user texture

	bool generateMips{ false };

	void ClearOverrideUniforms() {
		overrideUniforms.clear();
	}

	void OverrideUniform(const ShaderProperty* shaderProperty, float valueIn);
	void OverrideUniform(const ShaderProperty* shaderProperty, float x, float y);
	void OverrideUniform(const ShaderProperty* shaderProperty, float x, float y, float z, float w);

	bool OverrideUniform(const ShaderPropertyScheme* propertyScheme, const ShaderPropertyProxy shaderProperty, 
		float valueIn);
	bool OverrideUniform(const ShaderPropertyScheme* propertyScheme, const ShaderPropertyProxy shaderProperty,
		float x, float y);
	bool OverrideUniform(const ShaderPropertyScheme* propertyScheme, const ShaderPropertyProxy shaderProperty, 
		float x, float y, float z, float w);

	// upload from a given map and apply override uniforms after that
	void UploadUniforms(GLuint programId, const ShaderPropertyStorage::PropertyValueMap* uniformsMap, bool skipTextureProperties) const;

	static void UploadUniformValue(GLuint programId, const ShaderPropertyValue& value, bool skipTextureProperties);

private:
	void UploadUniformsInternal(GLuint programId, const ShaderPropertyStorage::PropertyValueMap& uniformsMap, bool skipTextureProperties) const;
};

