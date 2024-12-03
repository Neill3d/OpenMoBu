
#pragma once

// ModelProxy.h
/*
Sergei <Neill3d> Solokhin 2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include <cstdint>
#include <glm/glm.hpp>
#include <GL/glew.h>

namespace Graphics
{
	
	// a class with interface to query any light implementation to get needed values from a light (type, FOV, far/near planes, is shadow caster, etc.)
	class LightProxy
	{
	public:
		virtual ~LightProxy() = default;

		// returns true if this light suppose to do any light distribution on objects
		virtual bool IsCastLightOnObject() const = 0;

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

		virtual bool HasCustomBoundingBox() const = 0;
		virtual bool GetCustomBoundingBox(glm::vec3& bbMin, glm::vec3& bbMax) const = 0;

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

		virtual bool IsCastsShadows() const = 0;
		virtual bool IsReceiveShadows() const = 0;

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
		virtual void Render(bool useNormalAttrib, GLint modelMatrixLoc, GLint normalMatrixLoc, GLuint programId) = 0;
	};

	// a class with interface to query a current camera render parameters
	class CameraProxy
	{
	public:
		virtual ~CameraProxy() = default;

	};

};