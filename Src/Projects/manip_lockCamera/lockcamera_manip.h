#ifndef __ORMANIP_LOCK_CAMERA_H__
#define __ORMANIP_LOCK_CAMERA_H__

/**	\file	lockcamera_manip.h

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration defines
#define MANIP_LOCKCAMERA__CLASSNAME		Manip_LockCamera
#define MANIP_LOCKCAMERA__CLASSSTR		"Manip_LockCamera"

//! Manipulator template
class Manip_LockCamera : public FBManipulator
{
	//--- FiLMBOX declaration.
	FBManipulatorDeclare(Manip_LockCamera, FBManipulator);

public:
	//! FiLMBOX Constructor.
	virtual bool FBCreate();

	//!< FiLMBOX Destructor.
	virtual void FBDestroy();

	//!	Deal with manipulator input
	virtual bool ViewInput(int pMouseX, int pMouseY, FBInputType pAction, int pButtonKey,int pModifier);

	//! Manipulator expose function
	virtual void ViewExpose();

	void OnPerFrameRenderingPipelineCallback(HISender pSender, HKEvent pEvent);
	void OnUIIdle(HISender pSender, HKEvent pEvent);
	void EventSceneChange(HISender pSender, HKEvent pEvent);
	void OnFileNew(HISender pSender, HKEvent pEvent);

public:
	
	//FBApplication		mApp;
	FBSystem			mSystem;

	enum
	{
		STATE_UNLOCK,
		STATE_LOCK_CAMERA,
		STATE_LOCK_SWITCHER
	};

	int			mRenderPaneIndex;
	int		mStates[4];		// for each pane

	HdlFBPlugTemplate<FBCamera>		mCameraPane0;
	HdlFBPlugTemplate<FBCamera>		mCameraPane1;
	HdlFBPlugTemplate<FBCamera>		mCameraPane2;
	HdlFBPlugTemplate<FBCamera>		mCameraPane3;

	FBVector2d	mPosition;
	FBVector2d	mSize;

private:

	unsigned int			mLockId;
	unsigned int			mUnLockId;

	FBString				mLockPath;
	FBString				mUnLockPath;

	double					mAlpha;	// rect transparency

	void LoadTextures();
	void FreeTextures();

	unsigned int UploadImageIntoTexture(const char *filename, const unsigned int useId);

	void LoadConfig();
};

#endif /* lock camera manip h */
