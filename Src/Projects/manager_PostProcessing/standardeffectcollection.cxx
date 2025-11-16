
/**	\file	postprocessing_effect.cxx

Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "standardeffectcollection.h"
#include "posteffectssao.h"
#include "posteffectdisplacement.h"
#include "posteffectmotionblur.h"
#include "posteffectlensflare.h"
#include "posteffectcolor.h"
#include "posteffectdof.h"
#include "posteffectfilmgrain.h"
#include "posteffectfisheye.h"
#include "posteffectvignetting.h"
#include "postprocessing_helper.h"
#include "fxmaskingshader.h"
#include "posteffectbase.h"
#include "posteffectshader_bilateral_blur.h"
#include "postpersistentdata.h"

#include "mobu_logging.h"
#include <FileUtils.h>

// shared shaders

#define SHADER_DEPTH_LINEARIZE_VERTEX		"\\GLSL\\simple.vsh"
#define SHADER_DEPTH_LINEARIZE_FRAGMENT		"\\GLSL\\depthLinearize.fsh"

// this is a depth based blur, fo SSAO
#define SHADER_BLUR_VERTEX					"\\GLSL\\simple.vsh"
#define SHADER_BLUR_FRAGMENT				"\\GLSL\\blur.fsh"

// this is a simple gaussian image blur
#define SHADER_IMAGE_BLUR_VERTEX			"\\GLSL\\simple.vsh"
#define SHADER_IMAGE_BLUR_FRAGMENT			"\\GLSL\\imageBlur.glslf"

#define SHADER_MIX_VERTEX					"\\GLSL\\simple.vsh"
#define SHADER_MIX_FRAGMENT					"\\GLSL\\mix.fsh"

#define SHADER_DOWNSCALE_VERTEX				"\\GLSL\\downscale.vsh"
#define SHADER_DOWNSCALE_FRAGMENT			"\\GLSL\\downscale.fsh"

#define SHADER_SCENE_MASKED_VERTEX			"\\GLSL\\scene_masked.glslv"
#define SHADER_SCENE_MASKED_FRAGMENT		"\\GLSL\\scene_masked.glslf"


void StandardEffectCollection::ChangeContext()
{
	FreeShaders();
	mNeedReloadShaders = true;
}

bool StandardEffectCollection::Prep(PostPersistentData* pData)
{
	bool lSuccess = true;

	if (pData)
	{
		if (pData->IsNeedToReloadShaders())
		{
			mNeedReloadShaders = true;
			pData->SetReloadShadersState(false);
		}
	}
	else
	{
		lSuccess = false;
	}
	
	if (mNeedReloadShaders)
	{
		if (!LoadShaders())
			lSuccess = false;

		mNeedReloadShaders = false;
	}

	return lSuccess;
}


PostEffectBase* StandardEffectCollection::ShaderFactory(const BuildInEffect effectType, const char *shadersLocation, bool immediatelyLoad)
{
	PostEffectBase* newEffect = nullptr;

	switch (effectType)
	{
	case BuildInEffect::FISHEYE:
		newEffect = new PostEffectFishEye();
		break;
	case BuildInEffect::COLOR:
		newEffect = new PostEffectColor();
		break;
	case BuildInEffect::VIGNETTE:
		newEffect = new PostEffectVignetting();
		break;
	case BuildInEffect::FILMGRAIN:
		newEffect = new PostEffectFilmGrain();
		break;
	case BuildInEffect::LENSFLARE:
		newEffect = new PostEffectLensFlare();
		break;
	case BuildInEffect::SSAO:
		newEffect = new PostEffectSSAO();
		break;
	case BuildInEffect::DOF:
		newEffect = new PostEffectDOF();
		break;
	case BuildInEffect::DISPLACEMENT:
		newEffect = new PostEffectDisplacement();
		break;
	case BuildInEffect::MOTIONBLUR:
		newEffect = new PostEffectMotionBlur();
		break;
	}

	if (immediatelyLoad && newEffect)
	{
		if (!newEffect->Load(shadersLocation))
		{
			LOGE("Post Effect %s failed to Load from %s\n", newEffect->GetName(), shadersLocation);

			delete newEffect;
			newEffect = nullptr;
		}
	}
	
	return newEffect;
}

bool StandardEffectCollection::CheckShadersPath(const char* path)
{
	const char* test_shaders[] = {
		SHADER_DEPTH_LINEARIZE_VERTEX,
		SHADER_DEPTH_LINEARIZE_FRAGMENT,

		SHADER_BLUR_VERTEX,
		SHADER_BLUR_FRAGMENT,
		SHADER_IMAGE_BLUR_FRAGMENT,

		SHADER_MIX_VERTEX,
		SHADER_MIX_FRAGMENT,

		SHADER_DOWNSCALE_VERTEX,
		SHADER_DOWNSCALE_FRAGMENT,

		SHADER_SCENE_MASKED_VERTEX,
		SHADER_SCENE_MASKED_FRAGMENT
	};
	LOGV("[CheckShadersPath] testing path %s\n", path);
	for (const char* shader_path : test_shaders)
	{
		FBString full_path(path, shader_path);

		if (!IsFileExists(full_path))
		{
			LOGV("[CheckShadersPath] %s is not found\n", shader_path);
			return false;
		}
	}

	return true;
}

bool StandardEffectCollection::LoadShaders()
{
	FreeShaders();

	constexpr int PATH_LENGTH = 260;
	char shadersPath[PATH_LENGTH];
	if (!FindEffectLocation(CheckShadersPath, shadersPath, PATH_LENGTH))
	{
		LOGE("[PostProcessing] Failed to find shaders location!\n");
		return false;
	}

	LOGE("[PostProcessing] Shaders Location - %s\n", shadersPath);

	mFishEye.reset(ShaderFactory(BuildInEffect::FISHEYE, shadersPath));
	mColor.reset(ShaderFactory(BuildInEffect::COLOR, shadersPath));
	mVignetting.reset(ShaderFactory(BuildInEffect::VIGNETTE, shadersPath));
	mFilmGrain.reset(ShaderFactory(BuildInEffect::FILMGRAIN, shadersPath));
	mLensFlare.reset(ShaderFactory(BuildInEffect::LENSFLARE, shadersPath));
	mSSAO.reset(ShaderFactory(BuildInEffect::SSAO, shadersPath));
	mDOF.reset(ShaderFactory(BuildInEffect::DOF, shadersPath));
	mDisplacement.reset(ShaderFactory(BuildInEffect::DISPLACEMENT, shadersPath));
	mMotionBlur.reset(ShaderFactory(BuildInEffect::MOTIONBLUR, shadersPath));

	// load shared shaders (blur, mix)

	bool lSuccess = true;

	try
	{
		//
		// DEPTH LINEARIZE

		mEffectDepthLinearize.reset(new PostEffectLinearDepth());
		if (!mEffectDepthLinearize->Load(shadersPath))
		{
			throw std::exception("failed to load and prepare depth linearize effect");
		}

		//
		// BLUR (for SSAO)

		mEffectBlur.reset(new PostEffectBlurLinearDepth());
		if (!mEffectBlur->Load(shadersPath))
		{
			throw std::exception("failed to load and prepare SSAO blur effect");
		}

		//
		// IMAGE BLUR, simple bilateral blur

		mEffectBilateralBlur.reset(new PostEffectBilateralBlur());
		if (!mEffectBilateralBlur->Load(shadersPath))
		{
			throw std::exception("failed to load and prepare image blur effect");
		}

		//
		// MIX
		std::unique_ptr<GLSLShaderProgram> pNewShader;
		pNewShader.reset(new GLSLShaderProgram);

		FBString vertex_path = FBString(shadersPath, SHADER_MIX_VERTEX);
		FBString fragment_path = FBString(shadersPath, SHADER_MIX_FRAGMENT);

		if (!pNewShader->LoadShaders(vertex_path, fragment_path))
		{
			throw std::exception("failed to load and prepare mix shader");
		}

		// samplers and locations
		pNewShader->Bind();

		GLint loc = pNewShader->findLocation("sampler0");
		if (loc >= 0)
			glUniform1i(loc, 0);
		loc = pNewShader->findLocation("sampler1");
		if (loc >= 0)
			glUniform1i(loc, 3);

		pNewShader->UnBind();

		mShaderMix.reset(pNewShader.release());

		//
		// DOWNSCALE

		mEffectDownscale.reset(new PostEffectDownscale());
		if (!mEffectDownscale->Load(shadersPath))
		{
			throw std::exception("failed to load and prepare downscale effect");
		}

		//
		// SCENE MASKED

		pNewShader.reset(new GLSLShaderProgram);

		vertex_path = FBString(shadersPath, SHADER_SCENE_MASKED_VERTEX);
		fragment_path = FBString(shadersPath, SHADER_SCENE_MASKED_FRAGMENT);

		if (!pNewShader->LoadShaders(vertex_path, fragment_path))
		{
			throw std::exception("failed to load and prepare downscale shader");
		}

		mShaderSceneMasked.reset(pNewShader.release());

	}
	catch (const std::exception &e)
	{
		LOGE("Post Effect Chain ERROR: %s\n", e.what());
		lSuccess = false;
	}

	return lSuccess;
}


void StandardEffectCollection::FreeShaders()
{
	mFishEye.reset(nullptr);
	mColor.reset(nullptr);
	mVignetting.reset(nullptr);
	mFilmGrain.reset(nullptr);
	mLensFlare.reset(nullptr);
	mSSAO.reset(nullptr);
	mDOF.reset(nullptr);
	mDisplacement.reset(nullptr);
	mMotionBlur.reset(nullptr);

	mEffectDepthLinearize.reset(nullptr);
	mEffectBilateralBlur.reset(nullptr);
	mEffectBlur.reset(nullptr);
	mShaderMix.reset(nullptr);
	mEffectDownscale.reset(nullptr);
}