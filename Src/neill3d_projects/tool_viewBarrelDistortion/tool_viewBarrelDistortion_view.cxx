
/**	\file	ortoolview3d_view.cxx

// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE

	GitHub repo - https://github.com/Neill3d/MoBu

	Author Sergey Solohin (Neill3d)
	 e-mail to: s@neill3d.com
	  www.neill3d.com

*/

//--- Class declaration
#include "tool_viewBarrelDistortion_view.h"
#include <math.h>
#include "utils.h"
#include "FileUtils.h"

FBClassImplementation( ORViewPaneWithDistortion );

extern void ToggleMaximize(const bool maximized);


////////////////////////
// global view processing instance

CViewProcessing		gViewProcessing;

/////////////////////////////////////////////////////////////////////////////////////////////////////
// CViewProcessing

CViewProcessing::CViewProcessing()
{
	mViewCounter = 0;
	mTryToLoadShader = true;
}

void CViewProcessing::ReloadShaders()
{
	mTryToLoadShader = true;
}

bool CViewProcessing::PrepFrameBuffer(const int width, const int height)
{
	if ( nullptr == mFrameBuffer.get() )
	{
		mFrameBuffer.reset( new FrameBuffer(width, height, FrameBuffer::depth32, 1) );
	}
	
	mFrameBuffer->resize(width, height);
	return true;
}

bool CViewProcessing::PrepShader()
{
	if ( nullptr == mShader.get() && true == mTryToLoadShader )
	{
		mTryToLoadShader = false;
		GLSLShader *pNewShader = nullptr;
		
		try
		{
			pNewShader = new GLSLShader();

			FBString out_path, out_fullname;
			if (false == FindEffectLocation( "\\GLSL\\barrelDistortion.frag", out_path, out_fullname ) )
				throw std::exception( "failed to locate shaders" );
		
			if (false == pNewShader->LoadShaders( FBString(out_path, "\\GLSL\\barrelDistortion.vert"), FBString(out_path, "\\GLSL\\barrelDistortion.frag") ) )
				throw std::exception( "failed to load shaders" );

			pNewShader->Bind();
			pNewShader->setUniformUINT( "texSampler", 0 );
			pNewShader->UnBind();
		}
		catch (const std::exception &e)
		{
			delete pNewShader;
			pNewShader = nullptr;
			FBMessageBox( "View Barrel Distortion Tool", e.what(), "Ok" );
		}

		mShader.reset(pNewShader);
	}
	return (mShader.get() != nullptr);
}

void CViewProcessing::IncView()
{
	mViewCounter += 1;
}

void CViewProcessing::DecView()
{
	mViewCounter -= 1;

	if (mViewCounter <= 0)
	{
		mFrameBuffer.reset(nullptr);
		mShader.reset(nullptr);
	}
}

FrameBuffer *CViewProcessing::GetFrameBufferPtr(const int width, const int height)
{
	PrepFrameBuffer(width, height);
	return mFrameBuffer.get();
}

GLSLShader *CViewProcessing::GetShaderPtr()
{
	PrepShader();
	return mShader.get();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// ORViewPaneWithDistortion

/************************************************
 *	Constructor
 ************************************************/
ORViewPaneWithDistortion::ORViewPaneWithDistortion(const int paneId) 
	: mRender(nullptr)
	, mPaneId(paneId)
{
	
	mRender = new FBRenderer(0);
	mIsMaximized = false;	
	mScaleDown = 1;

	gViewProcessing.IncView();
}

/************************************************
 *	Constructor
 ************************************************/
void ORViewPaneWithDistortion::FBDestroy()
{
	delete mRender;

	gViewProcessing.DecView();
}

/************************************************
 *	Refresh callback
 ************************************************/
void ORViewPaneWithDistortion::Refresh(bool pNow)
{
	FBView::Refresh(pNow);
}

/************************************************
 *	Expose callback
 ************************************************/


////////////////////////////////////////////////////////////////////////////////////////////////
//


void ComputeViewport(FBCamera *pCamera, const int regionWidth, const int regionHeight, const int scaledown, int &x, int &y, int &width, int &height)
{
	FBCameraFrameSizeMode sizeMode = pCamera->FrameSizeMode;
	FBCameraResolutionMode resMode = pCamera->ResolutionMode;
	double resWidth = pCamera->ResolutionWidth;		// relative coeff or absolute value depends on the sizeMode
	double resHeight = pCamera->ResolutionHeight;	// the same...
	//double resRatio = pCamera->PixelAspectRatio;

	double resRatio = 1.0 * resWidth / resHeight;
	double regionRatio = 1.0 * regionWidth / regionHeight;

	x=0;
	y=0;
	width=1;
	height=1;

	switch (sizeMode)
	{
	case kFBFrameSizeWindow:
		width = regionWidth;
		height = regionHeight;
		break;
	case kFBFrameSizeFixedRatio:

		resRatio = resWidth;

		if (regionRatio > resRatio)
		{
			height = regionHeight;
			width = resRatio * height;
		}
		else
		{
			width = regionWidth;
			height = 1.0 / resRatio * (double) width;
		}
			
		break;
	case kFBFrameSizeFixedHeightResolution:

		resRatio = resWidth;

		if (regionRatio > resRatio)
		{
			height = regionHeight;
			
			if (height > resHeight)
				height = resHeight;

			width = resRatio * height;
		}
		else
		{
			width = regionWidth;
			height = 1.0 / resRatio * (double) width;

			if (height > resHeight)
			{
				height = resHeight;
				width = resRatio * height;
			}
		}

		break;
	case kFBFrameSizeFixedWidthResolution:
		
		resRatio = 1.0 / resHeight;

		if (regionRatio > resRatio)
		{
			height = regionHeight;
			width = resRatio * height;

			if (width > resWidth)
			{
				width = resWidth;
				height = 1.0 / resRatio * width;
			}
		}
		else
		{
			width = regionWidth;

			if (width > resWidth)
				width = resWidth;

			height = 1.0 / resRatio * (double) width;
		}

		break;
	case kFBFrameSizeFixedResolution:
		width = resWidth;
		height = resHeight;

		resRatio = resWidth / resHeight;

		if (regionRatio > resRatio)
		{
			height = regionHeight;

			if (height > resHeight)
				height = resHeight;

			width = resRatio * height;

			if (width > resWidth)
			{
				width = resWidth;
				height = 1.0 / resRatio * width;
			}
		}
		else
		{
			width = regionWidth;

			if (width > resWidth)
				width = resWidth;

			height = 1.0 / resRatio * (double) width;

			if (height > resHeight)
			{
				height = resHeight;
				width = resRatio * height;
			}
		}

		break;
	}
	
	if (scaledown > 1)
	{
		width /= scaledown;
		height /= scaledown;
	}

	x = (regionWidth - width) / 2;
	y = (regionHeight - height) / 2;
}

void drawOrthoQuad2d(const int w, const int h, const float angle)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, w, 0.0, h, -1.0, 1.0); 
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//-------------------------
	
	glPushMatrix();
	glTranslatef(-0.5f * w, -0.5f * w, 0.0f);
	glRotatef(angle, 0.0f, 0.0f, 1.0f);
	glTranslatef(0.5f * w, 0.5f * w, 0.0f);

	glColor3d(1.0, 1.0, 1.0);
	glBegin(GL_QUADS);

	glTexCoord2d(0.0,		0.0);
		glVertex2d(0.0,			0.0);
	glTexCoord2d(1.0,		0.0);
		glVertex2d(w,	0.0);
	glTexCoord2d(1.0,		1.0);
		glVertex2d(w,	h);
	glTexCoord2d(0.0,		1.0);
		glVertex2d(0.0,			h);

	glEnd();

	glPopMatrix();
}

void ORViewPaneWithDistortion::ViewExpose()
{
	if ( nullptr == mRender ) 
		return;
	
	int paneCount = 0;
	FBCamera *pCameraPane0 = mSystem.Renderer->CurrentCamera;
	FBCamera *pCameraPane1 = nullptr;

	if (nullptr != pCameraPane0 && FBIS(pCameraPane0, FBCameraStereo) )
	{
		FBCameraStereo *pStereo = (FBCameraStereo*) pCameraPane0;
		pCameraPane0 = pStereo->LeftCamera;
		pCameraPane1 = pStereo->RightCamera;
	}

	//GetViewerPaneInfo( paneCount, pCameraPane0, pCameraPane1 );

	FBCamera *pCamera = (mPaneId > 0) ? pCameraPane1 : pCameraPane0;
	if (pCamera == nullptr) 
		return;

	mRender->CurrentCamera = pCamera;
	

	// DONE: resize frame buffer, bind and output alpha with simple shader
	// region is like a window size
	int regionWidth = Region.Position.X[1]-Region.Position.X[0];
	const int regionHeight = Region.Position.Y[1]-Region.Position.Y[0];

	int x=0, y=0, width=1, height=1;
	ComputeViewport( pCamera, regionWidth, regionHeight, mScaleDown, x, y, width, height );

	GLSLShader *pShader = gViewProcessing.GetShaderPtr();
	if (nullptr == pShader)
		return;
	FrameBuffer *pFrameBuffer = gViewProcessing.GetFrameBufferPtr(width, height);
	if (nullptr == pShader)
		return;

	mRender->SetViewingOptions( *mSystem.Renderer->GetViewingOptions() );

	pFrameBuffer->bind();

	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	if(mRender->RenderBegin(0, 0, width, height))
	{
		// This call will destroy the frame buffer
		mRender->PreRender();

		mRender->Render();
		mRender->RenderEnd();
	}

	pFrameBuffer->unbind();

	// apply barrel distortion
	glDisable(GL_DEPTH_TEST);

	pShader->Bind();

	pShader->setUniformVector2f( "centre", 0.0f, 0.0f );

	glBindTexture(GL_TEXTURE_2D, pFrameBuffer->getColorBuffer() );
	drawOrthoQuad2d(regionWidth, regionHeight, 0.0f);

	pShader->UnBind();
}


void ORViewPaneWithDistortion::ViewInput(int pMouseX,int pMouseY,FBInputType pAction,int pButtonKey,int pModifier)
{
	if (kFBButtonDoubleClick == pAction)
	{
		mIsMaximized = ~mIsMaximized;
		ToggleMaximize( mIsMaximized );
	}
}



