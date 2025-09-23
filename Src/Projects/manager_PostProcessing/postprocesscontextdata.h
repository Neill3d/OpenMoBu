
#pragma once

/** \file   PostProcessContextData.h

Sergei <Neill3d> Solokhin 2022

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

//#include "WGLFONT.h"
#include "postprocessing_fonts.h"
#include "posteffectbuffers.h"
#include "posteffectchain.h"

// number of entering in render callback
#define MAX_ATTACH_STACK		10

/// <summary>
/// All post process render data for an ogl context
/// </summary>
struct PostProcessContextData
{
public:
	FBSystem			mSystem;

	FBTime				mStartSystemTime;
	double				mLastSystemTime{ std::numeric_limits<double>::max() };
	double				mLastLocalTime{ std::numeric_limits<double>::max() };

	//
	int				mLastPaneCount{ 0 };
	
	bool			mSchematicView[4];
	bool			mVideoRendering = false;

	int				mViewport[4];		// x, y, width, height
	int				mViewerViewport[4];

	int				mEnterId = 0;
	size_t			mFrameId = 0;

	GLint			mAttachedFBO[MAX_ATTACH_STACK]{ 0 };


	//
	MainFrameBuffer						mMainFrameBuffer;

	std::unique_ptr<GLSLShaderProgram>			mShaderSimple;	//!< for simple blit quads on a screen

	PostEffectChain						mEffectChain;

	std::vector<PostPersistentData*>	mPaneSettings;	//!< choose a propriate settings according to a pane camera

	// if each pane has different size (in practice should be not more then 2
	std::unique_ptr<PostEffectBuffers> mEffectBuffers0;
	std::unique_ptr<PostEffectBuffers> mEffectBuffers1;
	std::unique_ptr<PostEffectBuffers> mEffectBuffers2;
	std::unique_ptr<PostEffectBuffers> mEffectBuffers3;

	void    Init();

	void	PreRenderFirstEntry();

	void	RenderBeforeRender(const bool processCompositions, const bool renderToBuffer);
	bool	RenderAfterRender(const bool processCompositions, const bool renderToBuffer);

	const PostEffectChain& GetEffectChain() const { return mEffectChain; }

private:
    bool EmptyGLErrorStack();

	bool PrepPaneSettings();

	// manager shaders
	bool	LoadShaders();
	const bool CheckShadersPath(const char* path) const;
	void	FreeShaders();

	void	FreeBuffers();


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


