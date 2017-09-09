
/**	\file	ortoolview360_view.cxx

	Author Sergey Solokhin (Neill3d)
	 e-mail to: s@neill3d.com
	  www.neill3d.com

*/

//--- Class declaration
#include "tool_view360_view.h"
#include <math.h>

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

	mTextureCube = 0;
	memset( mTextures, 0, sizeof(GLuint) * TEXTURE_COUNT );
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
	
	
	FBCamera *pCamera = mSystem.Renderer->CurrentCamera;
	if ( nullptr == pCamera ) 
		return;

	// find 6 cameras for each side

	if (false == PrepCameraSet(pCamera) )
		return;

	// DONE: resize frame buffer, bind and output alpha with simple shader
	// region is like a window size
	int regionWidth = Region.Position.X[1]-Region.Position.X[0];
	const int regionHeight = Region.Position.Y[1]-Region.Position.Y[0];

	int x=0, y=0, width=1, height=1;
	ComputeViewport( pCamera, regionWidth, regionHeight, mScaleDown, x, y, width, height );


	// TODO: render each side from different camera - 6 render passes

	if (false == PrepShader() )
		return;

	if (false == PrepFramebuffer(width, height) )
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

	for (int side=0; side<6; ++side)
	{
		mRender->CurrentCamera = pCameras[side];

		mFramebuffer->bind();
		mFramebuffer->attachColorTexture( GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + side, mTextureCube );

		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		if(mRender->RenderBegin(0, 0, width, height))
		{
			// This call will destroy the frame buffer
			mRender->PreRender();

			mRender->Render();
			mRender->RenderEnd();
		}

		mFramebuffer->unbind();
	}

	mRender->CurrentCamera = pCamera;

	// compose cubemap sides into a panorama image and output to the view

	glDisable(GL_DEPTH_TEST);

	mFramebuffer->bind();
	mFramebuffer->attachColorTexture( GL_TEXTURE_2D, mTextures[TEXTURE_PANO] );

	mShader->Bind();

	glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureCube);
	drawOrthoQuad2d(width, height, 0.0f);

	mShader->UnBind();

	mFramebuffer->unbind();

	// button to save frame image or timeline image sequence
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mTextures[TEXTURE_PANO]);
	drawOrthoQuad2d(width, height, 0.0f);
}


void ORView360::ViewInput(int pMouseX,int pMouseY,FBInputType pAction,int pButtonKey,int pModifier)
{
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

	mCameraFront = pCameras[0];
	mCameraBack = pCameras[1];
	mCameraLeft = pCameras[2];
	mCameraRight = pCameras[3];
	mCameraTop = pCameras[4];
	mCameraBottom = pCameras[5];

	return ( mCameraFront.Ok() && mCameraBack.Ok() && mCameraLeft.Ok()
		&& mCameraRight.Ok() && mCameraTop.Ok() && mCameraBottom.Ok() );
}

bool ORView360::PrepFramebuffer(const int width, const int height)
{
	if ( nullptr == mFramebuffer.get() )
	{
		mFramebuffer.reset( new FrameBuffer(1, 1) );
	}

	if ( true == mFramebuffer->resize(width, height) )
	{
		if (false == mFramebuffer->isOk() )
			return false;

		// prep textures for the framebuffer
		AllocTextures(width, height);
	}

	return true;
}

bool ORView360::PrepShader()
{
	if ( nullptr == mShader.get() )
	{
		GLSLShader *pNewShader = new GLSLShader();

		const char *vertex_text =
			"#define PI    3.141592653589793\n"
            "#define TWOPI 6.283185307179587\n"
			"void main(void)\n"
			"{\n"
			"gl_Position     = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
			"gl_TexCoord [0].st = vec2(gl_MultiTexCoord0.s, gl_MultiTexCoord0.t);\n"
			"}";
		/*
		const char *fragment_text = 
			"#version 120\n"
			"uniform samplerCube 	cubeMapSampler;\n"
			"\n"
			"void main()\n"
			"{\n"
			"vec2 sc = vec2(gl_TexCoord[0].st) * vec2(3.14159265, 2*3.14159265);\n"
			"sc.s -= 0.5 * 3.14159265;\n"
			"sc.t += 0.5 * 3.14159265;\n"
			"// convert to cartesian unit vector\n"
			"vec3 coords;\n"
			"coords.y = sin(sc.s);\n"
			"coords.x = cos(sc.s) * cos(sc.t);\n"
			"coords.z = cos(sc.s) * sin(sc.t);\n"
			"vec4 color = textureCube(cubeMapSampler, coords);\n"
			"gl_FragColor = color;\n"
			"}";
			*/
		/*
		const char *fragment_text = 
			"#version 120\n"
			"uniform samplerCube 	cubeMapSampler;\n"
			"\n"
			"void main()\n"
			"{\n"
			"vec2 sc = vec2(gl_TexCoord[0].st);\n"
			
			"float theta = 1.0 - sc.y;\n"
            "float phi = sc.x;\n"
            "vec3 unit = vec3(0,0,0);\n"
			"\n"
            "unit.x = sin(phi) * sin(theta) * -1;\n"
            "unit.y = cos(theta) * -1;\n"
			"unit.z = cos(phi) * sin(theta) * -1;\n"
			"\n"
			"vec4 color = textureCube(cubeMapSampler, unit);\n"
			"gl_FragColor = color;\n"
			"}";
		*/

		const char *fragment_text = 
			"#version 120\n"
			"uniform samplerCube 	cubeMapSampler;\n"
			"\n"
			"void main()\n"
			"{\n"
			"\n"
			"vec2 tc = gl_TexCoord[0].st / vec2(2.0) + 0.5;  //only line modified from the shader toy example\n"
			"vec2 thetaphi = ((tc * 2.0) - vec2(1.0)) * vec2(3.1415926535897932384626433832795, 1.5707963267948966192313216916398);\n"
			"vec3 rayDirection = vec3(cos(thetaphi.y) * cos(thetaphi.x), sin(thetaphi.y), cos(thetaphi.y) * sin(thetaphi.x));\n"
			"\n"
			"vec4 color = textureCube(cubeMapSampler, rayDirection);\n"
			"gl_FragColor = color;\n"
			"}";

		

		bool res = pNewShader->LoadShadersSource( vertex_text, fragment_text );

		if (false == res)
		{
			delete pNewShader;
			pNewShader = nullptr;
		}
		else
		{
			pNewShader->Bind();
			pNewShader->setUniformUINT( "cubeMapSampler", 0 );
			pNewShader->UnBind();
		}

		mShader.reset(pNewShader);
	}
	return true;
}

void ORView360::AllocTextures(const int width, const int height)
{
	FreeTextures();

	glGenTextures( 1, &mTextureCube );
	glGenTextures( TEXTURE_COUNT, mTextures );

	// color sides
	FrameBuffer::createCubemapTexture( &mTextureCube, width, width );

	// shared depth texture
	FrameBuffer::createColorTexture( &mTextures[TEXTURE_DEPTH], width, height, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT32, GL_FLOAT );

	// output panorama textures
	FrameBuffer::createColorTexture( &mTextures[TEXTURE_PANO], width, height );

	// attach shared depth
	if (nullptr != mFramebuffer.get() )
		mFramebuffer->attachDepthTexture(GL_TEXTURE_2D, mTextures[TEXTURE_DEPTH] );
}

void ORView360::FreeTextures()
{
	if (mTextures[0] > 0)
	{
		if ( nullptr != mFramebuffer.get() )
		{
			mFramebuffer->detachColorTexture(GL_TEXTURE_2D);
			mFramebuffer->detachDepthTexture(GL_TEXTURE_2D);
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