
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
		properties.depthBias = 1.0f;
		properties.offset = 1.0f;
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

		SaveFrameBuffer(&frameBufferBindingInfo);

		// Compute the world bounds for infinite light adjustment.
		// TODO: should bounding box be around casters only ?!
		FBVector4d fbWorldMin, fbWorldMax;
		ComputeWorldBounds(fbWorldMin, fbWorldMax);

		FBVectorToGLM(worldMin, fbWorldMin);
		FBVectorToGLM(worldMax, fbWorldMax);

		if (!frameBuffer.GetFrameBuffer())
			frameBuffer.Create();

		// Set the viewport to the proper size
		glViewport(0, 0, properties.shadowMapResolution, properties.shadowMapResolution);

		frameBuffer.Bind();

		// TODO: initialize framebuffer for needed amount of textures and shadow texture size

		// allocate a new texture
		
		if (shadowTexId == 0 || doNeedRecreateTextures)
		{
			shadowTexId = CreateDepthTexture(properties.shadowMapResolution, properties.usePCF);
			doNeedRecreateTextures = false;
		}
		
		frameBuffer.AttachTexture(GL_TEXTURE_2D, shadowTexId, FrameBuffer::eAttachmentTypeDepth, false);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		// Clear the depth buffer
		glEnable(GL_DEPTH_TEST);
		glClearDepth(1.0);
		glClear(GL_DEPTH_BUFFER_BIT);

		shader.Bind();

		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0f, 4.0f);

		for (auto& light : lights)
		{
			// setup global uniforms like light projection and world matrices

			if (!light->IsShadowCaster())
				continue;

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
		}

		glDisable(GL_POLYGON_OFFSET_FILL);

		shader.UnBind();

		// for each light, render casters into a texture

		frameBuffer.UnBind();

		// restore current framebuffer bind state
		RestoreFrameBuffer(&frameBufferBindingInfo);
	}

	// get a shadow map matrix for each calculated shadow map, in order to use it in lighting shader
	void ShadowManager::UpdateLightsBufferData(TLight& lightData)
	{
		lightData.shadowMapLayer = (shadowTexId > 0) ? 1.0f : -1.0f;

		if (!lights.empty())
		{
			lightData.shadowVP = lights[0]->GetProjectionMatrix() * glm::inverse(lights[0]->GetViewMatrix());
		}
	}

	// bind shadow map textures to use in lighting shader
	// bind a texture to a current active slot
	void ShadowManager::Bind()
	{
		if (shadowTexId > 0)
		{
			glBindTexture(GL_TEXTURE_2D, shadowTexId);
		}
	}

	void ShadowManager::UnBind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
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
			if (light->IsShadowCaster() && light->GetLightType() != LightProxy::LightType::Point)
			{
				renderLights.emplace_back(light);
			}
		}
	}

	GLuint ShadowManager::CreateDepthTexture(const int size, const bool use_hardware_pcf)
	{
		GLuint id = 0;

		// Create offscreen textures and FBOs for offscreen shadow map rendering.
		glGenTextures(1, &id);

		// Specify texture parameters and attach each texture to an FBO.
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, size, size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
		if (use_hardware_pcf)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		
		return id;
	}

};