
#pragma once

// ShadowManager.h
/*
Sergei <Neill3d> Solokhin 2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "ModelProxy.h"
#include "glslShader.h"
#include "Framebuffer.h"
#include <glm/glm.hpp>
#include "SuperShader_glsl.h"
#include "OGL_Utils.h"
#include "GPUBuffer.h"

#include <vector>
#include <unordered_map>
#include <memory>

namespace Graphics
{
	/*
	
	 @code

	// Setup lights and models
	ShadowManager shadowManager;
	shadowManager.SetLights(lightProxies);
	shadowManager.SetShadowCasters(modelProxies);

	// Configure shadow map properties
	ShadowProperties properties = { 4096, true, 3, 0.005f, 0.1f };
	shadowManager.SetProperties(properties);

	// Render shadows
	shadowManager.Render();

	// Use shadow maps in the lighting shader
	shadowManager.Bind();
	shader.SetUniformMatrix("shadowMapMatrix", shadowManager.GetShadowMapMatrix());
	shadowManager.UnBind();

	 @endcode
	
	*/


	

	// manager to render and handle shadow maps from a given lights and given list of model shadow casters.
	//  support spot lights with PSM shadows at the moment
	class ShadowManager
	{
	public:

		ShadowManager();

		// lights that cast shadows, it could be that we have one shadow per light, or 4 shadow textures per light in case of CSM
		// Pass by reference for normal use
		void SetLights(const std::vector<std::shared_ptr<LightProxy>>& lightsIn)
		{
			lights = lightsIn;
		}

		// Or, accept by rvalue reference for ownership transfer
		void SetLights(std::vector<std::shared_ptr<LightProxy>>&& lightsIn) {
			lights = std::move(lightsIn);
		}

		void ClearLights() { lights.clear(); }

		LightProxy* GetLightProxyPtr(const size_t index) { return lights[index].get(); }
		const LightProxy* GetLightProxyPtr(const size_t index) const { return lights[index].get(); }

		// set which models are going to take part in shadow rendering
		void SetShadowCasters(const std::vector<std::shared_ptr<ModelProxy>>& castersIn)
		{
			casters = castersIn;
		}

		void SetShadowCasters(std::vector<std::shared_ptr<ModelProxy>>&& castersIn)
		{
			casters = std::move(castersIn);
		}

		void ClearShadowCasters() { casters.clear(); }

		struct ShadowProperties {
			int shadowMapResolution;
			bool usePCF;
			int kernelSize;
			float offsetFactor; // multiplies the max depth slope of the polygon
			float offsetUnits; // a fixed const offset in depth units for all polygons
		};

		// quality properties, size of shadow map, use PCM, kernel size, depth bias and offset
		void SetProperties(const ShadowProperties& properties);

		bool Initialize();

		// you have to call it when opengl context has been changed and we should re-initialize the internal opengl handles
		void ChangeContext();

		// do actual rendering of shadow casters into each shadow texture using given input lights
		void Render();

		// using TLight buffer data and update with shadow values of view proj matrix and shadow map layer
		//void UpdateLightsBufferData(TLight& lightData);

		// bind shadow map textures to use in lighting shader
		void Bind();
		void UnBind() const;

		void BindShadowsBuffer(GLuint shadowsBufferLoc);

		int GetNumberOfShadows() const { return static_cast<int>(shadowsData.size()); }
		const TShadow& GetShadowDataRef(size_t index) const { return shadowsData[index]; }

	private:

		bool	doNeedInitialization{ true };

		ShadowProperties			properties;

		std::vector<std::shared_ptr<LightProxy>>		lights; // lights that are going to cast shadows

		std::vector<std::shared_ptr<ModelProxy>>		casters; // models that are casting shadows and participate in rendering shadow maps

		FrameBuffer		frameBuffer; //!< used to render into shadow textures

		GLSLShader		shader; //!< a shader to render models into a shadow maps

		GLint shaderProjMatrixLoc{ -1 };
		GLint shaderViewMatrixLoc{ -1 };
		GLint shaderModelMatrixLoc{ -1 };

		glm::vec3 worldMin;
		glm::vec3 worldMax;

		void PrepareListOfLights(std::vector<std::shared_ptr<LightProxy>>& renderLights);

		static bool IsLightCastShadow(const LightProxy* lightProxy);

	private:

		// SSBO with TShadow array

		std::vector<TShadow>		shadowsData;
		
		//std::unordered_map<LightProxy*, int> mapLightProxyToOutput;

		GPUBufferSSBO			shadowsBuffer;        ///< describe each shadow projection for the lighting shader

		void UpdateShadowsData(const std::vector<std::shared_ptr<LightProxy>>& lightsIn, bool uploadOnGPU);

	private:
		
		struct TextureCreationInfo
		{
			int textureSize{ 64 };
			int numberOfMaps{ 1 }; // for texture array
			bool useHardwarePCF{ true };
			bool useMultisampling{ false };
			int samplesCount{ 4 };

			bool operator != (const ShadowManager::TextureCreationInfo& anotherInfo)
			{
				if (textureSize != anotherInfo.textureSize)
					return true;
				if (numberOfMaps != anotherInfo.numberOfMaps)
					return true;
				if (useHardwarePCF != anotherInfo.useHardwarePCF)
					return true;
				if (useMultisampling != anotherInfo.useMultisampling)
					return true;
				if (samplesCount != anotherInfo.samplesCount)
					return true;

				return false;
			}

		};

		

		TextureCreationInfo	textureInfo;

		bool		doNeedRecreateTextures{ true };

		GLuint shadowTexId{ 0 };

		FrameBufferInfo frameBufferBindingInfo;

		static TextureCreationInfo CalculateCurrentTextureCreationInfo(const ShadowProperties& propertiesIn, const std::vector<std::shared_ptr<LightProxy>>& lightsIn);

		// return a texture target type, depends on current options (like multisampling, texture array, etc.)
		static GLenum GetTextureTarget(const TextureCreationInfo& info);

		static GLuint CreateDepthTextureArray(const TextureCreationInfo& info);
		static GLuint CreateDepthTexture(const TextureCreationInfo& info);

		void FreeTextures();

		void SetDefaultProperties(ShadowProperties& properties);
	};

};