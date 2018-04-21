
/**	\file	postprocessing_association.cxx

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "postprocessing_association.h"
#include "postprocessing_data.h"

FBApplyManagerRuleImplementation(PostProcessingAssociation);
FBRegisterApplyManagerRule(PostProcessingAssociation, "PostProcessingAssociation", "Apply Manager Rule for Post Processing plugin");


bool PostProcessingAssociation::IsValidSrc(FBComponent *pSrc)
{
	if (pSrc == nullptr)
		return false;

	if (FBIS(pSrc, FBCamera) || FBIS(pSrc, PostPersistentData) || FBIS(pSrc, FBLight) || FBIS(pSrc, FBModelNull))
		return true;

	return false;
}

bool PostProcessingAssociation::IsValidConnection(FBComponent* pSrc, FBComponent*& pDst, bool& pAllowMultiple)
{ 
	pAllowMultiple = false;

	if ( FBIS(pDst, PostPersistentData) )
	{
		if (FBIS(pSrc, FBCamera) )
		{
			((PostPersistentData*)pDst)->ConnectCamera( (FBCamera*) pSrc);
			pAllowMultiple = true;
			return false;
		}
		else if (FBIS(pSrc, FBLight))
		{
			((PostPersistentData*)pDst)->ConnectLight((FBLight*)pSrc);
			pAllowMultiple = false;
			return false;
		}
		else if (FBIS(pSrc, FBModelNull))
		{
			((PostPersistentData*)pDst)->ConnectFocus((FBModelNull*)pSrc);
			pAllowMultiple = false;
			return false;
		}
		else if (FBIS(pSrc, PostPersistentData))
		{
			// DONE: copy settings from one data to another
			((PostPersistentData*)pDst)->CopyValues((PostPersistentData*)pSrc);
			return false;
		}
	}

	if ( FBIS(pSrc, PostPersistentData) )
	{
		if (FBIS(pDst, FBCamera) )
		{
			((PostPersistentData*)pSrc)->ConnectCamera((FBCamera*)pDst);
			pAllowMultiple = true;
			return false;
		}
		else if (FBIS(pDst, FBLight))
		{
			((PostPersistentData*)pSrc)->ConnectLight((FBLight*)pDst);
			pAllowMultiple = false;
			return false;
		}
		else if (FBIS(pDst, FBModelNull))
		{
			((PostPersistentData*)pSrc)->ConnectFocus((FBModelNull*)pDst);
			pAllowMultiple = false;
			return false;
		}
		else if (FBIS(pDst, PostPersistentData))
		{
			// TODO: copy settings from one data to another
			return false;
		}
	}
	
	return false;
}

bool PostProcessingAssociation::MenuBuild(FBAMMenu* pAMMenu, FBComponent* pFocusedObject)
{
	mEditManipulatorId = -1;
	mDuplicateId = -1;
	/*
	if(pFocusedObject)
	{
		if( pFocusedObject->Is(FBModel::TypeInfo) && (((FBModel*)pFocusedObject)->Geometry != nullptr) )
		{
			pAMMenu->AddOption( "" );
			mEditManipulatorId = pAMMenu->AddOption("Connect Camera As Projector", -1, true);
			//mCloseManipulatorId = pAMMenu->AddOption("Close sculpt brush", -1, true);
			pAMMenu->AddOption( "" );
			return true;
		}
	}
	*/

	if (pFocusedObject)
	{
		
	}

	return false;
}

bool PostProcessingAssociation::MenuAction(int pMenuId, FBComponent* pFocusedObject)
{
	bool result = false;

	if (pMenuId == -1)
		return result;

	if (pMenuId == mDuplicateId)
	{
		
	}

	/*
	if ( (mEditManipulatorId >= 0) && (pMenuId == mEditManipulatorId) )
	{
		printf("connect camera\n");
	}
	*/
	return result;
}