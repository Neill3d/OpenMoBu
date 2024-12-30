
/**	\file	postprocessing_effect.cxx

Sergei <Neill3d> Solokhin 2018-2019

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectbuffers.h"

////////////////////////////////////////////////////////////////////////////////////
// post effect buffers

#define GL_COMPRESSED_ETC1_RGB8_OES                      0x8D64 // ETC1 - GL_OES_compressed_ETC1_RGB8_texture

PostEffectBuffers::PostEffectBuffers()
	: mWidth(1)
	, mHeight(1)
	, mOutputColorObject(0)
	, mPreviewSignal(false)
	, mPreviewRunning(false)
	, mPreviewWidth(1)
	, mPreviewHeight(1)
	, mSrc(0)
	, mDst(1)
	, mCompressionInternal(GL_COMPRESSED_ETC1_RGB8_OES)
	, mCompressionFormat(GL_RGB)
	, mCompressionType(GL_UNSIGNED_SHORT_5_6_5)
	, mCompressedPreviewId(0)
	, mCompressOnFlyId(0)
	, mCurPBO(0)
	, mCurUnPack(0)
	, mUnCompressSize(0)
	, mCompressedSize(0)
{
	mPBOs[0] = mPBOs[1] = 0;
	mUnPackPBOs[0] = mUnPackPBOs[1] = 0;
}

PostEffectBuffers::~PostEffectBuffers()
{
	ChangeContext();
}

void PostEffectBuffers::ChangeContext()
{
	FreeBuffers();
	FreeTextures();

	OnContextChanged();
}

unsigned int nearestPowerOf2(unsigned int value)
{
	unsigned int v = value; // compute the next highest power of 2 of 32-bit v

	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;

	return v;
}

int PostEffectBuffers::GetFlagsForMainColorBuffer()
{
	return FrameBuffer::eCreateColorTexture | FrameBuffer::eCreateDepthTexture | FrameBuffer::eDeleteFramebufferOnCleanup;
}
void PostEffectBuffers::SetParametersForMainColorBuffer(FrameBuffer* buffer, bool filterMips)
{
	buffer->SetDepthFormat(GL_DEPTH_STENCIL);
	buffer->SetDepthInternalFormat(GL_DEPTH24_STENCIL8);
	buffer->SetDepthType(GL_UNSIGNED_INT_24_8);

	for (int i = 0; i < buffer->GetMaxColorAttachemnts(); ++i)
	{
		buffer->SetClamp(i, GL_CLAMP_TO_EDGE);
		buffer->SetFilter(i, (filterMips) ? FrameBuffer::filterMipmap : FrameBuffer::filterLinear);
	}
}
void PostEffectBuffers::SetParametersForMainDepthBuffer(FrameBuffer* buffer)
{
	buffer->SetColorFormat(0, GL_RED);
	buffer->SetColorInternalFormat(0, GL_R32F);
	buffer->SetColorType(0, GL_FLOAT);
	buffer->SetFilter(0, FrameBuffer::filterNearest);
	buffer->SetClamp(0, GL_CLAMP_TO_EDGE);
}

bool PostEffectBuffers::ReSize(const int w, const int h, bool useScale, double scaleFactor, bool filterMips)
{
	bool lSuccess = true;

	mWidth = w;
	mHeight = h;

	for (const auto& framebufferEntry : framebufferPool)
	{
		framebufferEntry.second.framebuffer->ReSize(w, h);
	}
	/*
	// resize fbos
	const int flags = FrameBuffer::eCreateColorTexture | FrameBuffer::eCreateDepthTexture | FrameBuffer::eDeleteFramebufferOnCleanup;

	if (!mBufferPost0.get())
	{
		mBufferPost0.reset(new FrameBuffer(1, 1, flags));
		
		//mBufferPost0->SetDepthFormat(GL_DEPTH);
		//mBufferPost0->SetDepthInternalFormat(GL_DEPTH_COMPONENT32F);
		//mBufferPost0->SetDepthType(GL_FLOAT);
		
		mBufferPost0->SetDepthFormat(GL_DEPTH_STENCIL);
		mBufferPost0->SetDepthInternalFormat(GL_DEPTH24_STENCIL8);
		mBufferPost0->SetDepthType(GL_UNSIGNED_INT_24_8);

		mBufferPost0->SetClamp(0, GL_CLAMP_TO_EDGE);
		mBufferPost0->SetFilter(0, (filterMips) ? FrameBuffer::filterMipmap : FrameBuffer::filterLinear);
	}
	if (!mBufferPost1.get())
	{
		mBufferPost1.reset(new FrameBuffer(1, 1, flags));
		
		//mBufferPost1->SetDepthFormat(GL_DEPTH);
		//mBufferPost1->SetDepthInternalFormat(GL_DEPTH_COMPONENT32F);
		//mBufferPost1->SetDepthType(GL_FLOAT);
		
		mBufferPost1->SetDepthFormat(GL_DEPTH_STENCIL);
		mBufferPost1->SetDepthInternalFormat(GL_DEPTH24_STENCIL8);
		mBufferPost1->SetDepthType(GL_UNSIGNED_INT_24_8);

		mBufferPost1->SetClamp(0, GL_CLAMP_TO_EDGE);
		mBufferPost1->SetFilter(0, (filterMips) ? FrameBuffer::filterMipmap : FrameBuffer::filterLinear);
	}

	if (!mBufferDepth.get())
	{
		mBufferDepth.reset(new FrameBuffer(1, 1));
		mBufferDepth->SetColorFormat(0, GL_RED);
		mBufferDepth->SetColorInternalFormat(0, GL_R32F);
		mBufferDepth->SetColorType(0, GL_FLOAT);
		mBufferDepth->SetFilter(0, FrameBuffer::filterNearest);
		mBufferDepth->SetClamp(0, GL_CLAMP_TO_EDGE);
	}

	if (!mBufferBlur.get())
	{
		mBufferBlur.reset(new FrameBuffer(1, 1));
	}

	if (!mBufferDownscale.get())
	{
		mBufferDownscale.reset(new FrameBuffer(1, 1));
	}

	if (!mBufferMasking.get())
	{
		// prepare framebuffer with 4 color attachments as we have support for 4 masks
		mBufferMasking.reset(new FrameBuffer(1, 1, FrameBuffer::eCreateColorTexture | FrameBuffer::eDeleteFramebufferOnCleanup, 4));
	}
	
	if (!mBufferPost0->ReSize(w, h))
		lSuccess = false;
	if (!mBufferPost1->ReSize(w, h))
		lSuccess = false;
	if (!mBufferDepth->ReSize(w, h))
		lSuccess = false;
	if (!mBufferBlur->ReSize(w, h))
		lSuccess = false;
	if (!mBufferMasking->ReSize(w, h))
		lSuccess = false;

	if (useScale)
	{
		const double sw = 0.01 * static_cast<double>(w) * scaleFactor;
		const double sh = 0.01 * static_cast<double>(h) * scaleFactor;

		// find nearest power of two
		mPreviewWidth = nearestPowerOf2(static_cast<unsigned int>(sw));
		mPreviewHeight = nearestPowerOf2(static_cast<unsigned int>(sh));

		//mPreviewWidth = 256;
		//mPreviewHeight = 128;

		if (mBufferDownscale->ReSize(mPreviewWidth, mPreviewHeight))
		{
			AllocPreviewTexture(mPreviewWidth, mPreviewHeight);
		}
	}
	*/
	if (lSuccess)
	{
		if (mPBOs[0] > 0)
		{
			glDeleteBuffers(2, mPBOs);
			mPBOs[0] = mPBOs[1] = 0;
		}
		mCurPBO = 0;
	}

	return lSuccess;
}

bool PostEffectBuffers::Ok()
{
	/*
	if (!mBufferPost0.get() || !mBufferPost1.get() || !mBufferDepth.get() || !mBufferBlur.get() || !mBufferMasking.get())
	{
		return false;
	}
	if (!mBufferPost0->GetFrameBuffer() || !mBufferPost1->GetFrameBuffer() || !mBufferDepth->GetFrameBuffer()
		|| !mBufferBlur->GetFrameBuffer() || !mBufferMasking->GetFrameBuffer())
	{
		return false;
	}
	*/
	return true;
}

void PostEffectBuffers::FreeBuffers()
{
	//mBufferPost0.reset(nullptr);
	//mBufferPost1.reset(nullptr);
	//mBufferDepth.reset(nullptr);
	//mBufferBlur.reset(nullptr);
	//mBufferDownscale.reset(nullptr);
	//mBufferMasking.reset(nullptr);
}
/*
const GLuint PostEffectBuffers::PrepAndGetBufferObject()
{
	mSrc = 0;
	mDst = 1;

	return 0; // mBufferPost0->GetFrameBuffer();
}
*/
/*
FrameBuffer *PostEffectBuffers::GetSrcBufferPtr()
{
	return (0 == mSrc) ? mBufferPost0.get() : mBufferPost1.get();
}
FrameBuffer *PostEffectBuffers::GetDstBufferPtr()
{
	return (0 == mDst) ? mBufferPost0.get() : mBufferPost1.get();
}
FrameBuffer *PostEffectBuffers::GetBufferDepthPtr()
{
	return mBufferDepth.get();
}
FrameBuffer *PostEffectBuffers::GetBufferBlurPtr()
{
	return mBufferBlur.get();
}

FrameBuffer* PostEffectBuffers::GetBufferMaskPtr()
{
	return mBufferMasking.get();
}

FrameBuffer *PostEffectBuffers::GetBufferDownscalePtr()
{
	return mBufferDownscale.get();
}

void PostEffectBuffers::SwapBuffers()
{
	// swap buffers
	int temp = mDst;
	mDst = mSrc;
	mSrc = temp;
}
*/
/*
// get a result of effect computation
const GLuint PostEffectBuffers::GetFinalColor()
{
	return (0==mSrc) ? mBufferPost0->GetColorObject() : mBufferPost1->GetColorObject();
}

const GLuint PostEffectBuffers::GetFinalFBO()
{
	return (0 == mSrc) ? mBufferPost0->GetFrameBuffer() : mBufferPost1->GetFrameBuffer();
}

const GLuint PostEffectBuffers::GetPreviewColor()
{
	return (mBufferDownscale.get()) ? mBufferDownscale->GetColorObject() : 0;
}

const GLuint PostEffectBuffers::GetPreviewFBO()
{
	return (mBufferDownscale.get()) ? mBufferDownscale->GetFrameBuffer() : 0;
}
*/
const GLuint PostEffectBuffers::GetPreviewCompressedColor()
{
	//return mBufferDownscale->GetColorObject();
	return mCompressOnFlyId;
	//return mCompressedPreviewId;
}

void PostEffectBuffers::AllocPreviewTexture(int w, int h)
{
	FreeTextures();

}

void PostEffectBuffers::FreeTextures()
{
	if (mCompressedPreviewId > 0)
	{
		glDeleteTextures(1, &mCompressedPreviewId);
		mCompressedPreviewId = 0;
	}

	if (mCompressOnFlyId > 0)
	{
		glDeleteTextures(1, &mCompressOnFlyId);
		mCompressOnFlyId = 0;
	}

	if (mPBOs[0] > 0)
	{
		glDeleteBuffers(2, mPBOs);
		mPBOs[0] = mPBOs[1] = 0;
		mCurPBO = 0;
	}
}

bool PostEffectBuffers::PreviewOpenGLCompress(EImageCompression	compressionType, GLint &compressionCode)
{
	//if (nullptr == mBufferDownscale.get() || 0 == mBufferDownscale->GetColorObject())
//		return false;

//	if (0 == mBufferDownscale->GetColorObject())
	//	return false;

	if (mPreviewWidth <= 1 || mPreviewHeight <= 1)
		return false;

	size_t imageSize = mPreviewWidth * mPreviewHeight * 3;
	mUnCompressSize = imageSize;

	//glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
	
	if (0 == mPBOs[0])
	{
		glGenBuffers(2, mPBOs);

		// create 2 pixel buffer objects, you need to delete them when program exits.
		// glBufferDataARB with NULL pointer reserves only memory space.
		for (int i = 0; i<2; ++i)
		{
			glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBOs[i]);
			glBufferData(GL_PIXEL_PACK_BUFFER, imageSize, 0, GL_STREAM_DRAW);
		}
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	}

	//
	mCurPBO = 1 - mCurPBO;

	// read pixels from framebuffer to PBO
//	mBufferDownscale->Bind();

	glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBOs[mCurPBO]);
	glReadPixels(0, 0, mPreviewWidth, mPreviewHeight, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

//	mBufferDownscale->UnBind();
	/*
	const GLuint srcId = mBufferDownscale->GetColorObject();
	glBindTexture(GL_TEXTURE_2D, srcId);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, mUnCompressData.data());
	glBindTexture(GL_TEXTURE_2D, 0);
	*/
	//glMemoryBarrier(GL_PIXEL_BUFFER_BARRIER_BIT);

	// map the PBO to process it's data by CPU
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, mPBOs[1 - mCurPBO]);
	//GLubyte *ptr = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

	//if (nullptr != ptr)
	{
		glHint(GL_TEXTURE_COMPRESSION_HINT, GL_FASTEST);

		if (0 == mCompressOnFlyId)
		{
			GLint internalFormat = GL_COMPRESSED_RGB;

			switch (compressionType)
			{
			case eImageCompressionETC2:
				internalFormat = GL_COMPRESSED_RGB8_ETC2;
				break;
			case eImageCompressionS3TC:
				internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
				break;
			case eImageCompressionASTC:
				internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR;
				break;
			default:
				internalFormat = GL_COMPRESSED_RGB;
				break;
			}
			compressionCode = internalFormat;

			glGenTextures(1, &mCompressOnFlyId);
			glBindTexture(GL_TEXTURE_2D, mCompressOnFlyId);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, mPreviewWidth, mPreviewHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0); // ptr);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, mCompressOnFlyId);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mPreviewWidth, mPreviewHeight, GL_RGB, GL_UNSIGNED_BYTE, 0); // ptr);
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB, mPreviewWidth, mPreviewHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, mUnCompressData.data());

			// get stat
			GLint compressed = 0;
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, &compressed);

			if (compressed == GL_TRUE)
			{
				GLint compressed_size;
				GLint internalFormat;

				glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
				glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressed_size);

				mCompressedSize = compressed_size;
				compressionCode = internalFormat;
			}

			glBindTexture(GL_TEXTURE_2D, 0);
		}

		//glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	}

	// back to conventional pixel operation
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	
	return true;
}
/*
void PostEffectBuffers::MapCompressedData(const float timestamp, Network::CPacketImageHeader &header)
{
	glBindTexture(GL_TEXTURE_2D, mCompressOnFlyId);

	GLint compressed = 0;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, &compressed);

	if (compressed == GL_TRUE)
	{
		GLint compressed_size;
		GLint internalFormat;

		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressed_size);

		mCompressedData.resize(compressed_size + 1); // sizeof(CompressImageHeader));

		//CompressImageHeader *pHeader = (CompressImageHeader*)mCompressedData.data();
		//pHeader->timestamp = timestamp;
		header.aspect = 1.0f * (float)mWidth / (float)mHeight;
		header.width = (unsigned int)mPreviewWidth;
		header.height = (unsigned int)mPreviewHeight;
		header.internalFormat = internalFormat;
		header.dataSize = compressed_size;
		header.dataFormat = GL_UNSIGNED_BYTE;

		unsigned char *ptr = mCompressedData.data();
		glGetCompressedTexImage(GL_TEXTURE_2D, 0, ptr);

		//printf("compressed size - %d\n", compressed_size);
	}
	else
	{
		GLint internalFormat;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);

		GLint size = mPreviewWidth * mPreviewHeight * 3;
		mCompressedData.resize(size + 1); // sizeof(CompressImageHeader));

		header.aspect = 1.0f * (float)mWidth / (float)mHeight;
		header.width = (unsigned int)mPreviewWidth;
		header.height = (unsigned int)mPreviewHeight;
		header.internalFormat = GL_RGB8;
		header.dataSize = size;
		header.dataFormat = GL_UNSIGNED_BYTE;

		unsigned char *ptr = mCompressedData.data();
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, ptr);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

bool PostEffectBuffers::PreviewCompressBegin()
{
	if (false == mPreviewSignal)
		return false;

	if (0 == mCompressedPreviewId)
		return false;
	if (nullptr == mBufferDownscale.get() || 0 == mBufferDownscale->GetColorObject())
		return false;

	if (0 == mBufferDownscale->GetColorObject())
		return false;

	if (mPreviewWidth <= 1 || mPreviewHeight <= 1)
		return false;

	mUnCompressData.resize(mPreviewWidth * mPreviewHeight * 4);
	mCompressedData.resize(mPreviewWidth * mPreviewHeight / 2);

	const GLuint srcId = mBufferDownscale->GetColorObject();
	glBindTexture(GL_TEXTURE_2D, srcId);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, mUnCompressData.data());
	glBindTexture(GL_TEXTURE_2D, 0);

	int pitch = mPreviewWidth * 4; // * bpp
	CompressImageBegin(mCompressedData.data(), mPreviewWidth, mPreviewHeight, mUnCompressData.data(), pitch, 0);

	mPreviewRunning = false; // true;
	mPreviewSignal = false;
	return true;
}

bool PostEffectBuffers::PreviewCompressEnd()
{
	if (0 == mCompressedPreviewId)
		return false;
	if (nullptr == mBufferDownscale.get() || 0 == mBufferDownscale->GetColorObject())
		return false;

	if (false == mPreviewRunning)
		return false;

	CompressImageEnd();
	
	if (mCompressedData.size() > 0)
	{

		if (0 == mCompressedPreviewId)
		{
			glGenTextures(1, &mCompressedPreviewId);

			glBindTexture(GL_TEXTURE_2D, mCompressedPreviewId);
			glCompressedTexImage2D(GL_TEXTURE_2D, 0, mCompressionInternal, mPreviewWidth, mPreviewHeight, 0, mCompressedData.size(), mCompressedData.data());
			//glTexImage2D(GL_TEXTURE_2D, 0, mCompressionFormat, w, h, 0, GL_RGB, mCompressionType, nullptr);
			glBindTexture(GL_TEXTURE_2D, 0);
			CHECK_GL_ERROR();
		}
		
		glBindTexture(GL_TEXTURE_2D, mCompressedPreviewId);
		glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mPreviewWidth, mPreviewHeight, mCompressionInternal, mCompressedData.size(), mCompressedData.data());
		//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mPreviewWidth, mPreviewHeight, GL_RGB, mCompressionType, mCompressedData.data());
		glBindTexture(GL_TEXTURE_2D, 0);
		CHECK_GL_ERROR();
	}

	return true;
}
*/