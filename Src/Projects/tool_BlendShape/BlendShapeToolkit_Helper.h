
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BlendShapeToolkit_Helper.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <GL/glew.h>
//--- SDK include
#include <fbsdk/fbsdk.h>

enum FBShapeChangeReason
{
	kFBShapeRemove,
	kFBShapeRename,
	kFBShapeDublicate,
	kFBShapeMoveUp,
	kFBShapeMoveDown
};

enum FBBlendShapeLoadMode
{
	kFBShapeLoad=1,
	kFBShapeMerge,
	kFBShapeAppend
};


//
void FindUniqueBlendShapeName(FBModel *pModel, const char *defaultName, FBString &outName);

//

const int CalculateDifference( const int numberOfVertices, const FBVertex *positions, const FBVertex *positionsBase, int *outOriginIndex, FBVertex *outPositions, const bool calcNormals, const FBNormal *normals, const FBNormal *normalsBase, FBNormal *outNormals );
int AddBlendShape( FBModel *pBaseModel, FBModel *pModel );
int AddBlendShape( FBModel *pBaseModel, const int pointCount, const int *difOriginIndex, const int difCount, const FBVertex *difPositions, const FBNormal *difNormals, const char *defaultShapeName = "newShape" );

void ModifyGeometryShapes(FBGeometry *pGeometry, bool *checkList, const FBShapeChangeReason reason);

bool	Blendshapes_SaveXML( FBModelList &modelList, const char *filename );
bool	Blendshapes_LoadXML( FBModelList &modelList, const char *filename, const FBBlendShapeLoadMode mode );

//
// get in mesh and compute out mesh according to the deformation information in the model
//
FBModel *CalculateDeformedMesh( FBModel *in );

//

void UnTransformBlendShapes( FBVertex &vertex, const int index, FBModel *pModel );
void CalculateDeltaMesh( FBModel *pModelBASE, const int numberOfVertices, const double *length, const FBVertex *vertices, const FBNormal *normals, FBVertex *outVertices, FBNormal *outNormals);

bool HasModelCluster(FBModel *pmodel);
bool HasModelBlendshapes(FBModel *pModel);

////////////////////////////////////////////////////////////////////////////////
//

void	SculptMode( FBModel *pFocusedObject, bool enter );
void	InitTempDeformer(FBModel *pModel, FBManipulator *pManipulator);
void	FreeTempDeformer();
FBConstraint *GetTempDeformer();