
// graphics_framebuffer.cpp
// Sergei <Neill3d> Solokhin 2018

#include "graphics_framebuffer.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
//

extern void DebugOGL_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void*userParam);

/*
const GLuint AttachedFrameBufferData::prepTexture(GLuint id, int width, int height, GLenum internalFormat, GLenum format, GLenum type)
{
	if (id == 0)
		return 0;

	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);
			
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			
	glBindTexture(GL_TEXTURE_2D, 0);

	CHECK_GL_ERROR();

	return id;
}
*/


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// MainFrameBuffer::TexturesPack

MainFrameBuffer::texture_pack::texture_pack()
{
	color_texture = 0;
	depth_texture = 0;
	stencil_texture = 0;
	accum_texture = 0;

	depthRBO = 0;
	accumRBO = 0;

#ifdef MANY_ATTACHMENTS
	normal_texture = 0;
	mask_texture = 0;
#endif
}

MainFrameBuffer::texture_pack::~texture_pack()
{
	deleteTextures();
}

void MainFrameBuffer::texture_pack::deleteTextures()
{
	if (color_texture > 0)
	{
		glDeleteTextures(1, &color_texture);
		color_texture = 0;
	}
	if (depth_texture > 0)
	{
		glDeleteTextures(1, &depth_texture);
		depth_texture = 0;
	}
	if (stencil_texture > 0)
	{
		glDeleteTextures(1, &stencil_texture);
		stencil_texture = 0;
	}
	if (accum_texture > 0)
	{
		glDeleteTextures(1, &accum_texture);
		accum_texture = 0;
	}
	if (depthRBO > 0)
	{
		glDeleteRenderbuffers(1, &depthRBO);
		depthRBO = 0;
	}
	if (accumRBO > 0)
	{
		glDeleteRenderbuffers(1, &accumRBO);
		accumRBO = 0;
	}

#ifdef MANY_ATTACHMENTS
	if (normal_texture > 0)
	{
		glDeleteTextures(1, &normal_texture);
		normal_texture = 0;
	}
	if (mask_texture > 0)
	{
		glDeleteTextures(1, &mask_texture);
		mask_texture = 0;
	}
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MainFrameBuffer

MainFrameBuffer::MainFrameBuffer()
{
	
	InitialValues();
}

void MainFrameBuffer::InitTextureInternalFormat()
{
	colorInternalFormat = GL_RGBA8;
	colorFormat = GL_RGBA;
	colorType = GL_UNSIGNED_BYTE;
	/*
	// TODO: is depth component32F supported !!
	depthAttachment = GL_DEPTH_ATTACHMENT;
	depthInternalFormat = GL_DEPTH_COMPONENT32F;
	depthFormat = GL_DEPTH_COMPONENT;
	depthType = GL_FLOAT;
	*/
	
	//depthAttachment = GL_DEPTH_STENCIL_ATTACHMENT;
	depthAttachment = GL_DEPTH_STENCIL_ATTACHMENT;
	depthInternalFormat = GL_DEPTH24_STENCIL8;
	depthFormat = GL_DEPTH_STENCIL;
	depthType = GL_UNSIGNED_INT_24_8;
	
	accumAttachment = GL_COLOR_ATTACHMENT1;
	accumInternalFormat = GL_RGBA16;
	accumFormat = GL_RGBA;
	accumType = GL_FLOAT;

	/*
	depthAttachment = GL_DEPTH_STENCIL_ATTACHMENT;
	depthInternalFormat = GL_DEPTH32F_STENCIL8;
	depthFormat = GL_DEPTH_STENCIL;
	depthType = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
	*/
	stencilInternalFormat = GL_STENCIL_INDEX8;
	stencilFormat = GL_R; //GL_STENCIL_COMPONENTS;
	stencilType = GL_UNSIGNED_BYTE;

	maskInternalFormat = GL_RGBA8;
	maskFormat = GL_RGBA;
	maskType = GL_UNSIGNED_BYTE;

	normalFormat = GL_RGBA;	// TODO: ?!
	normalType = GL_FLOAT;
	GLenum format = fpRgbaFormatWithPrecision();
	if (format == 0)
	{
		//FBMessageBox( "Composite GBuffer", "Floating texture format is not supported by your graphic card!", "Ok" ); 
		normalInternalFormat = GL_RGBA8;
	}
	else
	{
		normalInternalFormat = format;
	}
}

void MainFrameBuffer::InitialValues()
{
	fbo_attached = 0;
	fbobig = 0;
	fboms = 0;
	fbo = 0;
}

void MainFrameBuffer::DeleteFBO()
{
	if (fbobig > 0)
	{
		glDeleteFramebuffers(1, &fbobig);
		fbobig = 0;
	}
	if (fboms > 0)
	{
		glDeleteFramebuffers(1, &fboms);
		fboms = 0;
	}
	if (fbo > 0)
	{
		glDeleteFramebuffers(1, &fbo);
		fbo = 0;
	}
}

void MainFrameBuffer::DeleteTextures()
{
	textures.deleteTextures();
	textures_ms.deleteTextures();
	textures_big.deleteTextures();
}

void MainFrameBuffer::PrepAttachedFBO()
{
	if (fbo_attached == 0)
		return;

	glBindFramebuffer(GL_FRAMEBUFFER, fbo_attached);

	//
	// collect information about the mobu framebuffer

	int objectType, objectName;
	GLint width = 0, height = 0, format = 0, samples = 0;

	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &objectType);
	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &objectName);

	if (objectType == 0)
	{
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &objectType);
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &objectName);
	}

	if (objectType == GL_RENDERBUFFER && glIsRenderbuffer(objectName) == GL_TRUE)
	{
		glBindRenderbuffer(GL_RENDERBUFFER, objectName);

		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT, &format);
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &samples);
		/*
		if (samples > 0)
		{
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, linfo.samples, linfo.depthInternalFormat, linfo.width, linfo.height);
		}
		else
		{
			glRenderbufferStorage     ( GL_RENDERBUFFER, linfo.depthInternalFormat, linfo.width, linfo.height );
		}
		*/
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			GL_RENDERBUFFER, 0);	// objectName 
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
			GL_RENDERBUFFER, 0);	// COMPOSITE MASTER USE FORMAT WITHOUT STENCIL !!!

		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
	else if (glIsTexture(objectName) == GL_TRUE)
	{
		glGetTextureParameteriv(objectName, GL_TEXTURE_WIDTH, &width);
		glGetTextureParameteriv(objectName, GL_TEXTURE_HEIGHT, &height);
		glGetTextureParameteriv(objectName, GL_TEXTURE_SAMPLES, &samples);
	}

	if (width > 0 && height > 0)
	{
		extendedInfo.Set(1.0, samples, 0, width, height);

		//
		// detach mobu resources

		detachFBOAttachment(0, GL_COLOR_ATTACHMENT0, samples);

		/*
		CreateTextures(textures_extended, extendedInfo.GetBufferWidth(), extendedInfo.GetBufferHeight(),
		extendedInfo.GetNumberOfSamples(), extendedInfo.GetNumberOfCoverageSamples(), true, true, false, true, true);

		attachTexture2D(0, GL_COLOR_ATTACHMENT0, textures_extended.color_texture, extendedInfo.GetNumberOfSamples());
		attachTexture2D(0, depthAttachment, textures_extended.depth_texture, extendedInfo.GetNumberOfSamples());
		//attachTexture2D(0, GL_STENCIL_ATTACHMENT, textures_extended.stencil_texture, extendedInfo.GetNumberOfSamples());
		attachTexture2D(0, GL_COLOR_ATTACHMENT1, textures_extended.normal_texture, extendedInfo.GetNumberOfSamples());
		attachTexture2D(0, GL_COLOR_ATTACHMENT2, textures_extended.mask_texture, extendedInfo.GetNumberOfSamples());
		*/
		CreateTextures(textures_extended, extendedInfo.GetBufferWidth(), extendedInfo.GetBufferHeight(),
			extendedInfo.GetNumberOfSamples(), extendedInfo.GetNumberOfCoverageSamples(), true, true, true, true, true);

		attachTexture2D(0, GL_COLOR_ATTACHMENT0, textures_extended.color_texture, extendedInfo.GetNumberOfSamples());
		attachTexture2D(0, depthAttachment, textures_extended.depth_texture, extendedInfo.GetNumberOfSamples());
		attachTexture2D(0, GL_STENCIL_ATTACHMENT, textures_extended.stencil_texture, mainInfo.GetNumberOfSamples());

#ifdef MANY_ATTACHMENTS
		attachTexture2D(0, GL_COLOR_ATTACHMENT1, textures_extended.normal_texture, extendedInfo.GetNumberOfSamples());
		attachTexture2D(0, GL_COLOR_ATTACHMENT2, textures_extended.mask_texture, extendedInfo.GetNumberOfSamples());
#endif

	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

const GLuint MainFrameBuffer::prepTexture(GLuint id, int depthSamples, int coverageSamples, int width, int height, GLenum internalFormat, GLenum format, GLenum type)
{
	GLuint texId = id;
	if (texId == 0)
	{
		glGenTextures(1, &texId);
	}

	CHECK_GL_ERROR();

	if (depthSamples <= 1)
	{
		glTextureStorage2DEXT(texId, GL_TEXTURE_2D, 1, internalFormat, width, height);
		glTextureParameterfEXT(texId, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameterfEXT(texId, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameterfEXT(texId, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameterfEXT(texId, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else
	{
		if (coverageSamples > 1)
		{
			glTextureImage2DMultisampleCoverageNV(texId, GL_TEXTURE_2D_MULTISAMPLE, coverageSamples, depthSamples, internalFormat, width, height, GL_TRUE);
		}
		else
		{
			glTextureStorage2DMultisampleEXT(texId, GL_TEXTURE_2D_MULTISAMPLE, depthSamples, internalFormat, width, height, GL_TRUE);
		}
	}

	CHECK_GL_ERROR();
	return texId;
}

void MainFrameBuffer::DetachTexturesFromFBO(const GLuint _fbo, const int samples)
{
	if (_fbo > 0)
	{
		bindFBO(_fbo);
		detachFBOAttachment(0, GL_COLOR_ATTACHMENT0, samples);

#ifdef MANY_ATTACHMENTS
		detachFBOAttachment(0, GL_COLOR_ATTACHMENT1, samples);
		detachFBOAttachment(0, GL_COLOR_ATTACHMENT2, samples);
#endif
		detachFBOAttachment(0, depthAttachment, samples);
		//detachFBOAttachment(0, GL_STENCIL_ATTACHMENT, samples);
		//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);

		unbindFBO();
	}
}

void MainFrameBuffer::AttachTexturesToFBO(const GLuint _fbo, const texture_pack &pack, const int samples)
{
	if (_fbo > 0)
	{
		bindFBO(_fbo);
			
		attachTexture2D(0, GL_COLOR_ATTACHMENT0, pack.color_texture, samples);

#ifdef MANY_ATTACHMENTS
		attachTexture2D(0, GL_COLOR_ATTACHMENT1, pack.normal_texture, samples);
		attachTexture2D(0, GL_COLOR_ATTACHMENT2, pack.mask_texture, samples);
#endif

		//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, pack.depthRBO);

		attachTexture2D(0, depthAttachment, pack.depth_texture, samples);
		//attachTexture2D(0, GL_STENCIL_ATTACHMENT, pack.depth_texture, samples);
		//attachTexture2D(0, GL_STENCIL_ATTACHMENT, pack.stencil_texture, samples);

		checkFboStatus(_fbo);
		unbindFBO();
	}
}
/*
void MainFrameBuffer::CreateTextures(int _width, int _height, double ssfact, int _depthSamples, int _coverageSamples, bool createColor, bool createDepth, bool createNormal, bool createMask)
{
	scaleFactor = ssfact;

	depthSamples = _depthSamples;
	coverageSamples = _coverageSamples;

//	multisample = (_depthSamples > 1);
//	csaa = (coverageSamples > depthSamples);

	bool ismultisample = depthSamples > 1;

	width = _width;
	height = _height;

	bufw = (int) (scaleFactor * (double)width);
	bufh = (int) (scaleFactor * (double)height);

	if (createColor)
	{
		if (scaleFactor > 1.0)
			textures_big.color_texture = prepTexture(0, 0, 0, bufw, bufh, colorInternalFormat, colorFormat, colorType);

		if (ismultisample)
			textures_ms.color_texture = prepTexture(0, depthSamples, coverageSamples, bufw, bufh, colorInternalFormat, colorFormat, colorType);

		textures.color_texture = prepTexture(0, 0, 0, width, height, colorInternalFormat, colorFormat, colorType);
	}

	if (createDepth)
	{
		if (scaleFactor > 1.0)
			textures_big.depth_texture = prepTexture(0, 0, 0, bufw, bufh, depthInternalFormat, depthFormat, depthType);

		if (ismultisample)
			textures_ms.depth_texture = prepTexture(0, depthSamples, coverageSamples, bufw, bufh, depthInternalFormat, depthFormat, depthType);

		textures.depth_texture = prepTexture(0, 0, 0, bufw, bufh, depthInternalFormat, depthFormat, depthType);
	}

	if (createNormal)
	{
		if (scaleFactor > 1.0)
			textures_big.normal_texture = prepTexture(0, 0, 0, bufw, bufh, normalInternalFormat, normalFormat, normalType);

		if (ismultisample)
			textures_ms.normal_texture = prepTexture(0, depthSamples, coverageSamples, bufw, bufh, normalInternalFormat, normalFormat, normalType);

		textures.normal_texture = prepTexture(0, 0, 0, bufw, bufh, normalInternalFormat, normalFormat, normalType);
	}

	if (createMask)
	{
		if (scaleFactor > 1.0)
			textures_big.mask_texture = prepTexture(0, 0, 0, bufw, bufh, maskInternalFormat, maskFormat, maskType);

		if (ismultisample)
			textures_ms.mask_texture = prepTexture(0, depthSamples, coverageSamples, bufw, bufh, maskInternalFormat, maskFormat, maskType);

		textures.mask_texture = prepTexture(0, 0, 0, bufw, bufh, maskInternalFormat, maskFormat, maskType);
	}
}
*/
void MainFrameBuffer::CreateTextures(texture_pack &pack, int width, int height, int depthSamples, int coverageSamples, bool createColor, bool createDepth, bool createStencil, bool createNormal, bool createMask)
{
	if (createColor)
		pack.color_texture = prepTexture(0, depthSamples, coverageSamples, width, height, colorInternalFormat, colorFormat, colorType);
	
	if (createStencil)
	{
		//pack.stencil_texture = prepTexture(0, depthSamples, coverageSamples, width, height, stencilInternalFormat, stencilFormat, stencilType);
		// create a render buffer object for the depth buffer
		glGenRenderbuffers(1, &pack.depthRBO);
		// bind the texture
		glBindRenderbuffer(GL_RENDERBUFFER, pack.depthRBO);
		// create the render buffer in the GPU
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		// unbind the render buffer
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	if (createDepth)
		pack.depth_texture = prepTexture(0, depthSamples, coverageSamples, width, height, depthInternalFormat, depthFormat, depthType);
	
#ifdef MANY_ATTACHMENTS
	if (createNormal)
		pack.normal_texture = prepTexture(0, depthSamples, coverageSamples, width, height, normalInternalFormat, normalFormat, normalType);
	
	if (createMask)
		pack.mask_texture = prepTexture(0, depthSamples, coverageSamples, width, height, maskInternalFormat, maskFormat, maskType);
#endif
}

bool MainFrameBuffer::ReSize(const int newWidth, const int newHeight, double ssfact, int _depthSamples, int _coverageSamples)
{
	bool result = false;

#ifdef OGL_DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback( DebugOGL_Callback, nullptr );
#endif

	bool needUpdate = false;
	bool firstTime = false;
	if (fbo == 0)
	{
		fbo = createFBO();
		needUpdate = true;
		firstTime = true;
	}
	if (fbobig == 0)
	{
		fbobig = createFBO();
		needUpdate = true;
		firstTime = true;
	}
	if (fboms == 0)
	{
		fboms = createFBO();
		needUpdate = true;
		firstTime = true;
	}

	CHECK_GL_ERROR();

	double local_ssfact = ssfact;
	int local_samples = _depthSamples;
	int local_coverageSamples = _coverageSamples;
	int local_width = newWidth;
	int local_height = newHeight;

	if (isFboAttached() )
	{
		local_ssfact = 1.0;
		local_samples = extendedInfo.GetNumberOfSamples();
		local_coverageSamples = extendedInfo.GetNumberOfCoverageSamples();
		local_width = extendedInfo.GetBufferWidth();
		local_height = extendedInfo.GetBufferHeight();
	}

	if ( needUpdate == true 
		|| mainInfo.NotEqual(local_ssfact, local_samples, local_coverageSamples, local_width, local_height) )
	{
		result = true;

		DeleteTextures();
		if (firstTime == false)
		{
			DetachTexturesFromFBO(fbo, 0);
			if (mainInfo.GetScaleFactor() > 1.0)
				DetachTexturesFromFBO(fbobig, 0);
			if (mainInfo.GetNumberOfSamples() > 1)
				DetachTexturesFromFBO(fboms, mainInfo.GetNumberOfSamples() );
		}

		CHECK_GL_ERROR();

		mainInfo.Set(local_ssfact, local_samples, local_coverageSamples, local_width, local_height);

		unbindFBO();
		
		if (local_ssfact > 1.0)
		{
			CreateTextures(textures_big, mainInfo.GetBufferWidth(), mainInfo.GetBufferHeight(), 
				0, 0, true, true, false, true, true);
			AttachTexturesToFBO(fbobig, textures_big, 0);
		}
		if (false == isFboAttached() && local_samples > 1)
		{
			CreateTextures(textures_ms, mainInfo.GetBufferWidth(), mainInfo.GetBufferHeight(), 
				local_samples, local_coverageSamples, true, true, true, true, true);
			AttachTexturesToFBO(fboms, textures_ms, mainInfo.GetNumberOfSamples());
		}
		CreateTextures(textures, mainInfo.GetWidth(), mainInfo.GetHeight(), 0, 0, true, true, true, true, true); 
		AttachTexturesToFBO(fbo, textures, 0);
		
		CHECK_GL_ERROR();
	}

	return result;
}

void MainFrameBuffer::PrepForPostProcessing(bool drawToBack)
{
	if (isFboAttached() )
		PrepForPostProcessingExtended(drawToBack);
	else
		PrepForPostProcessingInternal(drawToBack);
}

void MainFrameBuffer::PrepForPostProcessingExtended(bool drawToBack)
{
	const int bufw = extendedInfo.GetBufferWidth();
	const int bufh = extendedInfo.GetBufferHeight();

	const int width = extendedInfo.GetWidth();
	const int height = extendedInfo.GetHeight();

	if (extendedInfo.GetNumberOfSamples() > 1)
	{
		BlitFBOToFBO( fbo_attached, bufw, bufh,
					fbo, bufw, bufh,
				true, true, false );
		
	} // if multisample

	if (drawToBack)
	{
		BlitFBOToScreen(fbo_attached, bufw, bufh, width, height);
	}
}

void MainFrameBuffer::PrepForPostProcessingInternal(bool drawToBack)
{
	const int bufw = mainInfo.GetBufferWidth();
	const int bufh = mainInfo.GetBufferHeight();

	const int width = mainInfo.GetWidth();
	const int height = mainInfo.GetHeight();

	if (mainInfo.GetNumberOfSamples() > 1)
	{
		if (mainInfo.GetScaleFactor() > 1.0)
		{
			/*
			BlitFBOToFBO( fboms, bufw, bufh,
						fbobig, bufw, bufh,
					true, true );

			CHECK_GL_ERROR();

			// downsample textures with specified filtering
			DownscaleToFinalTextures(drawToBack);
			*/

			BlitFBOToFBO( fboms, bufw, bufh,
						fbobig, bufw, bufh,
					false, false, false );

			glBindTexture( GL_TEXTURE_2D, textures_big.color_texture );
			drawOrthoQuad2d(0, 0, width, height, width, height);
			glBindTexture( GL_TEXTURE_2D, 0 );

			CHECK_GL_ERROR();
		}
		else
		{
			
			BlitFBOToFBO( fboms, bufw, bufh,
						fbo, bufw, bufh,
					true, true, false );
			
			if (drawToBack)
			{
				BlitFBOToScreen(fboms, bufw, bufh, width, height);
			}
		}
	} // if multisample
	else
	{	
		if (drawToBack)
		{
			BlitFBOToScreen(fbo, width, height, width, height);
		}
	}
}

void MainFrameBuffer::ChangeContext()
{
	
	DeleteTextures();
	DeleteFBO();

	DetachFBO();
}

void MainFrameBuffer::BeginRender()
{
	if (fbo_attached > 0)
		BeginRenderAttached();
	else
		BeginRenderInternal();
}

void MainFrameBuffer::BeginRenderAttached()
{
	
	const int bufw = extendedInfo.GetBufferWidth();
	const int bufh = extendedInfo.GetBufferHeight();
	const int samples = extendedInfo.GetNumberOfSamples();
	
	glViewport(0, 0, bufw, bufh);
	
	bindFBO(fbo_attached);

#ifdef MANY_ATTACHMENTS
	GLenum buffers [] = { GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers( 2, buffers );
	glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
#endif

	GLenum buffers2 [] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers( 1, buffers2 );

	// enable multisample
	if (samples > 1)
		glEnable(GL_MULTISAMPLE);
	else
		glDisable(GL_MULTISAMPLE);
}

void MainFrameBuffer::BeginRenderInternal()
{
	const int bufw = mainInfo.GetBufferWidth();
	const int bufh = mainInfo.GetBufferHeight();
	const int samples = mainInfo.GetNumberOfSamples();

	// bind proper buffer
	glViewport(0, 0, bufw, bufh);
		
	if (samples > 1)
		bindFBO( fboms );
	else
	{
		if (mainInfo.GetScaleFactor() > 1.0)
			bindFBO( fbobig );
		else
			bindFBO( fbo );
	}

#ifdef MANY_ATTACHMENTS
	GLenum buffers [] = { GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers( 2, buffers );
	glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
#endif

	GLenum buffers2 [] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers( 1, buffers2 );

	// enable multisample
	if (samples > 1)
		glEnable(GL_MULTISAMPLE);
	else
		glDisable(GL_MULTISAMPLE);
}

void MainFrameBuffer::EndRender()
{
	unbindFBO();
}