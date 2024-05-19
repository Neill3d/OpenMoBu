#ifndef __CAMERA_LINKVIS_MANAGER_H__
#define __CAMERA_LINKVIS_MANAGER_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: camera_linkvis_manager.h
//
//	Author Sergei Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/OpenMoBu
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration defines
#define CAMERA_LINKVIS__CLASSNAME Manager_CameraLinkVis
#define CAMERA_LINKVIS__CLASSSTR  "Manager_CameraLinkVis"

/** Camera Link Visibility Manager.
*/
class Manager_CameraLinkVis : public FBCustomManager
{
    //--- FiLMBOX box declaration.
    FBCustomManagerDeclare( Manager_CameraLinkVis );

public:
    virtual bool FBCreate();        //!< FiLMBOX creation function.
    virtual void FBDestroy();       //!< FiLMBOX destruction function.

    virtual bool Init();
    virtual bool Open();
    virtual bool Clear();
    virtual bool Close();

	void EventConnNotify						(HISender pSender, HKEvent pEvent);
	void EventConnDataNotify                    (HISender pSender, HKEvent pEvent);

	void EventUIIdle							(HISender pSender, HKEvent pEvent);
	void EventSceneChange						(HISender pSender, HKEvent pEvent);

	void EventRender							(HISender pSender, HKEvent pEvent);

private:
	FBSystem			mSystem;
	FBCamera			*mLastCamera;

	static FBGroup* gGroup;
	static FBCamera* gCamera;

	void LeaveCamera(FBCamera *pCamera);
	void EnterCamera(FBCamera *pCamera);

	FBCamera* FindCameraByGroup(const char* groupName);
	bool GroupVis(const char* groupName, const bool show);
};

#endif /* __CAMERA_LINKVIS_MANAGER_H__ */
