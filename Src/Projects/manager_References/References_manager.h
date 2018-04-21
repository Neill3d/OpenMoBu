#ifndef __REFERENCES_MANAGER_H__
#define __REFERENCES_MANAGER_H__


/** \file   references_manager.h

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#include <set>

//--- Registration defines
#define REFERENCES_MANAGER__CLASSNAME	ReferencesManager
#define REFERENCES_MANAGER__CLASSSTR	"ReferencesManager"

///////////////////////////////////////////////////////////////////////////////////

/** References Manager Class.
*/
class ReferencesManager : public FBCustomManager
{
    //--- FiLMBOX box declaration.
	FBCustomManagerDeclare(ReferencesManager);

public:
    virtual bool FBCreate();        //!< FiLMBOX creation function.
    virtual void FBDestroy();       //!< FiLMBOX destruction function.

    virtual bool Init();
    virtual bool Open();
    virtual bool Clear();
    virtual bool Close();

	void OnFileNew(HISender pSender, HKEvent pEvent);
	void OnFileNewCompleted(HISender pSender, HKEvent pEvent);
	void OnFileOpenCompleted(HISender pSender, HKEvent pEvent);
	void OnFileExit(HISender pSender, HKEvent pEvent);

	void OnUIIdle(HISender pSender, HKEvent pEvent);
	void OnUIIdleAfterOpen(HISender pSender, HKEvent pEvent);

	void OnPlugConnNotify(HISender pSender, HKEvent pEvent);
	void OnPlugDataNotify(HISender pSender, HKEvent pEvent);
	void OnSceneChange(HISender pSender, HKEvent pEvent);
	void OnMenuActivate(HISender pSender, HKEvent pEvent);

	void OnFileRefChange(HISender pSender, HKEvent pEvent);

protected:

	FBApplication	mApp;
	FBSystem		mSystem;
	//FBFileMonitoringManager	mMon;

	FBString		mOldName;	// store an old name during naming process

	bool			mFirstTime;
	//bool			mProcessChanges;

	bool			mNeedReload;

	std::set<FBFileReference*>	mVectorUnload;
	std::set<FBFileReference*>	mVectorLoad;
	std::set<FBFileReference*>	mVectorReload;

	// on file delete, let's remove a holder
	void RemoveHolder(FBFileReference *pRef); // FBString &refpath);

	void ReferencesPostLoad();
};

#endif /* __REFERENCES_MANAGER_H__ */
