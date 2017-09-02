#ifndef __RENDERADVANCE_VIEW_H__
#define __RENDERADVANCE_VIEW_H__


/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://code.google.com/p/motioncodelibrary/wiki/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>

//! each pane has region dimentions and current camera index
struct		Viewport
{
	//! viewport rect
	int x;
	int y;
	int w;
	int h;

	//! camera index in scene cameras list
	int camera;

	void Set(int g_x, int g_y, int g_w, int g_h)
	{
		x = g_x;
		y = g_y;
		w = g_w;
		h = g_h;
	}

	Viewport()
	{
		x = 0;
		y = 0;
		w = 320;
		h = 240;
		camera = 0;
	}
};

#define	PANE_STEP		2

//! collection of layouts with different number of panes
struct	ViewportLayout
{
public:
	//! a constructor
	ViewportLayout();
	//! resize viewport panes
	void ReSize(int w, int h);
	//! set current layout
	void SetLayout(int n);
	//! cameras for panes
	void SetCamera1(int n);
	void SetCamera2(int n);
	void SetCamera3(int n);
	void SetCamera4(int n);
	//! render in current layout
	void Render(FBRenderer	*pRender,	FBView	*pView);

private:
	Viewport		layout1;		//! single view
	Viewport		layout2[2];		//! two views
	Viewport		layout3[3];		//! three views
	Viewport		layout4[4];		//! four views
	//! current layout
	int			currLayout;
};

/**	View 3D class.
*/
class ORView3D : public FBView
{
	FBClassDeclare( ORView3D, FBView );

private:
	FBSystem	mSystem;
	FBRenderer	*mRender;	//!< Renderer object.
    FBString    mFileName;  //!< File name to save under.
    FBString    mFilePath;  //!< File path to save under.	

	ViewportLayout		mLayout;

	int			mFontDisplayList;

	bool		mRenderStart;

public:
	//! Constructor.
	ORView3D();
	//! destruction function.
	virtual void FBDestroy();
	//! Refresh callback.
	virtual void Refresh(bool pNow=false);
	//! Expose callback.
	virtual void ViewExpose();
	//! Input callback.
	virtual void ViewInput(int pMouseX, int pMouseY, FBInputType pAction, int pButtonKey, int pModifier);

	static void RefreshViewCallback(void* pThis);

    FBString GetFileName();
    void SetFileName( FBString pFileName );
    FBString GetFilePath();
    void SetFilePath( FBString pFilePath );

	void CustomGLDrawString(int pX, int pY, char* pText);

	void SetPaneLayout(int panes)
	{
		mLayout.SetLayout(panes);
	}
	void SetPaneCamera(int pane, int camera)
	{
		switch(pane)
		{
		case 0: mLayout.SetCamera1(camera); break;
		case 1: mLayout.SetCamera2(camera); break;
		case 2: mLayout.SetCamera3(camera); break;
		case 3: mLayout.SetCamera4(camera); break;
		}
	}

	FBVideoGrabber			VideoGrabber;
	FBVideoGrabStats		mStats;

		// stats
	FBLabel		mTotalFrameCount;
	FBLabel		mRemainingFrameCount;
	FBLabel		mTotalTimeElapsed;
	FBLabel		mEstimatedTime;
	FBLabel		mEstimatedTimeRemaining;
	FBLabel		mTimePerFrame;

	// update video grabber statistics
	void	RefreshStats		();
	// video grabber work
	void Render(FBVideoGrabOptions	&GrabOptions);
};

#endif /* __RENDERADVANCE_VIEW_H__ */
