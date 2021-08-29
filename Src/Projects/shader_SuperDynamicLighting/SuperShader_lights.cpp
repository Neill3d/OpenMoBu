
// SuperShader_lights.cpp
/*
Sergei <Neill3d> Solokhin 2018-2021

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "SuperShader.h"

namespace Graphics
{
	constexpr int SHADER_DIR_LIGHTS_UNITID{ 2 };
	constexpr int SHADER_POINT_LIGHTS_UNITID{ 3 };

	constexpr double sDefaultAttenuationNone[3] = { 1.0, 0.0, 0.0 };
	constexpr double sDefaultAttenuationLinear[3] = { 0.0, 0.01, 0.0 };
	constexpr double sDefaultAttenuationQuadratic[3] = { 0.0, 0.0, 0.0001 };

	///////////////////////////////////////////////////////////////////////////////////////////////////

	void FBVectorToVec3(const double *v, float *dst)
	{
		dst[0] = (float)v[0];
		dst[1] = (float)v[1];
		dst[2] = (float)v[2];
	}

	void FBVectorToVec4KeepW(const double *v, float *dst)
	{
		dst[0] = (float)v[0];
		dst[1] = (float)v[1];
		dst[2] = (float)v[2];
	}

	void FBVectorToVec4(const double *v, float *dst)
	{
		dst[0] = (float)v[0];
		dst[1] = (float)v[1];
		dst[2] = (float)v[2];
		dst[3] = (float)v[3];
	}

	void SuperShader::ConstructDefaultLight0(bool inEyeSpace, const mat4 &lViewMatrix, const mat4 &lViewRotationMatrix, TLight &light)
	{
		memset(&light, 0, sizeof(TLight));

		// If there is no light in the scene, we must put the two default lights
		// in order to have almost the same behavior as MB. Those two lights are
		// not present as soon as there is one light in the scene.
		const vec4 kPosition(0.0f, 0.0f, 1.0f, 0.0f);
		const vec4 kDirection0(0.2f, -0.2f, -0.6f, 0.0f);

		if (inEyeSpace)
			light.position = lViewMatrix * kPosition;
		else
			light.position = kPosition;

		light.type = LIGHT_TYPE_DIRECTION;

		light.color = vec3(0.8f, 0.8f, 0.8f);
		light.attenuations = vec4(1.0f, 0.0f, 0.0f, 1.0f);

		if (inEyeSpace)
			light.dir = lViewRotationMatrix * kDirection0;
		else
			light.dir = kDirection0;
	}

	void SuperShader::ConstructDefaultLight1(bool inEyeSpace, const mat4 &lViewMatrix, const mat4 &lViewRotationMatrix, TLight &light)
	{
		memset(&light, 0, sizeof(TLight));

		// If there is no light in the scene, we must put the two default lights
		// in order to have almost the same behavior as MB. Those two lights are
		// not present as soon as there is one light in the scene.
		const vec4 kPosition(0.0f, 0.0f, 1.0f, 0.0f);
		const vec4 kDirection1(-0.6f, -0.4f, 0.75f, 0.0f);

		if (inEyeSpace)
			light.position = lViewMatrix * kPosition;
		else
			light.position = kPosition;

		light.type = LIGHT_TYPE_DIRECTION;

		light.color = vec3(0.8f, 0.8f, 0.8f);
		light.attenuations = vec4(1.0f, 0.0f, 0.0f, 1.0f);

		if (inEyeSpace)
			light.dir = lViewRotationMatrix * kDirection1;
		else
			light.dir = kDirection1;
	}

	void SuperShader::ConstructFromFBLight(const bool ToEyeSpace, const mat4 &lViewMatrix, const mat4 &lViewRotationMatrix, FBLight *pLight, TLight &light)
	{
		memset(&light, 0, sizeof(TLight));

		// Get the light's position, direction, colour, and cone angle (stored in direction's 4th component)
		// Also setup attenuation for the lighting equation ...
		// Note that for point/spot lights, we add a 1.0 to the 4th colour
		// component so we can know to generate the direction from position
		FBVector3d   pos;
		pLight->GetVector(pos); // Get Global Translation.


		light.position = vec3((float)pos[0], (float)pos[1], (float)pos[2]);
		if (ToEyeSpace)
			mult(light.position, lViewMatrix, vec4(light.position.x, light.position.y, light.position.z, 1.0f));
		
		// Set whether or not we take the spot factor into consideration in the last position spot ...
		switch (pLight->LightType)
		{
		case kFBLightTypeInfinite:
			light.type = LIGHT_TYPE_DIRECTION;
			break;
		case kFBLightTypePoint:
			light.type = LIGHT_TYPE_POINT;
			break;
		case kFBLightTypeSpot:
			light.type = LIGHT_TYPE_SPOT;
			break;
		}

		//FBVectorToVec3( lViewPosition.mValue, light.position.vec_array );

		// Setup color and attenuation 
		FBColorAndAlpha diffuseColor = (FBColor)pLight->DiffuseColor;
		double attenuation[4];

		if (pLight->LightType != kFBLightTypeInfinite)
		{
			switch (pLight->AttenuationType)
			{
			case kFBAttenuationLinear:
				attenuation[0] = sDefaultAttenuationLinear[0];
				attenuation[1] = sDefaultAttenuationLinear[1];
				attenuation[2] = sDefaultAttenuationLinear[2];
				break;
			case kFBAttenuationQuadratic:
			case kFBAttenuationCubic:
				//assert(light->AttenuationType != kFBAttenuationCubic);// "Don't support Cubic attenuation yet, use Quadratic instead.");
				attenuation[0] = sDefaultAttenuationQuadratic[0];
				attenuation[1] = sDefaultAttenuationQuadratic[1];
				attenuation[2] = sDefaultAttenuationQuadratic[2];
				break;
			case kFBAttenuationNone:
			default:
				attenuation[0] = sDefaultAttenuationNone[0];
				attenuation[1] = sDefaultAttenuationNone[1];
				attenuation[2] = sDefaultAttenuationNone[2];
				break;
			}
			attenuation[3] = pLight->Intensity * 0.01; // / 100.0;
		}
		else
		{
			attenuation[0] = 1.0; attenuation[1] = 0.0; attenuation[2] = 0.0;
			attenuation[3] = pLight->Intensity * 0.01;
		}

		FBVector3d scale(100.0, 100.0, 100.0);

		FBProperty *lprop = pLight->PropertyList.Find("Magnitude");
		if (lprop)
		{
			lprop->GetData(&scale[0], sizeof(double));
		}
		else
		{
			lprop = pLight->PropertyCreate("Magnitude", kFBPT_double, "Number", true, false);
			if (lprop)
				lprop->SetData(&scale[0]);
		}

		//pLight->GetVector( scale, kModelScaling );
		light.radius = (float)scale[0] * 20.0;


		FBVectorToVec3(diffuseColor, light.color.vec_array);
		FBVectorToVec4(attenuation, light.attenuations.vec_array);

		// Get the rotation matrix and multiply light vector with it
		FBMatrix rotationMatrix;
		FBVector4d direction;
		pLight->GetMatrix(rotationMatrix, kModelRotation, true);
		FBVector4d normal(0, -1, 0, 1);
		FBVectorMatrixMult(direction, rotationMatrix, normal);

		// Convert the ModelView (World) space
		light.dir = vec3((float)direction[0], (float)direction[1], (float)direction[2]);
		if (ToEyeSpace)
			mult(light.dir, lViewRotationMatrix, vec4(light.dir.x, light.dir.y, light.dir.z, 1.0f));
		//light.dir = lViewRotationMatrix * vec4(light.dir.x, light.dir.y, light.dir.z, 1.0f);
		//FBVector4d lViewDirection(direction);
		//if (ToEyeSpace)
		//	FBVectorMatrixMult(lViewDirection, lViewRotationMatrix, direction);
		//else
		//	lViewDirection = direction;

		double cosAngle;
		if (pLight->LightType == kFBLightTypeSpot)
			cosAngle = cos((3.141592654*pLight->ConeAngle / 180.0f) / 2.0f);
		else
			cosAngle = 0.0;

		light.spotAngle = (float)cosAngle;
		//FBVectorToVec3( lViewDirection.mValue, light.dir.vec_array );

		//
		light.castSpecularOnObject = 0.0f;

		/*
		// TODO: let's change this property value inside the Connection Data Change Event !

		FBProperty *pProp = pLight->PropertyList.Find("Cast Specular On Object");
		if (pProp)
		{
		light.castSpecularOnObject = (float) pProp->AsInt();
		}
		else
		{
		// add a new property

		pProp = pLight->PropertyCreate( "Cast Specular On Object", kFBPT_bool, "BOOL", false, false );
		if (pProp) pProp->SetInt( 0 );
		}
		*/
	}

	void SuperShader::PrepFBSceneLights()
	{
		FBMatrix pCamMatrix(mCameraCache.mv);
		FBMatrix lViewMatrix(pCamMatrix);

		FBRVector lViewRotation;
		FBMatrixToRotation(lViewRotation, lViewMatrix);

		FBMatrix lViewRotationMatrix;
		FBRotationToMatrix(lViewRotationMatrix, lViewRotation);

		mat4 camModelView(mCameraCache.mv4);
		mat4 camRotationMatrix;

		for (int i = 0; i < 16; ++i)
		{
			camModelView.mat_array[i] = (float)lViewMatrix[i];
			camRotationMatrix.mat_array[i] = (float)lViewRotationMatrix[i];
		}

		//
		FBScene *pScene = FBSystem::TheOne().Scene;
		const int numLights = pScene->Lights.GetCount();

		auto &lights = m_GPUSceneLights->GetLightsVector();
		auto &dirLights = m_GPUSceneLights->GetDirLightsVector();

		if (0 == numLights)
		{
			// only 2 directional lights and not cluster work
			dirLights.resize(2);
			lights.clear();
			
			m_GPUSceneLights->GetCastShadowLights().clear();
			m_GPUSceneLights->GetCastShadowLightsData().clear();

			ConstructDefaultLight0(false, camModelView, camRotationMatrix, dirLights[0]);
			ConstructDefaultLight1(false, camModelView, camRotationMatrix, dirLights[1]);
		}
		else
		{
			// process scene lights and enable clustering if some point/spot light exist

			m_UsedSceneLights.resize(numLights);

			int numDirLights = 0;
			int numPointLights = 0;
			int numLightCasters = 0;

			for (int i = 0; i < numLights; ++i)
			{
				FBLight *pLight = pScene->Lights[i];
				m_UsedSceneLights[i] = pLight;

				if (pLight->CastLightOnObject)
				{
					if (pLight->LightType == kFBLightTypeInfinite) numDirLights++;
					else numPointLights++;
				}

				// lights for shadows
				
				if (pLight->LightType != kFBLightTypePoint && pLight->CastShadows)
				{
					numLightCasters++;
				}
			}

			m_UsedInfiniteLights.resize(numDirLights);
			m_UsedPointLights.resize(numPointLights);
			
			dirLights.resize(numDirLights);
			lights.resize(numPointLights);
			m_GPUSceneLights->GetCastShadowLights().resize(numLightCasters);
			m_GPUSceneLights->GetCastShadowLightsData().resize(numLightCasters);

			numDirLights = 0;
			numPointLights = 0;
			numLightCasters = 0;

			for (auto iter = begin(m_UsedSceneLights); iter != end(m_UsedSceneLights); ++iter)
			{
				FBLight *pLight = *iter;
				TLight *pLightData = nullptr;

				if (pLight->CastLightOnObject)
				{
					if (pLight->LightType != kFBLightTypeInfinite)
					{
						ConstructFromFBLight(false, camModelView, camRotationMatrix, pLight, lights[numPointLights]);
						pLightData = &lights[numPointLights];
						m_UsedPointLights[numPointLights] = pLight;
						numPointLights++;
					}
					else
					{
						ConstructFromFBLight(false, camModelView, camRotationMatrix, pLight, dirLights[numDirLights]);
						pLightData = &dirLights[numDirLights];
						m_UsedInfiniteLights[numDirLights] = pLight;
						numDirLights++;
					}
				}
				
				// lights for shadows
				
				if (pLight->LightType != kFBLightTypePoint && pLight->CastShadows)
				{
					m_GPUSceneLights->GetCastShadowLights()[numLightCasters] = pLight;
					m_GPUSceneLights->GetCastShadowLightsData()[numLightCasters] = pLightData;

					numLightCasters++;
				}
			}
		}
	}

	void SuperShader::PrepLightsInViewSpace(CGPUShaderLights *pLights)
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
		mat4 modelrotation, modelscaling;

		for (int i = 0; i < 16; ++i)
		{
			modelrotation.mat_array[i] = (float)lViewRotationMatrix[i];
			modelscaling.mat_array[i] = (float)lViewScalingMatrix[i];
		}

		if (pLights != nullptr)
			pLights->UpdateTransformedLights(mCameraCache.mv4, modelrotation, modelscaling);
	}

	void SuperShader::PrepLightsFromFBList(CGPUShaderLights *pShaderLights,
		const CCameraInfoCache &cameraCache, std::vector<FBLight*> &mobuLights)
	{
		mat4 camModelView, camRotationMatrix;
		quat camRotation;

		camModelView = cameraCache.mv4;
		camModelView.as_rot(camRotation);
		camRotationMatrix.set_rot(camRotation);

		//
		const int numLights = static_cast<int>(mobuLights.size());

		auto &dstLights = pShaderLights->GetLightsVector();
		auto &dstDirLights = pShaderLights->GetDirLightsVector();

		if (numLights == 0)
		{
			// only 2 directional lights and not cluster work
			dstDirLights.resize(2);
			dstLights.clear();
			pShaderLights->GetCastShadowLights().clear();
			pShaderLights->GetCastShadowLightsData().clear();

			ConstructDefaultLight0(false, camModelView, camRotationMatrix, dstDirLights[0]);
			ConstructDefaultLight1(false, camModelView, camRotationMatrix, dstDirLights[1]);
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
					if (pLight->LightType == kFBLightTypeInfinite) numDirLights++;
					else numPointLights++;
				}

				// lights for shadows

				if (pLight->LightType != kFBLightTypePoint && pLight->CastShadows)
				{
					numLightCasters++;
				}
			}


			dstDirLights.resize(numDirLights);
			dstLights.resize(numPointLights);
			pShaderLights->GetCastShadowLights().resize(numLightCasters);
			pShaderLights->GetCastShadowLightsData().resize(numLightCasters);

			numDirLights = 0;
			numPointLights = 0;
			numLightCasters = 0;

			auto &srcLights = m_GPUSceneLights->GetLightsVector();
			auto &srcDirLights = m_GPUSceneLights->GetDirLightsVector();

			for (auto iter = begin(mobuLights); iter != end(mobuLights); ++iter)
			{
				FBLight *pLight = *iter;
				TLight *pLightData = nullptr;

				if (pLight->CastLightOnObject)
				{
					if (pLight->LightType != kFBLightTypeInfinite)
					{
						for (int i = 0, count = (int)m_UsedPointLights.size(); i < count; ++i)
						{
							if (pLight == m_UsedPointLights[i])
							{
								dstLights[numPointLights] = srcLights[i];
								pLightData = &dstLights[numPointLights];
								numPointLights++;
								break;
							}
						}
					}
					else
					{
						for (int i = 0, count = (int)m_UsedInfiniteLights.size(); i < count; ++i)
						{
							if (pLight == m_UsedInfiniteLights[i])
							{
								dstDirLights[numDirLights] = srcDirLights[i];
								pLightData = &dstDirLights[numDirLights];
								numDirLights++;
								break;
							}
						}
					}
				}
				
				// lights for shadows
				
				if (pLight->LightType != kFBLightTypePoint && pLight->CastShadows)
				{
					pShaderLights->GetCastShadowLights()[numLightCasters] = pLight;
					pShaderLights->GetCastShadowLightsData()[numLightCasters] = pLightData;
					
					numLightCasters++;
				}
			}
		}
	}

	void SuperShader::MapLightsOnGPU()
	{
		m_GPUSceneLights->MapOnGPU();
		m_GPUSceneLights->PrepGPUPtr();
	}


	bool SuperShader::PrepShaderLights(const bool useSceneLights, FBPropertyListObject *AffectingLights,
		std::vector<FBLight*> &shaderLightsPtr, CGPUShaderLights *shaderLights)
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

		return true;
	}

	bool SuperShader::BindLights(const bool resetLastBind, const CGPUShaderLights *pUserLights)
	{
		
		const CGPUShaderLights *pShaderLights = m_GPUSceneLights.get();

		if (nullptr != pUserLights)
		{
			if (pUserLights->GetNumberOfDirLights() > 0 || pUserLights->GetNumberOfLights() > 0)
			{
				pShaderLights = pUserLights;
			}
		}

		if (nullptr == pShaderLights)
		{
			UploadLightingInformation(0, 0);
			return false;
		}

		// TODO: check for last uberShader lights binded
		if (true == resetLastBind)
			mLastLightsBinded = nullptr;

		if (mLastLightsBinded == pShaderLights)
		{
			return true;
		}

		// bind a new buffer
		//const auto loc = mMaterialShaders->GetCurrentEffectLocationsPtr()->fptr();
		const GLint dirLights = SHADER_DIR_LIGHTS_UNITID; //  loc->GetLocation(Graphics::eCustomLocationDirLights);
		const GLint lights = SHADER_POINT_LIGHTS_UNITID; // loc->GetLocation(Graphics::eCustomLocationLights);

		//if (dirLights >= 0 || lights >= 0)
		{
			pShaderLights->Bind(mShaderShading->GetFragmentShader(), dirLights, lights);
			UploadLightingInformation(pShaderLights->GetNumberOfDirLights(), pShaderLights->GetNumberOfLights());

			mLastLightsBinded = (CGPUShaderLights*)pShaderLights;
		}
		/*
		else
		{
			UploadLightingInformation(0, 0);
			return false;
		}
		*/
		return true;
	}

	void SuperShader::UploadSwitchAlbedoTosRGB(bool sRGB)
	{
		if (mShadingLoc.switchAlbedoTosRGB >= 0)
			glUniform1f(mShadingLoc.switchAlbedoTosRGB, (sRGB) ? 1.0f : 0.0f);
	}

	void SuperShader::UploadGlobalAmbient(double *color)
	{
		if (mShadingLoc.globalAmbientLight >= 0)
			glUniform4f(mShadingLoc.globalAmbientLight, (float)color[0], (float)color[1], (float)color[2], 1.0f);
	}

	void SuperShader::UploadLightingInformation(const int numdir, const int numpoint)
	{
		if (mShadingLoc.numberOfDirLights >= 0)
			glUniform1i(mShadingLoc.numberOfDirLights, numdir);
		if (mShadingLoc.numberOfPointLights >= 0)
			glUniform1i(mShadingLoc.numberOfPointLights, numpoint);
	}
}

