
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BoundingBox.cpp
//
//	Author Sergei Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "BoundingBox.h"
//--- SDK include
#include <fbsdk/fbsdk.h>
#include <cfloat>
#include <utility>


namespace OpenMobu
{

	// Compute the world bounds for infinite light adjustment.
	void ComputeWorldBounds(FBVector4d& worldMin, FBVector4d& worldMax)
	{
		double dblMin[4] = { DBL_MIN, DBL_MIN, DBL_MIN, 1.0 };
		double dblMax[4] = { DBL_MAX, DBL_MAX, DBL_MAX, 1.0 };
		worldMin.Set(dblMax);
		worldMax.Set(dblMin);

		ComputeWorldBounds(FBSystem().SceneRootModel, worldMin, worldMax);
	}

	void ComputeWorldBounds(FBModel* pModel, FBVector4d& worldMin, FBVector4d& worldMax, bool pRecursive)
	{
		bool lLightCam = pModel->Is(FBCamera::TypeInfo) || pModel->Is(FBLight::TypeInfo);
		if (pModel->Geometry && !lLightCam)
		{
			// Only collect geometric world bounds.
			FBVector3d lMin, lMax;
			pModel->GetBoundingBox(lMin, lMax);
			FBVector4d l4Min, l4Max;
			l4Min[0] = lMin[0];
			l4Min[1] = lMin[1];
			l4Min[2] = lMin[2];
			l4Min[3] = 1.0;

			l4Max[0] = lMax[0];
			l4Max[1] = lMax[1];
			l4Max[2] = lMax[2];
			l4Max[3] = 1.0;

			FBMatrix lMtx;
			pModel->GetMatrix(lMtx);
			FBVectorMatrixMult(l4Min, lMtx, l4Min);
			FBVectorMatrixMult(l4Max, lMtx, l4Max);

			if (l4Min[0] < worldMin[0]) worldMin[0] = l4Min[0];
			if (l4Min[1] < worldMin[1]) worldMin[1] = l4Min[1];
			if (l4Min[2] < worldMin[2]) worldMin[2] = l4Min[2];

			if (l4Max[0] > worldMax[0]) worldMax[0] = l4Max[0];
			if (l4Max[1] > worldMax[1]) worldMax[1] = l4Max[1];
			if (l4Max[2] > worldMax[2]) worldMax[2] = l4Max[2];
		}

		// Recursively iterate through the scene hierarchy.
		if (pRecursive)
		{
			for (int i = 0; i < pModel->Children.GetCount(); i++)
			{
				ComputeWorldBounds(pModel->Children[i], worldMin, worldMax, true);
			}
		}
	}

	void GetWorldBounds(FBLight* pLight, double& pRadius, FBVector4d& pPos, const FBVector4d& worldMin, const FBVector4d& worldMax)
	{
		// Default light position is pointing down the -Y axis
		double lBaseLightDir[4] = { 0.0, -1.0, 0.0, 0.0 };
		FBVector4d lDir;
		lDir.Set(lBaseLightDir);

		FBMatrix rotationMat;
		pLight->GetMatrix(rotationMat, kModelRotation, true);

		// Compute transformation matrix.
		double base[16] =
		{
			1.0, 0.0, 0.0, 0.0,
			0.0, 0.0, -1.0, 0.0,
			0.0, 1.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 1.0
		};
		FBMatrix baseMat(base);
		FBMatrix transformMat, invTransformMat;
		FBMatrixMult(transformMat, rotationMat, baseMat);
		FBMatrixInverse(invTransformMat, transformMat);

		// Compute bounds in lightView space.
		FBVector4d lBounds[2];
		FBVectorMatrixMult(lBounds[0], invTransformMat, worldMin);
		FBVectorMatrixMult(lBounds[1], invTransformMat, worldMax);
		double dblMin[4] = { DBL_MIN, DBL_MIN, DBL_MIN, 1.0 };
		double dblMax[4] = { DBL_MAX, DBL_MAX, DBL_MAX, 1.0 };
		FBVector4d lLightMin, lLightMax;
		lLightMin.Set(dblMax);
		lLightMax.Set(dblMin);
		for (int i = 0; i < 3; i++)
		{
			if (lBounds[0][i] < lLightMin[i]) { lLightMin[i] = lBounds[0][i]; }
			if (lBounds[1][i] < lLightMin[i]) { lLightMin[i] = lBounds[1][i]; }
			if (lBounds[0][i] > lLightMax[i]) { lLightMax[i] = lBounds[0][i]; }
			if (lBounds[1][i] > lLightMax[i]) { lLightMax[i] = lBounds[1][i]; }
		}

		// Compute the box center (in world space)
		FBVector4d center;
		center[0] = 0.5 * (lLightMin[0] + lLightMax[0]);
		center[1] = 0.5 * (lLightMin[1] + lLightMax[1]);
		center[2] = 0.5 * (lLightMin[2] + lLightMax[2]);
		center[3] = 1.0;
		FBVectorMatrixMult(center, transformMat, center);

		double w = lLightMax[0] - lLightMin[0];
		double h = lLightMax[1] - lLightMin[1];
		double d = lLightMax[2] - lLightMin[2];

		// Expand the radius by 10%
		double sceneRadius = 0.5 * std::max(w, std::max(h, d));
		sceneRadius *= 1.1f;

		FBVectorMatrixMult(lDir, rotationMat, lDir);
		lDir[0] *= sceneRadius;
		lDir[1] *= sceneRadius;
		lDir[2] *= sceneRadius;

		FBVector4d newPos;
		newPos[0] = center[0] - lDir[0];
		newPos[1] = center[1] - lDir[1];
		newPos[2] = center[2] - lDir[2];
		newPos[3] = 1.0;

		// Return values
		pRadius = sceneRadius;
		pPos = newPos;
	}

}