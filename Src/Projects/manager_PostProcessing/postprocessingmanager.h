#ifndef _POST_PROCESSING_MANAGER_H__
#define _POST_PROCESSING_MANAGER_H__

/** \file   postprocessing_manager.h

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include
#include <fbsdk/fbsdk.h>
#include <map>

#include "GL/glew.h"

#include "graphics_framebuffer.h"
#include "postprocessing_data.h"

#include "glslShader.h"
#include "Framebuffer.h"

//#include "WGLFONT.h"
#include "postprocessing_fonts.h"

#include "postprocessing_effectChain.h"
#include "postprocesscontextdata.h"

//--- Registration defines
#define POSTPROCESSING_MANAGER__CLASSNAME PostProcessingManager
#define POSTPROCESSING_MANAGER__CLASSSTR  "PostProcessingManager"


// forward
class Socket;


////////////////////////////////////////////////////////////////////////////////////////
/** Post Processing Manager.
*/
class PostProcessingManager : public FBCustomManager
{
    //--- FiLMBOX box declaration.
	FBCustomManagerDeclare(PostProcessingManager);

public:
    virtual bool FBCreate();        //!< FiLMBOX creation function.
    virtual void FBDestroy();       //!< FiLMBOX destruction function.

    virtual bool Init();
    virtual bool Open();
    virtual bool Clear();
    virtual bool Close();

public: // CALLBACKS

	void EventSceneChange(HISender pSender, HKEvent pEvent);

	void EventFileNew(HISender pSender, HKEvent pEvent);
	void EventFileOpen(HISender pSender, HKEvent pEvent);
	void EventFileMerge(HISender pSender, HKEvent pEvent);
	void EventFileOpenComplete(HISender pSender, HKEvent pEvent);

	void OnPerFrameSynchronizationCallback(HISender pSender, HKEvent pEvent);
	void OnPerFrameRenderingPipelineCallback(HISender pSender, HKEvent pEvent);
	void OnPerFrameEvaluationPipelineCallback(HISender pSender, HKEvent pEvent);

	void EventConnNotify(HISender pSender, HKEvent pEvent);
	void EventConnDataNotify(HISender pSender, HKEvent pEvent);

	void OnUIIdle(HISender pSender, HKEvent pEvent);
	
	void OnVideoFrameRendering(HISender pSender, HKEvent pEvent);

private:

	bool				mFirstRun;

	FBApplication		mApplication;
	FBSystem			mSystem;
	
	bool		mDoVideoClipTimewrap;

	//
	static HGLRC			gCurrentContext;

	static std::map<HGLRC, PostProcessContextData*>	gContextMap;


	int				mEnterId;
	size_t			mFrameId;

	// Tango device experiment
	double				mLastSendTimeSecs;
	void				*mSocketSender;
	void				*mSocketRecv;

	bool				mIsSynced;
	FBTime				mLastSyncTime;
	FBTime				mSyncDuration;

#if BROADCAST_PREVIEW == 1
	bool				mSendPreview;
	int					mSocketPort;
	Network::Address	mSendAddress;

	unsigned char		mSendBuffer[MAX_UDP_BUFFER_SIZE];
#endif
	
	void	CheckForAContextChange();

	
	/*
	bool	OpenSocket(const int portSend, const int portRecv, bool blocking);
	void	CloseSocket();
	void	SendPreview(PostEffectBuffers *buffers);
	*/
	void	LoadConfig();

	void	PushUpperLowerClipForEffects();
	void	PopUpperLowerClipForEffects();

public:
	bool			mLastProcessCompositions;

	bool skipRender;

	void	PreRenderFirstEntry();

	bool ExternalRenderAfterRender();

	void PrepVideoClipsTimeWrap();
};

#endif /* _POST_PROCESSING_MANAGER_H__ */
