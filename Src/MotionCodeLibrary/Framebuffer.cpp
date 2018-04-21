
/*
Sergei <Neill3d> Solokhin 2018
		class is based on http://steps3d.narod.ru library
		Thanks for Alex Boreskov

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include	<assert.h>
#include	<stdlib.h>
#include	<stdio.h>

#include	"FrameBuffer.h"
#include	"checkglerror.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
// FrameBufferBase

FrameBuffer::FrameBuffer( int theWidth, int theHeight, int theFlags, int numberOfColorAttachments )
{
	mWidth         = theWidth;
	mHeight        = theHeight;
	mFlags         = theFlags;
	mFrameBuffer   = 0;
	
	mNumberOfColorAttachments = numberOfColorAttachments;

	// clear color buffers
	for (int i=0; i<MAX_FRAMEBUFFER_ATTACHMENTS; ++i)
	{
		mColorAttachments[i].id = 0;
		mColorAttachments[i].format = GL_RGBA;
		mColorAttachments[i].internalFormat = GL_RGBA8;
		mColorAttachments[i].target = GL_TEXTURE_2D;
		mColorAttachments[i].type = GL_UNSIGNED_BYTE;
		mColorAttachments[i].filter = filterLinear;
		mColorAttachments[i].clamp = GL_REPEAT;

		mColorAttachments[i].deleteOnCleanup = true;
	}

	// default depth
	
	mDepthAttachment.id = 0;
	mDepthAttachment.format = GL_DEPTH_COMPONENT;
	mDepthAttachment.internalFormat = GL_DEPTH_COMPONENT32F;
	mDepthAttachment.target = GL_TEXTURE_2D;
	mDepthAttachment.type = GL_FLOAT;
	mDepthAttachment.filter = filterLinear;
	mDepthAttachment.clamp = GL_REPEAT;
	mDepthAttachment.deleteOnCleanup = true;
	
	mStencilAttachment.id = 0;
	mStencilAttachment.format = GL_LUMINANCE;
	mStencilAttachment.internalFormat = GL_STENCIL_INDEX8;
	mStencilAttachment.target = GL_TEXTURE_2D;
	mStencilAttachment.type = GL_UNSIGNED_BYTE;
	mStencilAttachment.filter = filterLinear;
	mStencilAttachment.clamp = GL_REPEAT;
	mStencilAttachment.deleteOnCleanup = true;
}

FrameBuffer::~FrameBuffer()
{
	Cleanup();
}

void FrameBuffer::SetFlag(const int flag)
{
	mFlags |= flag;
}

void FrameBuffer::RemoveFlag(const int flag)
{
	mFlags &= ~(flag);
}

const bool FrameBuffer::IsFlag(const int flag)
{
	return ( (mFlags & flag) != 0);
}

bool FrameBuffer::IsOk() const
{
	GLint	currentFb;

	glGetIntegerv ( GL_FRAMEBUFFER_BINDING, &currentFb );

	if ( currentFb != (int) mFrameBuffer )
	{
		glBindFramebuffer ( GL_FRAMEBUFFER, mFrameBuffer );
		glReadBuffer         ( GL_COLOR_ATTACHMENT0 );
	}

	int	 status = glCheckFramebufferStatus ( GL_FRAMEBUFFER );

	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		printf( "test framebuffer\n" );
	}

	if ( currentFb != (int) mFrameBuffer )
		glBindFramebuffer ( GL_FRAMEBUFFER, currentFb );

	return 	status == GL_FRAMEBUFFER_COMPLETE;
}


void FrameBuffer::AttachFrameBufferObject(const GLuint framebuffer, const bool delete_framebuffer)
{
	assert(mFrameBuffer == 0);

	mFrameBuffer = framebuffer;

	if (delete_framebuffer)
		mFlags |= eDeleteFramebufferOnCleanup;
	else
		mFlags &= ~(eDeleteFramebufferOnCleanup);
}

bool FrameBuffer::AttachTexture2D(GLenum textarget, GLenum framebufferTarget, unsigned texId, const EAttachmentType type, bool deleteOnCleanup)
{
	if ( mFrameBuffer == 0 )
		return false;

	if ( (textarget != GL_TEXTURE_2D) && (textarget != GL_TEXTURE_RECTANGLE) && (textarget != GL_TEXTURE_2D_ARRAY)
		&& (framebufferTarget < GL_TEXTURE_CUBE_MAP_POSITIVE_X || framebufferTarget > GL_TEXTURE_CUBE_MAP_NEGATIVE_Z) )
	{
		return false;
	}

	// do we need it ?!
	//glBindTexture             ( textarget, texId );

	if (type == eAttachmentTypeDepth)
	{
		CleanUpAttachment(mDepthAttachment);

		mDepthAttachment.id = texId;
		mDepthAttachment.deleteOnCleanup = deleteOnCleanup;
		mDepthAttachment.target = framebufferTarget;

		// attach depth texture
		glFramebufferTexture2D ( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, framebufferTarget, texId, 0 );
	}
	else
	{
		int index = (int) type - 1;
		CleanUpAttachment( mColorAttachments[index] );

		mColorAttachments[index].id = texId;
		mColorAttachments[index].deleteOnCleanup = deleteOnCleanup;
		mColorAttachments[index].target = framebufferTarget;
	
		glFramebufferTexture2D ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, framebufferTarget, texId, 0 );
	}

	//glBindTexture(textarget, 0);

	return true;
}

bool FrameBuffer::AttachTexture(GLenum target, unsigned texId, const EAttachmentType type, const bool deleteOnCleanup)
{
	if (mFrameBuffer == 0)
		return false;

	glBindTexture( target, texId  );

	if (type == eAttachmentTypeDepth)
	{
		CleanUpAttachment(mDepthAttachment);

		mDepthAttachment.id = texId;
		mDepthAttachment.deleteOnCleanup = deleteOnCleanup;
		mDepthAttachment.target = target;

		glFramebufferTexture( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texId, 0 );
	}
	else
	{
		int index = (int) type - 1;
		CleanUpAttachment( mColorAttachments[index] );

		mColorAttachments[index].id = texId;
		mColorAttachments[index].deleteOnCleanup = deleteOnCleanup;
		mColorAttachments[index].target = target;
		glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, texId, 0 );
	}

	glBindTexture( target, 0 );
	
	int	 status = glCheckFramebufferStatus ( GL_FRAMEBUFFER );

	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		printf( "test framebuffer\n" );
	}

	return true;
}

bool FrameBuffer::AttachTextureLayer(GLenum target, unsigned texId, unsigned layerId, const EAttachmentType type, const bool deleteOnCleanup)
{
	if (mFrameBuffer == 0)
		return false;

	glBindTexture( target, texId  );

	if (type == eAttachmentTypeDepth)
	{
		CleanUpAttachment(mDepthAttachment);

		mDepthAttachment.id = texId;
		mDepthAttachment.deleteOnCleanup = deleteOnCleanup;
		mDepthAttachment.target = target;

		glFramebufferTextureLayer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texId, 0, layerId );
	}
	else
	{
		int index = (int) type - 1;
		CleanUpAttachment( mColorAttachments[index] );

		mColorAttachments[index].id = texId;
		mColorAttachments[index].deleteOnCleanup = deleteOnCleanup;
		mColorAttachments[index].target = target;
		glFramebufferTextureLayer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, texId, 0, layerId );
	}

	glBindTexture( target, 0 );
	
	int	 status = glCheckFramebufferStatus ( GL_FRAMEBUFFER );

	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		printf( "test framebuffer\n" );
	}

	return true;
}

/*
bool FrameBuffer::AttachTexture3DSlice ( unsigned texId, int zOffs, int no )
{
	if ( mFrameBuffer == 0 )
		return false;
	
	glBindTexture             ( GL_TEXTURE_3D, colorBuffer [no] = texId );
	glFramebufferTexture3D ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + no, GL_TEXTURE_3D, texId, 0, zOffs );
	
	return true;
}
*/
bool FrameBuffer::DetachTexture ( const GLenum target, const EAttachmentType type )
{
	if (target == GL_TEXTURE_CUBE_MAP || target == GL_TEXTURE_2D_ARRAY)
		return AttachTexture(target, 0, type, false);
	else
		return AttachTexture2D(target, target, 0, type, false);
}

bool FrameBuffer::Create()
{
	if ( mWidth <= 0 || mHeight <= 0 )
		return false;

	CHECK_GL_ERROR();
	
	if (mFrameBuffer == 0)
		glGenFramebuffers ( 1, &mFrameBuffer );
	
	glBindFramebuffer ( GL_FRAMEBUFFER, mFrameBuffer );

	//
	//
	if ( mFlags & eCreateColorTexture )
	{
		GLuint id;

		for (int i=0; i<mNumberOfColorAttachments; ++i)
		{
			id = 0;
			EAttachmentType type = (EAttachmentType) ((int)eAttachmentTypeColor0+i);

			if (mFlags & eCreateCubemap)
			{
				id = CreateTextureCube(mWidth, mHeight, mColorAttachments[i].format, mColorAttachments[i].internalFormat, mColorAttachments[i].type, &mColorAttachments[i].id);
				mColorAttachments[i].deleteOnCleanup = false;
				AttachTexture( GL_TEXTURE_CUBE_MAP, id, type, true );
			}
			else
			{
				id = CreateTexture2D( mWidth, mHeight, mColorAttachments[i].format, mColorAttachments[i].internalFormat, mColorAttachments[i].type, 
					&mColorAttachments[i].id, mColorAttachments[i].clamp, mColorAttachments[i].filter, false, IsFlag(eUnPackAlignment) );
				mColorAttachments[i].deleteOnCleanup = false;
				AttachTexture2D( mColorAttachments[i].target, mColorAttachments[i].target, id, type, true );
			}
			
		}
	}

	CHECK_GL_ERROR();

	//
	// choose between texture and renderbuffer

	if (mFlags & eCreateDepthTexture)
	{
		GLuint id=0;

		if (mFlags & eCreateCubemap)
		{
			id = CreateTextureCube(mWidth, mHeight, mDepthAttachment.format, mDepthAttachment.internalFormat, mDepthAttachment.type, &mDepthAttachment.id);
			mDepthAttachment.deleteOnCleanup = false;
			AttachTexture( GL_TEXTURE_CUBE_MAP, id, eAttachmentTypeDepth, true );
		}
		else
		{
			id = CreateTexture2D(mWidth, mHeight, mDepthAttachment.format, mDepthAttachment.internalFormat, mDepthAttachment.type, &mDepthAttachment.id);
			mDepthAttachment.deleteOnCleanup = false;
			AttachTexture2D( mDepthAttachment.target, mDepthAttachment.target, id, eAttachmentTypeDepth, true );
		}

		// if we need additional stencil for that framebuffer
		if ( mFlags & eCreateStencilRenderbuffer )
		{
			CleanUpAttachment(mStencilAttachment);

			glGenRenderbuffers        ( 1, &mStencilAttachment.id );
			glBindRenderbuffer       ( GL_RENDERBUFFER, mStencilAttachment.id );
			glRenderbufferStorage    ( GL_RENDERBUFFER, mStencilAttachment.internalFormat, mWidth, mHeight );
			glFramebufferRenderbuffer ( GL_FRAMEBUFFER,  GL_STENCIL_ATTACHMENT,
											GL_RENDERBUFFER, mStencilAttachment.id );
		}
	}
	else
	{
		if ( mFlags & eCreateDepthStencilRenderbuffer )
		{
			CleanUpAttachment(mDepthAttachment);

			glGenRenderbuffers(1, &mDepthAttachment.id);
			glBindRenderbuffer(GL_RENDERBUFFER, mDepthAttachment.id);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mWidth, mHeight);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			mDepthAttachment.internalFormat = GL_DEPTH24_STENCIL8;
			mDepthAttachment.target = GL_RENDERBUFFER;
			mDepthAttachment.deleteOnCleanup = true;

			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 
				GL_RENDERBUFFER, mDepthAttachment.id);
		}
		else if (mFlags & eCreateDepthRenderbuffer || mFlags & eCreateStencilRenderbuffer)
		{
			if ( mFlags & eCreateDepthRenderbuffer )
			{
				CleanUpAttachment(mDepthAttachment);

				glGenRenderbuffers        ( 1, &mDepthAttachment.id );
				glBindRenderbuffer        ( GL_RENDERBUFFER, mDepthAttachment.id );
				glRenderbufferStorage     ( GL_RENDERBUFFER, mDepthAttachment.internalFormat, mWidth, mHeight );
				glFramebufferRenderbuffer ( GL_FRAMEBUFFER,  GL_DEPTH_ATTACHMENT,
											   GL_RENDERBUFFER, mDepthAttachment.id );
			}

			if ( mFlags & eCreateStencilRenderbuffer )
			{
				CleanUpAttachment(mStencilAttachment);

				glGenRenderbuffers        ( 1, &mStencilAttachment.id );
				glBindRenderbuffer       ( GL_RENDERBUFFER, mStencilAttachment.id );
				glRenderbufferStorage    ( GL_RENDERBUFFER, mStencilAttachment.internalFormat, mWidth, mHeight );
				glFramebufferRenderbuffer ( GL_FRAMEBUFFER,  GL_STENCIL_ATTACHMENT,
											   GL_RENDERBUFFER, mStencilAttachment.id );
			}
		}
	}

	CHECK_GL_ERROR();

	//
	OnCreate();

	GLenum status = glCheckFramebufferStatus ( GL_FRAMEBUFFER );

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	return status == GL_FRAMEBUFFER_COMPLETE;
}

bool FrameBuffer::ReSize(const int w, const int h)
{
	if (mFrameBuffer == 0 || mWidth != w || mHeight != h)
	{
		mWidth = w;
		mHeight = h;

		Cleanup();
		Create();

		return true;
	}

	return false;
}

void FrameBuffer::Cleanup()
{
	OnCleanup();

	if ( (mFrameBuffer != 0) && (mFlags & eDeleteFramebufferOnCleanup) )
	{
		glDeleteFramebuffers(1, &mFrameBuffer);
		mFrameBuffer = 0;
	}

	for (int i=0; i<MAX_FRAMEBUFFER_ATTACHMENTS; ++i)
		CleanUpAttachment( mColorAttachments[i] );

	CleanUpAttachment( mDepthAttachment );
	CleanUpAttachment( mStencilAttachment );
}

void FrameBuffer::CleanUpAttachment( AttachmentObject &object )
{
	if (object.id > 0 && object.deleteOnCleanup)
	{
		if (object.target == GL_RENDERBUFFER)
			glDeleteRenderbuffers(1, &object.id);
		else
			glDeleteTextures(1, &object.id);

		object.id = 0;
	}
}


bool FrameBuffer::Bind () const
{
	if ( mFrameBuffer == 0 )
	{
		return false;
	}

	glFlush ();
	
	// save current viewport
	if (mFlags & eSaveViewport)
		glGetIntegerv( GL_VIEWPORT, (GLint*)&mSaveViewport[0] );
	
	glBindFramebuffer ( GL_FRAMEBUFFER, mFrameBuffer );
	glReadBuffer         ( GL_COLOR_ATTACHMENT0 );

	if (mNumberOfColorAttachments == 0)
	{
		glDrawBuffer(GL_NONE);
	}
	if (mNumberOfColorAttachments == 1)
	{
		glDrawBuffer         ( GL_COLOR_ATTACHMENT0 );
	}
	else if (mNumberOfColorAttachments == 2)
	{
		GLenum buffers [] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers( 2, buffers );
	}
	//glViewport           ( 0, 0, getWidth (), getHeight () );

	return true;
}


bool FrameBuffer::UnBind ( bool genMipmaps, GLenum target ) const
{
	if ( mFrameBuffer == 0 )
		return false;

	
	glFlush ();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	if (genMipmaps)
	{
		glBindTexture(mColorAttachments[0].target, mColorAttachments[0].id);
		glGenerateMipmap(mColorAttachments[0].target);
		glBindTexture(mColorAttachments[0].target, 0);
	}
	
	if (mFlags & eSaveViewport)
		glViewport( mSaveViewport[0], mSaveViewport[1], mSaveViewport[2], mSaveViewport[3] );

	return true;
}


unsigned FrameBuffer::CreateTexture2D ( const int width, const int height, GLenum format, 
	GLenum internalFormat, GLenum type, GLuint *useId, GLenum clamp, int filter, bool generateMipMaps, bool unpackAlignment )
{
	GLuint	tex = (useId) ? *useId : 0;

	if (tex == 0)
		glGenTextures   ( 1, &tex );

    glBindTexture   ( GL_TEXTURE_2D, tex );
	if (true == unpackAlignment)
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);                         // set 1-byte alignment
	}
    glTexImage2D    ( GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL );

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

	if (generateMipMaps)
	{
		glGenerateMipmap(GL_TEXTURE_2D); // /!\ Allocate the mipmaps
	}


	glBindTexture(GL_TEXTURE_2D, 0 );

    return tex;
}

unsigned FrameBuffer::CreateTextureCube ( const int width, const int height, GLenum format, GLenum internalFormat, GLenum type, GLuint *useId, GLenum clamp, int filter )
{
	if ( width != height )
		return 0;
		
	GLuint	tex = (useId) ? *useId : 0;

	if (tex == 0)
		glGenTextures   ( 1, &tex );

    glBindTexture   ( GL_TEXTURE_CUBE_MAP, tex );
    glPixelStorei   ( GL_UNPACK_ALIGNMENT, 1 );                         // set 1-byte alignment
    glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, clamp );
    glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, clamp );
    glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, clamp );
	
    for ( int side = 0; side < 6; side++ )
        glTexImage2D ( GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, 0, internalFormat,
                       width, height, 0, format, type, NULL );
	
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
	
	glBindTexture   ( GL_TEXTURE_CUBE_MAP, 0 );

    return tex;
}

unsigned FrameBuffer::CreateTexture3D ( const int width, const int height, const int depth, GLenum format, GLenum internalFormat, GLuint *useId, GLenum clamp, int filter )
{
	if ( width != height )
		return 0;
		
	GLuint	tex = (useId) ? *useId : 0;

	if (tex == 0)
		glGenTextures   ( 1, &tex );

    glBindTexture   ( GL_TEXTURE_3D, tex );
    glPixelStorei   ( GL_UNPACK_ALIGNMENT, 1 );                         // set 1-byte alignment
    glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, clamp );
    glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, clamp );
    glTexParameteri ( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, clamp );
	
    glTexImage3D ( GL_TEXTURE_3D, 0, internalFormat, width, height, depth, 0, format, GL_UNSIGNED_BYTE, NULL );
	
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

unsigned FrameBuffer::CreateTextureRect ( const int width, const int height, GLenum format, GLenum internalFormat, GLuint *useId )
{
	GLuint	tex = (useId) ? *useId : 0;

	if (tex == 0)
		glGenTextures   ( 1, &tex );

    glPixelStorei   ( GL_UNPACK_ALIGNMENT, 1 );                         // set 1-byte alignment
    glBindTexture   ( GL_TEXTURE_RECTANGLE, tex );
    glTexImage2D    ( GL_TEXTURE_RECTANGLE, 0, internalFormat, width, height, 0,
                      format, GL_UNSIGNED_BYTE, NULL );

	glTexParameteri ( GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST   );
	glTexParameteri ( GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST   );

    glTexParameteri ( GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri ( GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP );

    return tex;
}

bool FrameBuffer::IsSupported ()
{
	// uses extension - GL_NV_depth_buffer_float

	//return (extgl_ExtensionSupported( "EXT_framebuffer_object" ) > 0);
//	return isExtensionSupported ( "EXT_framebuffer_object" );
	return true;
}

int FrameBuffer::GetMaxColorAttachemnts ()
{
    GLint n;

    glGetIntegerv ( GL_MAX_COLOR_ATTACHMENTS, &n );

	return n;
}

int FrameBuffer::GetMaxSize ()
{
    GLint sz;

    glGetIntegerv ( GL_MAX_RENDERBUFFER_SIZE, &sz );

	return sz;
}