
// ShadowManager.cpp
/*
Sergei <Neill3d> Solokhin 2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "ShadowManager.h"
#include <string>
#include "FBResourcePathResolver.h"
#include "BoundingBox.h"
#include "glm_utils.h"
#include <glm/gtc/type_ptr.hpp>

namespace Graphics
{
	
	void ShadowManager::SetDefaultProperties(ShadowProperties& properties)
	{
		properties.shadowMapResolution = 2048;
		properties.usePCF = true;
		properties.kernelSize = 9;
		properties.offsetFactor = 2.0f;
		properties.offsetUnits = 4.0f;
	}

	ShadowManager::ShadowManager()
		: frameBuffer(1024, 1024, 0, 0)
	{
		SetDefaultProperties(properties);
	}

	// quality properties, size of shadow map, use PCM, kernel size, depth bias and offset
	void ShadowManager::SetProperties(const ShadowProperties& propertiesIn)
	{
		properties = propertiesIn;
	}

	bool ShadowManager::Initialize()
	{
		const std::vector<std::string> test_shaders = {
			"scene_shadow.vsh",
			"scene_shadow.fsh"
		};

		FBShaderPathResolver	pathResolver;
		const std::filesystem::path shadersPath = pathResolver.FindShaderPath(test_shaders);

		if (shadersPath.empty())
		{
			LOGE("[SyperDynamicLighting] Failed to find shaders!\n");
			return false;
		}

		// Create the lighting shader
		const std::string vertexShaderFile = shadersPath.generic_string() + test_shaders[0];
		const std::string fragmentShaderFile = shadersPath.generic_string() + test_shaders[1];

		if (!shader.LoadShaders(vertexShaderFile.c_str(), fragmentShaderFile.c_str()))
		{
			LOGE("[ShadowManager] Failed to initialize a shadowing shader!\n");
			return false;
		}

		shader.Bind();
		shaderProjMatrixLoc = shader.findLocation("projMatrix");
		shaderViewMatrixLoc = shader.findLocation("viewMatrix");
		shaderModelMatrixLoc = shader.findLocation("modelMatrix");
		shader.UnBind();

		return true;
	}

	void ShadowManager::ChangeContext()
	{
		frameBuffer.Cleanup();
		doNeedRecreateTextures = true;
		doNeedInitialization = true;
	}

	// do actual rendering of shadow casters into each shadow texture using given input lights
	void ShadowManager::Render()
	{
		using namespace OpenMobu;

		if (doNeedInitialization)
		{
			Initialize();
			doNeedInitialization = false;
		}

		// make copies of lights and casters to avoid having changes during render evaluation
		const auto thisFrameLights = lights;
		const auto thisFrameCasters = casters;

		SaveFrameBuffer(&frameBufferBindingInfo);

		// Compute the world bounds for infinite light adjustment.
		// TODO: should bounding box be around casters only ?!
		FBVector4d fbWorldMin, fbWorldMax;
		ComputeWorldBounds(fbWorldMin, fbWorldMax);

		FBVectorToGLM(worldMin, fbWorldMin);
		FBVectorToGLM(worldMax, fbWorldMax);

		if (!frameBuffer.GetFrameBuffer())
			frameBuffer.Create();

		const TextureCreationInfo thisFrameTextureInfo = CalculateCurrentTextureCreationInfo(properties, thisFrameLights);

		// initialize framebuffer for needed amount of textures and shadow texture size
		
		if (shadowTexId == 0 || doNeedRecreateTextures || textureInfo != thisFrameTextureInfo)
		{
			if (shadowTexId)
			{
				FreeTextures();
			}
			
			shadowTexId = CreateDepthTextureArray(thisFrameTextureInfo);
			doNeedRecreateTextures = false;
			textureInfo = thisFrameTextureInfo;
		}
		
		const GLenum target = GetTextureTarget(thisFrameTextureInfo);
		//frameBuffer.AttachTexture(target, shadowTexId, FrameBuffer::eAttachmentTypeDepth, false);
		
		shader.Bind();

		glEnable(GL_POLYGON_OFFSET_FILL);
		
		glPolygonOffset(properties.offsetFactor, properties.offsetUnits);

		unsigned int textureIndex = 0;

		for (const auto& light : thisFrameLights)
		{
			if (!IsLightCastShadow(light.get()))
				continue;
			frameBuffer.Bind();

			// Set the viewport to the proper size
			glViewport(0, 0, properties.shadowMapResolution, properties.shadowMapResolution);

			frameBuffer.AttachTextureLayer(target, shadowTexId, textureIndex, FrameBuffer::eAttachmentTypeDepth, false);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);

			// Clear the depth buffer
			glEnable(GL_DEPTH_TEST);
			glClearDepth(1.0);
			glClear(GL_DEPTH_BUFFER_BIT);

			// setup global uniforms like light projection and world matrices

			light->PrepareMatrices(worldMin, worldMax);

			const glm::mat4& proj = light->GetProjectionMatrix();
			const glm::mat4 view = glm::inverse(light->GetViewMatrix());

			// bind uniforms
			shader.setUniformMatrix(shaderProjMatrixLoc, glm::value_ptr(proj));
			shader.setUniformMatrix(shaderViewMatrixLoc, glm::value_ptr(view));

			for (auto& model : casters)
			{
				model->Render(false, shaderModelMatrixLoc, -1);
			}

			frameBuffer.UnBind();

			textureIndex += 1;
		}

		glDisable(GL_POLYGON_OFFSET_FILL);

		shader.UnBind();

		// update shadow data SSBO
		UpdateShadowsData(thisFrameLights, false);

		// restore current framebuffer bind state
		RestoreFrameBuffer(&frameBufferBindingInfo);
	}

	bool ShadowManager::IsLightCastShadow(const LightProxy* lightProxy)
	{
		if (!lightProxy->IsCastLightOnObject() || !lightProxy->IsShadowCaster())
			return false;

		if (lightProxy->GetLightType() == LightProxy::LightType::Spot
			|| lightProxy->GetLightType() == LightProxy::LightType::Infinite)
		{
			return true;
		}
		return false;
	}

	ShadowManager::TextureCreationInfo ShadowManager::CalculateCurrentTextureCreationInfo(const ShadowProperties& propertiesIn, const std::vector<std::shared_ptr<LightProxy>>& lightsIn)
	{
		TextureCreationInfo info;

		info.textureSize = propertiesIn.shadowMapResolution;
		info.useHardwarePCF = propertiesIn.usePCF;

		int numberOfShadowMaps = 0;

		for (const auto& light : lightsIn)
		{
			if (IsLightCastShadow(light.get()))
			{
				numberOfShadowMaps += 1;
			}
		}

		info.numberOfMaps = numberOfShadowMaps;
		return info;
	}

	// bind shadow map textures to use in lighting shader
	// bind a texture to a current active slot
	void ShadowManager::Bind()
	{
		if (shadowTexId > 0)
		{
			const GLenum target = GetTextureTarget(textureInfo);
			glBindTexture(target, shadowTexId);
		}
	}



	void ShadowManager::UnBind() const
	{
		const GLenum target = GetTextureTarget(textureInfo);
		glBindTexture(target, 0);
	}

	void ShadowManager::BindShadowsBuffer(GLuint shadowsBufferLoc)
	{
		shadowsBuffer.Bind(shadowsBufferLoc);
	}

	void ShadowManager::PrepareListOfLights(std::vector<std::shared_ptr<LightProxy>>& renderLights)
	{
		renderLights.clear();

		if (lights.empty())
		{
			return;
		}

		for (auto& light : lights)
		{
			if (IsLightCastShadow(light.get()))
			{
				renderLights.emplace_back(light);
			}
		}
	}

	GLenum ShadowManager::GetTextureTarget(const TextureCreationInfo& info)
	{
		return (!info.useMultisampling) ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
	}

	GLuint ShadowManager::CreateDepthTexture(const TextureCreationInfo& info)
	{
		const GLenum target = GL_TEXTURE_2D;

		GLuint id = 0;

		// Create offscreen textures and FBOs for offscreen shadow map rendering.
		glGenTextures(1, &id);

		// Specify texture parameters and attach each texture to an FBO.
		glBindTexture(target, id);

		glTexImage2D(target, 0, GL_DEPTH_COMPONENT32, info.textureSize, info.textureSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, color);

		if (info.useHardwarePCF)
		{
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
			glTexParameteri(target, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		}
		else
		{
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		
		return id;
	}

	GLuint ShadowManager::CreateDepthTextureArray(const TextureCreationInfo& info)
	{
		const GLenum target = GetTextureTarget(info);

		GLuint id = 0;

		// Create offscreen textures and FBOs for offscreen shadow map rendering.
		glGenTextures(1, &id);

		// Specify texture parameters and attach each texture to an FBO.
		glBindTexture(target, id);

		if (info.useMultisampling)
		{
			glTexStorage3DMultisample(target, info.samplesCount, GL_DEPTH_COMPONENT32F, info.textureSize, info.textureSize, info.numberOfMaps, GL_FALSE);
		}
		else
		{
			glTexStorage3D(target, 1, GL_DEPTH_COMPONENT32F, info.textureSize, info.textureSize, info.numberOfMaps);
		}

		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, color);

		if (info.useHardwarePCF)
		{
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

		/*
		if (info.useHardwarePCF)
		{
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
			glTexParameteri(target, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		}
		else
		{
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		*/
		return id;
	}

	void ShadowManager::FreeTextures()
	{
		if (shadowTexId > 0)
		{
			glDeleteTextures(1, &shadowTexId);
			shadowTexId = 0;
		}
	}

	void ShadowManager::UpdateShadowsData(const std::vector<std::shared_ptr<LightProxy>>& lightsIn, bool uploadOnGPU)
	{
		shadowsData.resize(lightsIn.size());

		int shadowLayerIndex = 0;
		for (size_t i = 0; i < lightsIn.size(); ++i)
		{
			const auto& light = lightsIn[i];
			auto& shadow = shadowsData[i];

			shadow.shadowMapLayer = -1.0f;

			if (!IsLightCastShadow(light.get()))
				continue;

			shadow.shadowMapLayer = static_cast<float>(shadowLayerIndex);
			shadow.shadowVP = light->GetProjectionMatrix() * glm::inverse(light->GetViewMatrix());

			shadowLayerIndex += 1;
		}

		// upload data on gpu
		if (!shadowsData.empty() && uploadOnGPU)
		{
			shadowsBuffer.UpdateData(sizeof(TShadow), shadowsData.size(), shadowsData.data());
		}
	}
};