//
// C++ wrapper for framebuffer object
//
// Author: Alex V. Boreskoff <alexboreskoff@mtu-net.ru>, <steps3d@narod.ru>
//

#ifndef	__FRAME_BUFFER__
#define	__FRAME_BUFFER__

#ifdef	_WIN32
	#include	<windows.h>
#endif

#ifdef	MACOSX
	#include	<OpenGL/gl.h>
#else
#include <GL\glew.h>
#endif

class	FrameBuffer
{
	int		flags;
	int		width;
	int		height;
	GLuint	frameBuffer;					// id of framebuffer object
	GLuint	colorBuffer [8];				// texture id or buffer id
	GLuint	depthBuffer;
	GLuint	stencilBuffer;					// 
	GLint	saveViewport [4];				// saved viewport setting during bind
	
	
	int		numberOfColorTextures;			// automatic bind color textures to framebuffer
	GLenum	internalFormat;

public:
	FrameBuffer  ( int theWidth, int theHeight, int theFlags = 0, int colortextures = 1 );
	~FrameBuffer ();
	
	int	getWidth () const
	{
		return width;
	}
	
	int	getHeight () const
	{
		return height;
	}
	
	bool	hasStencil () const
	{
		return stencilBuffer != 0;
	}
	
	bool	hasDepth () const
	{
		return depthBuffer != 0;
	}
	
	unsigned	getColorBuffer ( int no = 0 ) const
	{
		return colorBuffer [no];
	}

	unsigned	getDepthBuffer () const
	{
		return depthBuffer;
	}

	void		setInternalFormat( GLenum format )
	{
		internalFormat = format;
	}

	bool	resize(const int w, const int h);
	void	cleanup();

	bool	isOk     () const;
	bool	create   ();
	bool	bind     ();
	bool	bindFace ( int no );
	bool	unbind   ( bool genMipmaps = false,  GLenum target = GL_TEXTURE_2D );
	
	bool	attachColorTexture   ( GLenum target, unsigned texId, int no = 0 );
	bool	attachDepthTexture   ( GLenum target, unsigned texId );
	bool	attach3DTextureSlice ( unsigned texId, int zOffs, int no = 0 );
	
	bool	detachColorTexture ( GLenum target )
	{
		return attachColorTexture ( target, 0 );
	}
	
	bool	detachDepthTexture ( GLenum target )
	{
		return attachDepthTexture ( target, 0 );
	}
	
										// create texture for attaching 
	static unsigned	createColorTexture     ( GLuint *ptex, int width, int height, GLenum format = GL_RGBA, GLenum internalFormat = GL_RGBA8, 
	                                     GLenum type = GL_UNSIGNED_BYTE, GLenum clamp = GL_REPEAT, int filter = filterLinear );
										 
	unsigned	createColorRectTexture ( GLenum format = GL_RGBA, GLenum internalFormat = GL_RGBA8 );
	
	static unsigned	createCubemapTexture   ( GLuint *ptex, int width, int height, GLenum format = GL_RGBA, GLenum internalFormat = GL_RGBA8, 
	                                     GLenum clamp = GL_CLAMP_TO_EDGE, int filter = filterLinear );
										 
	unsigned	create3DTexture       ( int depth, GLenum format = GL_RGBA, GLenum internalFormat = GL_RGBA8, 
	                                    GLenum clamp = GL_REPEAT, int filter = filterLinear );
										
										// mipmapping support
	void	buildMipmaps ( GLenum target = GL_TEXTURE_2D ) const
	{
										// be sure we're unbound
		glBindTexture       ( target, getColorBuffer () );
		glGenerateMipmapEXT ( target );
	}
	
	enum								// flags for depth and stencil buffers
	{
		depth16 = 1,					// 16-bit depth buffer
		depth24 = 2,					// 24-bit depth buffer
		depth32 = 4,					// 32-bit depth buffer
		
		stencil1  = 16,					// 1-bit stencil buffer
		stencil4  = 32,					// 4-bit stencil buffer
		stencil8  = 64,					// 8-bit stencil buffer
		stencil16 = 128					// 16-bit stencil buffer
	};
	
	enum								// filter modes
	{
		filterNearest = 0,
		filterLinear  = 1,
		filterMipmap  = 2
	};
	
	static	bool	isSupported         ();
	static	int		maxColorAttachemnts ();
	static	int		maxSize             ();
};

#endif
