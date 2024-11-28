
// SceneManager.cpp
/*
Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "SceneManager.h"
#include "CheckGLError.h"
#include "mobu_logging.h"
#include <algorithm>
#include <unordered_map>
#include <glm/gtc/matrix_transform.hpp>
#include "glm_utils.h"
#include "FBLightProxy.h"
#include <glm/gtc/quaternion.hpp>

namespace Graphics 
{	
	
	SceneManager::SceneManager()
	{
		mGPUSceneLights.reset(new LightGPUBuffersManager());
	}

	SceneManager::~SceneManager()
	{

	}

	bool SceneManager::BeginShading(FBRenderOptions* pRenderOptions)
	{
		if (pRenderOptions->IsIDBufferRendering())
			return false;

		PrepCameraInfoCache(pRenderOptions->GetRenderingCamera(), mCameraCache);
			
		PrepFBSceneLights();

		PrepLightsInViewSpace(mGPUSceneLights.get());

		MapLightsOnGPU();

		return true;
	}

	bool SceneManager::PrepCameraInfoCache(FBCamera *pCamera, CameraInfoCache& cache)
	{
		if (!pCamera)
			return false;

		FBMatrix mv, p, mvInv;

		pCamera->GetCameraMatrix(mv, kFBModelView);
		pCamera->GetCameraMatrix(p, kFBProjection);

		FBMatrixInverse(mvInv, mv);

		FBMatrixToGLM(cache.mv4, mv);
		FBMatrixToGLM(cache.mvInv4, mvInv);
		FBMatrixToGLM(cache.p4, p);

		memcpy(cache.mv, mv, sizeof(double) * 16);

		FBVector3d v;
		pCamera->GetVector(v);
		for (int i = 0; i<3; ++i)
			cache.pos[i] = static_cast<float>(v[i]);
		
		cache.fov = pCamera->FieldOfView;
		cache.width = pCamera->CameraViewportWidth;
		cache.height = pCamera->CameraViewportHeight;
		cache.nearPlane = pCamera->NearPlaneDistance;
		cache.farPlane = pCamera->FarPlaneDistance;

		return true;
	}

	void SceneManager::PrepFBSceneLights()
	{
		FBMatrix pCamMatrix(mCameraCache.mv);
		FBMatrix lViewMatrix(pCamMatrix);

		FBRVector lViewRotation;
		FBMatrixToRotation(lViewRotation, lViewMatrix);

		FBMatrix lViewRotationMatrix;
		FBRotationToMatrix(lViewRotationMatrix, lViewRotation);

		glm::mat4 camModelView(mCameraCache.mv4);
		glm::mat4 camRotationMatrix;

		FBMatrixToGLM(camModelView, lViewMatrix);
		FBMatrixToGLM(camRotationMatrix, lViewRotationMatrix);

		//
		FBScene* pScene = FBSystem::TheOne().Scene;
		const int numSceneLights = pScene->Lights.GetCount();

		if (0 == numSceneLights)
		{
			// only 2 directional lights and not cluster work
			mGPUSceneLights->Resize(2);

			TLight::ConstructDefaultLight0(mGPUSceneLights->GetLightRef(0), false, camModelView, camRotationMatrix);
			TLight::ConstructDefaultLight1(mGPUSceneLights->GetLightRef(1), false, camModelView, camRotationMatrix);
		}
		else
		{
			int numberOfActiveLights = 0;

			for (int i = 0; i < numSceneLights; ++i)
			{
				FBLight* sceneLight = pScene->Lights[i];

				if (sceneLight->CastLightOnObject)
				{
					numberOfActiveLights += 1;
				}
			}

			mUsedSceneLights.resize(numberOfActiveLights);
			mGPUSceneLights->Resize(numberOfActiveLights);

			numberOfActiveLights = 0;
			for (int i = 0; i < numSceneLights; ++i)
			{
				FBLight* sceneLight = pScene->Lights[i];

				if (sceneLight->CastLightOnObject)
				{
					mUsedSceneLights[numberOfActiveLights] = sceneLight;
					FBLightProxy::ConstructFromFBLight(mGPUSceneLights->GetLightRef(numberOfActiveLights), false, camModelView, camRotationMatrix, sceneLight);

					numberOfActiveLights += 1;
				}
			}
		}
	}

	void SceneManager::PrepLightsInViewSpace(LightGPUBuffersManager* pLights) const
	{
		FBMatrix pCamMatrix(mCameraCache.mv); // (cameraCache.mv);
		FBMatrix lViewMatrix(pCamMatrix);

		FBRVector lViewRotation;
		FBMatrixToRotation(lViewRotation, lViewMatrix);

		FBMatrix lViewRotationMatrix;
		FBRotationToMatrix(lViewRotationMatrix, lViewRotation);

		FBSVector lViewScaling;
		FBMatrixToScaling(lViewScaling, lViewMatrix);
		FBMatrix lViewScalingMatrix;
		FBScalingToMatrix(lViewScalingMatrix, lViewScaling);

		lViewScalingMatrix.Identity();
		glm::mat4 modelrotation, modelscaling;

		FBMatrixToGLM(modelrotation, lViewRotationMatrix);
		FBMatrixToGLM(modelscaling, lViewScalingMatrix);

		if (pLights)
		{
			pLights->UpdateTransformedLights(mCameraCache.mv4, modelrotation, modelscaling);
		}
	}

	void SceneManager::MapLightsOnGPU()
	{
		mGPUSceneLights->MapOnGPU();
		mGPUSceneLights->PrepGPUPtr();
	}

	bool SceneManager::PrepShaderLights(FBPropertyListObject* AffectingLightsIn, std::vector<FBLight*>& shaderLightsPtrOut, LightGPUBuffersManager* shaderLightManagerOut)
	{
		if (!shaderLightManagerOut)
			return false;

		if (AffectingLightsIn->GetCount() == 0)
		{
			shaderLightsPtrOut.clear();
			shaderLightManagerOut->Resize(0);
			return true;
		}

		const int numberOfExlLights = AffectingLightsIn->GetCount();
		shaderLightsPtrOut.resize(numberOfExlLights);

		for (int i = 0; i < numberOfExlLights; ++i)
		{
			shaderLightsPtrOut[i] = (FBLight*)AffectingLightsIn->GetAt(i);
		}

		if (numberOfExlLights == 0)
			return true;

		// DONE: don't construct lights, just copy values from scene lights
		PrepLightsFromFBList(shaderLightManagerOut, shaderLightsPtrOut);

		return true;
	}

	void SceneManager::PrepLightsFromFBList(LightGPUBuffersManager* shaderLightsOut, const std::vector<FBLight*>& affectedLights)
	{
		const int numLights = static_cast<int>(affectedLights.size());

		if (numLights == 0)
		{
			// only 2 directional lights and not cluster work
			shaderLightsOut->Resize(2);

			TLight::ConstructDefaultLight0(shaderLightsOut->GetLightRef(0), false, glm::mat4(1.0f), glm::mat4(1.0f));
			TLight::ConstructDefaultLight1(shaderLightsOut->GetLightRef(1), false, glm::mat4(1.0f), glm::mat4(1.0f));
		}
		else
		{
			std::unordered_map<FBLight*, int> lightToIndex;
			for (int i = 0; i < mUsedSceneLights.size(); ++i) {
				lightToIndex[mUsedSceneLights[i]] = i;
			}

			int activeLightIndex = 0;
			shaderLightsOut->Resize(std::count_if(begin(affectedLights), end(affectedLights), [](FBLight* lightPtr) {
				return lightPtr->CastLightOnObject == true;
				}));

			for (auto lightPtr : affectedLights) {
				if (!lightPtr->CastLightOnObject)
					continue;

				auto it = lightToIndex.find(lightPtr);
				if (it != lightToIndex.end()) {
					shaderLightsOut->SetLightData(activeLightIndex++, mGPUSceneLights->GetLightRef(it->second));
				}
			}

			assert(activeLightIndex == shaderLightsOut->Size());
		}
	}

	void SceneManager::ResetShadowInformation()
	{
		for (size_t j = 0; j < mUsedSceneLights.size(); ++j)
		{
			TLight& lightData = mGPUSceneLights->GetLightRef(j);
			lightData.shadowMapLayer = -1.0f;
		}
	}

	void SceneManager::UpdateShadowInformation(const ShadowManager* shadowManagerIn)
	{
		for (size_t i = 0; i < static_cast<size_t>(shadowManagerIn->GetNumberOfShadows()); ++i)
		{
			const FBLightProxy* shadowLightProxy = (const FBLightProxy*)(shadowManagerIn->GetLightProxyPtr(i));

			for (size_t j=0; j<mUsedSceneLights.size(); ++j)
			{
				FBLight* sceneLight = mUsedSceneLights[j];

				if (sceneLight == shadowLightProxy->GetLightPtr())
				{
					TLight& lightData = mGPUSceneLights->GetLightRef(j);
					const TShadow& shadowData = shadowManagerIn->GetShadowDataRef(i);
					
					lightData.shadowMapLayer = shadowData.shadowMapLayer;
					lightData.shadowVP = shadowData.shadowVP;
				}
			}
		}

	}

};