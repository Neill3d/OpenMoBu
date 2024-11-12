
#pragma once

// SuperShader.h
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

#define kMaxDrawInstancedSize  100

namespace Graphics
{
	typedef std::vector<TLight>			lights_vector;


	/////////////////////////////////////////////////////////////////////////////////////////////////////
	//

	struct CCameraInfoCache
	{
		//FBCamera *pCamera;
		void				*pUserData;

		//
		int					offsetX;
		int					offsetY;
		int					width;
		int					height;

		//
		double				fov;

		double				farPlane;
		double				nearPlane;
		double				realFarPlane;

		glm::vec4		pos;	// camera eye pos

		glm::mat4		mv4;
		glm::mat4		mvInv4; // mv inverse
		glm::mat4		p4;	// projection matrix
		glm::mat4		proj2d;

		double				mv[16]; //!< modelview in double precision

		/*
		// pre-loaded data from camera
		FBMatrix			mv;
		FBMatrix			mvInv;
		FBMatrix			p;

		static void Prep(FBCamera *pCamera, CCameraInfoCache &cache);
		*/
	};


    /**
     * @class ShaderLightManager
     * @brief Manages directional and point/spot lights in view space for GPU shaders.
     */
    class ShaderLightManager {
    public:
        /**
            * @brief Constructs a new ShaderLightManager instance.
            */
        ShaderLightManager();

        /**
            * @brief Destroys the ShaderLightManager instance and releases GPU resources.
            */
        ~ShaderLightManager();

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
            * @brief Gets the number of directional lights.
            *
            * @return Number of directional lights.
            */
        std::size_t GetNumberOfDirLights() const { return dirLights.size(); }

        /**
            * @brief Gets the number of point/spot lights.
            *
            * @return Number of point/spot lights.
            */
        std::size_t GetNumberOfLights() const { return lights.size(); }

        /**
            * @brief Gets the vector of point/spot lights.
            *
            * @return A const reference to the vector of lights.
            */
        const lights_vector& GetLightsVector() const { return lights; }

        /**
            * @brief Gets the vector of directional lights.
            *
            * @return A const reference to the vector of directional lights.
            */
        const lights_vector& GetDirLightsVector() const { return dirLights; }


        void Resize(std::size_t dirLightsCount, std::size_t lightCount);

        TLight& GetLightRef(std::size_t index);
        TLight& GetDirLightRef(std::size_t index);

        /**
            * @brief Updates the lights to view space using the provided matrices.
            *
            * @param modelview Model-view matrix for transformations.
            * @param rotation Rotation matrix.
            * @param scaling Scaling matrix.
            */
        void UpdateTransformedLights(const glm::mat4& modelview, const glm::mat4& rotation, const glm::mat4& scaling);

        //void UpdateShadowInfo()

    public:

        
    protected:
        lights_vector dirLights;           ///< Directional lights in view space.
        lights_vector lights;              ///< Point/spot lights in view space.
        lights_vector transformedDirLights;///< Transformed directional lights.
        lights_vector transformedLights;   ///< Transformed point/spot lights.

        GPUBufferSSBO bufferLights;        ///< SSBO for point/spot lights data.
        GPUBufferSSBO bufferDirLights;     ///< SSBO for directional lights data.
    };


};