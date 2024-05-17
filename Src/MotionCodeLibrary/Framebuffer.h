
#ifndef	__FRAME_BUFFER_H__
#define	__FRAME_BUFFER_H__

///////////////////////////////////////////////////////////////////////
//
// FrameBuffer class
//

/*
Sergei <Neill3d> Solokhin 2018
		class is based on http://steps3d.narod.ru library
		Thanks for Alex Boreskov

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#ifdef	_WIN32
	#include	<windows.h>
#endif

#ifdef	MACOSX
	#include	<OpenGL/gl.h>
#else
	#include	<GL\glew.h>
#endif

#define MAX_FRAMEBUFFER_ATTACHMENTS			8

//////////////////////////////////////////////////////////////////////////////
//
class FrameBuffer
{
public:
	enum								// flags for depth and stencil buffers
	{
		eCreateDepthRenderbuffer	= 1 << 0,
		eCreateStencilRenderbuffer	= 1 << 1,
		eCreateDepthStencilRenderbuffer = 1 << 2,

		eCreateColorTexture = 1 << 3,
		eCreateDepthTexture = 1 << 4,

		eCreateCubemap		= 1 << 5,

		eDeleteFramebufferOnCleanup = 1 << 6,

		eSaveViewport = 1 << 7,

		eUnPackAlignment = 1 << 8		// set unpack aligment to 1
	};

	enum								// filter modes
	{
		filterNearest = 0,
		filterLinear  = 1,
		filterMipmap  = 2
	};

	enum EAttachmentType
	{
		eAttachmentTypeDepth=0,
		eAttachmentTypeColor0,
		eAttachmentTypeColor1,
		eAttachmentTypeColor2,
		eAttachmentTypeColor3,
		eAttachmentTypeColor4,
		eAttachmentTypeColor5,
		eAttachmentTypeColor6,
		eAttachmentTypeColor7,
		eAttachmentTypeStencil
	};

public:
	
	//! a constructor
	FrameBuffer(int theWidth, int theHeight, int theFlags = eCreateColorTexture | eDeleteFramebufferOnCleanup, int numberOfColorAttachments=1);
	//! a destructor
	virtual ~FrameBuffer();

	// work with flags
	
	void	SetFlag(const int flag);
	void	RemoveFlag(const int flag);
	bool	IsFlag(const int flag);

	// attach external resources

	void	AttachFrameBufferObject(const GLuint framebuffer, const bool delete_framebuffer=false);
	//! for next functions framebuffer should be already binded !
	bool	AttachTexture2D   ( GLenum textarget, GLenum framebufferTarget, unsigned texId, const EAttachmentType type = eAttachmentTypeColor0, bool deleteOnCleanup=false );
	bool	AttachTexture ( GLenum target, unsigned texId, const EAttachmentType type=eAttachmentTypeColor0, const bool deleteOnCleanup = false );
	bool	AttachTextureLayer ( GLenum target, unsigned texId, unsigned layerId, const EAttachmentType type=eAttachmentTypeColor0, const bool deleteOnCleanup = false );
	//bool	AttachTexture3DSlice ( unsigned texId, int zOffs, int no = 0 );
	
	bool	DetachTexture ( const GLenum target, const EAttachmentType type=eAttachmentTypeColor0 );

	void	Cleanup();

	bool	IsOk     () const;
	bool	Create   ();
	
	// manage framebuffer work
	bool	ReSize(const int w, const int h);

	bool	Bind     () const;
	bool	UnBind   ( bool genMipmaps = false,  GLenum target = GL_TEXTURE_2D ) const;

	// helper functions for making textures

										// create texture for attaching 
	static unsigned	CreateTexture2D ( const int width, const int height, GLenum format, GLenum internalFormat, GLenum type=GL_UNSIGNED_BYTE, GLuint *useId=nullptr,
	                                     GLenum clamp = GL_REPEAT, int filter = filterLinear, bool generateMipMaps = false, bool unpackAlignment = false );
											

	static unsigned	CreateTextureRect ( const int width, const int height, GLenum format = GL_RGBA, GLenum internalFormat = GL_RGBA8,
		GLuint *useId=nullptr);
	
	static unsigned	CreateTextureCube ( const int width=512, const int height=512, GLenum format = GL_RGBA, GLenum internalFormat = GL_RGBA8, GLenum type=GL_UNSIGNED_BYTE,
	                                     GLuint *useId=nullptr, GLenum clamp = GL_CLAMP_TO_EDGE, int filter = filterLinear );
										 
	static unsigned	CreateTexture3D ( const int width, const int height, const int depth, GLenum format = GL_RGBA, GLenum internalFormat = GL_RGBA8, 
	                                    GLuint *useId=nullptr, GLenum clamp = GL_REPEAT, int filter = filterLinear );
										
										// mipmapping support
	void	BuildMipmaps ( GLenum target = GL_TEXTURE_2D ) const
	{
										// be sure we're unbound
		//glBindTexture       ( target, getColorBuffer () );
		glGenerateMipmapEXT ( target );
	}
	
	// methods for taking framebuffer information

	const GLuint GetFrameBuffer() const
	{
		return mFrameBuffer;
	}

	unsigned	GetColorObject ( const int index = 0 ) const
	{
		return mColorAttachments[index].id;
	}
	unsigned	GetDepthObject() const
	{
		return mDepthAttachment.id;
	}

	void SetColorFormat(const int index=0, const GLenum format=GL_RGBA)
	{
		mColorAttachments[index].format = format;
	}
	GLenum		GetColorFormat (const int index=0) const
	{
		return mColorAttachments[index].format;
	}
	GLenum		GetDepthFormat () const
	{
		return mDepthAttachment.format;
	}
	void SetDepthFormat(const GLenum format=GL_DEPTH_COMPONENT)
	{
		mDepthAttachment.format = format;
	}
	GLenum		GetColorInternalFormat(const int index=0) const
	{
		return mColorAttachments[index].internalFormat;
	}
	void SetColorInternalFormat(const int index=0, const GLenum internalFormat=GL_RGBA8)
	{
		mColorAttachments[index].internalFormat = internalFormat;
	}
	void SetColorType(const int index=0, const GLenum type=GL_UNSIGNED_BYTE)
	{
		mColorAttachments[index].type = type;
	}
	void SetFilter(const int index=0, const GLenum filterId = filterLinear)
	{
		mColorAttachments[index].filter = filterId;
	}
	void SetClamp(const int index=0, const GLenum clampId = GL_REPEAT)
	{
		mColorAttachments[index].clamp = clampId;
	}
	GLenum		GetDepthInternalFormat() const
	{
		return mDepthAttachment.internalFormat;
	}
	void SetDepthInternalFormat(const GLenum internalFormat = GL_DEPTH_COMPONENT32F)
	{
		mDepthAttachment.internalFormat=internalFormat;
	}
	void SetDepthType(const GLenum _type = GL_FLOAT)
	{
		mDepthAttachment.type = _type;
	}

	int	GetWidth () const
	{
		return mWidth;
	}
	
	int	GetHeight () const
	{
		return mHeight;
	}

	//
	//

	static	bool	IsSupported         ();
	static	int		GetMaxColorAttachemnts ();
	static	int		GetMaxSize             ();

protected:

	int		mFlags;
	int		mWidth;
	int		mHeight;

	GLuint		mFrameBuffer{ 0 };
	GLint		mSaveViewport[4] { 0 };				// saved viewport setting during bind

	// color attachments information
	int		mNumberOfColorAttachments;

	struct	AttachmentObject
	{
		GLuint		id;
		GLenum		format;
		GLenum		internalFormat;
		GLenum		target;
		GLenum		type;
		GLenum		filter;
		GLenum		clamp;

		bool		deleteOnCleanup;
	};
	
	
	AttachmentObject	mColorAttachments[MAX_FRAMEBUFFER_ATTACHMENTS];
	AttachmentObject	mDepthAttachment;
	AttachmentObject	mStencilAttachment;

	virtual void OnCreate() {}
	virtual void OnCleanup() {}
	virtual void OnResize(const int w, const int h) {}
	virtual void OnBind() {}
	virtual void OnUnBind() {}

private:

	void CleanUpAttachment( AttachmentObject &object );
};


#endif // __FRAME_BUFFER_H__
