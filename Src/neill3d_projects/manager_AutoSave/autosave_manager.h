
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://code.google.com/p/motioncodelibrary/wiki/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////

#ifndef __AUTOSAVE_MANAGER_H__
#define __AUTOSAVE_MANAGER_H__

/** \file   autosave_manager.h
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration defines
#define AUTOSAVE__CLASSNAME AutoSaveManager
#define AUTOSAVE__CLASSSTR  "AutoSaveManager"

/** Autosave Manager.
*/
class AutoSaveManager : public FBCustomManager
{
    //--- FiLMBOX box declaration.
    FBCustomManagerDeclare( AutoSaveManager );

public:
    virtual bool FBCreate();        //!< FiLMBOX creation function.
    virtual void FBDestroy();       //!< FiLMBOX destruction function.

    bool Init();
    bool Open();
    bool Clear();
    bool Close();

	//-- idle event (do saving operation)
	void		EventIdle( HISender pSender, HKEvent pEvent );

private:
	bool		mFirstLoad;

	//-- local params (workflow)
	FBTime		mLastTime;			// holding system time on clear state
	int			mIndex;				// file postfix
};

#endif /* __AUTOSAVE_MANAGER_H__ */
