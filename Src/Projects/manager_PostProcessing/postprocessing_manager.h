#ifndef _POST_PROCESSING_MANAGER_H__
#define _POST_PROCESSING_MANAGER_H__

/** \file   postprocessing_manager.h

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

#include <unordered_map>

//--- SDK include
#include <fbsdk/fbsdk.h>

#include "GL/glew.h"

#include "graphics_framebuffer.h"
#include "postprocessing_data.h"

#include "glslShader.h"
#include "Framebuffer.h"

//#include "WGLFONT.h"
#include "postprocessing_fonts.h"

#include "postprocessing_effectChain.h"


//--- Registration defines
#define POSTPROCESSING_MANAGER__CLASSNAME Manager_PostProcessing
#define POSTPROCESSING_MANAGER__CLASSSTR  "Manager_PostProcessing"

// number of entering in render callback
#define MAX_ATTACH_STACK		10

//
class Socket;

////////////////////////////////////////////////////////////////////////////////////////
/** Post Processing Manager.
*/
class Manager_PostProcessing : public FBCustomManager
{
    //--- FiLMBOX box declaration.
	FBCustomManagerDeclare(Manager_PostProcessing);

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
	
	bool				mDoVideoClipTimewrap;


	struct SContextData
	{
		bool			mSchematicView[4];
		bool			mVideoRendering;

		int				mViewport[4];		// x, y, width, height

		//int				mLastPostPane;
		int				mViewerViewport[4];

		FBSystem			mSystem;
		MainFrameBuffer	mMainFrameBuffer;

		//
		int				mLastPaneCount;

		int				mEnterId;
		size_t			mFrameId;

		GLint			mAttachedFBO[MAX_ATTACH_STACK];

		std::unique_ptr<GLSLShader>			mShaderSimple;	// for simple blit quads on a screen

		PostEffectChain						mEffectChain;

		std::vector<PostPersistentData*>	mPaneSettings;	// choose a propriate settings according to a pane camera

		// if each pane has different size (in practice should be not more then 2
		PostEffectBuffers					mEffectBuffers0;
		PostEffectBuffers					mEffectBuffers1;
		PostEffectBuffers					mEffectBuffers2;
		PostEffectBuffers					mEffectBuffers3;

		bool			mLastProcessCompositions;

#if defined(HUD_FONT)
		std::vector<CFont*>					mElemFonts;
#endif
		std::vector<FBHUDRectElement*>		mRectElements;
		std::vector<FBHUDTextElement*>		mTextElements;

		void Init();

		void OnRenderCallback(bool isCallbackBeforeRender);
		void PreRenderFirstEntry();

		void	RenderBeforeRender(const bool processCompositions, const bool renderToBuffer);
		bool	RenderAfterRender(const bool processCompositions, const bool renderToBuffer);

		void	DrawHUD(int panex, int paney, int panew, int paneh, int vieww, int viewh);
		void	DrawHUDRect(FBHUDRectElement *pElem, int panex, int paney, int panew, int paneh, int vieww, int viewh);
#if defined(HUD_FONT)
		void	DrawHUDText(FBHUDTextElement *pElem, CFont *pFont, int panex, int paney, int panew, int paneh, int vieww, int viewh);
#endif
		void	FreeFonts();

		bool PrepPaneSettings();

		// manager shaders
		bool	LoadShaders();
		
		void	FreeShaders();

		void	FreeBuffers();
	};

	// track the state of OpenGL viewport context
	HGLRC	mCurrentContext = 0;

	std::unordered_map<HGLRC, SContextData*>	mContextDataMap;


	


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
	
	

	

	static bool EmptyGLErrorStack();
	/*
	bool	OpenSocket(const int portSend, const int portRecv, bool blocking);
	void	CloseSocket();
	void	SendPreview(PostEffectBuffers *buffers);
	*/
	void	LoadConfig();

	void	PushUpperLowerClipForEffects();
	void	PopUpperLowerClipForEffects();

public:
	
	bool RenderAfterRender()
	{
		CheckForAContextChange();
		return mContextDataMap[mCurrentContext]->RenderAfterRender(mContextDataMap[mCurrentContext]->mLastProcessCompositions, false);
	}

	void PrepVideoClipsTimeWrap();
};

#endif /* _POST_PROCESSING_MANAGER_H__ */
