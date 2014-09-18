
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

//--- Class declaration
#include "renderadvance_view.h"

#include <windows.h>	//For wglUseFontBitmaps
#pragma comment(lib,"user32.lib")   // This is to avoid changing the template for the .dsp files.

#ifndef OLD_OPENGL_HEADER
#include <fbsdk\fbsdk-opengl.h>
#endif

FBClassImplementation( ORView3D );

// stats captions
#define TOTALFRAMECOUNT					"Total Frame Count: "
#define REMAININGFRAMECOUNT			"Remaining Frame Count: "
#define	TOTALTIMEELAPSED				"Total Time Elapsed: "
#define	ESTIMATEDTIME						"Estimated Time: "
#define ESTIMATEDTIMEREMAINING	"Estimated Time Remaining: "
#define	TIMEPERFRAME						"Time Per Frame: "

FBProgress		*g_pProgress = NULL;



ViewportLayout::ViewportLayout()
{
	currLayout = 0;
	ReSize(320, 240);
}

void ViewportLayout::ReSize(int w, int h)
{
	switch(currLayout)
	{
	case 0:
		// single view
		layout1.Set( 0, 0, w, h );
		break;
	case 1:
		// two views
		layout2[0].Set( 0, 0, w/2-PANE_STEP, h );
		layout2[1].Set( w/2+PANE_STEP, 0, w/2-PANE_STEP, h );
		break;
	case 2:
		// tree views
		layout3[0].Set( 0, 0, w/2-PANE_STEP, h );
		layout3[1].Set( w/2+PANE_STEP, h/2+PANE_STEP, w/2-PANE_STEP, h/2-PANE_STEP );
		layout3[2].Set( w/2+PANE_STEP, PANE_STEP, w/2-PANE_STEP, h/2-PANE_STEP );
		break;
	case 3:
		// four views
		layout4[0].Set( PANE_STEP, h/2+PANE_STEP, w/2-PANE_STEP, h/2-PANE_STEP );
		layout4[1].Set( w/2+PANE_STEP, h/2+PANE_STEP, w/2-PANE_STEP, h/2-PANE_STEP );
		layout4[2].Set( PANE_STEP, PANE_STEP, w/2-PANE_STEP, h/2-PANE_STEP );
		layout4[3].Set( w/2+PANE_STEP, PANE_STEP, w/2-PANE_STEP, h/2-PANE_STEP );
		
		break;
	}
}

void ViewportLayout::SetLayout(int n)
{
	currLayout = n;
}
void ViewportLayout::SetCamera1(int n)
{
	layout1.camera = n;
	layout2[0].camera = n;
	layout3[0].camera = n;
	layout4[0].camera = n;
}
void ViewportLayout::SetCamera2(int n)
{
	layout2[1].camera = n;
	layout3[1].camera = n;
	layout4[1].camera = n;
}
void ViewportLayout::SetCamera3(int n)
{
	layout3[2].camera = n;
	layout4[2].camera = n;
}
void ViewportLayout::SetCamera4(int n)
{
	layout4[3].camera = n;
}

void ViewportLayout::Render(FBRenderer	*pRender,	FBView	*pView)
{
	FBCamera	*currCam = pRender->CurrentCamera;
	FBColor	color = currCam->BackGroundColor;

	switch(currLayout)
	{
		// single view
		case 0:
			{
				pRender->CurrentCamera = FBSystem::TheOne().Scene->Cameras[layout1.camera];
				if( pRender->RenderBegin(0, 0, layout1.w, layout1.h) )
				{
					pRender->PreRender();

					glClearColor(color[0], color[1], color[2], color[3]);
					glClear(GL_COLOR_BUFFER_BIT);

					pRender->Render();
					pRender->RenderEnd(pView);
				}
			} break;
		// two views
		case 1:
			{
				glEnable(GL_SCISSOR_TEST);
				Viewport	&lViewport = layout2[0];
				pRender->CurrentCamera = FBSystem().Scene->Cameras[lViewport.camera];
				glScissor(lViewport.x, lViewport.y, lViewport.w, lViewport.h);
				if( pRender->RenderBegin(lViewport.x, lViewport.y, lViewport.w, lViewport.h) )
				{
					pRender->PreRender();
					// set camera background by hands
					glClearColor(color[0], color[1], color[2], color[3]);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					pRender->Render();
					pRender->RenderEnd(pView);
				}
				
				Viewport	&lViewport2 = layout2[1];
				pRender->CurrentCamera = FBSystem().Scene->Cameras[lViewport2.camera];
				glScissor(lViewport2.x, lViewport2.y, lViewport2.w, lViewport2.h);
				if( pRender->RenderBegin(lViewport2.x, lViewport2.y, lViewport2.w, lViewport2.h) )
				{
					pRender->PreRender();
					// set camera background by hands
					glClearColor(color[0], color[1], color[2], color[3]);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

					pRender->Render();
					pRender->RenderEnd(pView);
				}
				
				glDisable(GL_SCISSOR_TEST);
			}break;
		// three views
		case 2:
			{
				glEnable(GL_SCISSOR_TEST);
				Viewport	&lViewport = layout3[0];
				pRender->CurrentCamera = FBSystem().Scene->Cameras[lViewport.camera];
				glScissor(lViewport.x, lViewport.y, lViewport.w, lViewport.h);
				if( pRender->RenderBegin(lViewport.x, lViewport.y, lViewport.w, lViewport.h) )
				{
					pRender->PreRender();
					// set camera background
					glClearColor(color[0], color[1], color[2], color[3]);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					pRender->Render();
					pRender->RenderEnd(pView);
				}
				for (int i=1; i<3; i++)
				{
					Viewport	&lViewport2 = layout3[i];
					pRender->CurrentCamera = FBSystem().Scene->Cameras[lViewport2.camera];
					glScissor(lViewport2.x, lViewport2.y, lViewport2.w, lViewport2.h);
					if( pRender->RenderBegin(lViewport2.x, lViewport2.y, lViewport2.w, lViewport2.h) )
					{
						pRender->PreRender();
						// set camera background by hands
						glClearColor(color[0], color[1], color[2], color[3]);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

						pRender->Render();
						pRender->RenderEnd(pView);
					}
				}
			}break;
		// four views
		case 3:
			{
				glEnable(GL_SCISSOR_TEST);
				Viewport	*lViewport = &layout4[0];
				pRender->CurrentCamera = FBSystem().Scene->Cameras[lViewport->camera];
				glScissor(lViewport->x, lViewport->y, lViewport->w, lViewport->h);
				if( pRender->RenderBegin(lViewport->x, lViewport->y, lViewport->w, lViewport->h) )
				{
					pRender->PreRender();
					// set camera background
					glClearColor(color[0], color[1], color[2], color[3]);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					pRender->Render();
					pRender->RenderEnd(pView);
				}
				for (int i=1; i<4; i++)
				{
					lViewport = &layout4[i];
					pRender->CurrentCamera = FBSystem().Scene->Cameras[lViewport->camera];
					glScissor(lViewport->x, lViewport->y, lViewport->w, lViewport->h);
					if( pRender->RenderBegin(lViewport->x, lViewport->y, lViewport->w, lViewport->h) )
					{
						pRender->PreRender();
						// set camera background by hands
						glClearColor(color[0], color[1], color[2], color[3]);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

						pRender->Render();
						pRender->RenderEnd(pView);
					}
				}
			}break;
	}

	pRender->CurrentCamera = currCam;
}


/************************************************
 *	Constructor
 ************************************************/
ORView3D::ORView3D() :
	mRender(NULL),
	mFontDisplayList(0)
{
	mRender = mSystem.Renderer;
	

    // The resulting file will depend on the extension of the output file.
    // "output.avi" will generate a Windows Media file, and "output.mov"
    // will generate a QuickTime file. The QuickTime authoring module must
    // be installed on your machine to generate this type of file. On Windows
    // it comes with the free version of the player, but is not installed
    // by default.
    mFileName = "output.avi";
    mFilePath = "C:";

	//Here, we show that it is easy to enable/disable final artifacts,
	//like the SafeArea or the CameraLabel. IMPORTANT: For this, we must
	//not forget to pass this FBView to RenderEnd or these won't be drawed.
	FBViewingOptions* lVO = mRender->GetViewingOptions();
	lVO->ShowTimeCode() = false;
	lVO->ShowSafeArea() = false;
	lVO->ShowCameraLabel() = false;

	mRenderStart = false;
	RefreshStats();
}

/************************************************
 *	Destructor
 ************************************************/
void ORView3D::FBDestroy()
{
	if( mFontDisplayList )
	{
		glDeleteLists(mFontDisplayList,256);
		mFontDisplayList = 0;
	}
}

/************************************************
 *	Refresh callback
 ************************************************/
void ORView3D::Refresh(bool pNow)
{
	FBView::Refresh(pNow);
}

/************************************************
 *	Expose callback
 ************************************************/
void ORView3D::ViewExpose()
{
	if (mRenderStart)
	{
		RefreshStats();
		
	}

	int Width = Region.Position.X[1] - Region.Position.X[0];
	int Height = Region.Position.Y[1] - Region.Position.Y[0];

	mLayout.ReSize( Width, Height );
	mLayout.Render( mRender, this );
}

/************************************************
 *	user input
 ************************************************/
void ORView3D::ViewInput(int pMouseX, int pMouseY, FBInputType pAction, int pButtonKey, int pModifier)
{
}

/************************************************
 *	Refresh grabbing statistics
 ************************************************/
void ORView3D::RefreshStats()
{
	FBTime		lTime;
	mStats = VideoGrabber.GetStatistics();
	char	text[256];
	
	sprintf( text, "%s %d", TOTALFRAMECOUNT, mStats.mTotalFrameCount );
	mTotalFrameCount.Caption = text;
	
	sprintf( text, "%s %d", REMAININGFRAMECOUNT, mStats.mRemainingFrameCount );
	mRemainingFrameCount.Caption = text;
	mRemainingFrameCount.Refresh(true);

	lTime = mStats.mTotalTimeElapsed;
	sprintf( text, "%s %-10.2lf", TOTALTIMEELAPSED, lTime.GetSecondDouble() );
	mTotalTimeElapsed.Caption = text;
	mTotalTimeElapsed.Refresh(true);

	lTime = mStats.mEstimatedTime;
	sprintf( text, "%s %-10.2lf", ESTIMATEDTIME, lTime.GetSecondDouble() );
	mEstimatedTime.Caption = text;
	mEstimatedTime.Refresh(true);

	lTime = mStats.mEstimatedTimeRemaining;
	sprintf( text, "%s %-10.2lf", ESTIMATEDTIMEREMAINING, lTime.GetSecondDouble() );
	mEstimatedTimeRemaining.Caption = text;
	mEstimatedTimeRemaining.Refresh(true);

	lTime = mStats.mTimePerFrame;
	sprintf( text, "%s %-10.2lf", TIMEPERFRAME, lTime.GetSecondDouble() );
	mTimePerFrame.Caption = text;
	mTimePerFrame.Refresh(true);

	// update progress bar (if exists)
	if (g_pProgress)
	{
		double percent = 100.0 * (mStats.mTotalFrameCount - mStats.mRemainingFrameCount) / mStats.mTotalFrameCount;
		g_pProgress->Percent = (int) percent;
		g_pProgress->Refresh(true);
	}
}

/************************************************
 *	begin rendering process
 ************************************************/
void ORView3D::Render(FBVideoGrabOptions	&GrabOptions)
{
	if( VideoGrabber.BeginGrab() )	//Check if we can grab
	{
		FBProgress		lProgress;
		lProgress.Caption = "Render";
		lProgress.Text = "in progress";
		lProgress.Percent = 0;
		g_pProgress = &lProgress;

		mRenderStart = true;
		VideoGrabber.SetOptions(&GrabOptions);
		//mStats = VideoGrabber.GetStatistics();
		RefreshStats();

		VideoGrabber.Grab();		//Grab all frame
		VideoGrabber.EndGrab();		//End grabbing session
	}
	mRenderStart = false;
	g_pProgress = NULL;
}

/************************************************
 *	static view callback
 ************************************************/
void ORView3D::RefreshViewCallback(void* pThis)
{
	ORView3D* lThis = (ORView3D*)pThis;
	lThis->Refresh(true);
}

FBString ORView3D::GetFileName()
{
    return mFileName;
}

void ORView3D::SetFileName( FBString pFileName )
{
    mFileName = pFileName;
}

FBString ORView3D::GetFilePath()
{
    return mFilePath;
}

void ORView3D::SetFilePath( FBString pFilePath )
{
    mFilePath = pFilePath;
}

void ORView3D::CustomGLDrawString(int pX, int pY, char* pText)
{
// For now we cannot draw custom strings on Mac OSX...
#if defined( KARCH_ENV_WIN )
	if( mFontDisplayList == 0 )	//Create fonts display list. (256 is for the ascii table size)
	{
		mFontDisplayList = glGenLists(256);
		if( !wglUseFontBitmaps(GetDC(NULL), 0, 256, mFontDisplayList) )
		{
			glDeleteLists(mFontDisplayList, 256);
			mFontDisplayList = 0;
		}
	}

	if( mFontDisplayList )
	{
		//We must initialize 2D coordinate system so that the upper-left corner is the origin.
		int Width = Region.Position.X[1] - Region.Position.X[0];
		int Height = Region.Position.Y[1] - Region.Position.Y[0];
		glViewport(0, 0, Width, Height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, Width, Height, 0, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//Draw text
		glRasterPos2i(pX, pY);									//Position the string in space 
		glListBase(mFontDisplayList);							//Indicate start of glyph display lists
		glCallLists(strlen(pText), GL_UNSIGNED_BYTE, pText);	//Now draw the characters in a string
	}
#endif
}
