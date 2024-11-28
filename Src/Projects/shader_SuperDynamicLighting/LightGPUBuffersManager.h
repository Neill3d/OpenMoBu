
#pragma once

// LightGPUBuffersManager.h
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "glslShader.h"
#include "SuperShader_glsl.h"
#include "GPUBuffer.h"

#include <vector>
#include <memory>

namespace Graphics
{
	typedef std::vector<TLight>			lights_vector;

    /**
     * @class ShaderLightManager
     * @brief Manages directional and point/spot lights in view space for GPU shaders.
     */
    class LightGPUBuffersManager {
    public:
        /**
            * @brief Constructs a new ShaderLightManager instance.
            */
        LightGPUBuffersManager();

        /**
            * @brief Destroys the ShaderLightManager instance and releases GPU resources.
            */
        ~LightGPUBuffersManager();

        /**
            * @brief Maps light data onto the GPU.
            */
        void MapOnGPU();

        /**
            * @brief Prepares GPU pointers for shader access.
            */
        void PrepGPUPtr();

        /**
            * @brief Binds the light data to a shader program.
            *
            * @param programId ID of the shader program.
            * @param dirLightsLoc Location of directional lights in the shader.
            * @param lightsLoc Location of point/spot lights in the shader.
            */
        void Bind(GLuint programId, GLuint dirLightsLoc, GLuint lightsLoc) const;

        /**
            * @brief Unbinds the light data from the shader program.
            */
        void UnBind() const;

        /**
            * @brief Gets the number of point/spot lights.
            *
            * @return Number of point/spot lights.
            */
        std::size_t GetNumberOfLights() const { return lights.size(); }

        std::size_t GetNumberOfTransformedDirLights() const { return transformedDirLights.size(); }
        std::size_t GetNumberOfTransformedSpotLights() const { return transformedLights.size(); }

        /**
            * @brief Gets the vector of point/spot lights.
            *
            * @return A const reference to the vector of lights.
            */
        const lights_vector& GetLightsVector() const { return lights; }


        void Resize(std::size_t lightCount);

        TLight& GetLightRef(std::size_t index);
        
        void SetLightData(std::size_t index, const TLight& lightIn);

        /**
            * @brief Updates the lights to view space using the provided matrices.
            *
            * @param modelview Model-view matrix for transformations.
            * @param rotation Rotation matrix.
            * @param scaling Scaling matrix.
            */
        void UpdateTransformedLights(const glm::mat4& modelview, const glm::mat4& rotation, const glm::mat4& scaling);

    public:

        
    protected:
        
        lights_vector lights;              ///< Point/spot lights in world space.
        
        // we split lights into 2 buffers - dir lights and omni/spot lights
        
        lights_vector transformedDirLights;///< Transformed directional lights in view space.
        lights_vector transformedLights;   ///< Transformed point/spot lights in view space.

        GPUBufferSSBO bufferLights;        ///< SSBO for point/spot lights data.
        GPUBufferSSBO bufferDirLights;     ///< SSBO for directional lights data.
    };

};