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
#include "posteffectcontextmobu.h"
#include "posteffectbuffers.h"

#include "posteffectshader_downscale.h"
#include "posteffectshader_lineardepth.h"
#include "posteffectshader_bilateral_blur.h"
#include "posteffectshader_blur_lineardepth.h"

#include "glslShaderProgram.h"
#include "Framebuffer.h"

#include <memory>
#include <bitset>

// forward
class PostEffectBuffers;

/// <summary>
/// chain of post processing effects, apply effects in an order
/// </summary>
class PostEffectChain
{
public:
	PostEffectChain();
	virtual ~PostEffectChain() = default;

	void ChangeContext();
	/// w,h - local buffer size for processing, pCamera - current pane camera for processing
	bool Prep(PostPersistentData *pData, const PostEffectContextMoBu& effectContext);

	bool BeginFrame(PostEffectBuffers* buffers);

	/// <summary>
	/// render each effect with a defined order
	/// </summary>
	bool Process(PostEffectBuffers* buffers, double time, const PostEffectContextMoBu& effectContext);

	bool IsCompressedDataReady() const
	{
		return mIsCompressedDataReady;
	}

	const PostEffectBase* GetFishEyeEffect() const { return mFishEye.get(); }
	const PostEffectBase* GetColorEffect() const { return mColor.get(); }
	const PostEffectBase* GetVignettingEffect() const { return mVignetting.get(); }
	const PostEffectBase* GetFilmGrainEffect() const { return mFilmGrain.get(); }
	const PostEffectBase* GetLensFlareEffect() const { return mLensFlare.get(); }
	const PostEffectBase* GetSSAOEffect() const { return mSSAO.get(); }
	const PostEffectBase* GetDOFEffect() const { return mDOF.get(); }
	const PostEffectBase* GetDisplacementEffect() const { return mDisplacement.get(); }
	const PostEffectBase* GetMotionBlurEffect() const { return mMotionBlur.get(); }

	PingPongData* GetPingPongDataPtr() { return &mDoubleBufferPingPongData; }

protected:

	FBSystem								mSystem;
	HdlFBPlugTemplate<PostPersistentData>	mSettings;
	FBCamera* mLastCamera{ nullptr };

	// build-in effects
	std::unique_ptr<PostEffectBase>		mFishEye;
	std::unique_ptr<PostEffectBase>		mColor;
	std::unique_ptr<PostEffectBase>		mVignetting;
	std::unique_ptr<PostEffectBase>		mFilmGrain;
	std::unique_ptr<PostEffectBase>		mLensFlare;
	std::unique_ptr<PostEffectBase>		mSSAO;
	std::unique_ptr<PostEffectBase>		mDOF;
	std::unique_ptr<PostEffectBase>		mDisplacement;
	std::unique_ptr<PostEffectBase>		mMotionBlur;

	// shared shaders
	
	std::unique_ptr<PostEffectLinearDepth>		mEffectDepthLinearize;	//!< linearize depth for other filters (DOF, SSAO, Bilateral Blur, etc.)
	std::unique_ptr<PostEffectBlurLinearDepth>	mEffectBlur;		//!< bilateral blur effect, for SSAO
	//std::unique_ptr<GLSLShaderProgram>		mShaderImageBlur;	//!< for masking
	std::unique_ptr<PostEffectBilateralBlur>	mEffectBilateralBlur; //!< for masking
	std::unique_ptr<GLSLShaderProgram>			mShaderMix;			//!< multiplication result of two inputs, (for SSAO)
	std::unique_ptr<PostEffectDownscale>		mEffectDownscale; // effect for downscaling the preview image (send to client)

	std::unique_ptr<GLSLShaderProgram>			mShaderSceneMasked; //!< render models into mask with some additional filtering

	PingPongData					mDoubleBufferPingPongData;

	bool							mNeedReloadShaders{ true };
	bool							mIsCompressedDataReady{ false };
	double							mLastCompressTime{ 0.0 };

	PostEffectBase* ShaderFactory(const BuildInEffect effectType, const char *shadersLocation, bool immediatelyLoad=true);

	bool LoadShaders();
	void FreeShaders();

private:
	static bool CheckShadersPath(const char* path);

	/// <summary>
	/// prepare /ref mChain order of effects for rendering
	/// blurAndMix - index of effect where bilateral blur is requested (SSAO)
	/// blurAndMix2 - index of effect where bilateral blur and mix is requested (Bloom for ColorCorrection)
	/// </summary>
	/// <returns>true if chain of effects is not empty</returns>
	bool PrepareChainOrder(std::vector<PostEffectBase*>& chain, int& blurAndMix, int& blurAndMix2);

	FrameBuffer* RequestMaskFrameBuffer(PostEffectBuffers* buffers);
	void ReleaseMaskFrameBuffer(PostEffectBuffers* buffers);
	
	friend class MaskFramebufferRequestScope;

	FrameBuffer* RequestDoubleFrameBuffer(PostEffectBuffers* buffers);
	void ReleaseDoubleFrameBuffer(PostEffectBuffers* buffers);

	friend class DoubleFramebufferRequestScope;

	/// <summary>
	/// render a linear depth (for SSAO)
	/// </summary>
	void RenderLinearDepth(PostEffectBuffers* buffers, const GLuint depthId, const PostEffectContextMoBu& effectContext);

	void RenderWorldNormals(PostEffectBuffers* buffers);

	/// <summary>
	/// this is a pass of bluring the image for SSAO
	/// </summary>
	void BilateralBlurPass(PostEffectBuffers* buffers);

	/// <summary>
	/// this is a pass of bluring for Bloom (Color Correction)
	/// </summary>
	void BilateralBlurAndMixPass(PostEffectBuffers* buffers);

	/// <summary>
	/// when a blur is used in any of masks
	/// </summary>
	void BlurMasksPass(const int maskIndex, PostEffectBuffers* buffers, const PostEffectContextMoBu& effectContext);

	/// <summary>
	/// mix masks = mask A * mask B
	///  result is written back to mask A color attachment
	/// </summary>
	void MixMasksPass(const int maskindex, const int maskIndex2, PostEffectBuffers* buffers);

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

	void RenderSceneMaskToTexture(const int maskIndex, PostPersistentData::SMaskProperties& maskProps, PostEffectBuffers* buffers);
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