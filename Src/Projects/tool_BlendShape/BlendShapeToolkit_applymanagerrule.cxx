
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BlendShapeToolkit_applymanagerrule.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declaration
#include "BlendShapeToolkit_applymanagerrule.h"
#include "BlendShapeToolkit_manip.h"

FBApplyManagerRuleImplementation(KSculptBrushAssociation);
FBRegisterApplyManagerRule( KSculptBrushAssociation, "KSculptBrushAssociation", "Apply Manager Rule for Sculpt brush context menu on model object");


bool KSculptBrushAssociation::MenuBuild( FBAMMenu* pAMMenu, FBComponent* pFocusedObject)
{
	mEditManipulatorId = -1;
	if(pFocusedObject && FBIS(pFocusedObject, FBModel) )
	{
		FBModel *pModel = (FBModel*) pFocusedObject;
		FBGeometry *pGeometry = pModel->Geometry;
		FBModelVertexData *pData = pModel->ModelVertexData;

		if( (nullptr != pGeometry) && (nullptr != pData)
			&& !FBIS(pModel, FBLight) && !FBIS(pModel, FBCamera) && !FBIS(pModel, FBModelNull)
			&& !FBIS(pModel, FBModelSkeleton) )
		{
			pAMMenu->AddOption( "" );
			mEditManipulatorId = pAMMenu->AddOption("Sculpt Brush Tool", -1, true);
			//mCloseManipulatorId = pAMMenu->AddOption("Close sculpt brush", -1, true);
			pAMMenu->AddOption( "" );
			return true;
		}
	}
	return false;
}



bool KSculptBrushAssociation::MenuAction( int pMenuId, FBComponent* pFocusedObject)
{
	bool result = false;

	if ( (mEditManipulatorId >= 0) && (pMenuId == mEditManipulatorId) )
	{
		FBPopNormalTool( "Sculpt Brush Tool" );
		//SculptMode( (FBModel*) pFocusedObject, pMenuId == mEditManipulatorId );
	}
	return result;
}