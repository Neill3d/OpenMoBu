
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: IterativeFit.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "IterativeFit.h"
#include "BlendShapeToolkit_Helper.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// ITERATIVE FIT APPROACH
//


bool BlendShapeIterativeFit( FBModel *pBaseModel, FBModel *pModel )
{
	int shapeId = AddBlendShape( pBaseModel, pModel );
	if (shapeId < 0) return false;

	FBGeometry *pBaseGeometry = pBaseModel->Geometry;
	FBGeometry *pGeometry = pModel->Geometry;

	if (pBaseGeometry == nullptr || pGeometry == nullptr) return false;

	// pre setup
	
	// number of processing units
	//int difCount = pBaseGeometry->ShapeGetDiffPointCount(shapeId);

	
	return false;
}