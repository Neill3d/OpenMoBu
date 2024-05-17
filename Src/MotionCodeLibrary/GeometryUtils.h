
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: GeometryUtils.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>

//
// function to center model pivot (recalculate mesh vertex positions)
//
void CenterPivot(FBModel *pModel);

//
//
void ReComputeNormals(FBModel *pModel);
void InvertNormals(FBModel *pModel);


//
// work with meshes 
//

// ResetXForm - bake base model transformation into the snapshot mesh or not ?
FBModel *MakeSnapshot( FBModel *pModel, const bool ResetXForm );
// more corrent snapshot using FBX SDK
FBModel *MakeSnapshot2(FBModel *pModel, const bool ResetXForm, const bool CopyShaders );

FBModel *CombineModels(FBModelList &modelList);