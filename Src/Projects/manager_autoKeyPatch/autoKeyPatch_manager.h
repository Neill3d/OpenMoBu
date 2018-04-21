#ifndef __ORCUSTOMMANAGER_TEMPLATE_MANAGER_H__
#define __ORCUSTOMMANAGER_TEMPLATE_MANAGER_H__

/** \file   autoKeyPatch_manager.h

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE


*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration defines
#define ORCUSTOMMANAGER_TEMPLATE__CLASSNAME ORCustomManager_Template
#define ORCUSTOMMANAGER_TEMPLATE__CLASSSTR  "ORCustomManager_Template"

/** Custom Manager Template.
*/
class ORCustomManager_Template : public FBCustomManager
{
    //--- FiLMBOX box declaration.
    FBCustomManagerDeclare( ORCustomManager_Template );

public:
    virtual bool FBCreate();        //!< FiLMBOX creation function.
    virtual void FBDestroy();       //!< FiLMBOX destruction function.

    virtual bool Init();
    virtual bool Open();
    virtual bool Clear();
    virtual bool Close();

	void OnUIIdle(HISender pSender, HKEvent pEvent);

private:

	FBSystem			mSystem;
	FBPlayerControl		mPlayer;

	bool	mEnabled;		// by menu item (Auto interpolation)
	bool	mEnabledSuper;	// by key pressed

	FBTime	mZoomStart;
	FBTime	mZoomStop;

	FBTime  mLoopStart;
	FBTime  mLoopStop;

	bool	mNeedSuperAuto;
};

#endif /* __ORCUSTOMMANAGER_TEMPLATE_MANAGER_H__ */
