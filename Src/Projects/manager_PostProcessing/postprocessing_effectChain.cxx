
/**	\file	postprocessing_effect.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "postprocessing_effectChain.h"
#include "postprocessing_effectSSAO.h"
#include "postprocessing_effectDisplacement.h"
#include "postprocessing_effectMotionBlur.h"
#include "postprocessing_effectLensFlare.h"
#include "posteffectdof.h"
#include "postprocessing_helper.h"
#include "fxmaskingshader.h"
#include "posteffectbuffers.h"

#define SHADER_FISH_EYE_NAME			"Fish Eye"
#define SHADER_FISH_EYE_VERTEX			"\\GLSL\\fishEye.vsh"
#define SHADER_FISH_EYE_FRAGMENT		"\\GLSL\\fishEye.fsh"

#define SHADER_COLOR_NAME				"Color Correction"
#define SHADER_COLOR_VERTEX				"\\GLSL\\simple.vsh"
#define SHADER_COLOR_FRAGMENT			"\\GLSL\\color.fsh"

#define SHADER_VIGNETTE_NAME			"Vignetting"
#define SHADER_VIGNETTE_VERTEX			"\\GLSL\\simple.vsh"
#define SHADER_VIGNETTE_FRAGMENT		"\\GLSL\\vignetting.fsh"

#define SHADER_FILMGRAIN_NAME			"Film Grain"
#define SHADER_FILMGRAIN_VERTEX			"\\GLSL\\simple.vsh"
#define SHADER_FILMGRAIN_FRAGMENT		"\\GLSL\\filmGrain.fsh"

// shared shaders

#define SHADER_DEPTH_LINEARIZE_VERTEX		"\\GLSL\\simple.vsh"
#define SHADER_DEPTH_LINEARIZE_FRAGMENT		"\\GLSL\\depthLinearize.fsh"

// this is a depth based blur, fo SSAO
#define SHADER_BLUR_VERTEX					"\\GLSL\\simple.vsh"
#define SHADER_BLUR_FRAGMENT				"\\GLSL\\blur.fsh"

// this is a simple gaussian image blur
#define SHADER_IMAGE_BLUR_VERTEX			"\\GLSL\\simple.vsh"
#define SHADER_IMAGE_BLUR_FRAGMENT			"\\GLSL\\imageBlur.fsh"

#define SHADER_MIX_VERTEX					"\\GLSL\\simple.vsh"
#define SHADER_MIX_FRAGMENT					"\\GLSL\\mix.fsh"

#define SHADER_DOWNSCALE_VERTEX				"\\GLSL\\downscale.vsh"
#define SHADER_DOWNSCALE_FRAGMENT			"\\GLSL\\downscale.fsh"

//
extern void LOGE(const char* pFormatString, ...);

///////////////////////////////////////////////////////////////////////////
// Effect FishEye

//! a constructor
PostEffectFishEye::PostEffectFishEye()
: PostEffectBase()
{
	for (int i = 0; i < LOCATIONS_COUNT; ++i)
		mLocations[i] = -1;
}

//! a destructor
PostEffectFishEye::~PostEffectFishEye()
{

}

const char *PostEffectFishEye::GetName()
{
	return SHADER_FISH_EYE_NAME;
}

const char *PostEffectFishEye::GetVertexFname(const int shaderIndex)
{
	return SHADER_FISH_EYE_VERTEX;
}

const char *PostEffectFishEye::GetFragmentFname(const int shaderIndex)
{
	return SHADER_FISH_EYE_FRAGMENT;
}

bool PostEffectFishEye::PrepUniforms(const int shaderIndex)
{
	GLSLShader* shader = mShaders[shaderIndex];
	if (!shader)
		return false;

	shader->Bind();

	GLint loc = shader->findLocation("sampler0");
	if (loc >= 0)
		glUniform1i(loc, 0);
	PrepareUniformLocations(shader);

	mLocAmount = shader->findLocation("amount");
	mLocLensRadius = shader->findLocation("lensradius");
	mLocSignCurvature = shader->findLocation("signcurvature");

	shader->UnBind();
	return true;
}

bool PostEffectFishEye::CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera)
{
	const double amount = pData->FishEyeAmount;
	const double lensradius = pData->FishEyeLensRadius;
	const double signcurvature = pData->FishEyeSignCurvature;

	GLSLShader* shader = GetShaderPtr();
	if (!shader)
		return false;

	shader->Bind();
	UpdateUniforms(pData);

	if (mLocAmount >= 0)
		glUniform1f(mLocAmount, 0.01f * static_cast<float>(amount));
	if (mLocLensRadius >= 0)
		glUniform1f(mLocLensRadius, static_cast<float>(lensradius));
	if (mLocSignCurvature >= 0)
		glUniform1f(mLocSignCurvature, static_cast<float>(signcurvature));

	shader->UnBind();
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Effect Color Correction

//! a constructor
PostEffectColor::PostEffectColor()
: PostEffectBase()
{
	for (int i = 0; i < LOCATIONS_COUNT; ++i)
		mLocations[i] = -1;
}

//! a destructor
PostEffectColor::~PostEffectColor()
{

}

const char *PostEffectColor::GetName() 
{
	return SHADER_COLOR_NAME;
}

const char *PostEffectColor::GetVertexFname(const int)
{
	return SHADER_COLOR_VERTEX;
}

const char *PostEffectColor::GetFragmentFname(const int)
{
	return SHADER_COLOR_FRAGMENT;
}

bool PostEffectColor::PrepUniforms(const int shaderIndex)
{
	GLSLShader* shader = mShaders[shaderIndex];
	if (!shader)
		return false;
	
	shader->Bind();

	GLint loc = shader->findLocation("sampler0");
	if (loc >= 0)
		glUniform1i(loc, 0);

	mResolution = shader->findLocation("gResolution");
	mChromaticAberration = shader->findLocation("gCA");

	PrepareUniformLocations(shader);

	mLocCSB = shader->findLocation("gCSB");
	mLocHue = shader->findLocation("gHue");

	shader->UnBind();
	return true;
		
}

bool PostEffectColor::CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera)
{
	const float chromatic_aberration = (pData->ChromaticAberration) ? 1.0f : 0.0f;
	const FBVector2d ca_dir = pData->ChromaticAberrationDirection;

	double saturation = 1.0 + 0.01 * pData->Saturation;
	double brightness = 1.0 + 0.01 * pData->Brightness;
	double contrast = 1.0 + 0.01 * pData->Contrast;
	double gamma = 0.01 * pData->Gamma;

	const float inverse = (pData->Inverse) ? 1.0f : 0.0f;
	double hue = 0.01 * pData->Hue;
	double hueSat = 0.01 * pData->HueSaturation;
	double lightness = 0.01 * pData->Lightness;

	GLSLShader* mShader = GetShaderPtr();
	if (!mShader)
		return false;
	
	mShader->Bind();

	if (mResolution >= 0)
	{
		glUniform2f(mResolution, static_cast<float>(w), static_cast<float>(h));
	}

	if (mChromaticAberration >= 0)
	{
		glUniform4f(mChromaticAberration, static_cast<float>(ca_dir[0]), static_cast<float>(ca_dir[1]), 0.0f, chromatic_aberration);
	}

	UpdateUniforms(pData);

	if (mLocCSB >= 0)
		glUniform4f(mLocCSB, (float)contrast, (float)saturation, (float)brightness, (float)gamma);

	if (mLocHue >= 0)
		glUniform4f(mLocHue, (float)hue, (float)hueSat, (float)lightness, inverse);

	mShader->UnBind();
	return true;
}

////////////////////////////////////////////////////////////////////////////////////
// post vignetting

//! a constructor
PostEffectVignetting::PostEffectVignetting()
: PostEffectBase()
{
	for (int i = 0; i < LOCATIONS_COUNT; ++i)
		mLocations[i] = -1;
}

//! a destructor
PostEffectVignetting::~PostEffectVignetting()
{

}

const char *PostEffectVignetting::GetName()
{
	return SHADER_VIGNETTE_NAME;
}
const char *PostEffectVignetting::GetVertexFname(const int)
{
	return SHADER_VIGNETTE_VERTEX;
}
const char *PostEffectVignetting::GetFragmentFname(const int)
{
	return SHADER_VIGNETTE_FRAGMENT;
}

bool PostEffectVignetting::PrepUniforms(const int shaderIndex)
{
	GLSLShader* mShader = mShaders[shaderIndex];
	if (!mShader)
		return false;
	
	mShader->Bind();

	GLint loc = mShader->findLocation("sampler0");
	if (loc >= 0)
		glUniform1i(loc, 0);

	PrepareUniformLocations(mShader);

	mLocAmount = mShader->findLocation("amount");
	mLocVignOut = mShader->findLocation("vignout");
	mLocVignIn = mShader->findLocation("vignin");
	mLocVignFade = mShader->findLocation("vignfade");

	mShader->UnBind();
	return true;
}

bool PostEffectVignetting::CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera)
{
	const double amount = pData->VignAmount;
	const double vignout = pData->VignOut;
	const double vignin = pData->VignIn;
	const double vignfade = pData->VignFade;

	GLSLShader* mShader = GetShaderPtr();

	if (!mShader)
		return false;
	
	mShader->Bind();
	UpdateUniforms(pData);
		
	if (mLocAmount >= 0)
		glUniform1f(mLocAmount, 0.01f * static_cast<float>(amount));
	if (mLocVignOut >= 0)
		glUniform1f(mLocVignOut, 0.01f * static_cast<float>(vignout));
	if (mLocVignIn >= 0)
		glUniform1f(mLocVignIn, 0.01f * static_cast<float>(vignin));
	if (mLocVignFade >= 0)
		glUniform1f(mLocVignFade, static_cast<float>(vignfade));

	mShader->UnBind();
	return true;
}


////////////////////////////////////////////////////////////////////////////////////
// post film grain

//! a constructor
PostEffectFilmGrain::PostEffectFilmGrain()
: PostEffectBase()
{
	for (int i = 0; i < LOCATIONS_COUNT; ++i)
		mLocations[i] = -1;
}

//! a destructor
PostEffectFilmGrain::~PostEffectFilmGrain()
{

}

const char *PostEffectFilmGrain::GetName()
{
	return SHADER_FILMGRAIN_NAME;
}
const char *PostEffectFilmGrain::GetVertexFname(const int)
{
	return SHADER_FILMGRAIN_VERTEX;
}
const char *PostEffectFilmGrain::GetFragmentFname(const int)
{
	return SHADER_FILMGRAIN_FRAGMENT;
}

bool PostEffectFilmGrain::PrepUniforms(const int shaderIndex)
{
	GLSLShader* mShader = mShaders[shaderIndex];
	if (!mShader)
		return false;
	
	mShader->Bind();

	GLint loc = mShader->findLocation("sampler0");
	if (loc >= 0)
		glUniform1i(loc, 0);

	PrepareUniformLocations(mShader);

	textureWidth = mShader->findLocation("textureWidth");
	textureHeight = mShader->findLocation("textureHeight");

	timer = mShader->findLocation("timer");
	grainamount = mShader->findLocation("grainamount");
	colored = mShader->findLocation("colored");
	coloramount = mShader->findLocation("coloramount");
	grainsize = mShader->findLocation("grainsize");
	lumamount = mShader->findLocation("lumamount");

	mShader->UnBind();
	return true;
}

bool PostEffectFilmGrain::CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera)
{
	FBTime systemTime = (pData->FG_UsePlayTime) ? mSystem.LocalTime : mSystem.SystemTime;

	const double timerMult = pData->FG_TimeSpeed;
	const double _timer = 0.01 * timerMult * systemTime.GetSecondDouble();
	
	const double _grainamount = pData->FG_GrainAmount;
	const double _colored = (pData->FG_Colored) ? 1.0 : 0.0;
	const double _coloramount = pData->FG_ColorAmount;
	const double _grainsize = pData->FG_GrainSize;
	const double _lumamount = pData->FG_LumAmount;

	GLSLShader* mShader = GetShaderPtr();
	if (!mShader)
		return false;
	
	mShader->Bind();
	UpdateUniforms(pData);

	if (textureWidth >= 0)
		glUniform1f(textureWidth, static_cast<float>(w));
	if (textureHeight >= 0)
		glUniform1f(textureHeight, static_cast<float>(h));

	if (timer >= 0)
		glUniform1f(timer, static_cast<float>(_timer));
	if (grainamount >= 0)
		glUniform1f(grainamount, 0.01f * static_cast<float>(_grainamount));
	if (colored >= 0)
		glUniform1f(colored, static_cast<float>(_colored));
	if (coloramount >= 0)
		glUniform1f(coloramount, 0.01f * static_cast<float>(_coloramount));
	if (grainsize>= 0)
		glUniform1f(grainsize, 0.01f * static_cast<float>(_grainsize));
	if (lumamount>= 0)
		glUniform1f(lumamount, 0.01f * static_cast<float>(_lumamount));

	mShader->UnBind();
	return true;
}


////////////////////////////////////////////////////////////////////////////////////
// post effect chain

//! a constructor
PostEffectChain::PostEffectChain()
{
	mIsCompressedDataReady = false;
	mNeedReloadShaders = true;
	mLocDepthLinearizeClipInfo = -1;
	mLastCompressTime = 0.0;
}

//! a destructor
PostEffectChain::~PostEffectChain()
{

}

void PostEffectChain::ChangeContext()
{
	FreeShaders();
	mNeedReloadShaders = true;
	mIsCompressedDataReady = false;
	mLastCompressTime = 0.0;
}

bool PostEffectChain::Prep(PostPersistentData *pData, int w, int h, FBCamera *pCamera)
{
	bool lSuccess = true;

	mSettings = pData;
	mLastCamera = pCamera;

	if (false == mSettings.Ok())
		return false;

	if (true == mSettings.Ok())
	{
		if (true == mSettings->IsNeedToReloadShaders())
		{
			mNeedReloadShaders = true;
			mSettings->SetReloadShadersState(false);
		}
	}
	else
	{
		lSuccess = false;
	}

	if (mNeedReloadShaders)
	{
		if (false == LoadShaders())
			lSuccess = false;

		mNeedReloadShaders = false;
	}

	// update UI values

	if (true == mSettings->FishEye && nullptr != mFishEye.get())
		mFishEye->CollectUIValues(mSettings, w,h, pCamera);
	
	if (true == mSettings->ColorCorrection && nullptr != mColor.get())
		mColor->CollectUIValues(mSettings, w,h, pCamera);
	
	if (true == mSettings->Vignetting && nullptr != mVignetting.get())
		mVignetting->CollectUIValues(mSettings, w,h, pCamera);
	
	if (true == mSettings->FilmGrain && nullptr != mFilmGrain.get())
		mFilmGrain->CollectUIValues(mSettings, w,h, pCamera);

	if (true == mSettings->LensFlare && nullptr != mLensFlare.get())
		mLensFlare->CollectUIValues(mSettings, w,h, pCamera);

	if (true == mSettings->SSAO && nullptr != mSSAO.get())
		mSSAO->CollectUIValues(mSettings, w,h, pCamera);

	if (true == mSettings->DepthOfField && nullptr != mDOF.get())
		mDOF->CollectUIValues(mSettings, w, h, pCamera);

	if (true == mSettings->Displacement && nullptr != mDisplacement.get())
		mDisplacement->CollectUIValues(mSettings, w, h, pCamera);

	if (true == mSettings->MotionBlur && nullptr != mMotionBlur.get())
		mMotionBlur->CollectUIValues(mSettings, w, h, pCamera);

	return lSuccess;
}

bool PostEffectChain::BeginFrame(PostEffectBuffers *buffers)
{
	return true;

	/*
	if (false == buffers->Ok())
		return false;

	if (false == mSettings.Ok())
		return false;

	bool lSuccess = false;

	// if we need a preview
	if (mSettings->OutputPreview && buffers->GetPreviewWidth() > 1 && nullptr != buffers->GetBufferDownscalePtr())
	{
		
		// grab texture data

		lSuccess = buffers->PreviewCompressBegin();
	}

	return lSuccess;
	*/
}

bool PostEffectChain::HasMaskUsedByEffect() const
{
	if (!mSettings.Ok())
		return false;

	return mSettings->HasAnyActiveMasking();
}

bool PostEffectChain::HasAnyMaskedObject() const
{
	FBScene* scene = mSystem.Scene;

	for (int i = 0; i < scene->Shaders.GetCount(); ++i)
	{
		FBShader* shader = scene->Shaders[i];
		if (FBIS(shader, FXMaskingShader))
		{
			for (int j = 0; j < shader->GetDstCount(); ++j)
			{
				if (FBIS(shader->GetDst(j), FBModel))
					return true;
			}
		}
	}
	return false;
}

bool PostEffectChain::Process(PostEffectBuffers *buffers, double systime)
{
	mIsCompressedDataReady = false;

	if (!buffers->Ok())
		return false;

	if (!mSettings.Ok())
		return false;

	// prepare chain count and order

	int count = 0;

	if (mSettings->FishEye)
		count += 1;
	if (mSettings->ColorCorrection)
		count += 1;
	if (mSettings->Vignetting)
		count += 1;
	if (mSettings->FilmGrain)
		count += 1;
	if (mSettings->LensFlare)
		count += 1;
	if (mSettings->SSAO)
		count += 1;
	if (mSettings->DepthOfField)
		count += 1;
	if (mSettings->Displacement)
		count += 1;
	if (mSettings->MotionBlur)
		count += 1;

	if (0 == count && 0 == mSettings->OutputPreview.AsInt())
		return false;

	mChain.resize(count);

	count = 0;

	int blurAndMix = -1;
	int blurAndMix2 = -1;

	// ordering HERE

	if (mSettings->SSAO)
	{
		mChain[count] = mSSAO.get();

		if (mSettings->SSAO_Blur)
		{
			blurAndMix = count;
		}
		count += 1;
	}
	if (mSettings->MotionBlur)
	{
		mChain[count] = mMotionBlur.get();
		count += 1;
	}
	if (mSettings->DepthOfField)
	{
		mChain[count] = mDOF.get();
		count += 1;
	}
	if (mSettings->ColorCorrection)
	{
		mChain[count] = mColor.get();
		if (mSettings->Bloom)
		{
			blurAndMix2 = count;
		}
		count += 1;
	}
	if (mSettings->LensFlare)
	{
		mChain[count] = mLensFlare.get();
		count += 1;
	}
	if (mSettings->Displacement)
	{
		mChain[count] = mDisplacement.get();
		count += 1;
	}
	if (mSettings->FishEye)
	{
		mChain[count] = mFishEye.get();
		count += 1;
	}
	if (mSettings->FilmGrain)
	{
		mChain[count] = mFilmGrain.get();
		count += 1;
	}
	if (mSettings->Vignetting)
	{
		mChain[count] = mVignetting.get();
		count += 1;
	}

	bool isMaskTextureBinded = false;
	bool isMaskBlurRequested = false;

	if (HasMaskUsedByEffect() && HasAnyMaskedObject() && mLastCamera)
	{
		FrameBuffer* maskBuffer = buffers->GetBufferMaskPtr();
		maskBuffer->Bind();

		glViewport(0, 0, maskBuffer->GetWidth(), maskBuffer->GetHeight());

		if (!mSettings->InvertMask)
		{
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		}
		else
		{
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		}
		
		glClear(GL_COLOR_BUFFER_BIT);

		if (!mSettings->InvertMask)
		{
			glColor3d(1.0, 1.0, 1.0);
		}
		else
		{
			glColor3d(0.0, 0.0, 0.0);
		}

		RenderMaskedModels(mLastCamera);

		maskBuffer->UnBind();

		if (mSettings->BlurMask)
		{
			isMaskBlurRequested = true;
			// Bilateral Blur Pass

			GLuint texid = maskBuffer->GetColorObject();
			glBindTexture(GL_TEXTURE_2D, texid);

			buffers->GetBufferBlurPtr()->Bind();
			mShaderBlur->Bind();

			const int w = buffers->GetWidth();
			const int h = buffers->GetHeight();

			const float blurSharpness = 0.1f * (float)mSettings->SSAO_BlurSharpness;
			const float invRes[2] = { 1.0f / float(w), 1.0f / float(h) };

			if (mLocBlurSharpness >= 0)
				glUniform1f(mLocBlurSharpness, blurSharpness);
			if (mLocBlurRes >= 0)
				glUniform2f(mLocBlurRes, invRes[0], invRes[1]);

			const float color_shift = (mSettings->Bloom) ? static_cast<float>(0.01 * mSettings->BloomMinBright) : 0.0f;
			mShaderBlur->setUniformFloat("g_ColorShift", color_shift);

			drawOrthoQuad2d(w, h);

			mShaderBlur->UnBind();
			buffers->GetBufferBlurPtr()->UnBind();

			glBindTexture(GL_TEXTURE_2D, 0);
		}

		// bind a mask texture, debug draw on a screen

		if (mSettings->DebugDisplyMasking && !mSettings->BlurMask)
		{
			if (mSettings->BlurMask)
			{
				BlitFBOToFBO(buffers->GetBufferBlurPtr()->GetFrameBuffer(), buffers->GetBufferBlurPtr()->GetWidth(), buffers->GetBufferBlurPtr()->GetHeight(),
					buffers->GetDstBufferPtr()->GetFrameBuffer(), buffers->GetDstBufferPtr()->GetWidth(), buffers->GetDstBufferPtr()->GetHeight(), false, false, false);
			}
			else
			{
				BlitFBOToFBO(maskBuffer->GetFrameBuffer(), maskBuffer->GetWidth(), maskBuffer->GetHeight(),
					buffers->GetDstBufferPtr()->GetFrameBuffer(), buffers->GetDstBufferPtr()->GetWidth(), buffers->GetDstBufferPtr()->GetHeight(), false, false, false);
			}
			
			buffers->SwapBuffers();
			return true;
		}

		
		isMaskTextureBinded = true;
	}

	if (mSettings->SSAO || isMaskBlurRequested)
	{
		const GLuint depthId = buffers->GetSrcBufferPtr()->GetDepthObject();

		// prep data

		const float znear = static_cast<float>(mLastCamera->NearPlaneDistance);
		const float zfar = static_cast<float>(mLastCamera->FarPlaneDistance);
		FBCameraType cameraType;
		mLastCamera->Type.GetData(&cameraType, sizeof(FBCameraType));
		const bool perspective = (cameraType == FBCameraType::kFBCameraTypePerspective);

		const float clipInfo[4]{
			znear* zfar,
			znear - zfar,
			zfar,
			(perspective) ? 1.0f : 0.0f
		};

		FrameBuffer *pBufferDepth = buffers->GetBufferDepthPtr();

		// render

		pBufferDepth->Bind();

		mShaderDepthLinearize->Bind();

		glBindTexture(GL_TEXTURE_2D, depthId);

		if (mLocDepthLinearizeClipInfo >= 0)
			glUniform4fv(mLocDepthLinearizeClipInfo, 1, clipInfo);

		drawOrthoQuad2d(pBufferDepth->GetWidth(), pBufferDepth->GetHeight());

		mShaderDepthLinearize->UnBind();
		pBufferDepth->UnBind();

		// DONE: bind a depth texture
		const GLuint linearDepthId = pBufferDepth->GetColorObject();

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, linearDepthId);
		glActiveTexture(GL_TEXTURE0);
	}
	
	// blur mask

	if (isMaskBlurRequested)
	{
		FrameBuffer* maskBuffer = buffers->GetBufferMaskPtr();

		// Bilateral Blur Pass

		GLuint texid = maskBuffer->GetColorObject();
		glBindTexture(GL_TEXTURE_2D, texid);

		buffers->GetBufferBlurPtr()->Bind();
		mShaderImageBlur->Bind();

		const int w = buffers->GetWidth();
		const int h = buffers->GetHeight();

		const FBVector2d blurMaskScale = mSettings->BlurMaskScale;

		if (mLocImageBlurScale >= 0)
			glUniform4f(mLocImageBlurScale, 
				blurMaskScale.mValue[0] / static_cast<float>(w), 
				blurMaskScale.mValue[1] / static_cast<float>(h), 
				1.0f / static_cast<float>(w), 
				1.0f / static_cast<float>(h));
		
		drawOrthoQuad2d(w, h);

		mShaderImageBlur->UnBind();
		buffers->GetBufferBlurPtr()->UnBind();

		glBindTexture(GL_TEXTURE_2D, 0);

		if (mSettings->DebugDisplyMasking)
		{
			BlitFBOToFBO(buffers->GetBufferBlurPtr()->GetFrameBuffer(), buffers->GetBufferBlurPtr()->GetWidth(), buffers->GetBufferBlurPtr()->GetHeight(),
				buffers->GetDstBufferPtr()->GetFrameBuffer(), buffers->GetDstBufferPtr()->GetWidth(), buffers->GetDstBufferPtr()->GetHeight(), false, false, false);
		
			buffers->SwapBuffers();
			return true;
		}

		BlitFBOToFBO(buffers->GetBufferBlurPtr()->GetFrameBuffer(), buffers->GetBufferBlurPtr()->GetWidth(), buffers->GetBufferBlurPtr()->GetHeight(),
			buffers->GetBufferMaskPtr()->GetFrameBuffer(), buffers->GetBufferMaskPtr()->GetWidth(), buffers->GetBufferMaskPtr()->GetHeight(), false, false, false);
	}


	if (isMaskTextureBinded)
	{
		const GLuint maskTextureId = buffers->GetBufferMaskPtr()->GetColorObject();
		glActiveTexture(GL_TEXTURE0 + CommonEffectUniforms::GetMaskSamplerSlot());
		glBindTexture(GL_TEXTURE_2D, maskTextureId);
		glActiveTexture(GL_TEXTURE0);
	}
	
	if (mSettings->DepthOfField)
	{
		const GLuint depthId = buffers->GetSrcBufferPtr()->GetDepthObject();

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthId);
		glActiveTexture(GL_TEXTURE0);
	}

	// compute effect chain with double buffer

	// DONE: when buffer is attached, buffer is used itself !
	bool lSuccess = false;
	const bool generateMips = mSettings->GenerateMipMaps;

	if (count > 0)
	{
		GLuint texid = 0;
		
		const int w = buffers->GetWidth();
		const int h = buffers->GetHeight();

		const float blurSharpness = 0.1f * (float)mSettings->SSAO_BlurSharpness;
		const float invRes[2] = { 1.0f / float(w), 1.0f / float(h) };

		// generate mipmaps for the first target
		texid = buffers->GetSrcBufferPtr()->GetColorObject();

		if (true == generateMips)
		{
			if (texid > 0)
			{
				glBindTexture(GL_TEXTURE_2D, texid);
				glGenerateMipmap(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}
		
		for (int i = 0; i < count; ++i)
		{
			if (!mChain[i])
				continue;

			mChain[i]->Bind();

			for (int j = 0; j < mChain[i]->GetNumberOfPasses(); ++j)
			{
				mChain[i]->PrepPass(j);

				texid = buffers->GetSrcBufferPtr()->GetColorObject();
				glBindTexture(GL_TEXTURE_2D, texid);

				if (generateMips)
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				}
				else
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				}

				buffers->GetDstBufferPtr()->Bind();

				drawOrthoQuad2d(w, h);

				buffers->GetDstBufferPtr()->UnBind(generateMips);

				//
				buffers->SwapBuffers();
			}

			mChain[i]->UnBind();

			// if we need more passes, blur and mix for SSAO
			if (i == blurAndMix || i == blurAndMix2)
			{
				if (false == mSettings->OnlyAO || (i == blurAndMix2))
				{
					// Bilateral Blur Pass

					texid = buffers->GetSrcBufferPtr()->GetColorObject();
					glBindTexture(GL_TEXTURE_2D, texid);

					buffers->GetBufferBlurPtr()->Bind();
					mShaderBlur->Bind();

					if (mLocBlurSharpness >= 0)
						glUniform1f(mLocBlurSharpness, blurSharpness);
					if (mLocBlurRes >= 0)
						glUniform2f(mLocBlurRes, invRes[0], invRes[1]);

					const float color_shift = (mSettings->Bloom) ? static_cast<float>(0.01 * mSettings->BloomMinBright) : 0.0f;
					mShaderBlur->setUniformFloat("g_ColorShift", color_shift);
					
					drawOrthoQuad2d(w, h);

					mShaderBlur->UnBind();
					buffers->GetBufferBlurPtr()->UnBind();

					//
					buffers->SwapBuffers();

					// Mix AO and Color Pass
					const GLuint blurId = buffers->GetBufferBlurPtr()->GetColorObject();
					texid = buffers->GetSrcBufferPtr()->GetColorObject();

					glActiveTexture(GL_TEXTURE3);
					glBindTexture(GL_TEXTURE_2D, blurId);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, texid);

					buffers->GetDstBufferPtr()->Bind();
					mShaderMix->Bind();

					if (mSettings->Bloom)
					{
						mShaderMix->setUniformVector("gBloom", static_cast<float>(0.01 * mSettings->BloomTone), static_cast<float>(0.01 * mSettings->BloomStretch), 0.0f, 1.0f);
					}
					else
					{
						mShaderMix->setUniformVector("gBloom", 0.0f, 0.0f, 0.0f, 0.0f);
					}

					drawOrthoQuad2d(w, h);

					mShaderMix->UnBind();
					buffers->GetDstBufferPtr()->UnBind();

					glActiveTexture(GL_TEXTURE3);
					glBindTexture(GL_TEXTURE_2D, 0);
					glActiveTexture(GL_TEXTURE0);

					//
					buffers->SwapBuffers();
				}
				else
				{
					// Bilateral Blur Pass

					texid = buffers->GetSrcBufferPtr()->GetColorObject();
					glBindTexture(GL_TEXTURE_2D, texid);

					buffers->GetDstBufferPtr()->Bind();
					mShaderBlur->Bind();

					if (mLocBlurSharpness >= 0)
						glUniform1f(mLocBlurSharpness, blurSharpness);
					if (mLocBlurRes >= 0)
						glUniform2f(mLocBlurRes, invRes[0], invRes[1]);

					drawOrthoQuad2d(w, h);

					mShaderBlur->UnBind();
					buffers->GetDstBufferPtr()->UnBind();

					//
					buffers->SwapBuffers();
				}
			}
		}

		lSuccess = true;
	}

	// if we need a preview (rate - compress 25-30 frames per second)
	if (mSettings->OutputPreview)
	{
		int updaterate = mSettings->OutputUpdateRate.AsInt();
		if (updaterate <= 0 || updaterate > 30)
			updaterate = 10;

		double rate = 1.0 / updaterate;

		if (0.0 == mLastCompressTime || (systime - mLastCompressTime) > rate)
		{
			GLuint texid = buffers->GetSrcBufferPtr()->GetColorObject();
			glBindTexture(GL_TEXTURE_2D, texid);

			buffers->GetBufferDownscalePtr()->Bind();
			mShaderDownscale->Bind();

			GLint loc = mShaderDownscale->findLocation("texelSize");
			if (loc >= 0)
				glUniform2f(loc, 1.0f / (float)buffers->GetWidth(), 1.0f / (float)buffers->GetHeight());

			drawOrthoQuad2d(buffers->GetPreviewWidth(), buffers->GetPreviewHeight());

			mShaderDownscale->UnBind();
			buffers->GetBufferDownscalePtr()->UnBind();

			const unsigned int previewW = buffers->GetPreviewWidth();
			const unsigned int previewH = buffers->GetPreviewHeight();
			const double ratio = 1.0 * (double)buffers->GetWidth() / (double)buffers->GetHeight();

			if (mSettings->OutputUseCompression)
			{
				GLint compressionCode = 0;
				EImageCompression imageCompression;
				mSettings->OutputCompression.GetData(&imageCompression, sizeof(EImageCompression));
				if (true == buffers->PreviewOpenGLCompress(imageCompression, compressionCode))
				{
					mSettings->SetPreviewTextureId(buffers->GetPreviewCompressedColor(), ratio, previewW, previewH,
						static_cast<int32_t>(buffers->GetUnCompressedSize()), 
						static_cast<int32_t>(buffers->GetCompressedSize()), 
						compressionCode, systime);

					mIsCompressedDataReady = true;
				}

				
			}
			else
			{
				GLint compressionCode = GL_RGB8;

				mSettings->SetPreviewTextureId(buffers->GetPreviewColor(), ratio, previewW, previewH,
					static_cast<int32_t>(buffers->GetUnCompressedSize()),
					static_cast<int32_t>(buffers->GetCompressedSize()), 
					compressionCode, systime);
					
			}
			
			mLastCompressTime = systime;
		}
	}

	//
	if (mSettings->DepthOfField)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	if (mSettings->SSAO)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	if (isMaskTextureBinded)
	{
		glActiveTexture(GL_TEXTURE0 + CommonEffectUniforms::GetMaskSamplerSlot());
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glActiveTexture(GL_TEXTURE0);
	return lSuccess;
}


PostEffectBase *PostEffectChain::ShaderFactory(const int type, const char *shadersLocation)
{

	PostEffectBase *newEffect = nullptr;

	switch (type)
	{
	case SHADER_TYPE_FISHEYE:
		newEffect = new PostEffectFishEye();
		break;
	case SHADER_TYPE_COLOR:
		newEffect = new PostEffectColor();
		break;
	case SHADER_TYPE_VIGNETTE:
		newEffect = new PostEffectVignetting();
		break;
	case SHADER_TYPE_FILMGRAIN:
		newEffect = new PostEffectFilmGrain();
		break;
	case SHADER_TYPE_LENSFLARE:
		newEffect = new PostEffectLensFlare();
		break;
	case SHADER_TYPE_SSAO:
		newEffect = new PostEffectSSAO();
		break;
	case SHADER_TYPE_DOF:
		newEffect = new PostEffectDOF();
		break;
	case SHADER_TYPE_DISPLACEMENT:
		newEffect = new PostEffectDisplacement();
		break;
	case SHADER_TYPE_MOTIONBLUR:
		newEffect = new PostEffectMotionBlur();
		break;
	}


	try
	{
		if (nullptr == newEffect)
		{
			throw std::exception("failed to allocate memory for the shader");
		}

		for (int i = 0; i < newEffect->GetNumberOfShaders(); ++i)
		{
			FBString vertex_path(shadersLocation, newEffect->GetVertexFname(i));
			FBString fragment_path(shadersLocation, newEffect->GetFragmentFname(i));

			if (false == newEffect->Load(i, vertex_path, fragment_path))
			{
				throw std::exception("failed to load and prepare effect");
			}

			// samplers and locations
			newEffect->PrepUniforms(i);
		}
	}
	catch (const std::exception &e)
	{
		FBTrace("Post Effect Chain ERROR: %s\n", e.what());

		delete newEffect;
		newEffect = nullptr;
	}

	return newEffect;
}

bool PostEffectChain::CheckShadersPath(const char* path)
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
		SHADER_DOWNSCALE_FRAGMENT	
	};

	for (const char* shader_path : test_shaders)
	{
		FBString full_path(path, shader_path);

		if (!IsFileExists(full_path))
		{
			return false;
		}
	}

	return true;
}

bool PostEffectChain::LoadShaders()
{
	FreeShaders();

	char shadersPath[MAX_PATH];
	if (!FindEffectLocation(CheckShadersPath, shadersPath, MAX_PATH))
	{
		FBTrace("[PostProcessing] Failed to find shaders location!\n");
		return false;
	}

	FBTrace("[PostProcessing] Shaders Location - %s\n", shadersPath);

	mFishEye.reset(ShaderFactory(SHADER_TYPE_FISHEYE, shadersPath));
	mColor.reset(ShaderFactory(SHADER_TYPE_COLOR, shadersPath));
	mVignetting.reset(ShaderFactory(SHADER_TYPE_VIGNETTE, shadersPath));
	mFilmGrain.reset(ShaderFactory(SHADER_TYPE_FILMGRAIN, shadersPath));
	mLensFlare.reset(ShaderFactory(SHADER_TYPE_LENSFLARE, shadersPath));
	mSSAO.reset(ShaderFactory(SHADER_TYPE_SSAO, shadersPath));
	mDOF.reset(ShaderFactory(SHADER_TYPE_DOF, shadersPath));
	mDisplacement.reset(ShaderFactory(SHADER_TYPE_DISPLACEMENT, shadersPath));
	mMotionBlur.reset(ShaderFactory(SHADER_TYPE_MOTIONBLUR, shadersPath));

	// load shared shaders (blur, mix)

	bool lSuccess = true;

	GLSLShader *pNewShader = nullptr;

	try
	{
		//
		// DEPTH LINEARIZE

		pNewShader = new GLSLShader();

		if (nullptr == pNewShader)
		{
			throw std::exception("failed to allocate memory for a depth linearize shader");
		}

		FBString vertex_path(shadersPath, SHADER_DEPTH_LINEARIZE_VERTEX);
		FBString fragment_path(shadersPath, SHADER_DEPTH_LINEARIZE_FRAGMENT);

		if (false == pNewShader->LoadShaders(vertex_path, fragment_path))
		{
			throw std::exception("failed to load and prepare depth linearize shader");
		}

		// samplers and locations
		pNewShader->Bind();

		GLint loc = pNewShader->findLocation("depthSampler");
		if (loc >= 0)
			glUniform1i(loc, 0);
		mLocDepthLinearizeClipInfo = pNewShader->findLocation("gClipInfo");

		pNewShader->UnBind();

		mShaderDepthLinearize.reset(pNewShader);

		//
		// BLUR (for SSAO)

		pNewShader = new GLSLShader();

		if (nullptr == pNewShader)
		{
			throw std::exception("failed to allocate memory for a blur shader");
		}

		vertex_path = FBString(shadersPath, SHADER_BLUR_VERTEX);
		fragment_path = FBString(shadersPath, SHADER_BLUR_FRAGMENT);

		if (false == pNewShader->LoadShaders(vertex_path, fragment_path))
		{
			throw std::exception("failed to load and prepare blur shader");
		}

		// samplers and locations
		pNewShader->Bind();

		loc = pNewShader->findLocation("sampler0");
		if (loc >= 0)
			glUniform1i(loc, 0);
		loc = pNewShader->findLocation("linearDepthSampler");
		if (loc >= 0)
			glUniform1i(loc, 2);

		mLocBlurSharpness = pNewShader->findLocation("g_Sharpness");
		mLocBlurRes = pNewShader->findLocation("g_InvResolutionDirection");

		pNewShader->UnBind();

		mShaderBlur.reset(pNewShader);

		//
		// IMAGE BLUR

		pNewShader = new GLSLShader();

		vertex_path = FBString(shadersPath, SHADER_IMAGE_BLUR_VERTEX);
		fragment_path = FBString(shadersPath, SHADER_IMAGE_BLUR_FRAGMENT);

		if (false == pNewShader->LoadShaders(vertex_path, fragment_path))
		{
			throw std::exception("failed to load and prepare image blur shader");
		}

		// samplers and locations
		pNewShader->Bind();

		loc = pNewShader->findLocation("colorSampler");
		if (loc >= 0)
			glUniform1i(loc, 0);
		
		mLocImageBlurScale = pNewShader->findLocation("scale");

		pNewShader->UnBind();

		mShaderImageBlur.reset(pNewShader);


		//
		// MIX

		pNewShader = new GLSLShader();

		if (nullptr == pNewShader)
		{
			throw std::exception("failed to allocate memory for a mix shader");
		}

		vertex_path = FBString(shadersPath, SHADER_MIX_VERTEX);
		fragment_path = FBString(shadersPath, SHADER_MIX_FRAGMENT);

		if (false == pNewShader->LoadShaders(vertex_path, fragment_path))
		{
			throw std::exception("failed to load and prepare mix shader");
		}

		// samplers and locations
		pNewShader->Bind();

		loc = pNewShader->findLocation("sampler0");
		if (loc >= 0)
			glUniform1i(loc, 0);
		loc = pNewShader->findLocation("sampler1");
		if (loc >= 0)
			glUniform1i(loc, 3);

		pNewShader->UnBind();

		mShaderMix.reset(pNewShader);

		//
		// DOWNSCALE

		pNewShader = new GLSLShader();

		if (nullptr == pNewShader)
		{
			throw std::exception("failed to allocate memory for a downscale shader");
		}

		vertex_path = FBString(shadersPath, SHADER_DOWNSCALE_VERTEX);
		fragment_path = FBString(shadersPath, SHADER_DOWNSCALE_FRAGMENT);

		if (false == pNewShader->LoadShaders(vertex_path, fragment_path))
		{
			throw std::exception("failed to load and prepare downscale shader");
		}

		// samplers and locations
		pNewShader->Bind();

		loc = pNewShader->findLocation("sampler");
		if (loc >= 0)
			glUniform1i(loc, 0);
		
		pNewShader->UnBind();

		mShaderDownscale.reset(pNewShader);
	}
	catch (const std::exception &e)
	{
		FBTrace("Post Effect Chain ERROR: %s\n", e.what());

		if (nullptr != pNewShader)
		{
			delete pNewShader;
			pNewShader = nullptr;
		}

		lSuccess = false;
	}

	return lSuccess;
}


void PostEffectChain::FreeShaders()
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

	mShaderDepthLinearize.reset(nullptr);
	mShaderBlur.reset(nullptr);
	mShaderMix.reset(nullptr);
	mShaderDownscale.reset(nullptr);
}