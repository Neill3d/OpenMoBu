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

#include "glslShader.h"
#include "Framebuffer.h"

#include <memory>
#include <bitset>

///////////////////////////
// double buffer for effect chain

class PostEffectBuffers
{
public:

	//! a constructor
	PostEffectBuffers();
	//! a destructor
	~PostEffectBuffers();

	void ChangeContext();

	bool ReSize(const int w, const int h, bool useScale, double scaleFactor, bool filterMips=false);

	bool Ok();

	const GLuint PrepAndGetBufferObject();

	FrameBuffer *GetSrcBufferPtr();
	FrameBuffer *GetDstBufferPtr();

	FrameBuffer *GetBufferDepthPtr();
	FrameBuffer *GetBufferBlurPtr();
	FrameBuffer* GetBufferMaskPtr();

	FrameBuffer *GetBufferDownscalePtr();

	void SwapBuffers();

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
	const GLuint GetFinalColor();
	const GLuint GetFinalFBO();

	const GLuint GetPreviewColor();
	const GLuint GetPreviewFBO();

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
protected:

	// DONE: double local buffer
	std::unique_ptr<FrameBuffer>			mBufferPost0;
	std::unique_ptr<FrameBuffer>			mBufferPost1;

	std::unique_ptr<FrameBuffer>			mBufferDepth;		//!< buffer to store a linearize depth
	std::unique_ptr<FrameBuffer>			mBufferBlur;

	std::unique_ptr<FrameBuffer>			mBufferDownscale;	//!< output for a preview

	std::unique_ptr<FrameBuffer>			mBufferMasking;		//!< render models into mask texture

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
