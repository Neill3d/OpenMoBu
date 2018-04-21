
/**	\file	ormanip_association_applymanagerrule.cxx

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//--- Class declaration
#include "references_applymanagerrule.h"
#include "References_Common.h"

FBApplyManagerRuleImplementation(ReferenceAssociation);
FBRegisterApplyManagerRule(ReferenceAssociation, "ReferenceAssociation", "Context menu on File Reference object");

//
//

extern bool ReplaceScriptNS(const char *scriptpath, const char *ns, const char *outpath, const char *arg=nullptr);
extern void EnterSkipPlugData();
extern void LeaveSkipPlugData();
extern bool IsSkipPlugData();


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ReferenceAssociation::MenuBuild(FBAMMenu* pAMMenu, FBComponent* pFocusedObject)
{
	if (FBIS(pFocusedObject, FBFileReference)
		|| (nullptr != pFocusedObject->GetOwnerFileReference()))
	{
		pAMMenu->AddOption("", -1, true);

		mSaveEditsId = pAMMenu->AddOption(ACTION_SAVEEDITS_MENU, -1, true);
		mRestoreEditsId = pAMMenu->AddOption(ACTION_RESTORE_MENU, -1, true);
		mBakeEditsId = pAMMenu->AddOption(ACTION_BAKE_MENU, -1, true);
		mChangePathId = pAMMenu->AddOption(ACTION_CHANGEPATH_MENU, -1, true);
		mReloadId = pAMMenu->AddOption(ACTION_RELOAD_MENU, -1, true);
		mDeleteId = pAMMenu->AddOption(ACTION_DELETE_MENU, -1, true);
		return true;
	}

	return false;
}

bool ReferenceAssociation::MenuAction(int pMenuId, FBComponent* pFocusedObject)
{
	bool lStatus = false;

	FBFileReference *pref = nullptr;
	
	if (FBIS(pFocusedObject, FBFileReference))
	{
		pref = (FBFileReference*)pFocusedObject;
	}
	else
	{
		pref = pFocusedObject->GetOwnerFileReference();
	}
	
	if (nullptr != pref)
    {
		if (mSaveEditsId == pMenuId)
		{
			FBString scriptPath(GetActionPath(), ACTION_SAVEEDITS);
			FBString outPath(GetSystemTempPath(), ACTION_SAVEEDITS_TEMP);
			
			if (true == ReplaceScriptNS(scriptPath, pref->LongName, outPath) )
				lStatus = mApp.ExecuteScript(outPath);
		}
		else if (mRestoreEditsId == pMenuId)
		{
			FBString scriptPath(GetActionPath(), ACTION_RESTORE);
			FBString outPath(GetSystemTempPath(), ACTION_RESTORE_TEMP);

			if (true == ReplaceScriptNS(scriptPath, pref->LongName, outPath))
				lStatus = mApp.ExecuteScript(outPath);
		}
		else if (mBakeEditsId == pMenuId)
		{
			FBString scriptPath(GetActionPath(), ACTION_BAKE);
			FBString outPath(GetSystemTempPath(), ACTION_BAKE_TEMP);

			if (true == ReplaceScriptNS(scriptPath, pref->LongName, outPath))
				lStatus = mApp.ExecuteScript(outPath);
		}
		else if (mChangePathId == pMenuId)
		{
			FBString scriptPath(GetActionPath(), ACTION_CHANGEPATH);
			FBString outPath(GetSystemTempPath(), ACTION_CHANGEPATH_TEMP);
			
			if (true == ReplaceScriptNS(scriptPath, pref->LongName, outPath) )
				lStatus = mApp.ExecuteScript(outPath);
		}
		else if (mReloadId == pMenuId)
		{
			FBString scriptPath(GetActionPath(), ACTION_RELOAD);
			FBString outPath(GetSystemTempPath(), ACTION_RELOAD_TEMP);

			EnterSkipPlugData();

			if (true == ReplaceScriptNS(scriptPath, pref->LongName, outPath))
			{
				mApp.ExecuteScript(outPath);
			}

			LeaveSkipPlugData();
		}
		else if (mDeleteId == pMenuId)
		{
			if (1 == FBMessageBox("Deleting A Reference", "Are you sure you want to delete a file reference?", "Yes", "Cancel"))
			{
				// remove a reference namespace
				FBString nsName(pref->LongName);
				lStatus = mSystem.Scene->NamespaceDelete(nsName);
			}
			
		}
    }
	return lStatus;
}