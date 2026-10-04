
/**	\file	postprocessing_effect.cxx

Sergei <Neill3d> Solokhin 2018-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "standardeffectcollection.h"
#include "posteffectshader_ssao.h"
#include "posteffectshader_displacement.h"
#include "posteffectshader_motionblur.h"
#include "posteffectshader_lensflare.h"
#include "posteffectshader_color.h"
#include "posteffectshader_dof.h"
#include "posteffectshader_filmgrain.h"
#include "posteffectshader_fisheye.h"
#include "posteffectshader_vignetting.h"
#include "postprocessing_helper.h"
#include "fxmaskingshader.h"
#include "posteffectshader_bilateral_blur.h"
#include "postpersistentdata.h"

#include "mobu_logging.h"
#include <FileUtils.h>

// shared shaders
namespace
{
	namespace fs = std::filesystem;

	const fs::path SHADER_DEPTH_LINEARIZE_VERTEX{ L"GLSL/simple130.glslv" };
	const fs::path SHADER_DEPTH_LINEARIZE_FRAGMENT{ L"GLSL/depthLinearize.fsh" };

	// Depth-based blur for SSAO.
	const fs::path SHADER_BLUR_VERTEX{ L"GLSL/simple130.glslv" };
	const fs::path SHADER_BLUR_FRAGMENT{ L"GLSL/blur.fsh" };

	// Simple Gaussian image blur.
	const fs::path SHADER_IMAGE_BLUR_VERTEX{ L"GLSL/simple130.glslv" };
	const fs::path SHADER_IMAGE_BLUR_FRAGMENT{ L"GLSL/imageBlur.glslf" };

	const fs::path SHADER_MIX_VERTEX{ L"GLSL/simple130.glslv" };
	const fs::path SHADER_MIX_FRAGMENT{ L"GLSL/mix.fsh" };

	const fs::path SHADER_DOWNSCALE_VERTEX{ L"GLSL/downscale.vsh" };
	const fs::path SHADER_DOWNSCALE_FRAGMENT{ L"GLSL/downscale.fsh" };

	const fs::path SHADER_SCENE_MASKED_VERTEX{ L"GLSL/scene_masked.glslv" };
	const fs::path SHADER_SCENE_MASKED_FRAGMENT{ L"GLSL/scene_masked.glslf" };
}

void StandardEffectCollection::ChangeContext()
{
	FreeShaders();
	mNeedReloadShaders = true;
}

bool StandardEffectCollection::ReloadShaders()
{
	if (!mNeedReloadShaders)
		return true;

	const bool success = LoadShaders();
	mNeedReloadShaders = !success;
	return success;
}

bool StandardEffectCollection::IsOk() const
{
	if (!mFishEye.get())
		return false;
	if (!mColor.get())
		return false;
	if (!mVignetting.get())
		return false;
	if (!mFilmGrain.get())
		return false;
	if (!mLensFlare.get())
		return false;
	if (!mSSAO.get())
		return false;
	if (!mDOF.get())
		return false;
	if (!mDisplacement.get())
		return false;
	
	if (!mEffectDepthLinearize.get())
		return false;
	if (!mMotionBlur.get())
		return false;
	if (!mEffectBilateralBlur.get())
		return false;
	if (!mEffectBlur.get())
		return false;
	if (!mEffectMix.get())
		return false;
	if (!mEffectDownscale.get())
		return false;

	if (!mShaderSceneMasked.get() || !mShaderSceneMasked->IsValid())
		return false;

	return true;
}

PostEffectBufferShader* StandardEffectCollection::ShaderFactory(
	BuildInEffect effectType,
	FBComponent* owner,
	const std::filesystem::path& shadersLocation,
	bool immediatelyLoad)
{
	PostEffectBufferShader* newEffect = nullptr;

	switch (effectType)
	{
	case BuildInEffect::FISHEYE:
		newEffect = new EffectShaderFishEye(owner);
		break;
	case BuildInEffect::COLOR:
		newEffect = new EffectShaderColor(owner);
		break;
	case BuildInEffect::VIGNETTE:
		newEffect = new EffectShaderVignetting(owner);
		break;
	case BuildInEffect::FILMGRAIN:
		newEffect = new EffectShaderFilmGrain(owner);
		break;
	case BuildInEffect::LENSFLARE:
		newEffect = new EffectShaderLensFlare(owner);
		break;
	case BuildInEffect::SSAO:
		newEffect = new EffectShaderSSAO(owner);
		break;
	case BuildInEffect::DOF:
		newEffect = new EffectShaderDOF(owner);
		break;
	case BuildInEffect::DISPLACEMENT:
		newEffect = new EffectShaderDisplacement(owner);
		break;
	case BuildInEffect::MOTIONBLUR:
		newEffect = new EffectShaderMotionBlur(owner);
		break;
	}

	if (immediatelyLoad && newEffect)
	{
		if (!newEffect->Load(shadersLocation))
		{
			LOGE("Post Effect %s failed to load from %ls\n", newEffect->GetName(), shadersLocation.c_str());

			delete newEffect;
			newEffect = nullptr;
		}
	}
	
	return newEffect;
}

bool StandardEffectCollection::CheckShadersPath(const std::filesystem::path& basePath)
{
	namespace fs = std::filesystem;

	if (basePath.empty())
		return false;

	const fs::path normalizedBase = basePath.lexically_normal();

	std::error_code error;
	if (!fs::is_directory(normalizedBase, error))
		return false;

	const fs::path requiredShaders[] = {
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

	LOGV("[CheckShadersPath] Testing path %ls\n", normalizedBase.c_str());

	for (const fs::path& shaderPath : requiredShaders)
	{
		// relative_path() also tolerates legacy constants such as
		// "/GLSL/simple.vsh".
		const fs::path relativePath = shaderPath.relative_path();

		if (relativePath.empty())
			return false;

		const fs::path fullPath = (normalizedBase / relativePath).lexically_normal();

		error.clear();

		if (!fs::is_regular_file(fullPath, error))
		{
			if (error)
			{
				LOGV("[CheckShadersPath] Failed to inspect %ls: %s\n", fullPath.c_str(), error.message().c_str());
			}
			else
			{
				LOGV("[CheckShadersPath] Required shader was not found: %ls\n", fullPath.c_str());
			}
			return false;
		}
	}
	return true;
}

bool StandardEffectCollection::LoadShaders()
{
	namespace fs = std::filesystem;

	FreeShaders();

	const auto shadersPath = FindEffectLocation([](const fs::path& candidate)
		{
			return CheckShadersPath(candidate);
		});

	if (!shadersPath)
	{
		LOGE("[PostProcessing] Failed to find shaders location\n");
		return false;
	}

	LOGI("[PostProcessing] Shaders location: %ls\n", shadersPath->c_str());

	constexpr FBComponent* owner = nullptr;

	mFishEye.reset(ShaderFactory(BuildInEffect::FISHEYE, owner, *shadersPath));
	mColor.reset(ShaderFactory(BuildInEffect::COLOR, owner, *shadersPath));
	mVignetting.reset(ShaderFactory(BuildInEffect::VIGNETTE, owner, *shadersPath));
	mFilmGrain.reset(ShaderFactory(BuildInEffect::FILMGRAIN, owner, *shadersPath));
	mLensFlare.reset(ShaderFactory(BuildInEffect::LENSFLARE, owner, *shadersPath));
	mSSAO.reset(ShaderFactory(BuildInEffect::SSAO, owner, *shadersPath));
	mDOF.reset(ShaderFactory(BuildInEffect::DOF, owner, *shadersPath));
	mDisplacement.reset(ShaderFactory(BuildInEffect::DISPLACEMENT, owner, *shadersPath));
	mMotionBlur.reset(ShaderFactory(BuildInEffect::MOTIONBLUR, owner, *shadersPath));

	if (!mFishEye || !mColor || !mVignetting ||
		!mFilmGrain || !mLensFlare || !mSSAO ||
		!mDOF || !mDisplacement || !mMotionBlur)
	{
		LOGE("[PostProcessing] Failed to load a standard effect\n");
		FreeShaders();
		return false;
	}

	const auto loadShared = [&](auto& destination, auto shader, const char* description) -> bool
		{
			if (!shader->Load(*shadersPath))
			{
				LOGE("[PostProcessing] Failed to load %s\n", description);
				return false;
			}

			destination = std::move(shader);
			return true;
		};

	if (!loadShared(mEffectDepthLinearize, std::make_unique<PostEffectShaderLinearDepth>(), "depth linearize effect")
		|| !loadShared(mEffectBlur, std::make_unique<EffectShaderBlurLinearDepth>(owner), "SSAO blur effect")
		|| !loadShared(mEffectBilateralBlur, std::make_unique<PostEffectShaderBilateralBlur>(), "image blur effect")
		|| !loadShared(mEffectMix, std::make_unique<EffectShaderMix>(), "mix effect")
		|| !loadShared(mEffectDownscale, std::make_unique<PostEffectShaderDownscale>(), "downscale effect"))
	{
		FreeShaders();
		return false;
	}

	const fs::path vertexPath = (*shadersPath / SHADER_SCENE_MASKED_VERTEX).lexically_normal();
	const fs::path fragmentPath = (*shadersPath / SHADER_SCENE_MASKED_FRAGMENT).lexically_normal();

	auto sceneMaskedShader = std::make_unique<GLSLShaderProgram>();

	if (!sceneMaskedShader->LoadShaders(vertexPath, fragmentPath))
	{
		LOGE("[PostProcessing] Failed to load scene-masked shader\n");
		FreeShaders();
		return false;
	}

	mShaderSceneMasked = std::move(sceneMaskedShader);
	return true;
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
	mEffectMix.reset(nullptr);
	mEffectDownscale.reset(nullptr);

	mShaderSceneMasked.reset();
}
