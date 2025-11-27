#pragma once

// postprocessing_effectChain
/*
Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#include "GL/glew.h"

#include "graphics_framebuffer.h"
#include "postpersistentdata.h"
#include "posteffectbase.h"
#include "posteffectbuffers.h"
#include "shaderpropertystorage.h"

#include "glslShaderProgram.h"
#include "Framebuffer.h"

#include "posteffectshader_bilateral_blur.h"

#include <atomic>
#include <memory>
#include <bitset>

// forward
class PostEffectBuffers;
class StandardEffectCollection;
class PostEffectContextMoBu;

/// <summary>
/// chain of post processing effects, apply effects in an order
/// </summary>
class PostEffectChain
{
public:
	PostEffectChain();
	virtual ~PostEffectChain() = default;

	void Evaluate(StandardEffectCollection& effectCollection, PostEffectContextMoBu* effectContext);
	void Synchronize();

	void ChangeContext();
	/// w,h - local buffer size for processing, pCamera - current pane camera for processing
	bool Prep(PostPersistentData *pData, const PostEffectContextMoBu* effectContext);

	bool BeginFrame(PostEffectBuffers* buffers);

	/// <summary>
	/// render each effect with a defined order
	/// </summary>
	bool Process(
		PostEffectBuffers* buffers, 
		double systemTime, 
		PostEffectContextMoBu* effectContext, 
		const StandardEffectCollection& effectCollection);

	bool IsCompressedDataReady() const
	{
		return mIsCompressedDataReady;
	}

	
	PingPongData* GetPingPongDataPtr() { return &mDoubleBufferPingPongData; }

protected:

	HdlFBPlugTemplate<PostPersistentData>	mSettings;
	FBCamera* mLastCamera{ nullptr };

	// shaders to evaluate
	struct RenderData
	{
		// prepared effects to be processed during render
		std::vector<PostEffectBase*>		mChain; 
		
		bool isReady = false;

		bool isMaskTextureBinded = false;
		bool isMaskBlurRequested = false;
		bool isMaskMixRequested = false;

		bool maskRenderFlags[PostPersistentData::NUMBER_OF_MASKS] = { false, false, false, false };

		bool isDepthSamplerBinded = false;
		bool isLinearDepthSamplerBinded = false;
		bool isWorldNormalSamplerBinded = false;
	};

	RenderData								mRenderData[2]; // double buffered from evaluation thread

	// 0 or 1, which buffer is currently “active”
	std::atomic<uint8_t>					gActiveData{ 0 };

	
	bool							mIsCompressedDataReady{ false };
	double							mLastCompressTime{ 0.0 };

	PingPongData					mDoubleBufferPingPongData;

private:
	
	static uint32_t DOUBLE_BUFFER_NAME_KEY;
	static uint32_t MASK_BUFFER_NAME_KEY;
	static uint32_t DEPTH_LINEAR_BUFFER_NAME_KEY;

	/// <summary>
	/// prepare /ref mChain order of effects for rendering
	/// blurAndMix - index of effect where bilateral blur is requested (SSAO)
	/// blurAndMix2 - index of effect where bilateral blur and mix is requested (Bloom for ColorCorrection)
	/// </summary>
	/// <returns>true if chain of effects is not empty</returns>
	bool PrepareChainOrder(std::vector<PostEffectBase*>& chain, StandardEffectCollection* collection);

	FrameBuffer* RequestMaskFrameBuffer(PostEffectBuffers* buffers);
	void ReleaseMaskFrameBuffer(PostEffectBuffers* buffers);
	
	friend class MaskFramebufferRequestScope;

	FrameBuffer* RequestDoubleFrameBuffer(PostEffectBuffers* buffers);
	void ReleaseDoubleFrameBuffer(PostEffectBuffers* buffers);

	friend class DoubleFramebufferRequestScope;

	/// <summary>
	/// render a linear depth (for SSAO)
	/// @param effect with a shader to render a linear depth
	/// @param makeDownscale - if true, the depth will be downscaled to half-size
	/// </summary>
	void RenderLinearDepth(PostEffectBuffers* buffers, PostEffectBase* effect, const GLuint depthId, bool makeDownscale, PostEffectContextMoBu& effectContext);

	void RenderWorldNormals(PostEffectBuffers* buffers);

	/// <summary>
	/// when a blur is used in any of masks
	/// </summary>
	void BlurMasksPass(const int maskIndex, PostEffectBuffers* buffers, PostEffectBilateralBlur* effect, const PostEffectContextMoBu& effectContext);

	/// <summary>
	/// mix masks = mask A * mask B
	///  result is written back to mask A color attachment
	/// </summary>
	void MixMasksPass(GLSLShaderProgram* shader, const int maskindex, const int maskIndex2, PostEffectBuffers* buffers);

	/// <summary>
	/// send a packet with final post processed image
	/// </summary>
	void SendPreview(PostEffectBuffers* buffers, double systime);

	/// <summary>
	/// return true if a global masking active and any of effects uses a masking
	/// </summary>
	bool HasAnyMaskUsedByEffect() const;

	/// <summary>
	/// return true if global masking channel or any effect channel is equal to a given one
	/// </summary>
	bool IsMaskUsedByEffect(const EMaskingChannel maskId) const;

	/// <summary>
	/// return true if there is any model with FXMaskingShader, so that we could render it into a masking texture
	/// </summary>
	bool HasAnyObjectMasked() const;

	/// <summary>
	/// return true if there is a model with FBMaskingShader, so that a defined mask channel there equal to a given one
	/// </summary>
	bool IsAnyObjectMaskedByMaskId(const EMaskingChannel maskId) const;

	void RenderSceneMaskToTexture(GLSLShaderProgram* shader, const int maskIndex, PostPersistentData::SMaskProperties& maskProps, PostEffectBuffers* buffers);
};


/// <summary>
/// a local scope of request / release of framebuffer with MAX_MASKS (4) color attachment for masking
///  + 1 color attachment for intermediate processing
/// </summary>
class MaskFramebufferRequestScope
{
public:
	MaskFramebufferRequestScope(PostEffectChain* effectChainIn, PostEffectBuffers* buffersIn)
		: effectChain(effectChainIn)
		, buffers(buffersIn)
	{
		maskFB = effectChain->RequestMaskFrameBuffer(buffers);
	}

	FrameBuffer* GetFrameBufferPtr() { return maskFB; }

	FrameBuffer* operator->() {
		return maskFB;
	}

	FrameBuffer& operator*() {
		return *maskFB;
	}

	virtual ~MaskFramebufferRequestScope()
	{
		effectChain->ReleaseMaskFrameBuffer(buffers);
	}

private:
	PostEffectChain* effectChain;
	PostEffectBuffers* buffers;

	FrameBuffer* maskFB;
};

/// <summary>
/// a local scope of request / release of framebuffer with 2 color attachments and FramebufferPingPongHelper
/// </summary>
class DoubleFramebufferRequestScope
{
public:
	DoubleFramebufferRequestScope(PostEffectChain* effectChainIn, PostEffectBuffers* buffersIn)
		: effectChain(effectChainIn)
		, buffers(buffersIn)
	{
		doubleFB = effectChain->RequestDoubleFrameBuffer(buffers);
		pingPongHelper = new FramebufferPingPongHelper(doubleFB, effectChain->GetPingPongDataPtr());
	}

	FramebufferPingPongHelper* GetPtr() { return pingPongHelper; }

	FramebufferPingPongHelper* operator->() {
		return pingPongHelper;
	}

	FramebufferPingPongHelper& operator*() {
		return *pingPongHelper;
	}

	virtual ~DoubleFramebufferRequestScope()
	{
		delete pingPongHelper;
		pingPongHelper = nullptr;
		effectChain->ReleaseDoubleFrameBuffer(buffers);
	}

private:
	PostEffectChain* effectChain;
	PostEffectBuffers* buffers;

	FrameBuffer* doubleFB;
	FramebufferPingPongHelper* pingPongHelper;
};