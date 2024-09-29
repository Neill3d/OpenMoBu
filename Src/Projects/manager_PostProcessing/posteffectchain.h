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
#include "postpersistentdata.h"
#include "posteffectbase.h"

#include "glslShader.h"
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
	//! a constructor
	PostEffectChain();
	//! a destructor
	~PostEffectChain();

	void ChangeContext();
	/// w,h - local buffer size for processing, pCamera - current pane camera for processing
	bool Prep(PostPersistentData *pData, int w, int h, FBCamera *pCamera);

	bool BeginFrame(PostEffectBuffers* buffers);

	/// <summary>
	/// render each effect with a defined order
	/// </summary>
	bool Process(PostEffectBuffers* buffers, double time);

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

protected:

	FBSystem								mSystem;
	HdlFBPlugTemplate<PostPersistentData>	mSettings;
	FBCamera								*mLastCamera;

	// instances of each effect
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
	
	std::unique_ptr<GLSLShader>			mShaderDepthLinearize;	//!< linearize depth for other filters (DOF, SSAO, Bilateral Blur, etc.)
	std::unique_ptr<GLSLShader>			mShaderBlur;		//!< bilateral blur effect, for SSAO
	std::unique_ptr<GLSLShader>			mShaderImageBlur;	//!< for masking
	std::unique_ptr<GLSLShader>			mShaderMix;			//!< multiplication result of two inputs, (for SSAO)
	std::unique_ptr<GLSLShader>			mShaderDownscale;

	std::unique_ptr<GLSLShader>			mShaderSceneMasked; //!< render models into mask with some additional filtering

	// order execution chain
	std::vector<PostEffectBase*>		mChain;

	GLint							mLocDepthLinearizeClipInfo{ -1 };
	GLint							mLocBlurSharpness{ -1 };
	GLint							mLocBlurRes{ -1 };
	GLint							mLocImageBlurScale{ -1 };

	bool							mNeedReloadShaders{ true };
	bool							mIsCompressedDataReady{ false };
	double							mLastCompressTime{ 0.0 };

	PostEffectBase *ShaderFactory(const int type, const char *shadersLocation);

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
	bool PrepareChainOrder(int& blurAndMix, int& blurAndMix2);

	/// <summary>
	/// render a linear depth (for SSAO)
	/// </summary>
	void RenderLinearDepth(PostEffectBuffers* buffers);

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
	void BlurMasksPass(const int maskIndex, PostEffectBuffers* buffers);

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
