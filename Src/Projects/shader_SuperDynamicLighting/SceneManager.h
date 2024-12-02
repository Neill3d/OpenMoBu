
#pragma once

// SceneManager.h
/*
Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "ModelProxy.h"
#include "GPUBuffer.h"
#include "LightGPUBuffersManager.h"
#include "ShadowManager.h"
//--- SDK include
#include <fbsdk/fbsdk.h>
#include <vector>
#include <memory>

namespace Graphics
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	//

	struct CameraInfoCache
	{
		void* pUserData; //FBCamera *pCamera;

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
	* take care of current camera cache and scene list of lights
	*/
	class SceneManager
	{
	public:

		//! a constructor
		SceneManager();
		//! a destructor
		~SceneManager();

	protected:

		CameraInfoCache								mCameraCache;

		// list of used lights, could be exclusive from current composition lights list
		std::vector<FBLight*>	mUsedSceneLights;

		// prepare buffers to upload on GPU for the full list of scene lights
		//  each shader could copy needed part of it to upload on gpu for a custom affected lights list
		std::unique_ptr<LightGPUBuffersManager>			mGPUSceneLights;

	public:

		bool BeginShading(FBRenderOptions* pRenderOptions);

		bool PrepCameraInfoCache(FBCamera* pCamera, CameraInfoCache& cache);
		const CameraInfoCache &GetCameraCache() const { return mCameraCache; }

		// this is a GPU buffer preparation, not an actual binding
		void	PrepFBSceneLights();
		void	PrepLightsInViewSpace(LightGPUBuffersManager* pLights) const;

		/**
		* in a given lights manager, copy only those scene lights which are presented in the affected lights list
		*/
		void	PrepLightsFromFBList(LightGPUBuffersManager* pLightsManager, const std::vector<FBLight*>& affectedLights);

		void	MapLightsOnGPU();

		const int GetNumberOfUsedLights() const { return static_cast<int>(mUsedSceneLights.size()); }
		std::vector<FBLight*>& GetUsedLightsVector() { return mUsedSceneLights; }

		// just prepare gpu buffer (no binding)
		bool PrepShaderLights(FBPropertyListObject* AffectingLightsIn,
			std::vector<FBLight*>& shaderLightsPtrOut, LightGPUBuffersManager* shaderLightManagerOut);

		void ResetShadowInformation();
		void UpdateShadowInformation(const ShadowManager* shadowManagerIn);
		
		LightGPUBuffersManager* GetGPUSceneLightsPtr() { return mGPUSceneLights.get(); }
	};

};