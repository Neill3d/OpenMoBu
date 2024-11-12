
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

namespace Graphics
{
	double sDefaultAttenuationNone[3] = { 1.0, 0.0, 0.0 };
	double sDefaultAttenuationLinear[3] = { 0.0, 0.01, 0.0 };
	double sDefaultAttenuationQuadratic[3] = { 0.0, 0.0, 0.0001 };

	bool ComputeSpotLightMatrices(FBLight* pLight, FBMatrix& pLightView, FBMatrix& pLightProj)
	{
		if (pLight->LightType != kFBLightTypeSpot)
		{
			return false;
		}

		pLightView.Identity();
		pLightProj.Identity();

		// Spotlight support

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

		transformationMat(3, 0) = ((FBVector3d)pLight->Translation)[0];
		transformationMat(3, 1) = ((FBVector3d)pLight->Translation)[1];
		transformationMat(3, 2) = ((FBVector3d)pLight->Translation)[2];
		transformationMat(3, 3) = 1.0f;

		FBMatrixInverse(pLightView, transformationMat);

		// Ok .. now we just need a projection matrix ...
		float fov = 1.2f * pLight->OuterAngle / 2.0f;
		float fFar = (double)pLight->Intensity * 2.0f;
		float fNear = 1.0f;
		float top = tan(fov * 3.14159f / 360.0f) * fNear;
		float bottom = -top;
		float left = bottom;
		float right = top;
		double perspectiveValues[16] =
		{
			(2.0 * fNear) / (right - left),   0,                          0,                          0,
			0,                         (2.0 * fNear) / (top - bottom),    0,                          0,
			0,                         0,                           -(fFar + fNear) / (fFar - fNear), -(2.0f * fFar * fNear) / (fFar - fNear),
			0,                         0,                           -1.0f,                      0
		};

		pLightProj = FBMatrix(perspectiveValues);

		return true;
	}

	bool ComputeInfiniteLightMatrices(FBLight* pLight, FBMatrix& pLightView, FBMatrix& pLightProj, const FBVector4d& worldMin, const FBVector4d& worldMax)
	{
		if (pLight->LightType != kFBLightTypeInfinite)
		{
			return false;
		}

		pLightView.Identity();
		pLightProj.Identity();

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

		double radius;
		FBVector4d newPos;
		OpenMobu::GetWorldBounds(pLight, radius, newPos, worldMin, worldMax);

		transformationMat(3, 0) = newPos[0];
		transformationMat(3, 1) = newPos[1];
		transformationMat(3, 2) = newPos[2];
		transformationMat(3, 3) = 1.0f;

		FBMatrixInverse(pLightView, transformationMat);

		// Ok .. now we just need a projection matrix ...
		double left = -radius;
		double right = radius;
		double top = radius;
		double bottom = -radius;

		double fNear = 0.0f;
		double fFar = radius * 2.0f;

		double diffRL = 1.0 / (right - left);
		double diffTB = 1.0 / (top - bottom);
		double diffFN = 1.0 / (fFar - fNear);

		double orthoValues[16] =
		{
			2.0 * diffRL, 0,            0,                        0,
			0,            2.0 * diffTB, 0,                        0,
			0,            0,            -2.0 * diffFN,            0,
			0,            0,            -(fFar + fNear) * diffFN, 1.0
		};

		pLightProj = FBMatrix(orthoValues);

		return true;
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
	glm::mat4 FBLightProxy::GetProjectionMatrix() const
	{
		glm::mat4 tm;
		FBMatrixToGLM(tm, lightProj);
		return tm;
	}

	// Gets the view matrix for rendering the shadow map from the light's perspective
	glm::mat4 FBLightProxy::GetViewMatrix() const
	{
		glm::mat4 tm;
		FBMatrixToGLM(tm, lightView);
		return tm;
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

		light.spotAngle = (float)cosAngle;
		
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

};