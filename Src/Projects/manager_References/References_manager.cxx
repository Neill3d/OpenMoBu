
/** \file   References_manager.cxx

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//--- Class declaration
#include "References_manager.h"
#include "References_Exchange.h"
#include "References_Common.h"

#include "References_DescHolder.h"

#include <iostream>
#include <fstream>
#include <ostream>
#include <string.h>
#include <sstream>
#include <vector>

//--- Registration defines
#define REFERENCES_MANAGER__CLASS REFERENCES_MANAGER__CLASSNAME
#define REFERENCES_MANAGER__NAME  REFERENCES_MANAGER__CLASSSTR

//--- FiLMBOX implementation and registration
FBCustomManagerImplementation(REFERENCES_MANAGER__CLASS);  // Manager class name.
FBRegisterCustomManager(REFERENCES_MANAGER__CLASS);         // Manager class name.

///////////////////////////////////////////////////////////////////////////////////////////

static bool gNewTransaction = false;
static bool gSkipPlugData = false;

/////////////////////////////////////////////////////

bool ReplaceScriptNS(const char *scriptpath, const char *ns, const char *outpath, const char *arg1 = nullptr)
{
	const char *ctrlline = "lRefName = ''";
	const char *argline = "lArg1 = ''";

	// read
	std::ifstream ifs(scriptpath, std::ifstream::in);

	if (false == ifs.is_open())
		return false;

	std::ofstream ofs(outpath, std::ifstream::out);

	if (false == ofs.is_open())
		return false;

	std::string readout;
	while (std::getline(ifs, readout))
	{
		if (nullptr != strstr(readout.c_str(), ctrlline))
		{
			readout = "lRefName = '" + std::string(ns) + "'";
		}
		else if (nullptr != arg1 && nullptr != strstr(readout.c_str(), argline))
		{
			readout = "lArg1 = '" + std::string(arg1) + "'";
		}

		ofs.write(readout.c_str(), readout.size());
		ofs << "\n";
	}

	ifs.close();
	ofs.close();

	return true;
}

void EnterSkipPlugData()
{
	gSkipPlugData = true;
}

void LeaveSkipPlugData()
{
	gSkipPlugData = false;
}

bool IsSkipPlugData()
{
	return gSkipPlugData;
}

/************************************************
 *  FiLMBOX Creation
 ************************************************/
bool ReferencesManager::FBCreate()
{
	mFirstTime = true;
	//mProcessChanges = false;
    return true;
}


/************************************************
 *  FiLMBOX Destruction.
 ************************************************/
void ReferencesManager::FBDestroy()
{
    // Free any user memory here.
	
}


/************************************************
 *  Execution callback.
 ************************************************/
bool ReferencesManager::Init()
{
    return true;
}

bool ReferencesManager::Open()
{
	mSystem.OnUIIdle.Add(this, (FBCallback)&ReferencesManager::OnUIIdle);

    return true;
}

bool ReferencesManager::Clear()
{
    return true;
}

bool ReferencesManager::Close()
{
	mSystem.OnUIIdle.Remove(this, (FBCallback)&ReferencesManager::OnUIIdle);

	//mSystem.OnUIIdle.Remove(this, (FBCallback)&ReferencesManager::OnUIIdle);
	mSystem.OnConnectionDataNotify.Remove(this, (FBCallback)&ReferencesManager::OnPlugDataNotify);
	//mSystem.OnConnectionNotify.Remove(this, (FBCallback)&ReferencesManager::OnPlugConnNotify);
	mSystem.Scene->OnChange.Remove(this, (FBCallback)&ReferencesManager::OnSceneChange);

	mApp.OnFileNew.Remove(this, (FBCallback)&ReferencesManager::OnFileNew);
	mApp.OnFileNewCompleted.Remove(this, (FBCallback)&ReferencesManager::OnFileNewCompleted);
	mApp.OnFileOpenCompleted.Remove(this, (FBCallback)&ReferencesManager::OnFileOpenCompleted);
	mApp.OnFileExit.Remove(this, (FBCallback)&ReferencesManager::OnFileExit);

	FBFileMonitoringManager::TheOne().OnFileChangeFileReference.Remove(this, (FBCallback)&ReferencesManager::OnFileRefChange);

    return true;
}

void ReferencesManager::OnUIIdleAfterOpen(HISender pSender, HKEvent pEvent)
{
	mSystem.OnUIIdle.Remove(this, (FBCallback)&ReferencesManager::OnUIIdleAfterOpen);

	//
	// post load references
	ReferencesPostLoad();
}

void ReferencesManager::OnUIIdle(HISender pSender, HKEvent pEvent)
{
	//mSystem.OnUIIdle.Remove(this, (FBCallback)&ReferencesManager::OnUIIdle);

	// check queue to unload, queue to load

	if (false == FBMergeTransactionFileRefEditIsOn() && false == FBMergeTransactionIsOn() 
		&& false == IsSkipPlugData() )
	{
		EnterSkipPlugData();

		if (mVectorReload.size() > 0)
		{
			for (auto iter = begin(mVectorReload); iter != end(mVectorReload); ++iter)
			{
				FBFileReference *pref = *iter;
				
				FBString scriptPath(GetActionPath(), ACTION_RELOAD);
				FBString outPath(GetSystemTempPath(), ACTION_RELOAD_TEMP);

				if (true == ReplaceScriptNS(scriptPath, pref->LongName, outPath))
				{
					mApp.ExecuteScript(outPath);
				}
			}

			mVectorReload.clear();
		}
		
		if (mVectorUnload.size() > 0)
		{
			for (auto iter = begin(mVectorUnload); iter != end(mVectorUnload); ++iter)
			{
				FBFileReference *pref = *iter;
				//pref->IsLoaded = false;

				//
				if (FileExists(pref->ReferenceFilePath) && pref->IsLoaded)
				{
					FBString path(pref->ReferenceFilePath);
					FBFileMonitoringManager::TheOne().RemoveFileFromMonitor(path);
				}

				FBString scriptPath(GetActionPath(), ACTION_UNLOAD);
				FBString outPath(GetSystemTempPath(), ACTION_UNLOAD_TEMP);

				if (true == ReplaceScriptNS(scriptPath, pref->LongName, outPath))
				{
					mApp.ExecuteScript(outPath);
				}
			}
			mVectorUnload.clear();
		}
		
		//
		if (mVectorLoad.size() > 0)
		{
			for (auto iter = begin(mVectorLoad); iter != end(mVectorLoad); ++iter)
			{
				FBFileReference *pref = *iter;

				FBString scriptPath(GetActionPath(), ACTION_LOAD);
				FBString outPath(GetSystemTempPath(), ACTION_LOAD_TEMP);

				if (true == ReplaceScriptNS(scriptPath, pref->LongName, outPath))
				{
					mApp.ExecuteScript(outPath);
				}
				
				// check file path and add to file monitor
				if (FileExists(pref->ReferenceFilePath) && pref->IsLoaded)
				{
					FBString path(pref->ReferenceFilePath);
					FBFileMonitoringManager::TheOne().AddFileToMonitor(path, kFBFileMonitoring_FILEREFERENCE);
				}
			}
			mVectorLoad.clear();
		}
		
		LeaveSkipPlugData();
	}
	

	//
	if (true == mFirstTime)
	{
		mFirstTime = false;

		//
		// Main menu

		FBMenuManager man;
		FBGenericMenu *pMenu = man.GetMenu("File");

		if (nullptr != pMenu)
		{
			const char *importAfter = "Import &Point Cache...";
			FBGenericMenuItem *itemAfter = nullptr;

			for (FBGenericMenuItem *pItem = pMenu->GetFirstItem(); pItem != pMenu->GetLastItem(); pItem = pMenu->GetNextItem(pItem))
			{
				const char *name = pItem->Caption;
				if (0 == strcmp(importAfter, name))
				{
					itemAfter = pItem;
					break;
				}
			}

			if (nullptr != itemAfter)
			{
				FBGenericMenuItem *pSep = pMenu->InsertAfter(itemAfter, "", -1);
				pMenu->InsertAfter(pSep, ACTION_REFAFILE_MENU, 102);
			}

			pMenu->OnMenuActivate.Add(this, (FBCallback)&ReferencesManager::OnMenuActivate);
		}

		//
		// Script path

		FindAScriptPath();

		//
		// Events

		mSystem.OnConnectionDataNotify.Add(this, (FBCallback)&ReferencesManager::OnPlugDataNotify);
		//mSystem.OnConnectionNotify.Add(this, (FBCallback)&ReferencesManager::OnPlugConnNotify);
		mSystem.Scene->OnChange.Add(this, (FBCallback)&ReferencesManager::OnSceneChange);

		mApp.OnFileNew.Add(this, (FBCallback)&ReferencesManager::OnFileNew);
		mApp.OnFileNewCompleted.Add(this, (FBCallback)&ReferencesManager::OnFileNewCompleted);
		mApp.OnFileOpenCompleted.Add(this, (FBCallback)&ReferencesManager::OnFileOpenCompleted);

		mApp.OnFileExit.Add(this, (FBCallback)&ReferencesManager::OnFileExit);

		FBFileMonitoringManager::TheOne().OnFileChangeFileReference.Add(this, (FBCallback)&ReferencesManager::OnFileRefChange);
	}
}

void ReferencesManager::OnMenuActivate(HISender pSender, HKEvent pEvent)
{
	FBEventMenu lEvent(pEvent);

	if (102 == lEvent.Id)
	{
		FBString path(GetActionPath(), ACTION_REFAFILE);
		mApp.ExecuteScript(path);
	}
}

void ReferencesManager::OnFileNew(HISender pSender, HKEvent pEvent)
{
	gNewTransaction = true;
}

void ReferencesManager::OnFileExit(HISender pSender, HKEvent pEvent)
{
	gNewTransaction = true;
}

void ReferencesManager::OnFileNewCompleted(HISender pSender, HKEvent pEvent)
{
	gNewTransaction = false;
}

void ReferencesManager::OnFileOpenCompleted(HISender pSender, HKEvent pEvent)
{
	FBScene *pScene = mSystem.Scene;

	for (int i = 0, count = pScene->Namespaces.GetCount(); i < count; ++i)
	{
		if (FBIS(pScene->Namespaces[i], FBFileReference))
		{
			FBFileReference *pref = (FBFileReference*) pScene->Namespaces[i];

			if (true == pref->IsLoaded)
				mVectorLoad.insert(pref);
		}
	}

	mSystem.OnUIIdle.Add(this, (FBCallback)&ReferencesManager::OnUIIdleAfterOpen);
}

void ReferencesManager::OnSceneChange(HISender pSender, HKEvent pEvent)
{
	FBEventSceneChange lEvent(pEvent);

	//
	if (true == gNewTransaction)
		return;

	//

	if (kFBSceneChangeRename == lEvent.Type && FBIS(lEvent.Component, FBFileReference))
	{
		FBFileReference *pref = (FBFileReference*)(FBComponent*)lEvent.Component;
		mOldName = pref->LongName;
	}
	else if (kFBSceneChangeRenamed == lEvent.Type && FBIS(lEvent.Component, FBFileReference))
	{
		FBFileReference *pref = (FBFileReference*)(FBComponent*)lEvent.Component;

		FBString scriptPath(GetActionPath(), ACTION_RENAME);
		FBString outPath(GetSystemTempPath(), ACTION_RENAME_TEMP);
		
		if (true == ReplaceScriptNS(scriptPath, pref->LongName, outPath, mOldName))
		{
			mApp.ExecuteScript(outPath);
		}
	} // kFBSceneChangeDetach 
	else if (kFBSceneChangeDetach == lEvent.Type && FBIS(lEvent.ChildComponent, FBFileReference)
		&& FBIS(lEvent.Component, FBScene))
	{
		FBFileReference *pref = (FBFileReference*)(FBComponent*)lEvent.ChildComponent;
		FBString refpath = pref->ReferenceFilePath;

		char *str = refpath;
		int pos = refpath.Find('\\');
		while (pos >= 0)
		{
			str[pos] = '/';
			pos = refpath.Find('\\');
		}

		FBString scriptPath(GetActionPath(), ACTION_DELETE);
		FBString outPath(GetSystemTempPath(), ACTION_DELETE_TEMP);

		if (true == ReplaceScriptNS(scriptPath, pref->LongName, outPath, refpath))
		{
			mApp.ExecuteScript(outPath);
		}

		// remove holders
		RemoveHolder(pref); // refpath);
	}
	
}

void ReferencesManager::OnPlugConnNotify(HISender pSender, HKEvent pEvent)
{
	/*
	FBEventConnectionNotify	lEvent(pEvent);

	FBPlug *srcplug = lEvent.SrcPlug;
	FBPlug *dstplug = lEvent.DstPlug;
	FBConnectionAction	action = lEvent.Action;
	
	if (kFBDisconnectedSrc == action && FBIS(srcplug, FBFileReference) && FBIS(dstplug, FBScene))
	{
		FBFileReference *pref = (FBFileReference*)srcplug;
			
		FBString scriptPath(ACTION_DEV_PATH, ACTION_DELETE);
		FBString outPath(ACTION_DEV_PATH, ACTION_DELETE_TEMP);

		if (true == ReplaceScriptNS(scriptPath, pref->LongName, outPath))
		{
			mApp.ExecuteScript(outPath);
		}
	}
	*/
}

void ReferencesManager::OnPlugDataNotify(HISender pSender, HKEvent pEvent)
{
	FBEventConnectionDataNotify	lEvent(pEvent);

	FBPlug *plug = lEvent.Plug;

	if (true == FBMergeTransactionIsOn() || true == FBMergeTransactionFileRefEditIsOn())
		return;

	if (FBIS(plug, FBProperty) && FBIS(plug->GetOwner(), FBFileReference))
	{
		FBFileReference *pref = (FBFileReference*)plug->GetOwner();

		if (plug == &pref->IsLoaded && false == IsSkipPlugData())
		{
			if (kFBCandidate == lEvent.Action)
			{
				bool *newValue = (bool*)lEvent.GetData();
				if (false == *newValue)
				{
					mVectorUnload.insert(pref);
				}
			}
			else if (kFBCandidated == lEvent.Action)
			{
				bool *newValue = (bool*)lEvent.GetData();
				if (true == *newValue)
				{
					mVectorLoad.insert(pref);
				}
			}
		}
	}
}

void ReferencesManager::RemoveHolder(FBFileReference *pRef) // FBString &refpath)
{
	FBScene *pScene = mSystem.Scene;

	for (int i = 0, count = pScene->UserObjects.GetCount(); i < count; ++i)
	{

	}

	// let's find any other instance of ref namespace
	bool anyInstances = false;

	for (int i = 0, count = pScene->Namespaces.GetCount(); i < count; ++i)
	{
		if (FBIS(pScene->Namespaces[i], FBFileReference) && pScene->Namespaces[i] != pRef)
		{
			FBFileReference *otherRef = (FBFileReference*)pScene->Namespaces[i];
			if (0 == strcmp(otherRef->ReferenceFilePath, pRef->ReferenceFilePath))
			{
				anyInstances = true;
				break;
			}
		}
	}

	//
	if (false == anyInstances)
	{
		std::vector<DescriptionHolder*>		vectorToDelete;

		for (int i = 0, count = pRef->GetDstCount(); i < count; ++i)
		{
			if (FBIS(pRef->GetDst(i), DescriptionHolder))
			{
				vectorToDelete.push_back((DescriptionHolder*)pRef->GetDst(i));
			}
		}

		for (auto iter = begin(vectorToDelete); iter != end(vectorToDelete); ++iter)
		{
			DescriptionHolder *pHolder = *iter;
			pHolder->FBDelete();
		}
	}
}

void ReferencesManager::OnFileRefChange(HISender pSender, HKEvent pEvent)
{
	FBEventFileChange	lEvent(pEvent);
	
	if (kFBFileMonitoring_FILEREFERENCE == lEvent.Type)
	{
		FBString path(lEvent.Path);
		
		//
		FBScene *pScene = mSystem.Scene;
		for (int i = 0, count = pScene->Namespaces.GetCount(); i < count; ++i)
		{
			if (FBIS(pScene->Namespaces[i], FBFileReference))
			{
				FBFileReference *pref = (FBFileReference*)pScene->Namespaces[i];
				FBString refpath(pref->ReferenceFilePath);

				if (0 == strcmp(path, refpath))
				{
					mVectorReload.insert(pref);
				}
			}
		}
		
	}
}

void ReferencesManager::ReferencesPostLoad()
{
	FBString scriptPath(GetActionPath(), ACTION_RELOAD);
	FBString outPath(GetSystemTempPath(), ACTION_RELOAD_TEMP);

	EnterSkipPlugData();

	FBScene *pScene = mSystem.Scene;
	for (int i = 0, count = pScene->Namespaces.GetCount(); i < count; ++i)
	{
		if (FBIS(pScene->Namespaces[i], FBFileReference))
		{
			FBFileReference *pref = (FBFileReference*)pScene->Namespaces[i];
			
			FBProperty *prop = pref->PropertyList.Find("TempIsLoaded");

			if (nullptr != prop)
			{
				bool needToLoad = (prop->AsInt() > 0);

				if (needToLoad)
				{
					if (true == ReplaceScriptNS(scriptPath, pref->LongName, outPath))
					{
						mApp.ExecuteScript(outPath);
					}
				}

				pref->PropertyRemove(prop);
			}
		}
	}

	LeaveSkipPlugData();
}