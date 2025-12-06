
#pragma once

// posteffect_rendercontext.cxx
/*
Sergei <Neill3d> Solokhin 2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffect_rendercontext.h"
#include "mobu_logging.h"
#include "posteffectbase.h"


void PostEffectRenderContext::OverrideUniform(const IEffectShaderConnections::ShaderProperty& shaderProperty, float valueIn)
{
	IEffectShaderConnections::ShaderPropertyValue newValue(shaderProperty.GetDefaultValue());
	newValue.SetValue(valueIn);

	overrideUniforms.emplace_back(std::move(newValue));
}

void PostEffectRenderContext::OverrideUniform(const IEffectShaderConnections::ShaderProperty& shaderProperty, float x, float y)
{
	IEffectShaderConnections::ShaderPropertyValue newValue(shaderProperty.GetDefaultValue());
	newValue.SetValue(x, y);

	overrideUniforms.emplace_back(std::move(newValue));
}

void PostEffectRenderContext::OverrideUniform(const IEffectShaderConnections::ShaderProperty& shaderProperty, float x, float y, float z, float w)
{
	IEffectShaderConnections::ShaderPropertyValue newValue(shaderProperty.GetDefaultValue());
	newValue.SetValue(x, y, z, w);

	overrideUniforms.emplace_back(std::move(newValue));
}

void PostEffectRenderContext::UploadUniforms(const ShaderPropertyStorage::PropertyValueMap* uniformsMap, bool skipTextureProperties) const
{
	// given uniforms
	if (uniformsMap)
	{
		UploadUniformsInternal(*uniformsMap, skipTextureProperties);
	}
	// override uniforms if defined
	UploadUniformsInternal(overrideUniforms, true);
}


void PostEffectRenderContext::UploadUniformsInternal(const ShaderPropertyStorage::PropertyValueMap& uniformsMap, bool skipTextureProperties) const
{
	//GLint userTextureSlot = CommonEffect::UserSamplerSlot; //!< start index to bind user textures

	for (const IEffectShaderConnections::ShaderPropertyValue& value : uniformsMap)
	{
		UploadUniformValue(value, skipTextureProperties);
	}
}

void PostEffectRenderContext::UploadUniformValue(const IEffectShaderConnections::ShaderPropertyValue& value, bool skipTextureProperties)
{
	constexpr int MAX_USER_TEXTURE_SLOTS = 16;

	if (value.GetLocation() < 0)
	{
		if (value.IsRequired())
		{
			//auto iter = mProperties.find(value.GetNameHash());
			//if (iter != end(mProperties))
			//{
			//	LOGE("required property location is not found %s for shader %s\n", iter->second.GetName(), GetName());
			//}
			//else

			{
				LOGE("required property location is not found %u\n", value.GetNameHash());
			}
		}

		return;
	}

	const float* floatData = value.GetFloatData();
	if (!floatData) {
		LOGE("Property %u has null data\n", value.GetNameHash());
		return;
	}

	switch (value.GetType())
	{
	case IEffectShaderConnections::EPropertyType::INT:
		glUniform1i(value.GetLocation(), static_cast<int>(floatData[0]));
		break;

	case IEffectShaderConnections::EPropertyType::BOOL:
		glUniform1f(value.GetLocation(), floatData[0]);
		break;

	case IEffectShaderConnections::EPropertyType::FLOAT:
	{
		float scaledValue = value.GetScale() * floatData[0];
		if (value.IsInvertValue())
			scaledValue = 1.0f - scaledValue;

		glUniform1f(value.GetLocation(), scaledValue);
	} break;

	case IEffectShaderConnections::EPropertyType::VEC2:
		glUniform2fv(value.GetLocation(), 1, floatData);
		break;

	case IEffectShaderConnections::EPropertyType::VEC3:
		glUniform3fv(value.GetLocation(), 1, floatData);
		break;

	case IEffectShaderConnections::EPropertyType::VEC4:
		glUniform4fv(value.GetLocation(), 1, floatData);
		break;

	case IEffectShaderConnections::EPropertyType::TEXTURE:
	{

		// designed to be used with multi-pass rendering, when textures are bound from the first pass
		if (skipTextureProperties)
			break;

		//const ShaderPropertyValue& readValue = readValue->GetReadValue();
		/*
		if (FBTexture* texture = readValue.texture)
		{
			// bind sampler from a media resource texture

			int textureId = texture->TextureOGLId;
			if (textureId == 0)
			{
				texture->OGLInit();
				textureId = texture->TextureOGLId;
			}

			if (textureId > 0)
			{
				glUniform1i(value.GetLocation(), userTextureSlot);

				glActiveTexture(GL_TEXTURE0 + userTextureSlot);
				glBindTexture(GL_TEXTURE_2D, textureId);
				glActiveTexture(GL_TEXTURE0);

				userTextureSlot += 1;
			}
		}

		else if (EffectShaderUserObject* userObject = readValue.shaderUserObject)
		{
			PostEffectBufferShader* bufferShader = userObject->GetUserShaderPtr();

			UnBind();

			// bind sampler from another rendered buffer shader
			const std::string bufferName = std::string(GetName()) + "_" + std::string(userObject->Name);
			const uint32_t bufferNameKey = xxhash32(bufferName);

			int effectW = w;
			int effectH = h;
			userObject->RecalculateWidthAndHeight(effectW, effectH);

			FrameBuffer* buffer = buffers->RequestFramebuffer(bufferNameKey, effectW, effectH, PostEffectBuffers::GetFlagsForSingleColorBuffer(), 1, false);

			bufferShader->Render(buffers, buffer, 0, inputTextureId, effectW, effectH, generateMips, effectContext);

			const GLuint bufferTextureId = buffer->GetColorObject();
			buffers->ReleaseFramebuffer(bufferNameKey);

			// bind input buffers
			glActiveTexture(GL_TEXTURE0 + userTextureSlot);
			glBindTexture(GL_TEXTURE_2D, bufferTextureId);
			glActiveTexture(GL_TEXTURE0);

			Bind();

			glUniform1i(value.GetLocation(), userTextureSlot);

			userTextureSlot += 1;
			if (userTextureSlot >= MAX_USER_TEXTURE_SLOTS)
			{
				LOGE("too many user texture slots used in shader %s\n", GetName());
				break;
			}
		}
		else
		*/
		{
			const int textureSlot = static_cast<int>(floatData[0]);
			if (textureSlot >= 0 && textureSlot < MAX_USER_TEXTURE_SLOTS)
			{
				glUniform1i(value.GetLocation(), textureSlot);
			}
		}

	} break;

	default:
		LOGE("not supported property for auto upload into uniform %u\n", value.GetNameHash());
	}
}