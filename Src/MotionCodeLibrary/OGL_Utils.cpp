/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include <stdio.h>
#include <math.h>
#include <memory>

#include <Windows.h>

#include "OGL_Utils.h"
#include "CheckGLError.h"
#include "Logger.h"

#ifdef OGL_UTIL_CUBEMAP
#include "nv_dds/nv_dds.h"
#endif
/*
double log2( double n )
{  
	// log(n)/log(2) is log2.  
	return log( n ) / log( 2.0 );  
}
*/

void FitImageWithAspect(const int imgWidth, const int imgHeight, const int regionWidth, const int regionHeight, int &x, int &y, int &width, int &height)
{
	const double imgRatio = 1.0 * imgWidth / imgHeight;
	const double regionRatio = 1.0 * regionWidth / regionHeight;

	if (regionRatio > imgRatio)
	{
		height = regionHeight;
		width = static_cast<int>(imgRatio * height);
	}
	else
	{
		width = regionWidth;
		height = static_cast<int>(1.0 / imgRatio * (double)width);
	}

	x = (regionWidth - width) / 2;
	y = (regionHeight - height) / 2;
}



void Push2dViewport(int g_w, int g_h)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();

	glLoadIdentity();
	gluOrtho2D(0, g_w, 0, g_h);


	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glLoadIdentity();

}

void Pop2dViewport()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}


void Set2dViewport(int g_w, int g_h)
{
	glViewport(0, 0, g_w, g_h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, g_w, 0, g_h);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

void drawOrthoQuad2d(const double w, const double h)
{
	glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, w, 0.0, h, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//-------------------------

	// draw textured rects with shadow maps
	/*
	const float size = 256.0f;
	float positions[8] = {	0.0f,			0.0f,
	0.0f,			(float) h,
	(float)w,		0.0f,
	(float)w,		(float) h
	};
	const float uvs[8] = {	0.0, 0.0,
	0.0, 1.0,
	1.0, 0.0,
	1.0, 1.0
	};

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, positions);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, uvs);

	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );


	// finish

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	*/

	glColor3d(1.0, 1.0, 1.0);
	glBegin(GL_QUADS);

	glTexCoord2d(0.0, 0.0);
	glVertex2d(0.0, 0.0);
	glTexCoord2d(1.0, 0.0);
	glVertex2d(w, 0.0);
	glTexCoord2d(1.0, 1.0);
	glVertex2d(w, h);
	glTexCoord2d(0.0, 1.0);
	glVertex2d(0.0, h);

	glEnd();

}

void drawOrthoQuad2d(int x, int y, int x2, int y2, int w, int h)
{
	//glViewport(x, y, w, h);
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, w, 0.0, h, -1.0, 1.0); 
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//-------------------------
	
	glColor3d(1.0, 1.0, 1.0);
	glBegin(GL_QUADS);

	glTexCoord2d(0.0, 0.0);
		glVertex2d(x, y);

	glTexCoord2d(1.0, 0.0);
		glVertex2d(x2, y);

	glTexCoord2d(1.0, 1.0);
		glVertex2d(x2, y2);

	glTexCoord2d(0.0, 1.0);
		glVertex2d(x, y2);

	glEnd();
}

void drawQuad2d(double x, double y, double x2, double y2)
{
	glBegin(GL_QUADS);

	glTexCoord2d(0.0, 0.0);
		glVertex2d(x, y);

	glTexCoord2d(1.0, 0.0);
		glVertex2d(x2, y);

	glTexCoord2d(1.0, 1.0);
		glVertex2d(x2, y2);

	glTexCoord2d(0.0, 1.0);
		glVertex2d(x, y2);

	glEnd();
}

void drawOrthoQuad2dOffset(const double x, const double y, const double w, const double h)
{
	glViewport(0, 0, static_cast<GLsizei>(w + x), static_cast<GLsizei>(h + y));
	glMatrixMode(GL_PROJECTION);
	//glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, w + x, 0.0, h + y, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	//glPushMatrix();
	glLoadIdentity();
	//-------------------------

	glPushMatrix();

	glTranslated(x, y, 0.0);

	glColor3d(1.0, 1.0, 1.0);
	glBegin(GL_QUADS);

	glTexCoord2d(0.0, 0.0);
	glVertex2d(0.0, 0.0);
	glTexCoord2d(1.0, 0.0);
	glVertex2d(w, 0.0);
	glTexCoord2d(1.0, 1.0);
	glVertex2d(w, h);
	glTexCoord2d(0.0, 1.0);
	glVertex2d(0.0, h);

	glEnd();

	glPopMatrix();
	/*
	//
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	*/
}


void drawOrthoGradientQuad2d(const int w, const int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, w, 0.0, h, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	//-------------------------

	glColor3d(1.0, 1.0, 1.0);
	glBegin(GL_QUADS);

	glColor3d(0.5, 0.5, 0.5);
	glVertex2d(0.0, 0.0);
	glColor3d(0.5, 0.5, 0.5);
	glVertex2d(w, 0.0);
	glColor3d(0.2, 0.2, 0.2);
	glVertex2d(w, h);
	glColor3d(0.2, 0.2, 0.2);
	glVertex2d(0.0, h);

	glEnd();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//

GLuint		gQuadPrimitiveId = 0;

void PrepareOrthoView(const double w, const double h)
{
	glMatrixLoadIdentityEXT(GL_PROJECTION);
	glMatrixLoadIdentityEXT(GL_MODELVIEW);
	glMatrixOrthoEXT(GL_MODELVIEW, 0, w, 0, h, -1, 1);
}

// prepare vertex buffer for faster draw primitives
void PrepareDrawPrimitives()
{
	float positions[4][2] = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};

	glGenBuffers(1, &gQuadPrimitiveId);

	glBindBuffer(GL_ARRAY_BUFFER, gQuadPrimitiveId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, positions, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// will draw quad 1x1 with texcoords
void DrawQuad2d()
{
	if (gQuadPrimitiveId == 0)
		PrepareDrawPrimitives();

	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
	glEnableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, gQuadPrimitiveId);
	//glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, (const void*) 0 );
	glVertexPointer(2, GL_FLOAT, 0, (void *)0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glPopClientAttrib();
}

// if using vao, we need to recreate vao each time we change context
void ChangeContextDrawPrimitives()
{
	FreeDrawPrimitives();
}

// cleanup drawing primitives buffers
void FreeDrawPrimitives()
{
	if (gQuadPrimitiveId > 0)
	{
		glDeleteBuffers(1, &gQuadPrimitiveId);
		gQuadPrimitiveId = 0;
	}
}

///////////////////////////////////////////////////////////////////////////

void StoreCullMode(OGLCullFaceInfo &info)
{
	info.enabled = glIsEnabled(GL_CULL_FACE);
	glGetIntegerv( GL_CULL_FACE_MODE, &info.mode );
}

void FetchCullMode (const OGLCullFaceInfo &info)
{
	if (info.enabled == GL_TRUE) glEnable(GL_CULL_FACE);
	else glDisable(GL_CULL_FACE);
	glCullFace(info.mode);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////


void GenerateFBO(const int Width, const int Height, GLuint &color_buffer_id, GLuint &depth_id, GLuint &fbo_id, GLubyte *data)
{
	GLenum status;

	// create a texture we use as the color buffer
	glGenTextures(1, &color_buffer_id);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, color_buffer_id);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);

	// create a renderbuffer for the depth/stencil buffer
	/*
	glGenRenderbuffers(1, &depth_id);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_id);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, Width, Height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	*/

	glGenFramebuffers(1, &fbo_id);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_buffer_id, 0);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_id);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (GL_FRAMEBUFFER_COMPLETE != status)

	{
		// something went wrong :(
		//LOGE( "Failed to bind FBO with stencil\n" );
	}
	else
	{
		// profit :)
		//LOGE( "fbo setup is well!" );
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CleanupFBO(GLuint &color_buffer_id, GLuint &depth_id, GLuint &fbo_id)
{
	//Delete resources
	if (color_buffer_id > 0)
	{
		glDeleteTextures(1, &color_buffer_id);
		color_buffer_id = 0;
	}
	if (depth_id > 0)
	{
		glDeleteRenderbuffers(1, &depth_id);
		depth_id = 0;
	}
	if (fbo_id > 0)
	{
		glDeleteFramebuffers(1, &fbo_id);
		fbo_id = 0;
	}
}

//////////////////////////////////////
//


bool BlitFBOToFBO(const GLint FBO, const int width, const int height, const GLint defaultFBO, 
	const int defWidth, const int defHeight, const bool copyColor1, const bool copyColor2, const bool copyColor3)
{
	CHECK_GL_ERROR();

	if (FBO == 0 || defaultFBO == 0)
	{
		LOGE( "> BlitToDefaultFBO where FBO is empty !!\n" );
		return false;
	}
	/*
	FrameBuffer &lDefaultFBO = GetDefaultFrameBuffer();
	const int defWidth = lDefaultFBO.getWidth();
	const int defHeight = lDefaultFBO.getHeight();
	*/
	/*
	if (width != defWidth || height != defHeight)
	{
		LOGE( "> BlitToDefaultFBO where size is not equal !!\n" );
		return false;
	}
	*/
	glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defaultFBO );

	// DONE: color attachment1 (may be don't need it at all, use MS version instead!)
	if (copyColor1)
	{
		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		GLenum buffers1[1] = { GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(1, &buffers1[0] );

		glBlitFramebuffer(0, 0, width, height, 0, 0, defWidth, defHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		CHECK_GL_ERROR();
	}
	if (copyColor2)
	{
		glReadBuffer(GL_COLOR_ATTACHMENT2);
		glDrawBuffer(GL_COLOR_ATTACHMENT2);
		GLenum buffers1[1] = { GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(1, &buffers1[0] );
		// | GL_DEPTH_BUFFER_BIT
		glBlitFramebuffer(0, 0, width, height, 0, 0, defWidth, defHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		CHECK_GL_ERROR();
	}
	if (copyColor3)
	{
		glReadBuffer(GL_COLOR_ATTACHMENT3);
		glDrawBuffer(GL_COLOR_ATTACHMENT3);
		GLenum buffers1[1] = { GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(1, &buffers1[0] );
		// | GL_DEPTH_BUFFER_BIT
		glBlitFramebuffer(0, 0, width, height, 0, 0, defWidth, defHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		CHECK_GL_ERROR();
	}

	// color attachment0
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	GLenum buffers0[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, &buffers0[0] );

	glBlitFramebuffer(0, 0, width, height, 0, 0, defWidth, defHeight, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	CHECK_GL_ERROR();

	return true;
}

bool BlitFBOToFBOOffset(const GLint FBO, int x, int y, const int width, const int height, const GLint defaultFBO,
	int defX, int defY, const int defWidth, const int defHeight, bool copyDepth, const bool copyColor1, const bool copyColor2, const bool copyColor3)
{
	CHECK_GL_ERROR();

	if (FBO == 0 || defaultFBO == 0)
	{
		LOGE("> BlitToDefaultFBO where FBO is empty !!\n");
		return false;
	}
	/*
	FrameBuffer &lDefaultFBO = GetDefaultFrameBuffer();
	const int defWidth = lDefaultFBO.getWidth();
	const int defHeight = lDefaultFBO.getHeight();
	*/
	/*
	if (width != defWidth || height != defHeight)
	{
	LOGE( "> BlitToDefaultFBO where size is not equal !!\n" );
	return false;
	}
	*/
	glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defaultFBO);

	// DONE: color attachment1 (may be don't need it at all, use MS version instead!)
	if (copyColor1)
	{
		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		GLenum buffers1[1] = { GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(1, &buffers1[0]);

		glBlitFramebuffer(0, 0, width, height, 0, 0, defWidth, defHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		CHECK_GL_ERROR();
	}
	if (copyColor2)
	{
		glReadBuffer(GL_COLOR_ATTACHMENT2);
		glDrawBuffer(GL_COLOR_ATTACHMENT2);
		GLenum buffers1[1] = { GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(1, &buffers1[0]);
		// | GL_DEPTH_BUFFER_BIT
		glBlitFramebuffer(0, 0, width, height, 0, 0, defWidth, defHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		CHECK_GL_ERROR();
	}
	if (copyColor3)
	{
		glReadBuffer(GL_COLOR_ATTACHMENT3);
		glDrawBuffer(GL_COLOR_ATTACHMENT3);
		GLenum buffers1[1] = { GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(1, &buffers1[0]);
		// | GL_DEPTH_BUFFER_BIT
		glBlitFramebuffer(0, 0, width, height, 0, 0, defWidth, defHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		CHECK_GL_ERROR();
	}

	// color attachment0
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	GLenum buffers0[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, &buffers0[0]);

	GLbitfield mask = (copyDepth) ? (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) : GL_COLOR_BUFFER_BIT;

	glBlitFramebuffer(x, y, x+width, y+height, defX, defY, defX+defWidth, defY+defHeight, mask, GL_NEAREST);
	CHECK_GL_ERROR();

	return true;
}

bool BlitFBOToScreen(const GLint defaultFBO, const int defWidth, const int defHeight, const int screenWidth, const int screenHeight)
{
	//if (defWidth != screenWidth || defHeight != screenHeight)
	//	return false;

	GLenum filteringMode = (defWidth != screenWidth || defHeight != screenHeight) ? GL_LINEAR : GL_NEAREST;

	glBindFramebuffer(GL_READ_FRAMEBUFFER, defaultFBO );
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0 );

	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glDrawBuffer(GL_BACK);
	
	glBlitFramebuffer(0, 0, defWidth, defHeight, 0, 0, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT, filteringMode);

	CHECK_GL_ERROR();
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void TextureObjectGetInfo( const GLenum target, int &width, int &height, int &internalFormat, int &format, bool &isCompressed, int &pixelMemorySize, int &imageSize, int &numberOfLods )
{
	isCompressed = false;
	imageSize = 0;
	
	// get a compression result
	
	int compressed;
	int red, green, blue, alpha;
	
	glGetTexLevelParameteriv( target, 0, GL_TEXTURE_WIDTH, &width );
	glGetTexLevelParameteriv( target, 0, GL_TEXTURE_HEIGHT, &height );
			
	glGetTexLevelParameteriv( target, 0, GL_TEXTURE_RED_SIZE, &red);
	glGetTexLevelParameteriv( target, 0, GL_TEXTURE_GREEN_SIZE, &green );
	glGetTexLevelParameteriv( target, 0, GL_TEXTURE_BLUE_SIZE, &blue );
	glGetTexLevelParameteriv( target, 0, GL_TEXTURE_ALPHA_SIZE, &alpha );

	glGetTexLevelParameteriv( target, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat );
	glGetTexLevelParameteriv( target, 0, GL_TEXTURE_COMPRESSED, &compressed );

	format = (alpha>0) ? GL_RGBA : GL_RGB;
	pixelMemorySize = (red + green + blue + alpha) / 8;

	if (compressed == GL_TRUE )
	{
		glGetTexLevelParameteriv( target, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &imageSize );
		
		isCompressed = true;
	}
	else
	{
		imageSize = width * height * pixelMemorySize;
	}

	const int maxLength = (width > height) ? width : height;
	numberOfLods = (int) floor(log2( (double)maxLength ))-1;
}

void TextureObjectGetData( const GLenum target, GLubyte *localImageBuffer, int &width, int &height, int &internalFormat, int &format, bool &isCompressed, int &pixelMemorySize, int &imageSize, int &numberOfLods )
{
	isCompressed = false;
	imageSize = 0;
	
	// get a compression result
	
	int compressed;
	int red, green, blue, alpha;
	GLubyte *imageData = localImageBuffer;

	glGetTexLevelParameteriv( target, 0, GL_TEXTURE_WIDTH, &width );
	glGetTexLevelParameteriv( target, 0, GL_TEXTURE_HEIGHT, &height );
			
	glGetTexLevelParameteriv( target, 0, GL_TEXTURE_RED_SIZE, &red);
	glGetTexLevelParameteriv( target, 0, GL_TEXTURE_GREEN_SIZE, &green );
	glGetTexLevelParameteriv( target, 0, GL_TEXTURE_BLUE_SIZE, &blue );
	glGetTexLevelParameteriv( target, 0, GL_TEXTURE_ALPHA_SIZE, &alpha );

	glGetTexLevelParameteriv( target, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat );
	glGetTexLevelParameteriv( target, 0, GL_TEXTURE_COMPRESSED, &compressed );

	format = (alpha>0) ? GL_RGBA : GL_RGB;
	pixelMemorySize = (red + green + blue + alpha) / 8;

	if (compressed == GL_TRUE )
	{
		glGetTexLevelParameteriv( target, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &imageSize );
		glGetCompressedTexImage( target, 0, imageData );

		isCompressed = true;
	}
	else
	{
		imageSize = width * height * pixelMemorySize;
		glGetTexImage( target, 0, format, GL_UNSIGNED_BYTE, imageData );
	}

	const int maxLength = (width > height) ? width : height;
	numberOfLods = (int) floor(log2( (double) maxLength ))-1;
}


//////////////////////////////////////////////////////////////////////////////////////////////////

//


FrameBufferInfo		gFrameBufferInfo;

void SaveFrameBuffer(FrameBufferInfo *pInfo)
{
	if (pInfo == nullptr)
		pInfo = &gFrameBufferInfo;

	glGetIntegerv( GL_VIEWPORT, pInfo->gViewport );
	glGetIntegerv( GL_FRAMEBUFFER_BINDING, &pInfo->gLastFramebuffer );  
	if ( pInfo->gLastFramebuffer > 0 )
	{
		glGetIntegerv( GL_MAX_DRAW_BUFFERS, &pInfo->gMaxDrawBuffers ); 
		GLint lActualUse = 0; 
		for ( GLint i = 0; i < pInfo->gMaxDrawBuffers && i < MAX_DRAW_BUFFERS; i++ )
		{ 
			glGetIntegerv( GL_DRAW_BUFFER0+i, (GLint *)&(pInfo->gDrawBuffers[i]) );
			if ( pInfo->gDrawBuffers[i] == GL_NONE )
			{ 
				break; 
			}
			lActualUse ++;
		}
		pInfo->gMaxDrawBuffers = lActualUse;
	}
	else
	{ 
		pInfo->gLastFramebuffer = 0;
	}
}
	
void RestoreFrameBuffer(FrameBufferInfo *pInfo)
{
	if (pInfo == nullptr)
		pInfo = &gFrameBufferInfo;

	if ( pInfo->gLastFramebuffer > 0 )
	{ 
		glBindFramebuffer( GL_FRAMEBUFFER, pInfo->gLastFramebuffer );
		if ( pInfo->gMaxDrawBuffers > 0 )
		{ 
			glDrawBuffers( pInfo->gMaxDrawBuffers, &(pInfo->gDrawBuffers[0]) ); 
		}
		else
		{
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
		}
		glReadBuffer(GL_COLOR_ATTACHMENT0);
	}

	glViewport(pInfo->gViewport[0], pInfo->gViewport[1], pInfo->gViewport[2], pInfo->gViewport[3]);
}


const int mipLevelCount = 1;
const int compose_layerCount = 8;

void BindFrameBufferLayers(const GLuint textureId)
{
	for (int i=0; i<compose_layerCount; ++i)
	{
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, textureId, 0, GLint(i));
	}

	CHECK_GL_ERROR();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ZeroFrameBufferMS(CFrameBufferMS &fb)
{
	memset( &fb, 0, sizeof(CFrameBufferMS) );

	fb.width = 1;
	fb.height = 1;
	fb.samples = 1;
}

void FreeFrameBufferMS(CFrameBufferMS &fb)
{
	if (fb.tColor > 0)
		glDeleteTextures(1, &fb.tColor);
	if (fb.tNormal > 0)
		glDeleteTextures(1, &fb.tNormal);
	if (fb.tDepth > 0)
		glDeleteTextures(1, &fb.tDepth);

	if (fb.rbColor > 0)
		glDeleteRenderbuffers(1, &fb.rbColor);
	
	if (fb.rbNormal > 0)
		glDeleteRenderbuffers(1, &fb.rbNormal);

	if (fb.rbDepth > 0)
		glDeleteRenderbuffers(1, &fb.rbDepth);

	if (fb.fbo > 0)
		glDeleteFramebuffers(1, &fb.fbo);

	memset( &fb, 0, sizeof(CFrameBufferMS) );
}

bool PrepFrameBufferMS(const int width, const int height, const int samples, const int flags, const int normalFormat, CFrameBufferMS &fb)
{
	
	if ( fb.fbo > 0 && fb.width == width && fb.height == height && fb.samples == samples )
	{
		return true;
	}
	
	// prep buffer, we need that

	FreeFrameBufferMS(fb);
	
	fb.flags = flags;
	fb.width = width;
	fb.height = height;
	fb.samples = samples;

	bool useNormal = ((flags & FRAMEBUFFER_MS_NORMAL) > 0);

	if ( (flags & FRAMEBUFFER_MS_TEXTURES) > 0 )
	{
		glGenTextures(1, &fb.tColor);
		glGenTextures(1, &fb.tDepth);

		if (useNormal)
			glGenTextures(1, &fb.tNormal);
	
		if (samples > 1)
		{
			// Create multisample texture
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fb.tColor);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, fb.samples, GL_RGBA, width, height, GL_TRUE);
	
			if (useNormal)
			{
				glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fb.tNormal);
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, fb.samples, normalFormat, width, height, GL_TRUE);
			}

			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fb.tDepth);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, fb.samples, GL_DEPTH_COMPONENT32F_NV, width, height, GL_TRUE);
	
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, fb.tColor);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const void*) 0 );
	
			if (useNormal)
			{
				glBindTexture(GL_TEXTURE_2D, fb.tNormal);
				glTexImage2D(GL_TEXTURE_2D, 0, normalFormat, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const void*) 0 );
			}

			glBindTexture(GL_TEXTURE_2D, fb.tDepth);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F_NV, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, (const void*) 0 );
	
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	// Create and bind the FBO
	
	glGenFramebuffers(1, &fb.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);

	if ( (flags & FRAMEBUFFER_MS_RENDERBUFFERS) > 0 )
	{
		// Create color render buffer
		glGenRenderbuffers(1, &fb.rbColor);
		glBindRenderbuffer(GL_RENDERBUFFER, fb.rbColor);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, fb.rbColor);

		// Create depth render buffer (This is optional)
		glGenRenderbuffers(1, &fb.rbDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, fb.rbDepth);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT32F_NV, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fb.rbDepth);
		//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepthBuffer);
	}

	if ( (flags & FRAMEBUFFER_MS_TEXTURES) > 0 )
	{
		// Bind Texture assuming we have created a texture
		GLenum target = (samples > 1) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
	
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, fb.tColor, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, target, fb.tNormal, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target, fb.tDepth, 0);
	}

	GLenum status = glCheckFramebufferStatus ( GL_FRAMEBUFFER );

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		LOGE ( "framebuffer error!\n" );
		return false;
	}

	return true;
}

//////////////////////////////////////
//

/*
bool BlitFBOToFBO(const GLint FBO, const int width, const int height, const GLint defaultFBO, const int defWidth, const int defHeight, const bool copyColor1)
{
	CHECK_GL_ERROR();

	if (FBO == 0 || defaultFBO == 0)
	{
		LOGE( "> BlitToDefaultFBO where FBO is empty !!\n" );
		return false;
	}
	
	if (width != defWidth || height != defHeight)
	{
		LOGE( "> BlitToDefaultFBO where size is not equal !!\n" );
		return false;
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defaultFBO );

	// DONE: color attachment1 (may be don't need it at all, use MS version instead!)
	if (copyColor1)
	{
		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		GLenum buffers1[1] = { GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(1, &buffers1[0] );

		glBlitFramebuffer(0, 0, width, height, 0, 0, defWidth, defHeight, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		CHECK_GL_ERROR();
	}

	// color attachment0
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	GLenum buffers0[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, &buffers0[0] );

	glBlitFramebuffer(0, 0, width, height, 0, 0, defWidth, defHeight, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	CHECK_GL_ERROR();

	return true;
}

bool BlitFBOToScreen(const GLint defaultFBO, const int defWidth, const int defHeight, const int screenWidth, const int screenHeight)
{
	if (defWidth != screenWidth || defHeight != screenHeight)
		return false;

	glBindFramebuffer(GL_READ_FRAMEBUFFER, defaultFBO );
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0 );

	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glDrawBuffer(GL_BACK);
	
	glBlitFramebuffer(0, 0, defWidth, defHeight, 0, 0, screenWidth, screenHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	CHECK_GL_ERROR();
	return true;
}
*/

#ifdef OGL_UTIL_CUBEMAP

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OGLCubeMap

OGLCubeMap::OGLCubeMap()
{
	cubeId = 0;
}

OGLCubeMap::~OGLCubeMap()
{
	Clear();
}

void OGLCubeMap::Clear()
{
	if (cubeId > 0)
	{
		glDeleteTextures(1, &cubeId);
		cubeId = 0;
	}
}

bool OGLCubeMap::LoadCubeMap(const char *filename)
{
	glActiveTexture(GL_TEXTURE0);

	nv_dds::CDDSImage	cubeImage;

	if (false == cubeImage.load(filename, false))
	{
		//FBTrace( "Failed to load cubeImage\n" );
		return false;
	}

	if (false == cubeImage.is_cubemap())
	{
		//FBTrace( "DDS image is not a cubemap!" );
		return false;
	}

	memorySize = cubeImage.get_size();
	isCompressed = cubeImage.is_compressed();
	numberOfMipLevels = cubeImage.get_num_mipmaps();
	format = cubeImage.get_format();
	dimention = cubeImage.get_width();

	//

	if (cubeId == 0)
	{
		glGenTextures(1, &cubeId);
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeId);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (false == cubeImage.upload_textureCubemap())
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		//FBTrace( "failed to upload a texture cubemap\n" );
		return false;
	}

	return true;
}

void OGLCubeMap::GenerateMipMaps()
{
	if (cubeId > 0)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeId);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		//glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
}

void OGLCubeMap::Bind()
{
	if (cubeId > 0)
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeId);
}

void OGLCubeMap::UnBind()
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

#endif