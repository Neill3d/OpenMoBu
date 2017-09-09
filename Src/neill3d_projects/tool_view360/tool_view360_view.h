
#pragma once

/**	\file	ortoolview360_view.h
*	Tool with 3D viewer.

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

	enum
	{
		TEXTURE_DEPTH,
		TEXTURE_PANO,
		TEXTURE_COUNT

	};

	GLuint			mTextureCube;
	GLuint			mTextures[TEXTURE_COUNT];	// 6 color side, share one depth and output pano color

	std::auto_ptr<FrameBuffer>		mFramebuffer;
	std::auto_ptr<GLSLShader>		mShader;	// compose 6 sides into a panorama

	HdlFBPlugTemplate<FBCamera>		mCameraFront;
	HdlFBPlugTemplate<FBCamera>		mCameraBack;
	HdlFBPlugTemplate<FBCamera>		mCameraLeft;
	HdlFBPlugTemplate<FBCamera>		mCameraRight;
	HdlFBPlugTemplate<FBCamera>		mCameraTop;
	HdlFBPlugTemplate<FBCamera>		mCameraBottom;

	bool	PrepCameraSet(FBCamera *pCamera);

	bool	PrepFramebuffer(const int width, const int height);
	bool	PrepShader();

	void	AllocTextures(const int widht, const int height);
	void	FreeTextures();
};
