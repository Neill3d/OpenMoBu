
#pragma once

// ShadowManager.h
/*
Sergei <Neill3d> Solokhin 2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "glslShader.h"
#include "Framebuffer.h"
#include <glm/glm.hpp>
#include "SuperShader_glsl.h"
#include "OGL_Utils.h"
#include "GPUBuffer.h"

#include <vector>
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


	// a class with interface to query any light implementation to get needed values from a light (type, FOV, far/near planes, is shadow caster, etc.)
	class LightProxy
	{
	public:
		virtual ~LightProxy() = default;

		// Returns true if this light is set to cast shadows
		virtual bool IsShadowCaster() const = 0;

		enum class LightType : uint8_t 
		{
			Point = 0,  //!< Point light.
			Infinite,   //!< Infinite light (plane).
			Spot,       //!< Spot light.
			Area        //!< Area light. 
		};

		// Gets the light type (e.g., spotlight, directional, point)
		virtual LightType GetLightType() const = 0;

		// Returns the field of view (FOV) for perspective projection lights (e.g., spotlights)
		virtual float GetFOV() const = 0;

		// Gets the near and far planes for constructing the projection matrix
		virtual float GetNearPlane() const = 0;
		virtual float GetFarPlane() const = 0;

		// Gets the position of the light in world space
		virtual glm::vec3 GetPosition() const = 0;

		// Gets the direction the light is facing (for directional lights or spotlights)
		virtual glm::vec3 GetDirection() const = 0;

		virtual bool PrepareMatrices(const glm::vec3& worldMin, const glm::vec3& worldMax) = 0;

		// Gets the projection matrix for shadow mapping
		virtual const glm::mat4& GetProjectionMatrix() const = 0;

		// Gets the view matrix for rendering the shadow map from the light's perspective
		virtual const glm::mat4& GetViewMatrix() const = 0;

	};

	// a class with interface to query any model implementation to render model for our shadow texture
	class ModelProxy
	{
	public:
		virtual ~ModelProxy() = default;

		/*
		*  render model under the current opengl context
		*
		* @param useNormalAttrib the vertex normal attribute will be binded to slot 2 with vec4 alingment
		* @param modelMatrixLoc defines a location for a model matrix uniform, skip if -1
		* @param normalMatrixLoc defines a location for a model normal matrix uniform, skip if -1
		* 
		* vertex attributes
		*  0 - positions (vec4)
		*  2 - normals (vec4)
		* uniform matrix binding
		*  5 - model matrix (mat4)
		*  6 - normal matrix (mat4)
		*/
		virtual void Render(bool useNormalAttrib, GLint modelMatrixLoc, GLint normalMatrixLoc) = 0;
	};

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
			float depthBias;
			float offset;
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

		std::vector<TShadow>	shadowsData;

		GPUBufferSSBO			shadowsBuffer;        ///< describe each shadow projection for the lighting shader

		void UpdateShadowsData(const std::vector<std::shared_ptr<LightProxy>>& lightsIn);

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