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

#include "posteffectbase.h"

#include "posteffectshader_downscale.h"
#include "posteffectshader_lineardepth.h"
#include "posteffectshader_bilateral_blur.h"
#include "posteffectshader_blur_lineardepth.h"
#include "posteffectshader_mix.h"

#include "glslShaderProgram.h"

#include <memory>
#include <bitset>

/**
* Build-in effects collection
* initialized per render context and shared across several view panes (effects chains)
*/
class StandardEffectCollection
{
public:

	PostEffectBase* GetFishEyeEffect() { return mFishEye.get(); }
	const PostEffectBase* GetFishEyeEffect() const { return mFishEye.get(); }
	PostEffectBase* GetColorEffect() { return mColor.get(); }
	const PostEffectBase* GetColorEffect() const { return mColor.get(); }
	PostEffectBase* GetVignettingEffect() { return mVignetting.get(); }
	const PostEffectBase* GetVignettingEffect() const { return mVignetting.get(); }
	PostEffectBase* GetFilmGrainEffect() { return mFilmGrain.get(); }
	const PostEffectBase* GetFilmGrainEffect() const { return mFilmGrain.get(); }
	PostEffectBase* GetLensFlareEffect() { return mLensFlare.get(); }
	const PostEffectBase* GetLensFlareEffect() const { return mLensFlare.get(); }
	PostEffectBase* GetSSAOEffect() { return mSSAO.get(); }
	const PostEffectBase* GetSSAOEffect() const { return mSSAO.get(); }
	PostEffectBase* GetDOFEffect() { return mDOF.get(); }
	const PostEffectBase* GetDOFEffect() const { return mDOF.get(); }
	PostEffectBase* GetDisplacementEffect() { return mDisplacement.get(); }
	const PostEffectBase* GetDisplacementEffect() const { return mDisplacement.get(); }
	PostEffectBase* GetMotionBlurEffect() { return mMotionBlur.get(); }
	const PostEffectBase* GetMotionBlurEffect() const { return mMotionBlur.get(); }

	PostEffectBase* ShaderFactory(const BuildInEffect effectType, const char* shadersLocation, bool immediatelyLoad = true);

	void ChangeContext();
	bool Prep(PostPersistentData* pData);

	bool LoadShaders();
	void FreeShaders();

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
	std::unique_ptr<PostEffectMix>				mEffectMix;			//!< multiplication result of two inputs, (for SSAO)
	std::unique_ptr<PostEffectDownscale>		mEffectDownscale; // effect for downscaling the preview image (send to client)

	std::unique_ptr<GLSLShaderProgram>			mShaderSceneMasked; //!< render models into mask with some additional filtering

private:

	bool mNeedReloadShaders{ true };

	static bool CheckShadersPath(const char* path);

};