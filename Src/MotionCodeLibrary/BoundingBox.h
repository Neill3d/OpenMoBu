
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BoundingBox.h
//
//	Author Sergei Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>


namespace OpenMobu
{

	// Compute the world bounds for infinite light adjustment.
	void ComputeWorldBounds(FBVector4d& worldMin, FBVector4d& worldMax);

	void ComputeWorldBounds(FBModel* pModel, FBVector4d& worldMin, FBVector4d& worldMax, bool pRecursive = true);

	void GetWorldBounds(FBLight* pLight, double& pRadius, FBVector4d& pPos, const FBVector4d& worldMin, const FBVector4d& worldMax);
};
