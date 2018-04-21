
#pragma once

/**	\file	tool_viewStereoDistortion_view.h
*	Tool with 3D viewer.

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

#include "FrameBuffer.h"
#include "GLSLShader.h"

// share framebuffer and shader between views

class CViewProcessing
{
public:
	//! a constructor
	CViewProcessing();

	//
	void	IncView();	// if any view registered, let's allocate resources
	void	DecView();	// let's check if view count is zero, free resources

	GLSLShader		*GetShaderPtr();
	FrameBuffer		*GetFrameBufferPtr(const int width, const int height);

	void	ReloadShaders();

protected:

	int		mViewCounter;
	bool	mTryToLoadShader;

	std::auto_ptr<GLSLShader>	mShader;
	std::auto_ptr<FrameBuffer>	mFrameBuffer;

	bool	PrepShader();
	bool	PrepFrameBuffer(const int width, const int height);

};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**	View 3D class.
*/
class ORViewPaneWithDistortion : public FBView
{
	FBClassDeclare( ORViewPaneWithDistortion, FBView );
  private:
    FBSystem    mSystem;
	FBRenderer*	mRender;	//!< Renderer object.

  public:
	//! Constructor.
	ORViewPaneWithDistortion(const int paneId);
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

};
