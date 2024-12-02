
// LightGPUBuffersManager.cpp
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "LightGPUBuffersManager.h"
#include "CheckGLError.h"

namespace Graphics 
{

	LightGPUBuffersManager::LightGPUBuffersManager()
	{
	}

	LightGPUBuffersManager::~LightGPUBuffersManager()
	{
	}

	void LightGPUBuffersManager::UpdateTransformedLights(const glm::mat4& modelview, const glm::mat4& rotation, const glm::mat4& scaling)
	{
		size_t numberOfDirLights = 0;
		size_t numberOfSpotLights = 0;

		for (const auto& light : lights)
		{
			if (light.type == LIGHT_TYPE_DIRECTION)
			{
				numberOfDirLights += 1;
			}
			else
			{
				numberOfSpotLights += 1;
			}
		}

		// Ensure transformed vectors are the right size, if needed
		if (transformedLights.size() != numberOfSpotLights) {
			transformedLights.resize(numberOfSpotLights);
		}
		if (transformedDirLights.size() != numberOfDirLights) {
			transformedDirLights.resize(numberOfDirLights);
		}

		// transfer data
		numberOfDirLights = 0;
		numberOfSpotLights = 0;

		for (const auto& light : lights)
		{
			if (light.type == LIGHT_TYPE_DIRECTION)
			{
				auto& transformedLight = transformedDirLights[numberOfDirLights];
				transformedLight = light;
				transformedLight.dir = rotation * glm::vec4(light.dir, 0.0f);  // Transform direction only
				// Note: No position transform for directional lights as they represent directions.

				numberOfDirLights += 1;
			}
			else
			{
				auto& transformedLight = transformedLights[numberOfSpotLights];
				transformedLight = light;  // Copy source light to destination
				transformedLight.position = modelview * glm::vec4(transformedLight.position, 1.0f);  // Transform position
				transformedLight.dir = rotation * glm::vec4(transformedLight.dir, 0.0f);  // Transform direction

				numberOfSpotLights += 1;
			}
		}
	}

	void LightGPUBuffersManager::MapOnGPU()
	{
		// dir lights
		if (!transformedDirLights.empty())
		{
			bufferDirLights.UpdateData(sizeof(TLight), transformedDirLights.size(), transformedDirLights.data());
		}
		
		// point / spot lights
		if (!transformedLights.empty())
		{
			bufferLights.UpdateData(sizeof(TLight), transformedLights.size(), transformedLights.data());
		}
	}


	void LightGPUBuffersManager::Resize(std::size_t lightsCount)
	{
		if (lightsCount > 0 && lightsCount != lights.size())
		{
			lights.resize(lightsCount);
		}
		else if (lightsCount == 0)
		{
			lights.clear();
		}
	}

	TLight& LightGPUBuffersManager::GetLightRef(std::size_t index)
	{
		assert(index < lights.size());
		return lights[index];
	}

	void LightGPUBuffersManager::SetLightData(std::size_t index, const TLight& lightIn)
	{
		assert(index < lights.size());
		lights[index] = lightIn;
	}

	void LightGPUBuffersManager::PrepGPUPtr()
	{
		
	}

	void LightGPUBuffersManager::Bind(const GLuint programId, const GLuint dirLightsLoc, const GLuint lightsLoc) const
	{
		// bind dir lights uniforms
		if (programId > 0)
		{
			bufferDirLights.Bind(dirLightsLoc);
			bufferLights.Bind(lightsLoc);
		}
	}

	void LightGPUBuffersManager::UnBind() const
	{
	}
};