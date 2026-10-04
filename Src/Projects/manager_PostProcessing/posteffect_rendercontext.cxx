
#pragma once

// posteffect_rendercontext.cxx
/*
Sergei <Neill3d> Solokhin 2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffect_rendercontext.h"
#include "mobu_logging.h"
#include "hashUtils.h"

void PostEffectRenderContext::OverrideUniform(const ShaderProperty* shaderProperty, float valueIn)
{
	if (!shaderProperty)
		return;
	
	ShaderPropertyValue newValue(shaderProperty->GetDefaultValue());
	newValue.SetValue(valueIn);
	VERIFY(newValue.GetLocation() >= 0);
	overrideUniforms.emplace_back(std::move(newValue));
}

void PostEffectRenderContext::OverrideUniform(const ShaderProperty* shaderProperty, float x, float y)
{
	if (!shaderProperty)
		return;

	ShaderPropertyValue newValue(shaderProperty->GetDefaultValue());
	newValue.SetValue(x, y);
	VERIFY(newValue.GetLocation() >= 0);
	overrideUniforms.emplace_back(std::move(newValue));
}

void PostEffectRenderContext::OverrideUniform(const ShaderProperty* shaderProperty, float x, float y, float z, float w)
{
	if (!shaderProperty)
		return;

	ShaderPropertyValue newValue(shaderProperty->GetDefaultValue());
	newValue.SetValue(x, y, z, w);
	VERIFY(newValue.GetLocation() >= 0);
	overrideUniforms.emplace_back(std::move(newValue));
}

bool PostEffectRenderContext::OverrideUniform(const ShaderPropertyScheme* propertyScheme, const ShaderPropertyProxy shaderPropertyProxy, float valueIn)
{
	if (!propertyScheme)
		return false;
	const ShaderProperty* prop = propertyScheme->GetProperty(shaderPropertyProxy);
	if (!prop)
		return false;
	ShaderPropertyValue newValue(prop->GetDefaultValue());
	newValue.SetValue(valueIn);
	VERIFY(newValue.GetLocation() >= 0);
	overrideUniforms.emplace_back(std::move(newValue));
	return true;
}

bool PostEffectRenderContext::OverrideUniform(const ShaderPropertyScheme* propertyScheme, const ShaderPropertyProxy shaderPropertyProxy, 
	float x, float y)
{
	if (!propertyScheme)
		return false;
	const ShaderProperty* prop = propertyScheme->GetProperty(shaderPropertyProxy);
	if (!prop)
		return false;
	ShaderPropertyValue newValue(prop->GetDefaultValue());
	newValue.SetValue(x, y);
	VERIFY(newValue.GetLocation() >= 0);
	overrideUniforms.emplace_back(std::move(newValue));
	return true;
}

bool PostEffectRenderContext::OverrideUniform(const ShaderPropertyScheme* propertyScheme, const ShaderPropertyProxy shaderPropertyProxy,
	float x, float y, float z, float w)
{
	if (!propertyScheme)
		return false;
	const ShaderProperty* prop = propertyScheme->GetProperty(shaderPropertyProxy);
	if (!prop)
		return false;
	ShaderPropertyValue newValue(prop->GetDefaultValue());
	newValue.SetValue(x, y, z, w);
	VERIFY(newValue.GetLocation() >= 0);
	overrideUniforms.emplace_back(std::move(newValue));
	return true;
}

void PostEffectRenderContext::UploadUniforms(GLuint programId, const ShaderPropertyStorage::PropertyValueMap* uniformsMap, bool skipTextureProperties) const
{
	// given uniforms
	if (uniformsMap)
	{
		UploadUniformsInternal(programId, *uniformsMap, skipTextureProperties);
	}
	// override uniforms if defined
	UploadUniformsInternal(programId, overrideUniforms, true);
}


void PostEffectRenderContext::UploadUniformsInternal(GLuint programId, const ShaderPropertyStorage::PropertyValueMap& uniformsMap, bool skipTextureProperties) const
{
	for (const ShaderPropertyValue& value : uniformsMap)
	{
		UploadUniformValue(programId, value, skipTextureProperties);
	}
}

void PostEffectRenderContext::UploadUniformValue(GLuint programId, const ShaderPropertyValue& value, bool skipTextureProperties)
{
	constexpr int MAX_USER_TEXTURE_SLOTS = 16;
	GLint location = value.GetLocation();
	if (location < 0)
	{
		location = glGetUniformLocation(programId, ResolveHash32(value.GetNameHash()));

		if (location < 0 && value.IsRequired())
		{
			LOGE("failed to find property location [%u] %s\n", value.GetNameHash(), ResolveHash32(value.GetNameHash()));
			return;
		}
		//LOGE("required property location is not cached [%u] %s\n", value.GetNameHash(), ResolveHash32(value.GetNameHash()).c_str());

		if (location < 0)
		{
			return;
		}
	}

	const float* floatData = value.GetFloatData();
	if (!floatData) {
		LOGE("Property %u has null data\n", value.GetNameHash());
		return;
	}

	switch (value.GetType())
	{
	case EPropertyType::INT:
		glUniform1i(location, static_cast<int>(floatData[0]));
		break;

	case EPropertyType::BOOL:
		glUniform1f(location, floatData[0]);
		break;

	case EPropertyType::FLOAT:
	{
		float scaledValue = value.GetScale() * floatData[0];
		if (value.IsInvertValue())
			scaledValue = 1.0f - scaledValue;

		glUniform1f(location, scaledValue);
	} break;

	case EPropertyType::VEC2:
		glUniform2fv(location, 1, floatData);
		break;

	case EPropertyType::VEC3:
		glUniform3fv(location, 1, floatData);
		break;

	case EPropertyType::VEC4:
		glUniform4fv(location, 1, floatData);
		break;

	case EPropertyType::TEXTURE:
	case EPropertyType::SHADER_USER_OBJECT:
	{
		// designed to be used with multi-pass rendering, when textures are bound from the first pass
		if (skipTextureProperties)
			break;

		const int textureSlot = static_cast<int>(floatData[0]);
		if (textureSlot >= 0 && textureSlot < MAX_USER_TEXTURE_SLOTS)
		{
			glUniform1i(location, textureSlot);
		}	
	} break;

	default:
		LOGE("not supported property for auto upload into uniform [%u] %s\n", value.GetNameHash(), ResolveHash32(value.GetNameHash()));
	}
}