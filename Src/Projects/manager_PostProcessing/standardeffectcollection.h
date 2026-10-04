#pragma once

// standardeffect_collection.h
/*
Sergei <Neill3d> Solokhin 2018-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#include "GL/glew.h"

#include "posteffect_shader.h"

#include "posteffectshader_downscale.h"
#include "posteffectshader_lineardepth.h"
#include "posteffectshader_bilateral_blur.h"
#include "posteffectshader_blur_lineardepth.h"
#include "posteffectshader_mix.h"

#include "glslShaderProgram.h"

#include <memory>
#include <bitset>
#include <filesystem>

enum class BuildInEffect : uint8_t
{
	FISHEYE,
	COLOR,
	VIGNETTE,
	FILMGRAIN,
	LENSFLARE,
	SSAO,
	DOF,
	DISPLACEMENT,
	MOTIONBLUR,
	COUNT
};

/**
* Build-in effects collection
* initialized per render context and shared across several view panes (effects chains)
*/
class StandardEffectCollection
{
public:

	PostEffectBufferShader* GetFishEyeEffect() { return mFishEye.get(); }
	const PostEffectBufferShader* GetFishEyeEffect() const { return mFishEye.get(); }
	PostEffectBufferShader* GetColorEffect() { return mColor.get(); }
	const PostEffectBufferShader* GetColorEffect() const { return mColor.get(); }
	PostEffectBufferShader* GetVignettingEffect() { return mVignetting.get(); }
	const PostEffectBufferShader* GetVignettingEffect() const { return mVignetting.get(); }
	PostEffectBufferShader* GetFilmGrainEffect() { return mFilmGrain.get(); }
	const PostEffectBufferShader* GetFilmGrainEffect() const { return mFilmGrain.get(); }
	PostEffectBufferShader* GetLensFlareEffect() { return mLensFlare.get(); }
	const PostEffectBufferShader* GetLensFlareEffect() const { return mLensFlare.get(); }
	PostEffectBufferShader* GetSSAOEffect() { return mSSAO.get(); }
	const PostEffectBufferShader* GetSSAOEffect() const { return mSSAO.get(); }
	PostEffectBufferShader* GetDOFEffect() { return mDOF.get(); }
	const PostEffectBufferShader* GetDOFEffect() const { return mDOF.get(); }
	PostEffectBufferShader* GetDisplacementEffect() { return mDisplacement.get(); }
	const PostEffectBufferShader* GetDisplacementEffect() const { return mDisplacement.get(); }
	PostEffectBufferShader* GetMotionBlurEffect() { return mMotionBlur.get(); }
	const PostEffectBufferShader* GetMotionBlurEffect() const { return mMotionBlur.get(); }

	PostEffectBufferShader* ShaderFactory(
		BuildInEffect effectType,
		FBComponent* owner,
		const std::filesystem::path& shadersLocation,
		bool immediatelyLoad = true);

	const EffectShaderBlurLinearDepth* GetEffectBlurLinearDepth() const { return mEffectBlur.get(); }
	EffectShaderBlurLinearDepth* GetEffectBlurLinearDepth() { return mEffectBlur.get(); }
	const EffectShaderMix* GetEffectMix() const { return mEffectMix.get(); }
	EffectShaderMix* GetEffectMix() { return mEffectMix.get(); }
	const PostEffectShaderLinearDepth* GetShaderLinearDepth() const { return mEffectDepthLinearize.get(); }
	PostEffectShaderLinearDepth* GetShaderLinearDepth() { return mEffectDepthLinearize.get(); }

	void ChangeContext();
	// check reload of shaders was requested, then reload them
	bool ReloadShaders();

	// returns true if all shaders are loaded and compiled, ready to use
	bool IsOk() const;

	bool IsNeedToReloadShaders() const { return mNeedReloadShaders; }

	bool LoadShaders();
	void FreeShaders();

	// build-in effects
	std::unique_ptr<PostEffectBufferShader>		mFishEye;
	std::unique_ptr<PostEffectBufferShader>		mColor;
	std::unique_ptr<PostEffectBufferShader>		mVignetting;
	std::unique_ptr<PostEffectBufferShader>		mFilmGrain;
	std::unique_ptr<PostEffectBufferShader>		mLensFlare;
	std::unique_ptr<PostEffectBufferShader>		mSSAO;
	std::unique_ptr<PostEffectBufferShader>		mDOF;
	std::unique_ptr<PostEffectBufferShader>		mDisplacement;
	std::unique_ptr<PostEffectBufferShader>		mMotionBlur;

	// shared shaders

	std::unique_ptr<PostEffectShaderLinearDepth>		mEffectDepthLinearize;	//!< linearize depth for other filters (DOF, SSAO, Bilateral Blur, etc.)
	std::unique_ptr<EffectShaderBlurLinearDepth>		mEffectBlur;		//!< bilateral blur effect, for SSAO
	//std::unique_ptr<GLSLShaderProgram>		mShaderImageBlur;	//!< for masking
	std::unique_ptr<PostEffectShaderBilateralBlur>		mEffectBilateralBlur; //!< for masking
	std::unique_ptr<EffectShaderMix>					mEffectMix;			//!< multiplication result of two inputs, (for SSAO)
	std::unique_ptr<PostEffectShaderDownscale>			mEffectDownscale; // effect for downscaling the preview image (send to client)

	std::unique_ptr<GLSLShaderProgram>			mShaderSceneMasked; //!< render models into mask with some additional filtering

private:

	bool mNeedReloadShaders{ true };

	static bool CheckShadersPath(const std::filesystem::path& path);

};