
#pragma once

// FBLightProxy.h
/*
Sergei <Neill3d> Solokhin 2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "ShadowManager.h"
//--- SDK include
#include <fbsdk/fbsdk.h>

namespace Graphics
{
	
	// a class with interface to query any light implementation to get needed values from a light (type, FOV, far/near planes, is shadow caster, etc.)
	class FBLightProxy : public LightProxy
	{
	public:
		
		FBLightProxy(FBLight* lightIn)
			: lightPlug(lightIn)
		{}

		
		virtual ~FBLightProxy()
		{}

		// Returns true if this light is set to cast shadows
		virtual bool IsShadowCaster() const override
		{
			FBLight* light = lightPlug;
			return light->CastShadows;
		}

		// Gets the light type (e.g., spotlight, directional, point)
		virtual LightType GetLightType() const override
		{
			FBLight* light = lightPlug;

			switch (light->LightType)
			{
			case FBLightType::kFBLightTypeInfinite:
				return LightType::Infinite;
			case FBLightType::kFBLightTypeSpot:
				return LightType::Spot;
			case FBLightType::kFBLightTypeArea:
				return LightType::Area;
			default:
				return LightType::Point;
			}
		}

		// Returns the field of view (FOV) for perspective projection lights (e.g., spotlights)
		virtual float GetFOV() const override
		{
			return 35.0f;
		}

		// Gets the near and far planes for constructing the projection matrix
		virtual float GetNearPlane() const override
		{
			return 1.0f;
		}
		virtual float GetFarPlane() const override
		{
			return 1000.0f;
		}

		// Gets the position of the light in world space
		virtual glm::vec3 GetPosition() const override
		{
			return glm::vec3();
		}

		// Gets the direction the light is facing (for directional lights or spotlights)
		virtual glm::vec3 GetDirection() const override
		{
			return glm::vec3();
		}

		// do a preparation of light matrices with a given world min and max (for infinite light)
		//  matrices are used for rendering shadow maps and applying them in the scene
		bool PrepareMatrices(const glm::vec3& worldMin, const glm::vec3& worldMax) override;

		// Gets the projection matrix for shadow mapping
		virtual const glm::mat4& GetProjectionMatrix() const override;

		// Gets the view matrix for rendering the shadow map from the light's perspective
		virtual const glm::mat4& GetViewMatrix() const override;

		// Retrieves the shadow map texture associated with this light (if any)
		//virtual unsigned int GetShadowMapTextureID() const = 0;

		// Sets the shadow map texture ID (used by ShadowManager after rendering)
		//virtual void SetShadowMapTextureID(unsigned int textureID) = 0;

		
		static void ConstructFromFBLight(TLight& light, const bool ToEyeSpace, const glm::mat4& lViewMatrix,
			const glm::mat4& lViewRotationMatrix, FBLight* pLight);


	private:

		HdlFBPlugTemplate<FBLight> lightPlug;

		glm::mat4	lightView;
		glm::mat4	lightProj;
	};

};