
/**	\file	postprocessing_effect.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectbuffers.h"
#include "hashUtils.h"

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
int PostEffectBuffers::GetFlagsForSingleColorBuffer()
{
	return FrameBuffer::eCreateColorTexture | FrameBuffer::eDeleteFramebufferOnCleanup;
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
		if (framebufferEntry.second.isAutoResize)
		{
			framebufferEntry.second.framebuffer->ReSize(w, h);
		}
	}
	
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
	return true;
}

void PostEffectBuffers::FreeBuffers()
{
}

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


FrameBuffer* PostEffectBuffers::RequestFramebuffer(uint32_t nameKey)
{
	//const std::string key = name; // GenerateKey(name, width, height, flags, numColorAttachments);
	//const uint32_t key = xxhash32(name);

	auto it = framebufferPool.find(nameKey);
	if (it == end(framebufferPool))
	{
		auto framebuffer = std::make_unique<FrameBuffer>(mWidth, mHeight);
		framebufferPool[nameKey] = { std::move(framebuffer) }; // , name };
		it = framebufferPool.find(nameKey);
	}

	// increment reference count and return framebuffer
	it->second.AddReference();
	return it->second.framebuffer.get();
}

FrameBuffer* PostEffectBuffers::RequestFramebuffer(
	uint32_t nameKey,
	int width,
	int height,
	int flags,
	int numColorAttachments,
	bool isAutoResize,
	const std::function<void(FrameBuffer*)>& onInit)
{
	//const std::string key = name; // GenerateKey(name, width, height, flags, numColorAttachments);
	//const uint32_t key = xxhash32(name);

	auto it = framebufferPool.find(nameKey);
	if (it == end(framebufferPool) || width != it->second.width || height != it->second.height)
	{
		if (it == end(framebufferPool))
		{
			auto framebuffer = std::make_unique<FrameBuffer>(width, height, flags, numColorAttachments);
			if (onInit)
			{
				onInit(framebuffer.get());
			}
			framebuffer->ReSize(width, height);
			framebufferPool[nameKey] = { std::move(framebuffer), width, height, isAutoResize }; // , name
		}
		else
		{
			it->second.framebuffer.reset(new FrameBuffer(width, height, flags, numColorAttachments));
			if (onInit)
			{
				onInit(it->second.framebuffer.get());
			}
			it->second.width = width;
			it->second.height = height;
			it->second.framebuffer->ReSize(width, height);
			it->second.isAutoResize = isAutoResize;
		}

		it = framebufferPool.find(nameKey);
	}

	// increment reference count and return framebuffer
	it->second.AddReference();
	return it->second.framebuffer.get();
}

void PostEffectBuffers::ReleaseFramebuffer(uint32_t nameKey, bool doRemoveImmidiately)
{
	//const std::string key = name; // GenerateKey(...)
	
	auto it = framebufferPool.find(nameKey);
	if (it != end(framebufferPool))
	{
		// Decrement reference count
		it->second.RemoveReference();

		// if not forced, then delay for n-frame with removal, lazy erase
		if (doRemoveImmidiately && it->second.GetReferenceCount() == 0) 
		{
			framebufferPool.erase(it);
		}
	}
}

void PostEffectBuffers::OnFrameRendered() 
{
	for (auto it = begin(framebufferPool); it != end(framebufferPool); )
	{
		const FramebufferEntry& entry = it->second;

		if (entry.GetReferenceCount() == 0 && entry.ReadyToErase())
		{
			it = framebufferPool.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void PostEffectBuffers::OnContextChanged() {
	// clear all framebuffers
	framebufferPool.clear();
}