
#pragma once

// FBLightProxy.cpp
/*
Sergei <Neill3d> Solokhin 2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "FBLightProxy.h"
#include "BoundingBox.h"
#include "math3d.h"
#include "glm_utils.h"

#include <glm\mat4x4.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

namespace Graphics
{
	double sDefaultAttenuationNone[3] = { 1.0, 0.0, 0.0 };
	double sDefaultAttenuationLinear[3] = { 0.0, 0.01, 0.0 };
	double sDefaultAttenuationQuadratic[3] = { 0.0, 0.0, 0.0001 };

	bool ComputeSpotLightMatrices(FBLight* pLight, glm::mat4& lightView, glm::mat4& lightProj)
	{
		if (pLight->LightType != kFBLightTypeSpot)
		{
			return false;
		}

		lightView = glm::mat4(1.0f);
		lightProj = glm::mat4(1.0f);

		// Spotlight support

		// Get all the information necessary to setup the lighting matrix
		// Will need to create a MODELVIEWPROJ matrix using:
		//		- Transformation matrix of light
		//		- Custom projection matrix based on light

		// We need a base matrix because the transformation matrix doesn't take into account that lights
		// start out with transforms that are not represented ... grr ...
		const double base[16] =
		{
			1.0, 0.0, 0.0, 0.0,
			0.0, 0.0, -1.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 1.0
		};
		FBMatrix baseMat(base);

		// Factor out the scale, because we don't want it ...
		FBMatrix rotationMat;
		FBMatrix transformationMat;
		pLight->GetMatrix(rotationMat, kModelRotation, true);
		FBMatrixMult(transformationMat, rotationMat, baseMat);
		
		transformationMat(3, 0) = ((FBVector3d)pLight->Translation)[0];
		transformationMat(3, 1) = ((FBVector3d)pLight->Translation)[1];
		transformationMat(3, 2) = ((FBVector3d)pLight->Translation)[2];
		transformationMat(3, 3) = 1.0f;

		FBMatrixToGLM(lightView, transformationMat);
		
		const float fFar = 10000.0f;
		const float fNear = 0.1f;
		lightProj = glm::perspective(static_cast<float>(DEG2RAD(pLight->OuterAngle)), 1.0f, fNear, fFar);

		return true;
	}

	bool ComputeInfiniteLightMatrices(FBLight* pLight, glm::mat4& lightView, glm::mat4& lightProj, const FBVector4d& worldMin, const FBVector4d& worldMax)
	{
		if (pLight->LightType != kFBLightTypeInfinite)
		{
			return false;
		}

		lightView = glm::mat4(1.0f);
		lightProj = glm::mat4(1.0f);

		// Directional light support

		// Get all the information necessary to setup the lighting matrix
		// Will need to create a MODELVIEWPROJ matrix using:
		//		- Transformation matrix of light
		//		- Custom projection matrix based on light

		// We need a base matrix because the transformation matrix doesn't take into account that lights
		// start out with transforms that are not represented ... grr ...
		double base[16] =
		{
			1.0, 0.0, 0.0, 0.0,
			0.0, 0.0, -1.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 1.0
		};
		FBMatrix baseMat(base);

		// Factor out the scale, because we don't want it ...
		FBMatrix rotationMat;
		FBMatrix transformationMat;
		pLight->GetMatrix(rotationMat, kModelRotation, true);
		FBMatrixMult(transformationMat, rotationMat, baseMat);

		double dradius;
		FBVector4d newPos;
		OpenMobu::GetWorldBounds(pLight, dradius, newPos, worldMin, worldMax);
		const float radius = static_cast<float>(dradius);

		transformationMat(3, 0) = newPos[0];
		transformationMat(3, 1) = newPos[1];
		transformationMat(3, 2) = newPos[2];
		transformationMat(3, 3) = 1.0f;

		FBMatrixToGLM(lightView, transformationMat);
		//FBMatrixInverse(pLightView, transformationMat);

		// Ok .. now we just need a projection matrix ...
		const float left = -radius;
		const float right = radius;
		const float top = radius;
		const float bottom = -radius;

		const float fNear = 0.0f;
		const float fFar = radius * 2.0f;

		const float diffRL = 1.0 / (right - left);
		const float diffTB = 1.0 / (top - bottom);
		const float diffFN = 1.0 / (fFar - fNear);

		const float orthoValues[16] =
		{
			2.0f * diffRL, 0.0f,            0.0f,                        0.0f,
			0.0f,            2.0f * diffTB, 0.0f,                        0.0f,
			0.0f,            0.0f,            -2.0f * diffFN,            0.0f,
			0.0f,            0.0f,            -(fFar + fNear) * diffFN, 1.0f
		};

		lightProj = glm::make_mat4(orthoValues);

		return true;
	}

	bool FBLightProxy::HasCustomBoundingBox() const
	{
		if (FBLight* light = lightPlug) 
		{
			return light->PropertyList.Find("Shadow Bounding Box") != nullptr;
		}
		return false;
	}
	bool FBLightProxy::GetCustomBoundingBox(glm::vec3& bbMin, glm::vec3& bbMax) const
	{
		if (FBLight* light = lightPlug)
		{
			if (FBProperty* prop = light->PropertyList.Find("Shadow Bounding Box"))
			{
				if (FBPropertyListObject* listObjectProp = FBCast<FBPropertyListObject>(prop))
				{
					FBVector4d worldMin;
					FBVector4d worldMax;

					double dblMin[4] = { DBL_MIN, DBL_MIN, DBL_MIN, 1.0 };
					double dblMax[4] = { DBL_MAX, DBL_MAX, DBL_MAX, 1.0 };
					worldMin.Set(dblMax);
					worldMax.Set(dblMin);

					for (int i = 0; i < listObjectProp->GetCount(); ++i)
					{
						if (FBIS(listObjectProp->GetAt(i), FBModel))
						{
							FBModel* boundingBoxModel = FBCast<FBModel>(listObjectProp->GetAt(i));
							OpenMobu::ComputeWorldBounds(boundingBoxModel, worldMin, worldMax, false);
						}
					}

					for (int i = 0; i < 3; ++i)
					{
						bbMin[i] = static_cast<float>(worldMin[i]);
						bbMax[i] = static_cast<float>(worldMax[i]);
					}
				}
				return true;
			}
		}
		return false;
	}

	bool FBLightProxy::PrepareMatrices(const glm::vec3& worldMin, const glm::vec3& worldMax)
	{
		bool status = false;

		if (FBLight* light = lightPlug)
		{	
			if (light->LightType == kFBLightTypeSpot)
			{
				status = ComputeSpotLightMatrices(light, lightView, lightProj);
			}
			else if (light->LightType == kFBLightTypeInfinite)
			{
				status = ComputeInfiniteLightMatrices(light, lightView, lightProj,
					FBVector4d(static_cast<double>(worldMin.x), static_cast<double>(worldMin.y), static_cast<double>(worldMin.z), 1.0),
					FBVector4d(static_cast<double>(worldMax.x), static_cast<double>(worldMax.y), static_cast<double>(worldMax.z), 1.0));
			}
		}
		return status;
	}

	// Gets the projection matrix for shadow mapping
	const glm::mat4& FBLightProxy::GetProjectionMatrix() const
	{
		return lightProj;
	}

	// Gets the view matrix for rendering the shadow map from the light's perspective
	const glm::mat4& FBLightProxy::GetViewMatrix() const
	{
		return lightView;
	}

	void FBLightProxy::ConstructFromFBLight(TLight& light, const bool ToEyeSpace, const glm::mat4& lViewMatrix, const glm::mat4& lViewRotationMatrix, FBLight* pLight)
	{
		memset(&light, 0, sizeof(TLight));

		// Get the light's position, direction, colour, and cone angle (stored in direction's 4th component)
		// Also setup attenuation for the lighting equation ...
		// Note that for point/spot lights, we add a 1.0 to the 4th colour
		// component so we can know to generate the direction from position
		FBVector3d   pos;
		pLight->GetVector(pos); // Get Global Translation.

		// Convert the ModelView (World) space
		
		light.position = glm::vec3(static_cast<float>(pos[0]), (float)pos[1], (float)pos[2]);
		if (ToEyeSpace)
		{
			light.position = lViewMatrix * glm::vec4(light.position, 1.0f);
		}
		
		// Set whether or not we take the spot factor into consideration in the last position spot ...
		switch (pLight->LightType.AsInt())
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

		// Setup color and attenuation 
		FBColorAndAlpha diffuseColor = (FBColor)pLight->DiffuseColor;
		double attenuation[4];

		if (pLight->LightType.AsInt() != kFBLightTypeInfinite)
		{
			switch (pLight->AttenuationType.AsInt())
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

		FBProperty* lprop = pLight->PropertyList.Find("Magnitude");
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


		light.castSpecularOnObject = 1.0f;
		FBProperty* castSpecularProp = pLight->PropertyList.Find("Cast Specular On Object");
		if (castSpecularProp)
		{
			const bool castSpecularValue = castSpecularProp->AsInt() > 0;
			light.castSpecularOnObject = (castSpecularValue) ? 1.0f : -1.0f;
		}

		light.radius = (float)scale[0] * 20.0;

		FBVectorToVec3(diffuseColor, &light.color[0]);
		FBVectorToVec4(attenuation, &light.attenuations[0]);

		// Get the rotation matrix and multiply light vector with it
		FBMatrix rotationMatrix;
		FBVector4d direction;
		pLight->GetMatrix(rotationMatrix, kModelRotation, true);
		FBVector4d normal(0, -1, 0, 1);
		FBVectorMatrixMult(direction, rotationMatrix, normal);

		// Convert the ModelView (World) space
		light.dir = glm::vec3(static_cast<float>(direction[0]), static_cast<float>(direction[1]), static_cast<float>(direction[2]));
		if (ToEyeSpace)
		{
			light.dir = lViewRotationMatrix * glm::vec4(light.dir, 0.0f);
		}
			
		double cosAngle = 0.0;
		if (pLight->LightType.AsInt() == kFBLightTypeSpot)
		{
#if(PRODUCT_VERSION >= 2024)
			cosAngle = cos((3.141592654 * pLight->OuterAngle / 180.0f) / 2.0f);
#else
			cosAngle = cos((3.141592654 * pLight->ConeAngle / 180.0f) / 2.0f);
#endif
		}

		light.spotAngle = static_cast<float>(cosAngle);
	}

};