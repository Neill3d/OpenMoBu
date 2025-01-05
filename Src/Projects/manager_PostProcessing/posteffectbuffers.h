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

	virtual void ReleaseFramebuffer(const std::string& name) = 0;

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

	
	const int GetWidth() const { return mWidth; }
	const int GetHeight() const { return mHeight; }
	const unsigned int GetPreviewWidth() const { return mPreviewWidth; }
	const unsigned int GetPreviewHeight() const { return mPreviewHeight; }
	
	void		PreviewSignal() { mPreviewSignal = true; }

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

	//
	// IFramebufferProvider

	FrameBuffer* RequestFramebuffer(const std::string& name) override;

	FrameBuffer* RequestFramebuffer(
		const std::string& name,
		int width,
		int height,
		int flags,
		int numColorAttachments,
		bool isAutoResize,
		const std::function<void(FrameBuffer*)>& onInit = nullptr) override;
	
	void ReleaseFramebuffer(const std::string& name) override;

	void OnFrameRendered() override;

	void OnContextChanged() override;

private:

	struct FramebufferEntry {
		std::unique_ptr<FrameBuffer> framebuffer;
		std::string name;
		int width{ 1 };
		int height{ 1 };
		bool isAutoResize{ true };
		int referenceCount{ 0 };
		int lazyEraseCounter{ 15 };
		
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
