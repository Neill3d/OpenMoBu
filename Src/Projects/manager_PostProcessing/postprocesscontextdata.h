
#pragma once

/** \file   PostProcessContextData.h

Sergei <Neill3d> Solokhin 2022-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include
#include <fbsdk/fbsdk.h>
#include <map>
#include <limits>

#include "GL/glew.h"

#include "graphics_framebuffer.h"
#include "postpersistentdata.h"

#include "glslShaderProgram.h"
#include "Framebuffer.h"

#include "postprocessing_fonts.h"
#include "posteffect_buffers.h"
#include "posteffect_chain.h"
#include "posteffect_context.h"
#include "standardeffectcollection.h"
#include "shaderproperty_storage.h"

// keep track of begining / end render and recursive renders
struct RenderFrameGate
{
public:

	void Enter();
	void Leave();
	void Reset();

	bool IsFirstEnter() const;
	int GetEnterId() const { return mEnterId; }

	size_t mFrameId{ 0 };
	int mEnterId{ 0 };
};

/// <summary>
/// All post process render data for an ogl context
/// </summary>
struct PostProcessContextData
{
public:
	static const int MAX_PANE_COUNT = 4;

	FBTime				mStartSystemTime;
	double				mLastSystemTime{ std::numeric_limits<double>::max() };
	double				mLastLocalTime{ std::numeric_limits<double>::max() };

	bool			mIsTimeInitialized{ false };

	//
	int				mEvaluatePaneCount{ 0 }; // @see mEvaluatePanes
	int				mRenderPaneCount{ 0 }; // @see mRenderPanes
	
	int				mSchematicViewIndex{ -1 }; // -1 in case there is no pane with schematic view
	bool			mVideoRendering = false;
	bool			mHasPostProcessing = false;
	std::atomic<bool> isReadyToEvaluate{ false };
	std::atomic<bool> isNeedToResetPaneSettings{ false };

	int				mViewport[4];		// x, y, width, height
	int				mViewerViewport[4];

	RenderFrameGate mFrameGate;

	// number of entering in render callback
	constexpr static int MAX_ATTACH_STACK = 10;
	GLint			mAttachedFBO[MAX_ATTACH_STACK]{ 0 };


	//
	MainFrameBuffer						mMainFrameBuffer;

	std::unique_ptr<GLSLShaderProgram>	mShaderSimple;	//!< for simple blit quads on a screen

	struct SPaneData
	{
		PostEffectContextMoBu* fxContext{ nullptr };
		PostPersistentData* data{ nullptr };
		FBCamera* camera{ nullptr };
		int paneIndex{ -1 };

		bool hasValidCamera = false;
		bool isCameraChanged = false;
		bool hasPostProcess = false;

		bool IsValid() const
		{
			return (fxContext != nullptr) && (data != nullptr) && (camera != nullptr);
		}

		void Clear()
		{
			fxContext = nullptr;
			data = nullptr;
			camera = nullptr;
			paneIndex = -1;
			hasValidCamera = false;
			isCameraChanged = false;
			hasPostProcess = false;
		}

		void CopyFrom(const SPaneData& other)
		{
			fxContext = other.fxContext;
			data = other.data;
			camera = other.camera;
			paneIndex = other.paneIndex;
			hasValidCamera = other.hasValidCamera;
			isCameraChanged = other.isCameraChanged;
			hasPostProcess = other.hasPostProcess;
		}
	};
	
	SPaneData	mEvaluatePanes[MAX_PANE_COUNT];	//!< choose a propriate settings according to a pane camera
	SPaneData	mRenderPanes[MAX_PANE_COUNT];
	std::array<std::unique_ptr<PostEffectContextMoBu>, MAX_PANE_COUNT> mFXContexts; //!< temporary contexts for each pane

	// for each persistent data object we have a separate post fx context
	//std::unordered_map<PostPersistentData*, std::unique_ptr<PostEffectContextMoBu>>	mPostFXContextsMap;

	// build-in effects collection to be re-used per effect chain
	StandardEffectCollection standardEffectsCollection;

	// if each pane has different size (in practice should be not more then 2
	std::array< std::unique_ptr<PostEffectBuffers>, MAX_PANE_COUNT> mPaneEffectBuffers;
	
	void    Init();
	
	void VideoRenderingBegin();
	void VideoRenderingEnd();

	bool HasPostProcessing() const { return mHasPostProcessing; }
	void UpdatePostProcessingFlag();
	

	// run in custom thread to evaluate the processing data
	void	Evaluate(FBTime systemTime, FBTime localTime, FBEvaluateInfo* pEvaluateInfoIn);
	void	Synchronize();

	void	RenderBeforeRender();
	bool	RenderAfterRender(FBTime systemTime, FBTime localTime, FBEvaluateInfo* pEvaluateInfoIn);

	// thread-safe, atomic read the ready to evaluate flag
	bool IsReadyToEvaluate() const;
	// thread-safe, atomic update the ready to evaluate flag
	void SetReadyToEvaluate(bool ready);

	bool IsNeedToResetPaneSettings() const;
	void SetNeedToResetPaneSettings(bool reset);

	
	void ReloadShaders(PostPersistentData* data, PostEffectContextMoBu* fxContext,
		FBEvaluateInfo* pEvaluateInfoIn, FBCamera* pCamera, const PostEffectContextProxy::Parameters& contextParameters);

private:
    bool EmptyGLErrorStack();

	void PrepareEachPaneCamera();
	bool PrepareEachPanePersistanceData();
	void PrepareEachPaneContext();
	void PreparePaneBuffers();

	// manager shaders
	bool	LoadSimpleBlitShader();
	const bool CheckShadersPath(const char* path) const;
	void	FreeShaders();

	void	FreeBuffers();

	void PrepareContextParameters(PostEffectContextProxy::Parameters& contextParametersOut, FBTime systemTime, FBTime localTime) const;
	void PrepareContextParametersForCamera(PostEffectContextProxy::Parameters& contextParametersOut, FBCamera* pCamera, int nPane) const;

	void RenderPane(FBEvaluateInfo* pEvaluateInfoIn, 
		SPaneData& pane, 
		PostEffectBuffers* paneBuffers, 
		PostEffectContextProxy::Parameters& params,
		GLuint fboInOut);
	void BuffersPoolCollection();

	// once we load file, we should reset pane user object pointers 
	// and wait for next PrepPaneSettings call
	void	ResetPaneSettings();

	void	DrawHUD(int panex, int paney, int panew, int paneh, int vieww, int viewh);
	void	DrawHUDRect(FBHUDRectElement *pElem, int panex, int paney, int panew, int paneh, int vieww, int viewh);
#if defined(HUD_FONT)
	void	DrawHUDText(FBHUDTextElement *pElem, CFont *pFont, int panex, int paney, int panew, int paneh, int vieww, int viewh);
#endif
	void	FreeFonts();


#if defined(HUD_FONT)
	std::vector<CFont*>					mElemFonts;
#endif
	std::vector<FBHUDRectElement*>		mRectElements;
	std::vector<FBHUDTextElement*>		mTextElements;

};


