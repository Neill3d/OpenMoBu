
#pragma once

// graphics_framebuffer.h
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//-- 
#include <GL\glew.h>
#include <vector>

//--- SDK include
#include <fbsdk/fbsdk.h>

#include "fpTexture.h"
#include "CheckGLError.h"


#include "OGL_Utils.h"
#include "FileUtils.h"

////////////////////////////////////////////////////
//
void drawOrthoQuad2d(int x, int y, int x2, int y2, int w, int h);

////////////////////////////////////////////////////
//
struct MainFrameBuffer
{
protected:
	
	struct buffer_info
	{
	protected:
		double		scaleFactor;	// supersampling scale factor

		int			depthSamples;
		int			coverageSamples;

		int			format;

		// viewport original buffer size
		int			width;
		int			height;
		// scaled supersampling buffer size
		int			bufw;
		int			bufh;

	public:
		buffer_info()
		{
			scaleFactor = 1.0;

			depthSamples = 0;
			coverageSamples = 0;

			format = 0;

			width = 0;
			height = 0;
			bufw = 0;
			bufh = 0;
		}

		bool NotEqual(double _scaleFactor, int _depthSamples, int _coverageSamples,
			int _width, int _height)
		{
			return (scaleFactor != _scaleFactor || depthSamples != _depthSamples 
				|| coverageSamples != _coverageSamples || width != _width || height != _height );
		}
		void Set(double _scaleFactor, int _depthSamples, int _coverageSamples,
			int _width, int _height )
		{
			scaleFactor = _scaleFactor;
			depthSamples = _depthSamples;
			coverageSamples = _coverageSamples;
			width = _width;
			height = _height;

			bufw = (int) (scaleFactor * (double)width);
			bufh = (int) (scaleFactor * (double)height);
		}
		const int GetWidth() const {
			return width;
		}
		const int GetBufferWidth() const {
			return bufw;
		}
		const int GetHeight() const {
			return height;
		}
		const int GetBufferHeight() const {
			return bufh;
		}
		const double GetScaleFactor() const {
			return scaleFactor;
		}
		const int GetNumberOfSamples() const {
			return depthSamples;
		}
		const int GetNumberOfCoverageSamples() const {
			return coverageSamples;
		}
	};

	
	GLuint		fbo_attached;

	GLuint		fbobig;
	GLuint		fboms;
	GLuint		fbo;

	buffer_info		mainInfo;
	buffer_info		extendedInfo;	// mobu offline render buffer
	
	struct texture_pack
	{
		GLuint		color_texture;
		GLuint		depth_texture;
		GLuint		stencil_texture;	// this could be useful to compatibility with mobu reflection shader
		GLuint		accum_texture;

		GLuint		depthRBO;
		GLuint		accumRBO;

#ifdef MANY_ATTACHMENTS
		GLuint		normal_texture;
		GLuint		mask_texture;
#endif

		texture_pack();
		~texture_pack();

		void deleteTextures();
	};

	// allocate memory for all of this

	texture_pack	textures_big;
	texture_pack	textures_ms;
	// if textures are scaled, then downsample them another textures
	texture_pack	textures;

	// used for offline renderer to attach our own textures
	texture_pack	textures_extended;

	// common info

	GLenum		colorInternalFormat;
	GLenum		colorFormat;
	GLenum		colorType;

	GLenum		depthAttachment;
	GLenum		depthInternalFormat;
	GLenum		depthFormat;
	GLenum		depthType;

	GLenum		stencilInternalFormat;
	GLenum		stencilFormat;
	GLenum		stencilType;
	GLenum		normalInternalFormat;
	GLenum		normalFormat;
	GLenum		normalType;
	GLenum		maskInternalFormat;
	GLenum		maskFormat;
	GLenum		maskType;

	GLenum		accumAttachment;
	GLenum		accumInternalFormat;
	GLenum		accumFormat;
	GLenum		accumType;

public:

	const GLuint GetFinalColorObject() const
	{
		if (isFboAttached() && extendedInfo.GetNumberOfSamples() <= 1)
			return textures_extended.color_texture;
		
		return textures.color_texture;
	}
	const GLuint GetFinalDepthObject() const
	{
		if (isFboAttached() && extendedInfo.GetNumberOfSamples() <= 1)
			return textures_extended.depth_texture;

		return textures.depth_texture;
	}
	const GLuint GetFinalNormalObject() const
	{
#ifdef MANY_ATTACHMENTS
		if (isFboAttached() && extendedInfo.GetNumberOfSamples() <= 1)
			return textures_extended.normal_texture;

		return textures.normal_texture;
#else
		return 0;
#endif
	}
	const GLuint GetFinalMaskObject() const
	{
#ifdef MANY_ATTACHMENTS
		if (isFboAttached() && extendedInfo.GetNumberOfSamples() <= 1)
			return textures_extended.mask_texture;

		return textures.mask_texture;
#else
		return 0;
#endif
	}

	const GLuint GetColorObjectMS() const
	{
		if (isFboAttached() && extendedInfo.GetNumberOfSamples() > 1)
			return textures_extended.color_texture;

		return textures_ms.color_texture;
	}
	const GLuint GetDepthObjectMS() const
	{
		if (isFboAttached() && extendedInfo.GetNumberOfSamples() > 1)
			return textures_extended.depth_texture;
		return textures_ms.depth_texture;
	}
	const GLuint GetNormalObjectMS() const
	{
#ifdef MANY_ATTACHMENTS
		if (isFboAttached() && extendedInfo.GetNumberOfSamples() > 1)
			return textures_extended.normal_texture;
		return textures_ms.normal_texture;
#else
		return 0;
#endif
	}
	const GLuint GetMaskObjectMS() const
	{
#ifdef MANY_ATTACHMENTS
		if (isFboAttached() && extendedInfo.GetNumberOfSamples() > 1)
			return textures_extended.mask_texture;
		return textures_ms.mask_texture;
#else
		return 0;
#endif
	}

public:
	//! a constructor
	MainFrameBuffer();

	void InitTextureInternalFormat();
	
	const GLenum getDepthInternalFormat() const{
		return depthInternalFormat;
	}
	const GLenum getDepthFormat() const {
		return depthFormat;
	}
	const GLenum getDepthType() const {
		return depthType;
	}
	const GLenum getNormalInternalFormat() const {
		return normalInternalFormat;
	}
	const GLenum getNormalFormat() const {
		return normalFormat;
	}
	const GLenum getNormalType() const {
		return normalType;
	}
	const GLenum getMaskInternalFormat() const {
		return maskInternalFormat;
	}
	const GLenum getMaskFormat() const {
		return maskFormat;
	}
	const GLenum getMaskType() const {
		return maskType;
	}

	static const GLuint createFBO()
	{
		GLuint fbo;
		glGenFramebuffers(1, &fbo);
		return fbo;
	}

	static void bindFBO(const GLuint fbo)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	}
	static void unbindFBO()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	static bool checkFboStatus(const GLuint fbo)
	{
		if (fbo == 0)
			return false;

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		GLenum status = glCheckFramebufferStatus ( GL_FRAMEBUFFER );
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			//printf ( "framebuffer error!\n" );



			return false;
		}
		return true;
	}

	//static const GLuint prepTexture(GLuint id, int width, int height, GLenum internalFormat, GLenum format, GLenum type);
	static const GLuint prepTexture(GLuint id, int depthSamples, int coverageSamples, int width, int height, GLenum internalFormat, GLenum format, GLenum type);

	static bool attachTexture2D(GLuint framebuffer, GLenum attachment, GLuint textureID, int samples)
	{
		if (framebuffer > 0)
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		if (textureID > 0)
			glFramebufferTexture2D(GL_FRAMEBUFFER, attachment,
				samples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, textureID, 0 );
		return true;
	}

	static bool detachFBOAttachment(GLuint framebuffer, GLenum attachment, int samples)
	{
		if (framebuffer > 0)
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		int objectType;
		glGetFramebufferAttachmentParameteriv( GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &objectType );

		if (objectType == GL_TEXTURE)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, attachment,
				samples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, 0, 0 );
		}
		else if (objectType == GL_RENDERBUFFER)
		{
			glFramebufferRenderbuffer ( GL_FRAMEBUFFER,  attachment,
		                    GL_RENDERBUFFER, 0 );	// objectName 
		}

		return true;
	}
	
	void AttachFBO(GLuint _fbo)
	{
		if (fbo_attached != _fbo)
		{
			fbo_attached = _fbo;
			//fbo = _fbo;
			PrepAttachedFBO();
		}
	}
	void DetachFBO()
	{
		if (fbo_attached > 0)
		{
			textures_extended.deleteTextures();
			//DetachTexturesFromFBO(fbo_attached, extendedInfo.GetNumberOfSamples() );
		}
		fbo_attached = 0;
	}
	bool isFboAttached() const
	{
		return (fbo_attached > 0);
	}
	const GLuint GetAttachedFBO() const
	{
		return fbo_attached;
	}
	
	void PrepAttachedFBO();

	const GLuint GetFBOMS() const
	{
		return fboms;
	}
	const GLuint GetFBOBIG() const
	{
		return fbobig;
	}
	const GLuint GetFinalFBO() const
	{
		return fbo;
	}

	const int getNumberOfDepthSamples() const
	{
		if (isFboAttached() )
			return extendedInfo.GetNumberOfSamples();
		return mainInfo.GetNumberOfSamples();
	}
	const int getNumberOfCoverageSamples() const
	{
		return mainInfo.GetNumberOfCoverageSamples();
	}

	//void CreateTextures(int _width, int _height, double ssfact, int _depthSamples, int _coverageSamples, bool createColor, bool createDepth, bool createNormal, bool createMask);
	void CreateTextures(texture_pack &pack, int width, int height, int depthSamples, int coverageSamples, bool createColor, bool createDepth, bool createStencil, bool createNormal, bool createMask);

	const int GetWidth() const
	{
		return mainInfo.GetWidth();
	}
	const int GetHeight() const
	{
		return mainInfo.GetHeight();
	}
	const int GetBufferWidth() const
	{
		return mainInfo.GetBufferWidth();
	}
	const int GetBufferHeight() const
	{
		return mainInfo.GetBufferHeight();
	}

	const int GetExtendedBufferWidth() const
	{
		return extendedInfo.GetBufferWidth();
	}
	const int GetExtendedBufferHeight() const
	{
		return extendedInfo.GetBufferHeight();
	}

	bool ReSize(const int newWidth, const int newHeight, double ssfact, int _depthSamples, int _coverageSamples);

	void DetachTexturesFromFBO(const GLuint _fbo, const int samples);

	void AttachTexturesToFBO(const GLuint _fbo, const texture_pack &pack, const int samples);
	
	void InitialValues();

	void DeleteFBO();
	
	void DeleteTextures();
	
	
	// check if have to resolve MS fbo into normal and do downscaling
	// as a result we have a ready to use textures pack

	void PrepForPostProcessing(bool drawToBack);

	void PrepForPostProcessingExtended(bool drawToBack);
	void PrepForPostProcessingInternal(bool drawToBack);

	void ChangeContext();
	
	void BeginRender();
	void BeginRenderAttached();
	void BeginRenderInternal();
	
	void EndRender();
};
