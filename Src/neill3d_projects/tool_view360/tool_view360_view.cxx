
/**	\file	ortoolview360_view.cxx

	Author Sergey Solohin (Neill3d)
	 e-mail to: s@neill3d.com
	  www.neill3d.com

*/

//--- Class declaration
#include "tool_view360_view.h"
#include <math.h>
#include "utils.h"

FBClassImplementation( ORView360 );

extern void ToggleMaximize(const bool maximized);


/************************************************
 *	Constructor
 ************************************************/
ORView360::ORView360() 
	: mRender(nullptr)
	, mPaneId(0)
{
	
	mRender = new FBRenderer(0);
	mIsMaximized = false;	
	mScaleDown = 1;

}

/************************************************
 *	Constructor
 ************************************************/
void ORView360::FBDestroy()
{
	delete mRender;
}

/************************************************
 *	Refresh callback
 ************************************************/
void ORView360::Refresh(bool pNow)
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


void ORView360::ViewExpose()
{
	if ( nullptr == mRender ) 
		return;
	
	int paneCount = 0;
	FBCamera *pCameraPane0 = mSystem.Renderer->CurrentCamera;
	FBCamera *pCameraPane1 = nullptr;

	GetViewerPaneInfo( paneCount, pCameraPane0, pCameraPane1 );

	mRender->CurrentCamera = (mPaneId > 0) ? pCameraPane1 : mSystem.Renderer->CurrentCamera;
	FBCamera *pCamera = mRender->CurrentCamera;
	if (pCamera == nullptr) 
		return;

	// DONE: resize frame buffer, bind and output alpha with simple shader
	// region is like a window size
	int regionWidth = Region.Position.X[1]-Region.Position.X[0];
	const int regionHeight = Region.Position.Y[1]-Region.Position.Y[0];

	int x=0, y=0, width=1, height=1;
	ComputeViewport( pCamera, regionWidth, regionHeight, mScaleDown, x, y, width, height );


	// TODO: render each side from different camera - 6 render passes

	mRender->SetViewingOptions( *mSystem.Renderer->GetViewingOptions() );

	if(mRender->RenderBegin(0, 0, width, height))
	{
		// This call will destroy the frame buffer
		mRender->PreRender();

		mRender->Render();
		mRender->RenderEnd();
	}

	// compose cubemap sides into a panorama image and output to the view

	// button to save frame image or timeline image sequence
}


void ORView360::ViewInput(int pMouseX,int pMouseY,FBInputType pAction,int pButtonKey,int pModifier)
{
}
