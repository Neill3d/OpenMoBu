
#pragma once

/**	\file	ortoolview360_view.h

	Custom FBView that render scene from 6 sides and compose into a equirectangle panorama

// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE

	GitHub repo - https://github.com/Neill3d/MoBu

	Author Sergey Solokhin (Neill3d)
	 e-mail to: s@neill3d.com
	  www.neill3d.com

*/

//--- SDK include
#include <fbsdk/fbsdk.h>
#include <vector>

#include "Framebuffer.h"
#include "glslShader.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**	View 3D class.
*/
class ORView360 : public FBView
{
	FBClassDeclare( ORView360, FBView );
private:
    FBSystem    mSystem;
	FBRenderer*	mRender;	//!< Renderer object.

public:
	//! Constructor.
	ORView360();
	//! Destructor
	virtual void FBDestroy();

	//! Refresh callback.
	virtual void Refresh(bool pNow=false);
	//! Expose callback.
	virtual void ViewExpose();

	/** Input callback function. */
    virtual void ViewInput(int pMouseX,int pMouseY,FBInputType pAction,int pButtonKey,int pModifier);

private:
	
	int				mPaneId;
	bool			mIsMaximized;
	int				mScaleDown;

	bool			mSaveScreenshot;
	FBString		mSingleFilename;
	FBString		mSequenceFilename;

	enum
	{
		DISPLAY_MONO,
		DISPLAY_STEREO_SBS,
		DISPLAY_STEREO_TAB,
		DISPLAY_RAW_PANORAMA
	};

	int				mDisplayMode;
	bool			mFirstTime;
	FBMatrix		mInitRotation;

	enum
	{
		TEXTURE_DEPTH,
		TEXTURE_PANO,
		TEXTURE_COUNT
	};

	GLuint			mTextureCube;
	GLuint			mTextures[TEXTURE_COUNT];	// 6 color side, share one depth and output pano color

	std::auto_ptr<FrameBuffer>		mFramebufferCube;
	std::auto_ptr<FrameBuffer>		mFramebufferPano;

	bool							mTryToLoadShader;
	std::auto_ptr<GLSLShader>		mShaderCubeToPan;	// compose 6 sides into a panorama (equirectangle)
	std::auto_ptr<GLSLShader>		mShaderToSphere;	// equirectangleToSphere (for VR experience)

	HdlFBPlugTemplate<FBCamera>		mCameraFront;
	HdlFBPlugTemplate<FBCamera>		mCameraBack;
	HdlFBPlugTemplate<FBCamera>		mCameraLeft;
	HdlFBPlugTemplate<FBCamera>		mCameraRight;
	HdlFBPlugTemplate<FBCamera>		mCameraTop;
	HdlFBPlugTemplate<FBCamera>		mCameraBottom;

	// find 6 cameras in the scene
	bool	PrepCameraSet(FBCamera *pCamera);

	bool	PrepFramebuffer(const int width, const int height, const int panoWidth, const int panoHeight);
	bool	PrepShader();
	bool	PrepShaderEquiToSphere();

	void	AllocTextures(const int widht, const int height, const int panoWidth, const int panoHeight);
	void	FreeTextures();

	void ReloadShaders();

	// some render output
	void	CheckForScreenshot();

	void	SaveScreenshot(const char *filename);
	// TODO: need to be implemented !
	void	SaveTimeRangeSequence(const char *filename);
};
