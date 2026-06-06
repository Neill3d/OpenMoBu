#pragma once

/** \file   postprocessing_manager.h

Sergei <Neill3d> Solokhin 2018-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include
#include <fbsdk/fbsdk.h>
#include <map>

#include "GL/glew.h"

#include "graphics_framebuffer.h"
#include "postpersistentdata.h"

#include "glslShaderProgram.h"
#include "Framebuffer.h"

//#include "WGLFONT.h"
#include "postprocessing_fonts.h"

#include "posteffect_chain.h"
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
	void EventFileOpenOverride(HISender pSender, HKEvent pEvent);

	void OnPerFrameSynchronizationCallback(HISender pSender, HKEvent pEvent);
	void OnPerFrameRenderingPipelineCallback(HISender pSender, HKEvent pEvent);
	void OnPerFrameEvaluationPipelineCallback(HISender pSender, HKEvent pEvent);

	void EventConnNotify(HISender pSender, HKEvent pEvent);
	void EventConnDataNotify(HISender pSender, HKEvent pEvent);

	void OnUIIdle(HISender pSender, HKEvent pEvent);
	
	void OnVideoFrameRendering(HISender pSender, HKEvent pEvent);
	
private:

	bool				mFirstRun{ true };

	FBApplication		mApplication;
	FBSystem			mSystem;
	
	bool		mDoVideoClipTimewrap{ false };

	//
	std::atomic<PostProcessContextData*> mSyncContextData{ nullptr };
	std::atomic<PostProcessContextData*> mEvaluateContextData{ nullptr };
	static std::map<HGLRC, std::unique_ptr<PostProcessContextData>>	gContextMap;

	RenderFrameGate mFrameGate;

	// Tango device experiment
	double				mLastSendTimeSecs{ 0.0 };
	void* mSocketSender{ nullptr };
	void* mSocketRecv{ nullptr };

	bool				mIsSynced{ false };
	FBTime				mLastSyncTime;
	FBTime				mSyncDuration;

#if BROADCAST_PREVIEW == 1
	bool				mSendPreview;
	int					mSocketPort;
	Network::Address	mSendAddress;

	unsigned char		mSendBuffer[MAX_UDP_BUFFER_SIZE];
#endif
	
	void	CheckForAContextChange();

	PostProcessContextData* GetCurrentContextData();
	
	/*
	bool	OpenSocket(const int portSend, const int portRecv, bool blocking);
	void	CloseSocket();
	void	SendPreview(PostEffectBuffers *buffers);
	*/
	void	LoadConfig();

	void	PushUpperLowerClipForEffects();
	void	PopUpperLowerClipForEffects();

	void	LoadShaderTextInsertions();

public:
	
	std::atomic<bool> skipRender{ false };

	bool ExternalRenderAfterRender();

	void PrepVideoClipsTimeWrap();
};