
/**	\file	ortoolview360_view.cxx

	Custom FBView that render scene from 6 sides and compose into a equirectangle panorama

// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE

	GitHub repo - https://github.com/Neill3d/MoBu

	Author Sergey Solokhin (Neill3d)
	 e-mail to: s@neill3d.com
	  www.neill3d.com

*/

//--- Class declaration
#include "tool_view360_view.h"
#include <math.h>

#include <glm\mat4x4.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "FileUtils.h"

// TODO: at the moment it's a fixed side resolution, so that panorama is 4320 x 2160

#define CUBE_SIDE_RESOLUTION	1080

FBClassImplementation( ORView360 );

// HACK: direct access to Qt window to switch a maximize flag
extern void ToggleMaximize(const bool maximized);


/*
source: DEBUG_SOURCE_X where X may be API, 
SHADER_COMPILER, WINDOW_SYSTEM, 
THIRD_PARTY, APPLICATION, OTHER
type: DEBUG_TYPE_X where X may be ERROR, 
MARKER, OTHER, DEPRECATED_BEHAVIOR, 
UNDEFINED_BEHAVIOR, PERFORMANCE, 
PORTABILITY, {PUSH, POP}_GROUP
severity: DEBUG_SEVERITY_{HIGH, MEDIUM}, 
DEBUG_SEVERITY_{LOW, NOTIFICATION}
*/
void DebugOGL_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void*userParam)
{
	if (type == GL_DEBUG_TYPE_ERROR)
	{
		printf( ">> ERROR!\n" );
		printf( "debug message - %s\n", message );
	}
	//printf( "debug message - %s\n", message );
}


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
	mDisplayMode = DISPLAY_RAW_PANORAMA;
	mFirstTime = true;

	mTextureCube = 0;
	memset( mTextures, 0, sizeof(GLuint) * TEXTURE_COUNT );

	mSaveScreenshot = false;
	mSingleFilename = "";
	mSequenceFilename = "";

	mTryToLoadShader = true;
}

/************************************************
 *	Constructor
 ************************************************/
void ORView360::FBDestroy()
{
	FreeTextures();
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
	//FBCameraResolutionMode resMode = pCamera->ResolutionMode;
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

void ORView360::ViewExpose()
{
	if ( nullptr == mRender ) 
		return;
	
	//
#ifdef _DEBUG
	static bool onlyOnce = true;

	if (true == onlyOnce)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback( DebugOGL_Callback, nullptr );
		onlyOnce = false;
	}
#endif

	FBCamera *pCamera = mSystem.Renderer->CurrentCamera;
	if ( nullptr == pCamera ) 
		return;

	// find 6 cameras for each side

	if (false == PrepCameraSet(pCamera) )
		return;

	// DONE: resize frame buffer, bind and output alpha with simple shader
	// region is like a window size
	const int regionWidth = Region.Position.X[1]-Region.Position.X[0];
	const int regionHeight = Region.Position.Y[1]-Region.Position.Y[0];

	// at the moment it's a fixed resolution, panorama = 4320 x 2160 
	
	const int width = CUBE_SIDE_RESOLUTION;
	const int height = CUBE_SIDE_RESOLUTION;

	const int panoWidth = width * 4;
	const int panoHeight = height * 2;

	// DONE: render each side from different camera - 6 render passes

	if (false == PrepShader()
		|| false == PrepShaderEquiToSphere() )
	{
		mTryToLoadShader = false;
		return;
	}

	if (false == PrepFramebuffer(width, height, panoWidth, panoHeight) )
		return;

	mRender->SetViewingOptions( *mSystem.Renderer->GetViewingOptions() );

	// render cube 6 sided
	FBCamera *pCameras[6] = { 
		(FBCamera*)mCameraRight.GetPlug(),
		(FBCamera*)mCameraLeft.GetPlug(),
		(FBCamera*)mCameraBottom.GetPlug(),
		(FBCamera*)mCameraTop.GetPlug(), 
		
		(FBCamera*)mCameraFront.GetPlug(), 
		(FBCamera*)mCameraBack.GetPlug()
		 };

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, width, height);
	
	/*
	float colors[6][3] = { {1.0f, 0.0f, 0.0f},
							{0.0f, 1.0f, 0.0f},
							{0.0f, 0.0f, 1.0f},
							{1.0f, 1.0f, 0.0f},
							{0.0f, 1.0f, 1.0f},
							{1.0f, 0.0f, 1.0f} };
*/
	for (int side=0; side<6; ++side)
	{
		mRender->CurrentCamera = pCameras[side];

		mFramebufferCube->bind();
		mFramebufferCube->attachColorTexture( GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, mTextureCube );
		
		//glClearColor( colors[side][0], colors[side][1], colors[side][2], 1.0f );
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		
		if(mRender->RenderBegin(0, 0, width, height))
		{
			glViewport(0, 0, width, height);
			// This call will destroy the frame buffer
			mRender->PreRender();

			mRender->Render();
			mRender->RenderEnd();
		}
		
		mFramebufferCube->unbind();
	}

	mRender->CurrentCamera = pCamera;

	// compose cubemap sides into a panorama image and output to the view

	glDisable(GL_DEPTH_TEST);

	glMemoryBarrier( GL_FRAMEBUFFER_BARRIER_BIT );

	mFramebufferPano->bind();
	//mFramebufferCube->detachDepthTexture( GL_TEXTURE_2D );
	mFramebufferPano->attachColorTexture( GL_TEXTURE_2D, mTextures[TEXTURE_PANO] );

	mFramebufferPano->isOk();

	mShaderCubeToPan->Bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureCube);
	drawOrthoQuad2d(panoWidth, panoHeight, 0.0f);

	mShaderCubeToPan->UnBind();

	mFramebufferPano->unbind();

	//
	// button to save frame image or timeline image sequence
	glMemoryBarrier( GL_FRAMEBUFFER_BARRIER_BIT );

	const bool bindSphereShader = ( DISPLAY_RAW_PANORAMA != mDisplayMode );
	
	if ( bindSphereShader )
	{
		mShaderToSphere->Bind();

		FBMatrix rotationMatrix;
		rotationMatrix.Identity();
		FBVector4d direction;

		( (FBModel*)pCamera)->GetMatrix(rotationMatrix, kModelRotation);

		if (mFirstTime)
		{
			FBMatrixInverse(mInitRotation, rotationMatrix);
			mFirstTime = false;
		}

		//FBMatrixMult( rotationMatrix, mInitRotation, rotationMatrix );
		
		//FBMatrixTranspose( rotationMatrix, rotationMatrix );
		//FBMatrixInverse( rotationMatrix, rotationMatrix );
		
		//FBVector3d r;
		//pCamera->GetVector(r, kModelRotation);	

		FBVector4d normal(0, 0, -1, 1);
		FBVectorMatrixMult( direction, rotationMatrix, normal );

		glm::vec3 const up(0.f, 1.f, 0.f);
		glm::vec3 const pos( (float)direction[0], (float)direction[1], (float)direction[2] );
		glm::vec3 const target(0.f, 0.f, 0.f);
		glm::mat4x4 mat = glm::lookAt(pos, target, up);
		mat = glm::transpose(mat);
		mat = glm::inverse(mat);
		const float *fmat = glm::value_ptr(mat);

		/*
		for (int i=0; i<16; ++i)
			ftm[i] = (float) rotationMatrix[i];
			*/
		mShaderToSphere->setUniformMatrix( "rotation", fmat );

		mShaderToSphere->setUniformUINT( "mode", mDisplayMode );
		mShaderToSphere->setUniformVector2f( "u_TextureSize", (float)width, (float)height);

		glBindTexture(GL_TEXTURE_2D, mTextures[TEXTURE_PANO]);
		drawOrthoQuad2d(regionWidth, regionHeight, 0.0f);

		mShaderToSphere->UnBind();
	}
	else
	{
		glEnable(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, mTextures[TEXTURE_PANO]);
		drawOrthoQuad2d(regionWidth, regionHeight, 0.0f);
	}

	//
	// write texture data to disk if needed
	
	CheckForScreenshot();

}


void ORView360::ViewInput(int pMouseX,int pMouseY,FBInputType pAction,int pButtonKey,int pModifier)
{
	if (kFBButtonDoubleClick == pAction)
	{
		mIsMaximized = !mIsMaximized;
		ToggleMaximize( mIsMaximized );
	}
	else
	if (kFBButtonRelease == pAction && 3 == pButtonKey)
	{
		FBGenericMenu	*pMenu = new FBGenericMenu();

		pMenu->InsertLast( "Mono", 0 );
		pMenu->InsertLast( "Side By Side", 1 );
		pMenu->InsertLast( "Top Bottom", 2 );
		pMenu->InsertLast( "Raw Panorama", 3 );
		pMenu->InsertLast( "-", 4 );
		pMenu->InsertLast( "Reload Shaders", 5 );
		pMenu->InsertLast( "---", 6 );
		pMenu->InsertLast( "Save Screenshot...", 7 );
		pMenu->InsertLast( "Save TimeRange Sequence...", 8 );

		POINT	curPos;
		GetCursorPos(&curPos);

		auto pMenuItem = pMenu->Execute(curPos.x, curPos.y);
		const int id = pMenuItem->Id;

		switch(id)
		{
		case 0:
			mDisplayMode = DISPLAY_MONO;
			break;
		case 1:
			mDisplayMode = DISPLAY_STEREO_SBS;
			break;
		case 2:
			mDisplayMode = DISPLAY_STEREO_TAB;
			break;
		case 3:
			mDisplayMode = DISPLAY_RAW_PANORAMA;
			break;
		case 5:
			ReloadShaders();
			break;

		case 7:
			{
				FBFilePopup	lFilePopup;
				lFilePopup.Filter = "*.tif";
				lFilePopup.Style = kFBFilePopupSave;

				if (lFilePopup.Execute() )
				{
					mSaveScreenshot = true;
					mSingleFilename = lFilePopup.FullFilename;
				}
			}
			break;
		case 8:
			{
				FBFilePopup	lFilePopup;
				lFilePopup.Filter = "*.tif";
				lFilePopup.Style = kFBFilePopupSave;

				if (lFilePopup.Execute() )
				{
					mSaveScreenshot = true;
					mSequenceFilename = lFilePopup.FullFilename;
				}
			}
			break;
		}

		delete pMenu;
	}
}

bool ORView360::PrepCameraSet(FBCamera *pCamera)
{

	//
	// check if set is defined and ok to use

	if ( mCameraFront.Ok() && mCameraBack.Ok() && mCameraLeft.Ok()
		&& mCameraRight.Ok() && mCameraTop.Ok() && mCameraBottom.Ok() )
	{

		if ( pCamera == mCameraFront.GetPlug()
			|| pCamera == mCameraBack.GetPlug()
			|| pCamera == mCameraLeft.GetPlug()
			|| pCamera == mCameraRight.GetPlug()
			|| pCamera == mCameraTop.GetPlug()
			|| pCamera == mCameraBottom.GetPlug() )
		{
			return true;
		}
	}

	//
	// search for a 6 cameras set

	FBCamera *pCameras[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
	const char *szCameraNames[6] = { "_Front", "_Back", "_Left", "_Right", "_Top", "_Bottom" };

	auto fn_checkModel = [&pCameras, &szCameraNames] (FBModel *pModel) {

		const char *pModelName = pModel->Name;
		if ( FBIS( pModel, FBCamera ) )
		{
			for (int j=0; j<6; ++j)
			{
				if (nullptr != strstr( pModelName, szCameraNames[j] ) )
					pCameras[j] = (FBCamera*) pModel;
			}
		}
	};

	// 0 - search from the global scene camers

	const int camerasCount = mSystem.Scene->Cameras.GetCount();

	for (int i=0; i<camerasCount; ++i)
	{
		FBCamera *pCamera = mSystem.Scene->Cameras[i];
		fn_checkModel(pCamera);
	}
	/*
	// 1 - search from the current camera parent
	FBModel *pParent = pCamera->Parent;
	if (nullptr != pParent)
	{
		const int childrenCount = pParent->Children.GetCount();
		
		for (int i=0; i<childrenCount; ++i)
		{
			FBModel *pModel = (FBModel*) pParent->Children.GetAt(i);
			fn_checkModel(pModel);
		}
	}

	// 2 - look in side the current camera
	fn_checkModel( pCamera );

	// 3 - search inside the current camera children
	const int childrenCount  = pCamera->Children.GetCount();
	for (int i=0; i<childrenCount; ++i)
	{
		FBModel *pModel = (FBModel*) pCamera->Children.GetAt(i);
		fn_checkModel(pModel);
	}
	*/
	mCameraFront = pCameras[0];
	mCameraBack = pCameras[1];
	mCameraLeft = pCameras[2];
	mCameraRight = pCameras[3];
	mCameraTop = pCameras[4];
	mCameraBottom = pCameras[5];

	return ( mCameraFront.Ok() && mCameraBack.Ok() && mCameraLeft.Ok()
		&& mCameraRight.Ok() && mCameraTop.Ok() && mCameraBottom.Ok() );
}

bool ORView360::PrepFramebuffer(const int width, const int height, const int panoWidth, const int panoHeight)
{
	if ( nullptr == mFramebufferCube.get() )
	{
		mFramebufferCube.reset( new FrameBuffer(1, 1) );
	}
	if (nullptr == mFramebufferPano.get() )
	{
		mFramebufferPano.reset( new FrameBuffer(1, 1) );
	}

	if ( true == mFramebufferCube->resize(width, height)
		&& true == mFramebufferPano->resize(panoWidth, panoHeight) )
	{
		// prep textures for the framebuffer
		AllocTextures(width, height, panoWidth, panoHeight);
	}

	return ( mFramebufferCube->isOk() && mFramebufferPano->isOk() );
}

bool ORView360::PrepShader()
{
	if ( nullptr == mShaderCubeToPan.get() && true == mTryToLoadShader )
	{
		GLSLShader *pNewShader = nullptr;
		
		try
		{
			pNewShader = new GLSLShader();

			FBString out_path, out_fullname;
			if (false == FindEffectLocation( "\\GLSL\\cube2pan.frag", out_path, out_fullname ) )
				throw std::exception( "failed to locate shaders" );
		
			if (false == pNewShader->LoadShaders( FBString(out_path, "\\GLSL\\cube2Pan.vert"), FBString(out_path, "\\GLSL\\cube2Pan.frag") ) )
				throw std::exception( "failed to load shaders" );

			pNewShader->Bind();
			pNewShader->setUniformUINT( "cubeMapSampler", 0 );
			pNewShader->UnBind();
		}
		catch (const std::exception &e)
		{
			delete pNewShader;
			pNewShader = nullptr;
			FBMessageBox( "View 360 Tool", e.what(), "Ok" );
		}

		mShaderCubeToPan.reset(pNewShader);
	}
	return (mShaderCubeToPan.get() != nullptr);
}

bool ORView360::PrepShaderEquiToSphere()
{
	if ( nullptr == mShaderToSphere.get() && true == mTryToLoadShader )
	{
		GLSLShader *pNewShader = nullptr;
		
		try
		{
			pNewShader = new GLSLShader();

			FBString out_path, out_fullname;
			if (false == FindEffectLocation( "\\GLSL\\equirectangularsphere.frag", out_path, out_fullname ) )
				throw std::exception( "failed to locate equiToSphere shaders" );
		
			if (false == pNewShader->LoadShaders( FBString(out_path, "\\GLSL\\equirectangularsphere.vert"), 
				FBString(out_path, "\\GLSL\\equirectangularsphere.frag") ) )
			{
				throw std::exception( "failed to load equiToSphere shaders" );
			}
			pNewShader->Bind();
			pNewShader->setUniformUINT( "s_Texture", 0 );
			pNewShader->setUniformUINT( "mode", 1 ); // side by side
			pNewShader->UnBind();
		}
		catch (const std::exception &e)
		{
			delete pNewShader;
			pNewShader = nullptr;
			FBMessageBox( "View 360 Tool", e.what(), "Ok" );
		}

		mShaderToSphere.reset(pNewShader);
	}
	return (mShaderToSphere.get() != nullptr);

}

void ORView360::AllocTextures(const int width, const int height, const int panoWidth, const int panoHeight)
{
	FreeTextures();

	glGenTextures( 1, &mTextureCube );
	glGenTextures( TEXTURE_COUNT, mTextures );

	// color sides
	FrameBuffer::createCubemapTexture( &mTextureCube, width, width );

	// shared depth texture
	FrameBuffer::createColorTexture( &mTextures[TEXTURE_DEPTH], width, height, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT32, GL_FLOAT );

	// output panorama textures
	FrameBuffer::createColorTexture( &mTextures[TEXTURE_PANO], panoWidth, panoHeight );

	// attach shared depth
	if (nullptr != mFramebufferCube.get() )
	{
		mFramebufferCube->bind();
		mFramebufferCube->attachDepthTexture(GL_TEXTURE_2D, mTextures[TEXTURE_DEPTH] );
		mFramebufferCube->unbind();
	}
	if (nullptr != mFramebufferPano.get() )
	{
		mFramebufferPano->bind();
		mFramebufferPano->attachColorTexture(GL_TEXTURE_2D, mTextures[TEXTURE_PANO] );
		mFramebufferPano->unbind();
	}
}

void ORView360::FreeTextures()
{
	if (mTextures[0] > 0)
	{
		if ( nullptr != mFramebufferCube.get() )
		{
			mFramebufferCube->detachColorTexture(GL_TEXTURE_2D);
			mFramebufferCube->detachDepthTexture(GL_TEXTURE_2D);
		}
		if ( nullptr != mFramebufferPano.get() )
		{
			mFramebufferPano->detachColorTexture(GL_TEXTURE_2D);
		}

		glDeleteTextures(TEXTURE_COUNT, mTextures);
		memset( mTextures, 0, sizeof(GLuint) * TEXTURE_COUNT );
	}
	if (mTextureCube > 0)
	{
		glDeleteTextures(1, &mTextureCube);
		mTextureCube = 0;
	}
}

void ORView360::ReloadShaders()
{
	mShaderCubeToPan.reset(nullptr);
	mShaderToSphere.reset(nullptr);

	mTryToLoadShader = true;
}

void ORView360::CheckForScreenshot()
{
	if (mSaveScreenshot)
	{
		if (mSingleFilename.GetLen() > 0)
		{
			SaveScreenshot( mSingleFilename );
			mSingleFilename = "";
		}
		else if (mSequenceFilename.GetLen() > 0)
		{
			SaveTimeRangeSequence(mSequenceFilename);
			mSequenceFilename = "";
		}

		mSaveScreenshot = false;
	}
}

void ORView360::SaveScreenshot(const char *filename)
{
	if (nullptr == mFramebufferPano.get() )
		return;

	FBImage		newImage("");

	newImage.Init( kFBImageFormatRGBA32, mFramebufferPano->getWidth(), mFramebufferPano->getHeight() );

	unsigned char *buffer = newImage.GetBufferAddress();

	glBindTexture(GL_TEXTURE_2D, mTextures[TEXTURE_PANO] );
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	glBindTexture(GL_TEXTURE_2D, 0);

	newImage.WriteToTif( filename, "OpenMoBu 360 View Output", true );
}

void ORView360::SaveTimeRangeSequence(const char *filename)
{

}