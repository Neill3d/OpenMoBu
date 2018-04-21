
/** \file   autoKeyPatch_manager.cxx

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "qttestwidget.h"
#include "orcustommanager_template_manager.h"

#include <Windows.h>

//--- Registration defines
#define ORCUSTOMMANAGER_TEMPLATE__CLASS ORCUSTOMMANAGER_TEMPLATE__CLASSNAME
#define ORCUSTOMMANAGER_TEMPLATE__NAME  ORCUSTOMMANAGER_TEMPLATE__CLASSSTR

//--- FiLMBOX implementation and registration
FBCustomManagerImplementation( ORCUSTOMMANAGER_TEMPLATE__CLASS  );  // Manager class name.
FBRegisterCustomManager( ORCUSTOMMANAGER_TEMPLATE__CLASS );         // Manager class name.

bool static gFirstTime = true;
QtTestWidget* lWidget = nullptr;

/************************************************
 *  FiLMBOX Creation
 ************************************************/
bool ORCustomManager_Template::FBCreate()
{
	mNeedSuperAuto = false;
    return true;
}


/************************************************
 *  FiLMBOX Destruction.
 ************************************************/
void ORCustomManager_Template::FBDestroy()
{
    // Free any user memory here.
	if (lWidget)
	{
		delete lWidget;
		lWidget = nullptr;
	}
}


/************************************************
 *  Execution callback.
 ************************************************/
bool ORCustomManager_Template::Init()
{
    return true;
}

bool ORCustomManager_Template::Open()
{
	FBConfigFile	lConfig("@PATCHES.txt");

	char buffer[64] = { 0 };
	sprintf_s(buffer, sizeof(char)* 64, "True");
	const char *cbuffer = buffer;
	bool lStatusAuto = lConfig.GetOrSet("Auto Interpolation Patch", "Enable", cbuffer, "Put True/False to control the auto interpolation keys patch");

	if (true == lStatusAuto)
	{
		mEnabled = (0 == strcmp(cbuffer, "True"));
	}
	
	cbuffer = buffer;
	bool lStatusKeys = lConfig.GetOrSet("Auto Interpolation KeyPress", "Enable", cbuffer, "Put True/False to set auto interpolation by key press");

	if (true == lStatusKeys)
	{
		mEnabledSuper = (0 == strcmp(cbuffer, "True"));
	}

	if (true == mEnabled || true == mEnabledSuper)
	{
		mSystem.OnUIIdle.Add(this, (FBCallback)&ORCustomManager_Template::OnUIIdle);
	}
    return true;
}

bool ORCustomManager_Template::Clear()
{
    return true;
}

bool ORCustomManager_Template::Close()
{
	if (true == mEnabled)
	{
		mSystem.OnUIIdle.Remove(this, (FBCallback)&ORCustomManager_Template::OnUIIdle);
	}
    return true;
}



void ORCustomManager_Template::OnUIIdle(HISender pSender, HKEvent pEvent)
{
	//mSystem.OnUIIdle.Remove(this, (FBCallback)&ORToolQtTest::OnUIIdle);

	if (true == gFirstTime)
	{
		lWidget = new QtTestWidget(0);
		gFirstTime = false;
	}

	if (isTriggered())
	{
		//
		SetAutoInterpolation();
		FreeCurves();

		SetTrigger(false);
	}

	if (isSendKeys())
	{
		SetSendKeys(false);

		mZoomStart = mPlayer.ZoomWindowStart;
		mZoomStop = mPlayer.ZoomWindowStop;

		mLoopStart = mPlayer.LoopStart;
		mLoopStop = mPlayer.LoopStop;

		DoSendKeys();

		mNeedSuperAuto = true;
	}
	else if (true == mNeedSuperAuto)
	{
		mNeedSuperAuto = false;

		// check we timeline doesn't change (only one keyframe or all timeline selected)

		FBTime zoomStart, zoomStop;

		zoomStart = mPlayer.ZoomWindowStart;
		zoomStop = mPlayer.ZoomWindowStop;

		if (zoomStart == mZoomStart && zoomStop == mZoomStop)
		{
			int lOption = 3;
			
			FBModelList *pList = FBCreateModelList();
			FBGetSelectedModels(*pList);

			if (pList->GetCount() > 0)
				lOption = FBMessageBox("Auto interpolation patch", "Please choose Auto interpolation mode", "Under Cursor", "All Timeline", "Cancel");

			FBDestroyModelList(pList);

			if (1 == lOption)
			{
				FBTime localtime = mSystem.LocalTime;
				SetAutoInterpolationSuper(localtime, localtime);
			}
			else if (2 == lOption)
			{
				SetAutoInterpolationSuper(zoomStart, zoomStop);
			}
		}
		else
		{
			SetAutoInterpolationSuper(zoomStart, zoomStop);
		}
		

		mPlayer.LoopStop = mLoopStop;
		mPlayer.LoopStart = mLoopStart;
		
		mPlayer.ZoomWindowStop = mZoomStop;
		mPlayer.ZoomWindowStart = mZoomStart;
	}
}