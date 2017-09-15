//
// C++ wrapper for framebuffer object
//
// Author: Alex V. Boreskoff <alexboreskoff@mtu-net.ru>, <steps3d@narod.ru>
//

#include	"FrameBuffer.h"
#include	<stdlib.h>
#include	<stdio.h>

FrameBuffer :: FrameBuffer  ( int theWidth, int theHeight, int theFlags, int colortextures )
{
	width         = theWidth;
	height        = theHeight;
	flags         = theFlags;
	frameBuffer   = 0;
	depthBuffer   = 0;
	stencilBuffer = 0;
	
	internalFormat = GL_RGBA8;
	numberOfColorTextures = colortextures;

	for ( int i = 0; i < 8; i++ )
		colorBuffer [i] = 0;
}

FrameBuffer :: ~FrameBuffer ()
{
	cleanup();
}

bool	FrameBuffer :: create ()
{
	if ( width <= 0 || height <= 0 )
		return false;

	glGenFramebuffers ( 1, &frameBuffer );
	glBindFramebuffer ( GL_FRAMEBUFFER, frameBuffer );

	int	depthFormat   = 0;
	int stencilFormat = 0;

	if ( flags & depth16 )
		depthFormat = GL_DEPTH_COMPONENT16_ARB;
	else
	if ( flags & depth24 )
		depthFormat = GL_DEPTH_COMPONENT24_ARB;
	else
	if ( flags & depth32 )
		depthFormat = GL_DEPTH_COMPONENT32_ARB;

	if ( flags & stencil1 )
		stencilFormat = GL_STENCIL_INDEX1_EXT;
	else
	if ( flags & stencil4 )
		stencilFormat = GL_STENCIL_INDEX4_EXT;
	else
	if ( flags & stencil8 )
		stencilFormat = GL_STENCIL_INDEX8_EXT;
	else
	if ( flags & stencil16 )
		stencilFormat = GL_STENCIL_INDEX16_EXT;


	if ( depthFormat != 0 )
	{
		glGenRenderbuffers        ( 1, &depthBuffer );
		glBindRenderbuffer        ( GL_RENDERBUFFER, depthBuffer );
		glRenderbufferStorage     ( GL_RENDERBUFFER, depthFormat, width, height );
		glFramebufferRenderbuffer ( GL_FRAMEBUFFER,  GL_DEPTH_ATTACHMENT,
		                               GL_RENDERBUFFER, depthBuffer );
	}

	if ( stencilFormat != 0 )
	{
		glGenRenderbuffers        ( 1, &stencilBuffer );
		glBindRenderbuffer        ( GL_RENDERBUFFER, stencilBuffer );
		glRenderbufferStorage     ( GL_RENDERBUFFER, stencilFormat, width, height );
		glFramebufferRenderbuffer ( GL_FRAMEBUFFER,  GL_STENCIL_ATTACHMENT,
		                               GL_RENDERBUFFER, stencilBuffer );
	}

	GLenum status = glCheckFramebufferStatus ( GL_FRAMEBUFFER );
	
	for (int i=0; i<numberOfColorTextures; ++i)
	{
		GLuint id = createColorTexture( nullptr, getWidth(), getHeight(), GL_RGBA, internalFormat );
		attachColorTexture( GL_TEXTURE_2D, id, i );

		//GLuint id = createColorRectTexture( GL_RGBA, internalFormat );
		//attachColorTexture( GL_TEXTURE_RECTANGLE_ARB, id, i );
	}

	glBindFramebuffer ( GL_FRAMEBUFFER, 0 );

	return status == GL_FRAMEBUFFER_COMPLETE;
}

void FrameBuffer::cleanup()
{
	for (int i=0; i<numberOfColorTextures; ++i)
		if (colorBuffer[i] != 0)
		{
			glDeleteTextures(1, &colorBuffer[i]);
			colorBuffer[i] = 0;
		}

	if ( depthBuffer != 0 )
	{
		glDeleteRenderbuffers ( 1, &depthBuffer );
		depthBuffer = 0;
	}

	if ( stencilBuffer != 0 )
	{
		glDeleteRenderbuffers ( 1, &stencilBuffer );
		stencilBuffer = 0;
	}

	if ( frameBuffer != 0 )
	{
		glDeleteFramebuffers ( 1, &frameBuffer );
		frameBuffer = 0;
	}
}

bool FrameBuffer::resize(const int w, const int h)
{
	if (width != w || height != h)
	{
		cleanup();

		width = w;
		height = h;

		create();

		return true;
	}

	return false;
}

bool	FrameBuffer :: isOk () const
{
	GLint	currentFb;

	glGetIntegerv ( GL_FRAMEBUFFER_BINDING, &currentFb );

	if ( currentFb != frameBuffer )
	{
		glBindFramebuffer ( GL_FRAMEBUFFER, frameBuffer );
		glReadBuffer         ( GL_COLOR_ATTACHMENT0 );
	}

	int	 status = glCheckFramebufferStatus ( GL_FRAMEBUFFER );

	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		printf( "test framebuffer\n" );
	}

	if ( currentFb != frameBuffer )
		glBindFramebuffer ( GL_FRAMEBUFFER, currentFb );

	return 	status == GL_FRAMEBUFFER_COMPLETE;
}

bool	FrameBuffer :: bind ()
{
	if ( frameBuffer == 0 )
		return false;

	glFlush ();
								// save current viewport
	//glGetIntegerv        ( GL_VIEWPORT, saveViewport );
	//printf ( "viewport %d, %d, %d, %d\n", saveViewport[0], saveViewport[1], saveViewport[2], saveViewport[3] );
	glBindFramebuffer ( GL_FRAMEBUFFER, frameBuffer );
	glReadBuffer         ( GL_COLOR_ATTACHMENT0 );
	glDrawBuffer         ( GL_COLOR_ATTACHMENT0 );
	//glViewport           ( 0, 0, getWidth (), getHeight () );

	return true;
}

bool	FrameBuffer :: bindFace ( int face )
{
//	glBindTexture             ( GL_TEXTURE_CUBE_MAP_ARB, colorBuffer [0]  );
	glFramebufferTexture2D ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, colorBuffer [0], 0 );

	return bind ();
}

bool	FrameBuffer :: unbind ( bool genMipmaps, GLenum target )
{
	if ( frameBuffer == 0 )
		return false;

	glFlush ();

	if ( genMipmaps )
		glGenerateMipmap ( target );

	glBindFramebuffer ( GL_FRAMEBUFFER, 0 );
	//glViewport           ( saveViewport [0], saveViewport [1], saveViewport [2], saveViewport [3] );

	return true;
}

bool	FrameBuffer :: attachColorTexture ( GLenum target, unsigned texId, int no )
{
	if ( frameBuffer == 0 )
		return false;

	if ( target != GL_TEXTURE_2D && target != GL_TEXTURE_RECTANGLE && (target < GL_TEXTURE_CUBE_MAP_POSITIVE_X || target > GL_TEXTURE_CUBE_MAP_NEGATIVE_Z) )
		return false;

	colorBuffer [no] = texId;
	//glBindTexture             ( target, colorBuffer [no] = texId );
	glFramebufferTexture2D ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + no, target, texId, 0 );

	return true;
}

bool	FrameBuffer :: attach3DTextureSlice ( unsigned texId, int zOffs, int no )
{
	if ( frameBuffer == 0 )
		return false;

	glBindTexture             ( GL_TEXTURE_3D, colorBuffer [no] = texId );
	glFramebufferTexture3D ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + no, GL_TEXTURE_3D, texId, 0, zOffs );

	return true;
}

bool	FrameBuffer :: attachDepthTexture ( GLenum target, unsigned texId )
{
	if ( frameBuffer == 0 )
		return false;

	depthBuffer = texId;
	glFramebufferTexture2D ( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target, texId, 0 );

	return true;
}

unsigned	FrameBuffer :: createColorTexture ( GLuint *ptex, int width, int height, GLenum format, GLenum internalFormat, GLenum type, GLenum clamp, int filter )
{
	GLuint	tex;

	if (nullptr != ptex)
		tex = *ptex;
	else
		glGenTextures   ( 1, &tex );

    glBindTexture   ( GL_TEXTURE_2D, tex );
    glPixelStorei   ( GL_UNPACK_ALIGNMENT, 1 );                         // set 1-byte alignment
    glTexImage2D    ( GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
                      format, type, NULL );

	if ( filter == filterNearest )
	{
		glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	}
	else
	if ( filter == filterLinear )
	{
		glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	}
	else
	if ( filter == filterMipmap )
	{
		glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	}
	
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp );

    return tex;
}

unsigned	FrameBuffer :: createCubemapTexture ( GLuint *ptex, int width, int height, GLenum format, GLenum internalFormat, GLenum clamp, int filter )
{
	if ( width != height )
		return 0;
		
	GLuint	tex;

	if (nullptr != ptex)
		tex = *ptex;
	else
		glGenTextures   ( 1, &tex );

    glBindTexture   ( GL_TEXTURE_CUBE_MAP, tex );
    glPixelStorei   ( GL_UNPACK_ALIGNMENT, 1 );                         // set 1-byte alignment
    glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, clamp );
    glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, clamp );
    glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, clamp );
	
    for ( int side = 0; side < 6; side++ )
        glTexImage2D ( GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, 0, internalFormat,
                       width, height, 0, format, GL_UNSIGNED_BYTE, NULL );
	
	if ( filter == filterNearest )
	{
		glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	}
	else
	if ( filter == filterLinear )
	{
		glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	}
	else
	if ( filter == filterMipmap )
	{
		glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	}
	
    return tex;
}

unsigned	FrameBuffer :: create3DTexture ( int depth, GLenum format, GLenum internalFormat, GLenum clamp, int filter )
{
	if ( getWidth () != getHeight () )
		return 0;
		
	GLuint	tex;

	glGenTextures   ( 1, &tex );
    glBindTexture   ( GL_TEXTURE_3D, tex );
    glPixelStorei   ( GL_UNPACK_ALIGNMENT, 1 );                         // set 1-byte alignment
    glTexParameteri ( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, clamp );
    glTexParameteri ( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, clamp );
    glTexParameteri ( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, clamp );
	
//    glTexImage3DEXT ( GL_TEXTURE_3D, 0, internalFormat, getWidth (), getWidth (), depth, 0, format, GL_UNSIGNED_BYTE, NULL );
	
	if ( filter == filterNearest )
	{
		glTexParameteri ( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri ( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	}
	else
	if ( filter == filterLinear )
	{
		glTexParameteri ( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri ( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	}
	else
	if ( filter == filterMipmap )
	{
		glTexParameteri ( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri ( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	}
	
    return tex;
}

unsigned	FrameBuffer :: createColorRectTexture ( GLenum format, GLenum internalFormat )
{
	GLuint	tex;

	glGenTextures   ( 1, &tex );
    glPixelStorei   ( GL_UNPACK_ALIGNMENT, 1 );                         // set 1-byte alignment
    glBindTexture   ( GL_TEXTURE_RECTANGLE_ARB, tex );
    glTexImage2D    ( GL_TEXTURE_RECTANGLE_ARB, 0, internalFormat, getWidth (), getHeight (), 0,
                      format, GL_UNSIGNED_BYTE, NULL );

	glTexParameteri ( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST   );
	glTexParameteri ( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST   );

    glTexParameteri ( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri ( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP );

    return tex;
}

bool	FrameBuffer :: isSupported ()
{
	return (glewIsExtensionSupported( "GL_EXT_framebuffer_object" ) > 0);
//	return isExtensionSupported ( "EXT_framebuffer_object" );
}

int		FrameBuffer :: maxColorAttachemnts ()
{
    GLint n;

    glGetIntegerv ( GL_MAX_COLOR_ATTACHMENTS, &n );

	return n;
}

int		FrameBuffer :: maxSize ()
{
    GLint sz;

    glGetIntegerv ( GL_MAX_RENDERBUFFER_SIZE, &sz );

	return sz;
}
