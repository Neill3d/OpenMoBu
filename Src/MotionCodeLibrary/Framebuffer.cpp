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

	glGenFramebuffersEXT ( 1, &frameBuffer );
	glBindFramebufferEXT ( GL_FRAMEBUFFER_EXT, frameBuffer );

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
		glGenRenderbuffersEXT        ( 1, &depthBuffer );
		glBindRenderbufferEXT        ( GL_RENDERBUFFER_EXT, depthBuffer );
		glRenderbufferStorageEXT     ( GL_RENDERBUFFER_EXT, depthFormat, width, height );
		glFramebufferRenderbufferEXT ( GL_FRAMEBUFFER_EXT,  GL_DEPTH_ATTACHMENT_EXT,
		                               GL_RENDERBUFFER_EXT, depthBuffer );
	}

	if ( stencilFormat != 0 )
	{
		glGenRenderbuffersEXT        ( 1, &stencilBuffer );
		glBindRenderbufferEXT        ( GL_RENDERBUFFER_EXT, stencilBuffer );
		glRenderbufferStorageEXT     ( GL_RENDERBUFFER_EXT, stencilFormat, width, height );
		glFramebufferRenderbufferEXT ( GL_FRAMEBUFFER_EXT,  GL_STENCIL_ATTACHMENT_EXT,
		                               GL_RENDERBUFFER_EXT, stencilBuffer );
	}

	GLenum status = glCheckFramebufferStatusEXT ( GL_FRAMEBUFFER_EXT );
	
	for (int i=0; i<numberOfColorTextures; ++i)
	{
		GLuint id = createColorTexture( nullptr, getWidth(), getHeight(), GL_RGBA, internalFormat );
		attachColorTexture( GL_TEXTURE_2D, id, i );

		//GLuint id = createColorRectTexture( GL_RGBA, internalFormat );
		//attachColorTexture( GL_TEXTURE_RECTANGLE_ARB, id, i );
	}

	glBindFramebufferEXT ( GL_FRAMEBUFFER_EXT, 0 );

	return status == GL_FRAMEBUFFER_COMPLETE_EXT;
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
		glDeleteRenderbuffersEXT ( 1, &depthBuffer );
		depthBuffer = 0;
	}

	if ( stencilBuffer != 0 )
	{
		glDeleteRenderbuffersEXT ( 1, &stencilBuffer );
		stencilBuffer = 0;
	}

	if ( frameBuffer != 0 )
	{
		glDeleteFramebuffersEXT ( 1, &frameBuffer );
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

	glGetIntegerv ( GL_FRAMEBUFFER_BINDING_EXT, &currentFb );

	if ( currentFb != frameBuffer )
	{
		glBindFramebufferEXT ( GL_FRAMEBUFFER_EXT, frameBuffer );
		glReadBuffer         ( GL_COLOR_ATTACHMENT0_EXT );
	}

	int	 status = glCheckFramebufferStatusEXT ( GL_FRAMEBUFFER_EXT );

	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
	{
		printf( "test framebuffer\n" );
	}

	if ( currentFb != frameBuffer )
		glBindFramebufferEXT ( GL_FRAMEBUFFER_EXT, currentFb );

	return 	status == GL_FRAMEBUFFER_COMPLETE_EXT;
}

bool	FrameBuffer :: bind ()
{
	if ( frameBuffer == 0 )
		return false;

	glFlush ();
								// save current viewport
	//glGetIntegerv        ( GL_VIEWPORT, saveViewport );
	//printf ( "viewport %d, %d, %d, %d\n", saveViewport[0], saveViewport[1], saveViewport[2], saveViewport[3] );
	glBindFramebufferEXT ( GL_FRAMEBUFFER_EXT, frameBuffer );
	glReadBuffer         ( GL_COLOR_ATTACHMENT0_EXT );
	glDrawBuffer         ( GL_COLOR_ATTACHMENT0_EXT );
	//glViewport           ( 0, 0, getWidth (), getHeight () );

	return true;
}

bool	FrameBuffer :: bindFace ( int face )
{
//	glBindTexture             ( GL_TEXTURE_CUBE_MAP_ARB, colorBuffer [0]  );
	glFramebufferTexture2DEXT ( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + face, colorBuffer [0], 0 );

	return bind ();
}

bool	FrameBuffer :: unbind ( bool genMipmaps, GLenum target )
{
	if ( frameBuffer == 0 )
		return false;

	glFlush ();

	if ( genMipmaps )
		glGenerateMipmapEXT ( target );

	glBindFramebufferEXT ( GL_FRAMEBUFFER_EXT, 0 );
	//glViewport           ( saveViewport [0], saveViewport [1], saveViewport [2], saveViewport [3] );

	return true;
}

bool	FrameBuffer :: attachColorTexture ( GLenum target, unsigned texId, int no )
{
	if ( frameBuffer == 0 )
		return false;

	if ( target != GL_TEXTURE_2D && target != GL_TEXTURE_RECTANGLE_ARB && (target < GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB || target > GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB) )
		return false;

	glBindTexture             ( target, colorBuffer [no] = texId );
	glFramebufferTexture2DEXT ( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + no, target, texId, 0 );

	return true;
}

bool	FrameBuffer :: attach3DTextureSlice ( unsigned texId, int zOffs, int no )
{
	if ( frameBuffer == 0 )
		return false;

	glBindTexture             ( GL_TEXTURE_3D, colorBuffer [no] = texId );
	glFramebufferTexture3DEXT ( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + no, GL_TEXTURE_3D, texId, 0, zOffs );

	return true;
}

bool	FrameBuffer :: attachDepthTexture ( GLenum target, unsigned texId )
{
	if ( frameBuffer == 0 )
		return false;

	glFramebufferTexture2DEXT ( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, target, depthBuffer = texId, 0 );

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

    glBindTexture   ( GL_TEXTURE_CUBE_MAP_ARB, tex );
    glPixelStorei   ( GL_UNPACK_ALIGNMENT, 1 );                         // set 1-byte alignment
    glTexParameteri ( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, clamp );
    glTexParameteri ( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, clamp );
    glTexParameteri ( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, clamp );
	
    for ( int side = 0; side < 6; side++ )
        glTexImage2D ( GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + side, 0, internalFormat,
                       width, height, 0, format, GL_UNSIGNED_BYTE, NULL );
	
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

    glGetIntegerv ( GL_MAX_COLOR_ATTACHMENTS_EXT, &n );

	return n;
}

int		FrameBuffer :: maxSize ()
{
    GLint sz;

    glGetIntegerv ( GL_MAX_RENDERBUFFER_SIZE_EXT, &sz );

	return sz;
}
