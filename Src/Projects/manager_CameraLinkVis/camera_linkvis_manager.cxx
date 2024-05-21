
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: camera_linkvis_manager.cpp
//
//	Author Sergei Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/OpenMoBu
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declaration
#include "camera_linkvis_manager.h"

//--- Registration defines
#define CAMERA_LINKVIS__CLASS CAMERA_LINKVIS__CLASSNAME
#define CAMERA_LINKVIS__NAME  CAMERA_LINKVIS__CLASSSTR

//--- FiLMBOX implementation and registration
FBCustomManagerImplementation( CAMERA_LINKVIS__CLASS  );  // Manager class name.
FBRegisterCustomManager( CAMERA_LINKVIS__CLASS );         // Manager class name.

FBGroup* Manager_CameraLinkVis::gGroup = nullptr;
FBCamera* Manager_CameraLinkVis::gCamera = nullptr;

namespace NLinkVis_Internal
{
	constexpr const char* GROUP_NAME = "LinkedGroup";
}

/************************************************
 *  FiLMBOX Creation
 ************************************************/
bool Manager_CameraLinkVis::FBCreate()
{
	mLastCamera = nullptr;

    return true;
}


/************************************************
 *  FiLMBOX Destruction.
 ************************************************/
void Manager_CameraLinkVis::FBDestroy()
{
    // Free any user memory here.
}


/************************************************
 *  Execution callback.
 ************************************************/
bool Manager_CameraLinkVis::Init()
{
    return true;
}

bool Manager_CameraLinkVis::Open()
{
	mSystem.Scene->OnChange.Add(this, (FBCallback) &Manager_CameraLinkVis::EventSceneChange);

	FBEvaluateManager::TheOne().OnRenderingPipelineEvent.Add(this, (FBCallback) &Manager_CameraLinkVis::EventRender);
	
    return true;
}

bool Manager_CameraLinkVis::Clear()
{
    return true;
}

bool Manager_CameraLinkVis::Close()
{
	mSystem.Scene->OnChange.Remove(this, (FBCallback) &Manager_CameraLinkVis::EventSceneChange);

	FBEvaluateManager::TheOne().OnRenderingPipelineEvent.Remove(this, (FBCallback) &Manager_CameraLinkVis::EventRender);
	
    return true;
}

void Manager_CameraLinkVis::EventRender(HISender pSender, HKEvent pEvent)
{
	FBEventEvalGlobalCallback	levent(pEvent);

	if (levent.GetTiming() == kFBGlobalEvalCallbackBeforeRender)
	{
		EventUIIdle(nullptr, nullptr);
	}
}

void Manager_CameraLinkVis::EventUIIdle(HISender pSender, HKEvent pEvent)
{
	const int paneIndex = mSystem.Scene->Renderer->GetSelectedPaneIndex();

	if (paneIndex < 0)
		return;

	FBCamera *pCamera = mSystem.Scene->Renderer->GetCameraInPane(paneIndex);

	if (FBIS(pCamera, FBCameraSwitcher))
		pCamera = ( FBCast<FBCameraSwitcher>(pCamera) )->CurrentCamera;

	if (pCamera != mLastCamera)
	{
		// leave all cameras groups
		FBScene *pScene = mSystem.Scene;
		for (int i=0; i<pScene->Cameras.GetCount(); ++i)
		{
			FBCamera *iCam = FBCast<FBCamera>(pScene->Cameras[i]);
			if (iCam->SystemCamera == false)
				LeaveCamera(iCam);
		}

		// enter one current
		EnterCamera(pCamera);
	}

	mLastCamera = pCamera;
}

bool Manager_CameraLinkVis::GroupVis(const char *groupName, const bool show)
{
	FBScene *pScene = FBSystem::TheOne().Scene;

	for (int i=0; i<pScene->Groups.GetCount(); ++i)
	{
		if ( strcmp( pScene->Groups[i]->Name, groupName ) == 0 )
		{
			pScene->Groups[i]->Show = show;

			FBGroup *pGroup = pScene->Groups[i];
			for (int j=0, count=pGroup->Items.GetCount(); j<count; ++j)
			{
				FBComponent *pcomp = pGroup->Items[j];
				if (FBIS(pcomp, FBModel))
				{
					( FBCast<FBModel>(pcomp) )->Show = show;
				}
			}

			return true;
		}
	}

	return false;
}

void Manager_CameraLinkVis::LeaveCamera(FBCamera *pCamera)
{
	if (pCamera == nullptr) return;

	FBProperty *pProp = pCamera->PropertyList.Find(NLinkVis_Internal::GROUP_NAME);
	if (pProp)
	{
		const char *groupName = pProp->AsString();
		GroupVis( groupName, false );
	}
}

void Manager_CameraLinkVis::EnterCamera(FBCamera *pCamera)
{
	if (pCamera == nullptr) return;

	FBProperty *pProp = pCamera->PropertyList.Find(NLinkVis_Internal::GROUP_NAME);
	if (pProp)
	{
		const char *groupName = pProp->AsString();
		GroupVis( groupName, true );
	}
}

FBCamera* Manager_CameraLinkVis::FindCameraByGroup(const char *groupName)
{
	FBScene *pScene = FBSystem::TheOne().Scene;

	for (int i=0; i<pScene->Cameras.GetCount(); ++i)
	{
		FBCamera *pCamera = FBCast<FBCamera>(pScene->Cameras[i]);

		if (pCamera->SystemCamera == false)
		{
			FBProperty *lProp = pCamera->PropertyList.Find(NLinkVis_Internal::GROUP_NAME);
			if (lProp)
			{
				const char *lname = lProp->AsString();
				if ( strcmp( lname, groupName ) == 0 )
				{
					return pCamera;
				}
			}
		}
	}

	return nullptr;
}

void Manager_CameraLinkVis::EventSceneChange(HISender pSender, HKEvent pEvent)
{
	FBEventSceneChange sceneEvent(pEvent);

	if (sceneEvent.Type.AsInt() == kFBSceneChangeRename)
	{
		if ( FBIS(sceneEvent.Component, FBGroup) )
		{
			gGroup = (FBGroup*) (FBComponent*) sceneEvent.Component;
			gCamera = FindCameraByGroup(gGroup->Name);
		}
	}
	else if (sceneEvent.Type.AsInt() == kFBSceneChangeRenamed)
	{
		if (gGroup != nullptr && gCamera != nullptr)
		{
			FBProperty *lProp = gCamera->PropertyList.Find(NLinkVis_Internal::GROUP_NAME);
			if (lProp)
			{
				lProp->SetString( gGroup->Name );
			}
		}
	}
}