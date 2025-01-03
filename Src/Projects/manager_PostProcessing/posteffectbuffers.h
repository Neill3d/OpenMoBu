#pragma once

// postprocessing_effectChain
/*
Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#include "GL/glew.h"

#include "graphics_framebuffer.h"
#include "postprocessing_helper.h"

#include "glslShaderProgram.h"
#include "Framebuffer.h"

#include <memory>
#include <bitset>
#include <string>


class PingPongData
{
private:
	int readAttachment; //!< index of the current read attachment
	int writeAttachment; //!< index of the current write attachment

public:
	PingPongData()
		: readAttachment(0), writeAttachment(1)
	{}

	int GetReadAttachment() const { return readAttachment; }
	int GetWriteAttachment() const { return writeAttachment; }

	void Swap() { std::swap(readAttachment, writeAttachment); }
};

// a framebuffer with 2 attachments, so that we could read from one attachment and write into another, then swap
class FramebufferPingPongHelper
{
private:
	FrameBuffer* fb;
	PingPongData* data;

public:
	FramebufferPingPongHelper(FrameBuffer* framebufferIn, PingPongData* dataIn)
		: fb(framebufferIn), data(dataIn)
	{}

	int GetReadAttachment() const { return data->GetReadAttachment(); }
	int GetWriteAttachment() const { return data->GetWriteAttachment(); }

	FrameBuffer* GetPtr() { return fb; }

	void Swap() { data->Swap(); }

	GLuint GetReadColorObject() const { return fb->GetColorObject(data->GetReadAttachment()); }

	void Bind() const {
		fb->Bind(data->GetWriteAttachment());
	}
	void UnBind(bool generateMips=false) const {
		fb->UnBind(generateMips);
	}
};

class IFramebufferProvider {
public:
	virtual ~IFramebufferProvider() = default;

	virtual FrameBuffer* RequestFramebuffer(const std::string& name) = 0;

	// Request a framebuffer with specific dimensions or properties
	virtual FrameBuffer* RequestFramebuffer(
		const std::string& name, 
		int width, 
		int height, 
		int flags, 
		int numColorAttachments,
		bool isAutoResize,
		const std::function<void(FrameBuffer*)>& onInit=nullptr) = 0;

	virtual void OnFrameRendered() = 0;

	// Notify context change
	virtual void OnContextChanged() = 0;
};

///////////////////////////
// double buffer for effect chain



/// <summary>
/// manage framebuffer resources for a given context
///  also provide
///  - double buffers for effect chain
///  - framebuffer provider
/// </summary>
class PostEffectBuffers : public IFramebufferProvider
{
public:

	//! a constructor
	PostEffectBuffers();
	//! a destructor
	~PostEffectBuffers();

	void ChangeContext();

	bool ReSize(const int w, const int h, bool useScale, double scaleFactor, bool filterMips=false);

	bool Ok();

	//const GLuint PrepAndGetBufferObject();

	//FrameBuffer* GetSrcBufferPtr();
	//FrameBuffer* GetDstBufferPtr();

	//FrameBuffer* GetBufferDepthPtr();
	//FrameBuffer* GetBufferBlurPtr();
	//FrameBuffer* GetBufferMaskPtr();

	// TODO: request a framebuffer ?!

	//FrameBuffer *GetBufferDownscalePtr();

	//void SwapBuffers();
	
	const int GetWidth() const {
		return mWidth;
	}
	const int GetHeight() const {
		return mHeight;
	}
	const unsigned int GetPreviewWidth() const {
		return mPreviewWidth;
	}
	const unsigned int GetPreviewHeight() const {
		return mPreviewHeight;
	}
	// get a result of effect computation
	//const GLuint GetFinalColor();
	//const GLuint GetFinalFBO();

	//const GLuint GetPreviewColor();
	//const GLuint GetPreviewFBO();

	void		PreviewSignal() {
		mPreviewSignal = true;
	}

	//bool		PreviewCompressBegin();
	//bool		PreviewCompressEnd();
	//void		PrepPreviewCompressed();

	bool		PreviewOpenGLCompress(EImageCompression	compressionType, GLint &compressionCode);

	const GLuint GetPreviewCompressedColor();

	//void MapCompressedData(const float timestamp, Network::CPacketImageHeader &header);

	const size_t GetCompressedSize() const {
		return mCompressedSize;
	}
	const size_t GetUnCompressedSize() const {
		return mUnCompressSize;
	}

	static int GetFlagsForMainColorBuffer();
	static int GetFlagsForSingleColorBuffer();
	static void SetParametersForMainColorBuffer(FrameBuffer* buffer, bool filterMips);
	static void SetParametersForMainDepthBuffer(FrameBuffer* buffer);

	FrameBuffer* RequestFramebuffer(const std::string& name) override
	{
		const std::string key = name; // GenerateKey(name, width, height, flags, numColorAttachments);

		auto it = framebufferPool.find(key);
		if (it == end(framebufferPool))
		{
			auto framebuffer = std::make_unique<FrameBuffer>(mWidth, mHeight);
			framebufferPool[key] = { std::move(framebuffer), name };
			it = framebufferPool.find(key);
		}

		// increment reference count and return framebuffer
		it->second.AddReference();
		return it->second.framebuffer.get();
	}

	FrameBuffer* RequestFramebuffer(
		const std::string& name, 
		int width, 
		int height, 
		int flags, 
		int numColorAttachments,
		bool isAutoResize,
		const std::function<void(FrameBuffer*)>& onInit=nullptr) override
	{
		const std::string key = name; // GenerateKey(name, width, height, flags, numColorAttachments);

		auto it = framebufferPool.find(key);
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
				framebufferPool[key] = { std::move(framebuffer), name, width, height, isAutoResize };
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
			
			it = framebufferPool.find(key);
		}

		// increment reference count and return framebuffer
		it->second.AddReference();
		return it->second.framebuffer.get();
	}

	void ReleaseFramebuffer(const std::string& name)
	{
		const std::string key = name; // GenerateKey(...)

		auto it = framebufferPool.find(key);
		if (it != end(framebufferPool))
		{
			// Decrement reference count
			it->second.RemoveReference();

			if (it->second.GetReferenceCount() == 0) {
				// delay for n-frame with removal, lazy erase
				//framebufferPool.erase(it);
			}
		}
	}

	void OnFrameRendered() override {
		std::vector<std::string> listToErase;
		for (auto& entry : framebufferPool)
		{
			if (entry.second.GetReferenceCount() == 0)
			{
				if (entry.second.ReadyToErase())
				{
					listToErase.push_back(entry.first);
				}
			}
		}
		if (!listToErase.empty())
		{
			for (const auto& key : listToErase)
			{
				auto it = framebufferPool.find(key);
				if (it != end(framebufferPool))
				{
					framebufferPool.erase(it);
				}
			}
		}
	}

	void OnContextChanged() override {
		// clear all framebuffers
		framebufferPool.clear();
	}

private:

	struct FramebufferEntry {
		std::unique_ptr<FrameBuffer> framebuffer;
		std::string name;
		int width{ 1 };
		int height{ 1 };
		bool isAutoResize{ true };
		int referenceCount{ 0 };
		int lazyEraseCounter{ 15 };
		/*
		FramebufferEntry(std::unique_ptr<FrameBuffer>&& framebufferIn, const std::string& nameIn)
			: framebuffer(std::move(framebufferIn))
			, name(nameIn)
		{}
		*/
		void AddReference() { ++referenceCount; lazyEraseCounter = 15; }
		void RemoveReference() { if (referenceCount > 0) --referenceCount; }
		int GetReferenceCount() const { return referenceCount; }

		bool ReadyToErase() { --lazyEraseCounter; return lazyEraseCounter <= 0; }
	};

	std::unordered_map<std::string, FramebufferEntry> framebufferPool;
	
	std::string GenerateKey(const std::string& name, int width, int height, int flags, int numAttachments) {
		return name + "(" + std::to_string(flags) + "):" + std::to_string(numAttachments) + "x" + std::to_string(width) + "x" + std::to_string(height);
	}

protected:

	// DONE: double local buffer
	//std::unique_ptr<FrameBuffer>			mBufferPost0;
	//std::unique_ptr<FrameBuffer>			mBufferPost1;

	//std::unique_ptr<FrameBuffer>			mBufferDepth;		//!< buffer to store a linearize depth
	//std::unique_ptr<FrameBuffer>			mBufferBlur;

	//std::unique_ptr<FrameBuffer>			mBufferDownscale;	//!< output for a preview

	//std::unique_ptr<FrameBuffer>			mBufferMasking;		//!< render models into mask texture

	// last local buffers resize
	int								mWidth;
	int								mHeight;
	GLuint							mOutputColorObject;

	bool							mPreviewSignal;
	bool							mPreviewRunning;

	// downscaled size
	unsigned int					mPreviewWidth;
	unsigned int					mPreviewHeight;

	int			mSrc;
	int			mDst;

	// compressed ETC1 output texture
	GLenum								mCompressionInternal;
	GLenum								mCompressionFormat;
	GLenum								mCompressionType;
	GLuint								mCompressedPreviewId;

	GLuint								mCompressOnFlyId;

	int									mCurPBO;
	GLuint								mPBOs[2];

	// temp
	int									mCurUnPack;
	GLuint								mUnPackPBOs[2];

	size_t								mUnCompressSize;
	size_t								mCompressedSize;

	//CompressImageHeader				mCompressHeader;

	void		FreeBuffers();
	void AllocPreviewTexture(int w, int h);
	void		FreeTextures();
};
