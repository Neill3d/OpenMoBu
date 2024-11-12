
// SuperShader_lights.cpp
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "SuperShader.h"
#include "glm_utils.h"
#include <glm/gtc/quaternion.hpp>
#include "FBLightProxy.h"

#define SHADER_DIR_LIGHTS_UNITID		2
#define SHADER_POINT_LIGHTS_UNITID		3	

namespace Graphics
{

	///////////////////////////////////////////////////////////////////////////////////////////////////


	void SuperShader::PrepFBSceneLights()
	{
		/*
		glm::vec3 scale, translation, skew;
		glm::quat orientation;
		glm::vec4 perspective;
		glm::mat4 glmMV;

		for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
		glmMV[i][j] = mCameraCache.mv4[i][j];

		glm::decompose(glmMV, scale, orientation, translation, skew, perspective);

		quat camRotation(orientation[0], orientation[1], orientation[2], orientation[3]);

		mat4 camModelView(mCameraCache.mv4);
		mat4 camRotationMatrix;
		camRotationMatrix.set_rot(camRotation);
		//transpose(camRotationMatrix);
		*/

		FBMatrix pCamMatrix(mCameraCache.mv); // (cameraCache.mv);
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
		FBScene *pScene = FBSystem::TheOne().Scene;
		int numLights = pScene->Lights.GetCount();

		//auto &lights = mGPUSceneLights->GetLightsVector();
		//auto &dirLights = mGPUSceneLights->GetDirLightsVector();

		if (0 == numLights)
		{
			// only 2 directional lights and not cluster work
			mGPUSceneLights->Resize(2, 0);
			
			//pLightsManager->mLightCasters.clear();
			//pLightsManager->mLightCastersDataPtr.clear();

			TLight::ConstructDefaultLight0(mGPUSceneLights->GetDirLightRef(0), false, camModelView, camRotationMatrix);
			TLight::ConstructDefaultLight1(mGPUSceneLights->GetDirLightRef(1), false, camModelView, camRotationMatrix);
		}
		else
		{
			// process scene lights and enable clustering if some point/spot light exist

			mUsedSceneLights.resize(numLights);

			int numDirLights = 0;
			int numPointLights = 0;
			int numLightCasters = 0;

			for (int i = 0; i < numLights; ++i)
			{
				FBLight *pLight = pScene->Lights[i];
				mUsedSceneLights[i] = pLight;

				if (pLight->CastLightOnObject)
				{
					if (pLight->LightType.AsInt() == kFBLightTypeInfinite) numDirLights++;
					else numPointLights++;
				}

				// lights for shadows
				// temproary make only one cascaded shadow
				if (pLight->LightType.AsInt() == kFBLightTypeInfinite && numLightCasters == 0)
				{
					if (pLight->LightType.AsInt() != kFBLightTypePoint && pLight->CastShadows)
						numLightCasters++;
				}
			}

			mUsedInfiniteLights.resize(numDirLights);
			mUsedPointLights.resize(numPointLights);

			mGPUSceneLights->Resize(numDirLights, numPointLights);

			//pLightsManager->mLightCasters.resize(numLightCasters);
			//pLightsManager->mLightCastersDataPtr.resize(numLightCasters);

			numDirLights = 0;
			numPointLights = 0;
			numLightCasters = 0;

			for (auto iter = begin(mUsedSceneLights); iter != end(mUsedSceneLights); ++iter)
			{
				FBLight *pLight = *iter;
				TLight *pLightData = nullptr;

				if (pLight->CastLightOnObject)
				{
					if (pLight->LightType.AsInt() != kFBLightTypeInfinite)
					{
						FBLightProxy::ConstructFromFBLight(mGPUSceneLights->GetLightRef(numPointLights), false, camModelView, camRotationMatrix, pLight);
						pLightData = &mGPUSceneLights->GetLightRef(numPointLights);
						mUsedPointLights[numPointLights] = pLight;
						numPointLights++;
					}
					else
					{
						FBLightProxy::ConstructFromFBLight(mGPUSceneLights->GetDirLightRef(numDirLights), false, camModelView, camRotationMatrix, pLight);
						pLightData = &mGPUSceneLights->GetDirLightRef(numDirLights);
						mUsedInfiniteLights[numDirLights] = pLight;
						numDirLights++;
					}
				}
				/*
				// lights for shadows
				// temproary make only one cascaded shadow
				if (pLight->LightType == kFBLightTypeInfinite && numLightCasters == 0)
				{
				if (pLight->LightType != kFBLightTypePoint && pLight->CastShadows)
				{
				pLightsManager->mLightCasters[numLightCasters] = pLight;
				pLightsManager->mLightCastersDataPtr[numLightCasters] = pLightData;

				numLightCasters++;
				}
				}
				*/
			}
		}
	}

	void SuperShader::PrepLightsInViewSpace(ShaderLightManager *pLights) const
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

	void SuperShader::PrepLightsFromFBList(ShaderLightManager *pShaderLights,
		const CCameraInfoCache &cameraCache, std::vector<FBLight*> &mobuLights)
	{
		/*
		FBMatrix pCamMatrix;
		pCamera->GetCameraMatrix( pCamMatrix, kFBModelView );
		FBMatrix lViewMatrix( pCamMatrix );

		FBRVector lViewRotation;
		FBMatrixToRotation(lViewRotation, lViewMatrix);

		FBMatrix lViewRotationMatrix;
		FBRotationToMatrix(lViewRotationMatrix, lViewRotation);
		*/

		const glm::mat4 camModelView = cameraCache.mv4;
		const glm::quat camRotation = glm::quat_cast(camModelView);
		const glm::mat4 camRotationMatrix = glm::mat4_cast(camRotation);

		//
		const int numLights = (int)mobuLights.size();

		//auto &dstLights = pShaderLights->GetLightsVector();
		//auto &dstDirLights = pShaderLights->GetDirLightsVector();

		if (numLights == 0)
		{
			// only 2 directional lights and not cluster work
			pShaderLights->Resize(2, 0);

			//pLightsManager->mLightCasters.clear();
			//pLightsManager->mLightCastersDataPtr.clear();

			TLight::ConstructDefaultLight0(pShaderLights->GetDirLightRef(0), false, camModelView, camRotationMatrix);
			TLight::ConstructDefaultLight1(pShaderLights->GetDirLightRef(1), false, camModelView, camRotationMatrix);
		}
		else
		{
			// process scene lights and enable clustering if some point/spot light exist

			int numDirLights = 0;
			int numPointLights = 0;
			int numLightCasters = 0;

			for (auto iter = begin(mobuLights); iter != end(mobuLights); ++iter)
			{
				FBLight *pLight = *iter;

				if (pLight->CastLightOnObject)
				{
					if (pLight->LightType.AsInt() == kFBLightTypeInfinite) numDirLights++;
					else numPointLights++;
				}

				// lights for shadows
				// temproary make only one cascaded shadow
				if (pLight->LightType.AsInt() == kFBLightTypeInfinite && numLightCasters == 0)
				{
					if (pLight->LightType.AsInt() != kFBLightTypePoint && pLight->CastShadows)
						numLightCasters++;
				}
			}

			pShaderLights->Resize(numDirLights, numPointLights);

			//pLightsManager->mLightCasters.resize(numLightCasters);
			//pLightsManager->mLightCastersDataPtr.resize(numLightCasters);

			numDirLights = 0;
			numPointLights = 0;
			numLightCasters = 0;

			//auto &srcLights = mGPUSceneLights->GetLightsVector();
			//auto &srcDirLights = mGPUSceneLights->GetDirLightsVector();

			for (auto iter = begin(mobuLights); iter != end(mobuLights); ++iter)
			{
				FBLight *pLight = *iter;
				TLight *pLightData = nullptr;

				if (pLight->CastLightOnObject)
				{
					if (pLight->LightType.AsInt() != kFBLightTypeInfinite)
					{
						for (int i = 0, count = (int)mUsedPointLights.size(); i < count; ++i)
						{
							if (pLight == mUsedPointLights[i])
							{
								pShaderLights->GetLightRef(numPointLights) = mGPUSceneLights->GetLightRef(i);
								pLightData = &pShaderLights->GetLightRef(numPointLights);
								numPointLights++;
								break;
							}
						}
					}
					else
					{
						for (int i = 0, count = (int)mUsedInfiniteLights.size(); i < count; ++i)
						{
							if (pLight == mUsedInfiniteLights[i])
							{
								pShaderLights->GetDirLightRef(numDirLights) = mGPUSceneLights->GetDirLightRef(i);
								pLightData = &pShaderLights->GetDirLightRef(numDirLights);
								numDirLights++;
								break;
							}
						}
					}
				}
				/*
				// lights for shadows
				// temproary make only one cascaded shadow
				if (pLight->LightType == kFBLightTypeInfinite && numLightCasters == 0)
				{
				if (pLight->LightType != kFBLightTypePoint && pLight->CastShadows)
				{
				pLightsManager->mLightCasters[numLightCasters] = pLight;
				pLightsManager->mLightCastersDataPtr[numLightCasters] = pLightData;

				numLightCasters++;
				}
				}
				*/
			}
		}
	}

	void SuperShader::MapLightsOnGPU()
	{
		// TODO: update only on light changes ?!
		//PrepFBSceneLights();

		mGPUSceneLights->MapOnGPU();
		mGPUSceneLights->PrepGPUPtr();
		
	}



	bool SuperShader::PrepShaderLights(const bool useSceneLights, FBPropertyListObject *AffectingLights,
		std::vector<FBLight*> &shaderLightsPtr, ShaderLightManager *shaderLights)
	{
		if (nullptr == shaderLights)
			return false;

		const int numberOfExlLights = AffectingLights->GetCount();
		shaderLightsPtr.resize(numberOfExlLights);

		for (int i = 0; i<numberOfExlLights; ++i)
		{
			shaderLightsPtr[i] = (FBLight*)AffectingLights->GetAt(i);
		}

		if (numberOfExlLights == 0)
			return true;

		// DONE: don't construct lights, just copy values from scene lights
		PrepLightsFromFBList(shaderLights, GetCameraCache(), shaderLightsPtr);

		// TODO: reupdate gpu buffer only on changes, not every frame !
		//shaderLights->MapOnGPU();
		//shaderLights->PrepGPUPtr();

		return true;
	}

	bool SuperShader::BindLights(const bool resetLastBind, const ShaderLightManager *pUserLights)
	{
		
		const ShaderLightManager *pShaderLights = mGPUSceneLights.get();

		if (pUserLights)
		{
			if (pUserLights->GetNumberOfDirLights() > 0 || pUserLights->GetNumberOfLights() > 0)
			{
				pShaderLights = pUserLights;
			}
		}

		if (!pShaderLights)
		{
			UploadLightingInformation(0, 0);
			return false;
		}

		// TODO: check for last uberShader lights binded
		if (resetLastBind)
			mLastLightsBinded = nullptr;

		if (mLastLightsBinded == pShaderLights)
		{
			return true;
		}

		// bind a new buffer
		//const auto loc = mMaterialShaders->GetCurrentEffectLocationsPtr()->fptr();
		const GLint dirLights = SHADER_DIR_LIGHTS_UNITID; //  loc->GetLocation(Graphics::eCustomLocationDirLights);
		const GLint lights = SHADER_POINT_LIGHTS_UNITID; // loc->GetLocation(Graphics::eCustomLocationLights);

		if (dirLights >= 0 || lights >= 0)
		{
			pShaderLights->Bind(mShaderShading->GetFragmentShader(), dirLights, lights);
			UploadLightingInformation(pShaderLights->GetNumberOfDirLights(), pShaderLights->GetNumberOfLights());

			mLastLightsBinded = (ShaderLightManager*)pShaderLights;
		}
		else
		{
			UploadLightingInformation(0, 0);
			return false;
		}
		return true;
	}

	void SuperShader::UploadSwitchAlbedoTosRGB(bool sRGB)
	{
		if (PhongShaderUniformLocations.switchAlbedoTosRGB >= 0)
			glUniform1f(PhongShaderUniformLocations.switchAlbedoTosRGB, (sRGB) ? 1.0f : 0.0f);
	}

	void SuperShader::UploadGlobalAmbient(double *color)
	{
		if (PhongShaderUniformLocations.globalAmbientLight >= 0)
			glUniform4f(PhongShaderUniformLocations.globalAmbientLight, (float)color[0], (float)color[1], (float)color[2], 1.0f);
	}

	void SuperShader::UploadLightingInformation(const int numdir, const int numpoint)
	{
		if (PhongShaderUniformLocations.numberOfDirLights >= 0)
			glUniform1i(PhongShaderUniformLocations.numberOfDirLights, numdir);
		if (PhongShaderUniformLocations.numberOfPointLights >= 0)
			glUniform1i(PhongShaderUniformLocations.numberOfPointLights, numpoint);
	}
}

