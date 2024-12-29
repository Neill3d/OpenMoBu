
/**	\file	postprocessing_effect.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectchain.h"
//#include "posteffectssao.h"
#include "posteffectdisplacement.h"
//#include "posteffectmotionblur.h"
//#include "posteffectlensflare.h"
//#include "posteffectcolor.h"
//#include "posteffectdof.h"
//#include "posteffectfilmgrain.h"
//#include "posteffectfisheye.h"
//#include "posteffectvignetting.h"
//#include "posteffectdof.h"
#include "postprocessing_helper.h"
#include "fxmaskingshader.h"
#include "posteffectbuffers.h"

#include "posteffectshader_bilateral_blur.h"

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

#define SHADER_SCENE_MASKED_VERTEX			"\\GLSL\\scene_masked.glslv"
#define SHADER_SCENE_MASKED_FRAGMENT		"\\GLSL\\scene_masked.glslf"

//
extern void LOGE(const char* pFormatString, ...);

// define new task cycle index
FBProfiler_CreateTaskCycle(PostEffectChain, 0.5f, 0.1f, 0.1f);

//! a constructor
PostEffectChain::PostEffectChain()
{
	//
	// Register task cycle index in profiler.
	//
	FBProfiling_SetupTaskCycle(PostEffectChain);

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

bool PostEffectChain::Prep(PostPersistentData *pData, PostEffectContext& effectContext)
{
	bool lSuccess = true;

	mSettings = pData;
	mLastCamera = effectContext.camera;

	if (!mSettings.Ok())
		return false;

	if (mSettings.Ok())
	{
		if (mSettings->IsNeedToReloadShaders())
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
		if (!LoadShaders())
			lSuccess = false;

		mNeedReloadShaders = false;
	}

	// update UI values
	/*
	if (true == mSettings->FishEye && mFishEye.get())
		mFishEye->CollectUIValues(mSettings, effectContext);
	
	if (true == mSettings->ColorCorrection && mColor.get())
		mColor->CollectUIValues(mSettings, effectContext);
	
	if (true == mSettings->Vignetting && mVignetting.get())
		mVignetting->CollectUIValues(mSettings, effectContext);
	
	if (true == mSettings->FilmGrain && mFilmGrain.get())
		mFilmGrain->CollectUIValues(mSettings, effectContext);

	if (true == mSettings->LensFlare && mLensFlare.get())
		mLensFlare->CollectUIValues(mSettings, effectContext);

	if (true == mSettings->SSAO && mSSAO.get())
		mSSAO->CollectUIValues(mSettings, effectContext);

	if (true == mSettings->DepthOfField && mDOF.get())
		mDOF->CollectUIValues(mSettings, effectContext);

	if (true == mSettings->MotionBlur && mMotionBlur.get())
		mMotionBlur->CollectUIValues(mSettings, effectContext);
		*/
	
	if (true == mSettings->Displacement && mDisplacement.get())
		mDisplacement->CollectUIValues(mSettings, effectContext);

	if (mEffectBilateralBlur.get())
		mEffectBilateralBlur->CollectUIValues(pData, effectContext);

	for (int i = 0, count = mSettings->GetNumberOfActiveUserEffects(); i < count; ++i)
	{
		if (PostEffectBase* postEffect = mSettings->GetActiveUserEffect(i))
		{
			postEffect->CollectUIValues(mSettings, effectContext);
		}
	}

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

bool PostEffectChain::HasAnyMaskUsedByEffect() const
{
	if (!mSettings.Ok())
		return false;

	return mSettings->HasAnyActiveMasking();
}

bool PostEffectChain::IsMaskUsedByEffect(const EMaskingChannel maskId) const
{
	if (!mSettings.Ok())
		return false;

	return mSettings->IsMaskActive(maskId);
}

bool PostEffectChain::HasAnyObjectMasked() const
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

bool PostEffectChain::IsAnyObjectMaskedByMaskId(const EMaskingChannel maskId) const
{
	FBScene* scene = mSystem.Scene;

	for (int i = 0; i < scene->Shaders.GetCount(); ++i)
	{
		FBShader* shader = scene->Shaders[i];

		if (FBIS(shader, FXMaskingShader))
		{
			if (FXMaskingShader* maskingShader = FBCast<FXMaskingShader>(shader))
			{
				if ((maskingShader->CompositeMaskA && maskId == EMaskingChannel::eMaskA)
					|| (maskingShader->CompositeMaskB && maskId == EMaskingChannel::eMaskB)
					|| (maskingShader->CompositeMaskC && maskId == EMaskingChannel::eMaskC)
					|| (maskingShader->CompositeMaskD && maskId == EMaskingChannel::eMaskD))
				{
					for (int j = 0; j < shader->GetDstCount(); ++j)
					{
						if (FBIS(shader->GetDst(j), FBModel))
							return true;
					}
				}
			}
		}
	}
	return false;
}

void PostEffectChain::RenderSceneMaskToTexture(const int maskIndex, PostPersistentData::SMaskProperties& maskProps, PostEffectBuffers* buffers)
{
	if (mShaderSceneMasked.get() == nullptr)
		return;

	FrameBuffer* maskBuffer = buffers->GetBufferMaskPtr();
	maskBuffer->Bind(maskIndex);

	glViewport(0, 0, maskBuffer->GetWidth(), maskBuffer->GetHeight());

	if (!maskProps.InvertMask)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	}
	else
	{
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	}

	glClear(GL_COLOR_BUFFER_BIT);

	nv::vec4 baseColor;
	nv::vec4 rimColor;

	if (!maskProps.InvertMask)
	{
		baseColor = nv::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		rimColor = nv::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}
	else
	{
		baseColor = nv::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		rimColor = nv::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	mShaderSceneMasked->Bind();

	const float rimFactor = static_cast<float>(maskProps.UseRimForMask * 0.01);
	const float rimPower = static_cast<float>(maskProps.MaskRimPower * 0.01);

	mShaderSceneMasked->setUniformVector("baseColor", baseColor.x, baseColor.y, baseColor.z, baseColor.w);
	mShaderSceneMasked->setUniformVector("rimOptions", rimFactor, rimPower, 0.0f, 0.0f);
	mShaderSceneMasked->setUniformVector("rimColor", rimColor.x, rimColor.y, rimColor.z, rimColor.w);

	RenderMaskedModels(maskIndex, mLastCamera);

	mShaderSceneMasked->UnBind();

	maskBuffer->UnBind();
}

bool PostEffectChain::PrepareChainOrder(std::vector<PostEffectBase*>& chain, int& blurAndMix, int& blurAndMix2)
{
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

	count += mSettings->GetNumberOfActiveUserEffects();
	
	if (0 == count && 0 == mSettings->OutputPreview.AsInt())
		return false;

	chain.resize(count);

	count = 0;

	blurAndMix = -1;
	blurAndMix2 = -1;

	// ordering HERE

	if (mSettings->SSAO)
	{
		chain[count] = mSSAO.get();
		chain[count]->SetMaskIndex((mSettings->SSAO_UseMasking) ? static_cast<int>(mSettings->SSAO_MaskingChannel) : -1);
		if (mSettings->SSAO_Blur)
		{
			blurAndMix = count;
		}
		count += 1;
	}
	if (mSettings->MotionBlur)
	{
		chain[count] = mMotionBlur.get();
		chain[count]->SetMaskIndex((mSettings->MotionBlur_UseMasking) ? static_cast<int>(mSettings->MotionBlur_MaskingChannel) : -1);
		count += 1;
	}
	if (mSettings->DepthOfField)
	{
		chain[count] = mDOF.get();
		chain[count]->SetMaskIndex((mSettings->DOF_UseMasking) ? static_cast<int>(mSettings->DOF_MaskingChannel) : -1);
		count += 1;
	}
	if (mSettings->ColorCorrection)
	{
		chain[count] = mColor.get();
		chain[count]->SetMaskIndex((mSettings->ColorCorrection_UseMasking) ? static_cast<int>(mSettings->ColorCorrection_MaskingChannel) : -1);
		if (mSettings->Bloom)
		{
			blurAndMix2 = count;
		}
		count += 1;
	}
	if (mSettings->LensFlare)
	{
		chain[count] = mLensFlare.get();
		chain[count]->SetMaskIndex((mSettings->LensFlare_UseMasking) ? static_cast<int>(mSettings->LensFlare_MaskingChannel) : -1);
		count += 1;
	}
	if (mSettings->Displacement)
	{
		chain[count] = mDisplacement.get();
		chain[count]->SetMaskIndex((mSettings->Disp_UseMasking) ? static_cast<int>(mSettings->Disp_MaskingChannel) : -1);
		count += 1;
	}
	if (mSettings->FishEye)
	{
		chain[count] = mFishEye.get();
		chain[count]->SetMaskIndex((mSettings->FishEye_UseMasking) ? static_cast<int>(mSettings->FishEye_MaskingChannel) : -1);
		count += 1;
	}
	if (mSettings->FilmGrain)
	{
		chain[count] = mFilmGrain.get();
		chain[count]->SetMaskIndex((mSettings->FilmGrain_UseMasking) ? static_cast<int>(mSettings->FilmGrain_MaskingChannel) : -1);
		count += 1;
	}
	if (mSettings->Vignetting)
	{
		chain[count] = mVignetting.get();
		chain[count]->SetMaskIndex((mSettings->Vign_UseMasking) ? static_cast<int>(mSettings->Vign_MaskingChannel) : -1);
		count += 1;
	}

	// TODO: add user effects here !
	for (int i = 0; i < mSettings->GetNumberOfActiveUserEffects(); ++i)
	{
		chain[count] = mSettings->GetActiveUserEffect(i);
		count += 1;
	}

	return true;
}


void PostEffectChain::RenderEffectToChain(PostEffectBase* effect, PostEffectBuffers* chainBuffers, bool generateMips, int w, int h)
{
	const PostEffectBase::EffectContext context{
		chainBuffers->GetSrcBufferPtr()->GetColorObject(), // src texture id
		chainBuffers->GetBufferDepthPtr()->GetDepthObject(), // depth texture id
		chainBuffers,
		chainBuffers->GetDstBufferPtr(), // dst frame buffer
		w,
		h,
		generateMips
	};

	effect->Process(context);


	/*
	for (int j = 0; j < effect->GetNumberOfPasses(); ++j)
	{
		effect->PrepPass(j);

		// bind an input source image for processing by the effect
		const GLuint texid = chainBuffers->GetSrcBufferPtr()->GetColorObject();
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

		// apply effect into dst buffer
		chainBuffers->GetDstBufferPtr()->Bind();

		drawOrthoQuad2d(w, h);

		chainBuffers->GetDstBufferPtr()->UnBind(generateMips);

		//
		chainBuffers->SwapBuffers();
	}*/
}

void PostEffectChain::BilateralBlurPass(PostEffectBuffers* buffers)
{
	const int w = buffers->GetWidth();
	const int h = buffers->GetHeight();

	const float blurSharpness = 0.1f * (float)mSettings->SSAO_BlurSharpness;
	const float invRes[2] = { 1.0f / float(w), 1.0f / float(h) };

	// Bilateral Blur Pass

	const GLuint texid = buffers->GetSrcBufferPtr()->GetColorObject();
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

void PostEffectChain::BilateralBlurAndMixPass(PostEffectBuffers* buffers)
{
	const int w = buffers->GetWidth();
	const int h = buffers->GetHeight();

	const float blurSharpness = 0.1f * (float)mSettings->SSAO_BlurSharpness;
	const float invRes[2] = { 1.0f / float(w), 1.0f / float(h) };

	// Bilateral Blur Pass

	GLuint texid = buffers->GetSrcBufferPtr()->GetColorObject();
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

void PostEffectChain::SendPreview(PostEffectBuffers* buffers, double systime)
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

void PostEffectChain::RenderLinearDepth(PostEffectBuffers* buffers)
{
	const GLuint depthId = buffers->GetSrcBufferPtr()->GetDepthObject();

	// prep data

	const float znear = static_cast<float>(mLastCamera->NearPlaneDistance);
	const float zfar = static_cast<float>(mLastCamera->FarPlaneDistance);
	FBCameraType cameraType;
	mLastCamera->Type.GetData(&cameraType, sizeof(FBCameraType));
	const bool perspective = (cameraType == FBCameraType::kFBCameraTypePerspective);

	const float clipInfo[4]{
		znear * zfar,
		znear - zfar,
		zfar,
		(perspective) ? 1.0f : 0.0f
	};

	FrameBuffer* pBufferDepth = buffers->GetBufferDepthPtr();

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

	glActiveTexture(GL_TEXTURE0 + CommonEffectUniforms::GetLinearDepthSamplerSlot());
	glBindTexture(GL_TEXTURE_2D, linearDepthId);
	glActiveTexture(GL_TEXTURE0);
}

void PostEffectChain::BlurMasksPass(const int maskIndex, PostEffectBuffers* buffers)
{
	assert(maskIndex < PostPersistentData::NUMBER_OF_MASKS);

	// Bilateral Blur Pass

	

	const FrameBuffer* maskBuffer = buffers->GetBufferMaskPtr();
	const GLuint texid = maskBuffer->GetColorObject(maskIndex);
	const int w = buffers->GetWidth();
	const int h = buffers->GetHeight();

	PostEffectBase::EffectContext context
	{
		texid,
		0,
		nullptr,
		buffers->GetBufferBlurPtr(),
		w,
		h,
		false
	};

	mEffectBilateralBlur->SetMaskIndex(maskIndex);
	mEffectBilateralBlur->Process(context);

	BlitFBOToFBOCustomAttachment(buffers->GetBufferBlurPtr()->GetFrameBuffer(), buffers->GetBufferBlurPtr()->GetWidth(), buffers->GetBufferBlurPtr()->GetHeight(), 0,
		buffers->GetBufferMaskPtr()->GetFrameBuffer(), buffers->GetBufferMaskPtr()->GetWidth(), buffers->GetBufferMaskPtr()->GetHeight(), maskIndex);
}

void PostEffectChain::MixMasksPass(const int maskIndex, const int maskIndex2, PostEffectBuffers* buffers)
{
	const FrameBuffer* maskBuffer = buffers->GetBufferMaskPtr();

	// Mix masks = Mask A * Mask B

	const GLuint texid = maskBuffer->GetColorObject(maskIndex);
	const GLuint texid2 = maskBuffer->GetColorObject(maskIndex2);
	
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texid2);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texid);

	buffers->GetBufferBlurPtr()->Bind();
	mShaderMix->Bind();

	const int w = buffers->GetWidth();
	const int h = buffers->GetHeight();

	mShaderMix->setUniformVector("gBloom", 0.0f, 0.0f, 1.0f, 0.0f);

	drawOrthoQuad2d(w, h);

	mShaderMix->UnBind();
	buffers->GetBufferBlurPtr()->UnBind();

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	BlitFBOToFBOCustomAttachment(buffers->GetBufferBlurPtr()->GetFrameBuffer(), buffers->GetBufferBlurPtr()->GetWidth(), buffers->GetBufferBlurPtr()->GetHeight(), 0,
		buffers->GetBufferMaskPtr()->GetFrameBuffer(), buffers->GetBufferMaskPtr()->GetWidth(), buffers->GetBufferMaskPtr()->GetHeight(), maskIndex);
}

/*
bool PostEffectChain::Process(PostEffectBuffers *buffers, double systime)
{
	std::vector<PostEffectBase*>		mChain;

	//
	// Start PostEffectChain task cycle profiling, 
	//
	FBProfilerHelper lProfiling(FBProfiling_TaskCycleIndex(PostEffectChain), FBGetDisplayInfo(), FBGetRenderingTaskCycle());

	mIsCompressedDataReady = false;

	if (!buffers->Ok() || !mSettings.Ok())
		return false;

	// 1. prepare chain count and order
	int blurAndMix{ -1 };
	int blurAndMix2{ -1 };

	if (!PrepareChainOrder(mChain, blurAndMix, blurAndMix2))
		return false;

	// 2. prepare and render into masks
	
	bool isMaskTextureBinded = false;
	bool isMaskBlurRequested = false;
	bool isMaskMixRequested = false;

	bool maskRenderFlags[PostPersistentData::NUMBER_OF_MASKS] = { false, false, false, false };

	if (HasAnyMaskUsedByEffect() && HasAnyObjectMasked() && mLastCamera)
	{
		for (int i = 0; i < PostPersistentData::NUMBER_OF_MASKS; ++i)
		{
			const EMaskingChannel maskId = static_cast<EMaskingChannel>(i);
			if (IsMaskUsedByEffect(maskId) && IsAnyObjectMaskedByMaskId(maskId))
			{
				if (mSettings->Masks[i].BlurMask)
				{
					isMaskBlurRequested = true;
				}

				if (mSettings->Masks[i].UseMixWithMask && mSettings->Masks[i].MixWithMask != maskId)
				{
					isMaskMixRequested = true;
					const int maskIndex2 = static_cast<int>(mSettings->Masks[i].MixWithMask);
					maskRenderFlags[maskIndex2] = true;
				}

				maskRenderFlags[i] = true;
				isMaskTextureBinded = true;
			}
		}

		for (int i = 0; i < PostPersistentData::NUMBER_OF_MASKS; ++i)
		{
			if (maskRenderFlags[i])
			{
				RenderSceneMaskToTexture(i, mSettings->Masks[i], buffers);
			}
		}
	}

	// 3. in case of SSAO active, render a linear depth texture
	bool isLinearDepthSamplerBinded = mSettings->SSAO;
	if (!isLinearDepthSamplerBinded)
	{
		for (int i = 0, count = static_cast<int>(mChain.size()); i < count; ++i)
		{
			if (mChain[i] && mChain[i]->IsLinearDepthSamplerUsed())
			{
				isLinearDepthSamplerBinded = true;
				break;
			}
		}
	}
	if (isLinearDepthSamplerBinded)
	{
		RenderLinearDepth(buffers);
		isLinearDepthSamplerBinded = true;
	}
	
	// 4a. blur masks (if applied)

	if (isMaskBlurRequested)
	{
		for (int i = 0; i < PostPersistentData::NUMBER_OF_MASKS; ++i)
		{
			if (maskRenderFlags[i] && mSettings->Masks[i].BlurMask)
			{
				BlurMasksPass(i, buffers);
			}
		}
	}

	// 4b. mix masks (if applied)
	if (isMaskMixRequested)
	{
		for (int i = 0; i < PostPersistentData::NUMBER_OF_MASKS; ++i)
		{
			const int mask2 = static_cast<int>(mSettings->Masks[i].MixWithMask);

			if (maskRenderFlags[i] && mSettings->Masks[i].UseMixWithMask 
				&& i != mask2
				&& maskRenderFlags[mask2])
			{
				MixMasksPass(i, mask2, buffers);
			}
		}
	}

	// user option to show only mask result on a screen
	const unsigned int globalMaskingIndex = static_cast<unsigned int>(mSettings->GetGlobalMaskIndex());

	if (mSettings->DebugDisplyMasking)
	{
		FrameBuffer* maskBuffer = buffers->GetBufferMaskPtr();
		BlitFBOToFBOCustomAttachment(maskBuffer->GetFrameBuffer(), maskBuffer->GetWidth(), maskBuffer->GetHeight(), globalMaskingIndex,
			buffers->GetDstBufferPtr()->GetFrameBuffer(), buffers->GetDstBufferPtr()->GetWidth(), buffers->GetDstBufferPtr()->GetHeight(), 0u);
		buffers->SwapBuffers();
		return true;
	}

	// 5. bind textures of mask and depth for effects

	if (isMaskTextureBinded)
	{	
		const GLuint maskTextureId = buffers->GetBufferMaskPtr()->GetColorObject(globalMaskingIndex);
		glActiveTexture(GL_TEXTURE0 + CommonEffectUniforms::GetMaskSamplerSlot());
		glBindTexture(GL_TEXTURE_2D, maskTextureId);
		glActiveTexture(GL_TEXTURE0);
	}
	
	bool isDepthSamplerBinded = mSettings->DepthOfField;

	if (!isDepthSamplerBinded)
	{
		for (int i = 0, count = static_cast<int>(mChain.size()); i < count; ++i)
		{
			if (mChain[i] && mChain[i]->IsDepthSamplerUsed())
			{
				isDepthSamplerBinded = true;
				break;
			}
		}
	}
	
	if (isDepthSamplerBinded)
	{
		const GLuint depthId = buffers->GetSrcBufferPtr()->GetDepthObject();

		glActiveTexture(GL_TEXTURE0 + CommonEffectUniforms::GetDepthSamplerSlot());
		glBindTexture(GL_TEXTURE_2D, depthId);
		glActiveTexture(GL_TEXTURE0);
	}

	// compute effect chain with double buffer

	// DONE: when buffer is attached, buffer is used itself !
	bool lSuccess = false;
	const bool generateMips = mSettings->GenerateMipMaps;

	if (!mChain.empty())
	{
		// optional. generate mipmaps for the first target
		GLuint texid = buffers->GetSrcBufferPtr()->GetColorObject();
		const int w = buffers->GetWidth();
		const int h = buffers->GetHeight();

		if (true == generateMips)
		{
			if (texid > 0)
			{
				glBindTexture(GL_TEXTURE_2D, texid);
				glGenerateMipmap(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}
		
		// 6. render each effect in order

		for (int i = 0, count=static_cast<int>(mChain.size()); i < count; ++i)
		{
			PostEffectBase* effect = mChain[i];
			if (!effect)
				continue;

			// activate local mask for the effect if applied
			const unsigned int effectMaskingIndex = (effect->GetMaskIndex() >= 0) ? static_cast<unsigned int>(effect->GetMaskIndex()) : globalMaskingIndex;
			if (IsAnyObjectMaskedByMaskId(static_cast<EMaskingChannel>(effect->GetMaskIndex())) && effectMaskingIndex != globalMaskingIndex)
			{
				const GLuint maskTextureId = buffers->GetBufferMaskPtr()->GetColorObject(effectMaskingIndex);
				glActiveTexture(GL_TEXTURE0 + CommonEffectUniforms::GetMaskSamplerSlot());
				glBindTexture(GL_TEXTURE_2D, maskTextureId);
				glActiveTexture(GL_TEXTURE0);
			}

			if (!effect->IsRenderToTexture())
			{
				RenderEffectToChain(effect, buffers, generateMips, w, h);
			}
			else
			{
				RenderEffectToBuffer(effect, buffers->GetSrcBufferPtr()->GetColorObject());
			}
			
			// 7. blur effect if applied

			// if we need more passes, blur and mix for SSAO or Bloom (Color Correction)
			if (i == blurAndMix || i == blurAndMix2)
			{
				if (false == mSettings->OnlyAO || (i == blurAndMix2))
				{
					BilateralBlurAndMixPass(buffers);
				}
				else
				{
					BilateralBlurPass(buffers);
				}
			}

			// if local masking index was used, switch back to global mask for next effect
			if (effectMaskingIndex != globalMaskingIndex)
			{
				const GLuint maskTextureId = buffers->GetBufferMaskPtr()->GetColorObject(globalMaskingIndex);
				glActiveTexture(GL_TEXTURE0 + CommonEffectUniforms::GetMaskSamplerSlot());
				glBindTexture(GL_TEXTURE_2D, maskTextureId);
				glActiveTexture(GL_TEXTURE0);
			}
		}

		lSuccess = true;
	}

	// optional. send a preview packet if applied (rate - compress 25-30 frames per second)
	if (mSettings->OutputPreview)
	{
		SendPreview(buffers, systime);
	}

	// unbind additional texture slots (from depth, masks)

	if (isDepthSamplerBinded)
	{
		glActiveTexture(GL_TEXTURE0 + CommonEffectUniforms::GetDepthSamplerSlot());
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	if (isLinearDepthSamplerBinded)
	{
		glActiveTexture(GL_TEXTURE0 + CommonEffectUniforms::GetLinearDepthSamplerSlot());
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
*/

bool PostEffectChain::Process(PostEffectBuffers* buffers, double systime)
{
	std::vector<PostEffectBase*>		mChain;

	//
	// Start PostEffectChain task cycle profiling, 
	//
	FBProfilerHelper lProfiling(FBProfiling_TaskCycleIndex(PostEffectChain), FBGetDisplayInfo(), FBGetRenderingTaskCycle());

	mIsCompressedDataReady = false;

	if (!buffers->Ok() || !mSettings.Ok())
		return false;

	// 1. prepare chain count and order
	int blurAndMix{ -1 };
	int blurAndMix2{ -1 };

	if (!PrepareChainOrder(mChain, blurAndMix, blurAndMix2))
		return false;

	// 2. prepare and render into masks

	bool isMaskTextureBinded = false;
	bool isMaskBlurRequested = false;
	bool isMaskMixRequested = false;

	bool maskRenderFlags[PostPersistentData::NUMBER_OF_MASKS] = { false, false, false, false };

	if (HasAnyMaskUsedByEffect() && HasAnyObjectMasked() && mLastCamera)
	{
		for (int i = 0; i < PostPersistentData::NUMBER_OF_MASKS; ++i)
		{
			const EMaskingChannel maskId = static_cast<EMaskingChannel>(i);
			if (IsMaskUsedByEffect(maskId) && IsAnyObjectMaskedByMaskId(maskId))
			{
				if (mSettings->Masks[i].BlurMask)
				{
					isMaskBlurRequested = true;
				}

				if (mSettings->Masks[i].UseMixWithMask && mSettings->Masks[i].MixWithMask != maskId)
				{
					isMaskMixRequested = true;
					const int maskIndex2 = static_cast<int>(mSettings->Masks[i].MixWithMask);
					maskRenderFlags[maskIndex2] = true;
				}

				maskRenderFlags[i] = true;
				isMaskTextureBinded = true;
			}
		}

		for (int i = 0; i < PostPersistentData::NUMBER_OF_MASKS; ++i)
		{
			if (maskRenderFlags[i])
			{
				RenderSceneMaskToTexture(i, mSettings->Masks[i], buffers);
			}
		}
	}

	// 3. in case of SSAO active, render a linear depth texture
	const bool isLinearDepthSamplerBinded = std::find_if(begin(mChain), end(mChain), [](const PostEffectBase * effect) 
		{
			return effect->IsLinearDepthSamplerUsed();
		}) != end(mChain);
	
	if (isLinearDepthSamplerBinded)
	{
		RenderLinearDepth(buffers);
	}

	// 4a. blur masks (if applied)

	if (isMaskBlurRequested)
	{
		for (int i = 0; i < PostPersistentData::NUMBER_OF_MASKS; ++i)
		{
			if (maskRenderFlags[i] && mSettings->Masks[i].BlurMask)
			{
				BlurMasksPass(i, buffers);
			}
		}
	}

	// 4b. mix masks (if applied)
	if (isMaskMixRequested)
	{
		for (int i = 0; i < PostPersistentData::NUMBER_OF_MASKS; ++i)
		{
			const int mask2 = static_cast<int>(mSettings->Masks[i].MixWithMask);

			if (maskRenderFlags[i] && mSettings->Masks[i].UseMixWithMask
				&& i != mask2
				&& maskRenderFlags[mask2])
			{
				MixMasksPass(i, mask2, buffers);
			}
		}
	}

	// user option to show only mask result on a screen
	const unsigned int globalMaskingIndex = static_cast<unsigned int>(mSettings->GetGlobalMaskIndex());

	if (mSettings->DebugDisplyMasking)
	{
		FrameBuffer* maskBuffer = buffers->GetBufferMaskPtr();
		BlitFBOToFBOCustomAttachment(maskBuffer->GetFrameBuffer(), maskBuffer->GetWidth(), maskBuffer->GetHeight(), globalMaskingIndex,
			buffers->GetDstBufferPtr()->GetFrameBuffer(), buffers->GetDstBufferPtr()->GetWidth(), buffers->GetDstBufferPtr()->GetHeight(), 0u);
		buffers->SwapBuffers();
		return true;
	}

	// 5. bind textures of mask and depth for effects

	if (isMaskTextureBinded)
	{
		const GLuint maskTextureId = buffers->GetBufferMaskPtr()->GetColorObject(globalMaskingIndex);
		glActiveTexture(GL_TEXTURE0 + CommonEffectUniforms::GetMaskSamplerSlot());
		glBindTexture(GL_TEXTURE_2D, maskTextureId);
		glActiveTexture(GL_TEXTURE0);
	}

	const bool isDepthSamplerBinded = std::find_if(begin(mChain), end(mChain), [](const PostEffectBase* effect)
		{
			return effect->IsDepthSamplerUsed();
		}) != end(mChain);

	if (isDepthSamplerBinded)
	{
		const GLuint depthId = buffers->GetSrcBufferPtr()->GetDepthObject();

		glActiveTexture(GL_TEXTURE0 + CommonEffectUniforms::GetDepthSamplerSlot());
		glBindTexture(GL_TEXTURE_2D, depthId);
		glActiveTexture(GL_TEXTURE0);
	}

	// compute effect chain with double buffer

	// DONE: when buffer is attached, buffer is used itself !
	bool lSuccess = false;
	const bool generateMips = mSettings->GenerateMipMaps;

	if (!mChain.empty())
	{
		// optional. generate mipmaps for the first target
		GLuint texid = buffers->GetSrcBufferPtr()->GetColorObject();
		const int w = buffers->GetWidth();
		const int h = buffers->GetHeight();

		if (generateMips)
		{
			if (texid > 0)
			{
				glBindTexture(GL_TEXTURE_2D, texid);
				glGenerateMipmap(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}

		// 6. render each effect in order

		for (const auto& effect : mChain)
		{
			if (!effect)
				continue;

			// activate local mask for the effect if applied
			const unsigned int effectMaskingIndex = (effect->GetMaskIndex() >= 0) ? static_cast<unsigned int>(effect->GetMaskIndex()) : globalMaskingIndex;
			if (IsAnyObjectMaskedByMaskId(static_cast<EMaskingChannel>(effect->GetMaskIndex())) && effectMaskingIndex != globalMaskingIndex)
			{
				const GLuint maskTextureId = buffers->GetBufferMaskPtr()->GetColorObject(effectMaskingIndex);
				glActiveTexture(GL_TEXTURE0 + CommonEffectUniforms::GetMaskSamplerSlot());
				glBindTexture(GL_TEXTURE_2D, maskTextureId);
				glActiveTexture(GL_TEXTURE0);
			}

			RenderEffectToChain(effect, buffers, generateMips, w, h);
			
			// 7. blur effect if applied
			/*
			// if we need more passes, blur and mix for SSAO or Bloom (Color Correction)
			if (i == blurAndMix || i == blurAndMix2)
			{
				if (false == mSettings->OnlyAO || (i == blurAndMix2))
				{
					BilateralBlurAndMixPass(buffers);
				}
				else
				{
					BilateralBlurPass(buffers);
				}
			}
			*/
			// if local masking index was used, switch back to global mask for next effect
			if (effectMaskingIndex != globalMaskingIndex)
			{
				const GLuint maskTextureId = buffers->GetBufferMaskPtr()->GetColorObject(globalMaskingIndex);
				glActiveTexture(GL_TEXTURE0 + CommonEffectUniforms::GetMaskSamplerSlot());
				glBindTexture(GL_TEXTURE_2D, maskTextureId);
				glActiveTexture(GL_TEXTURE0);
			}
		}

		lSuccess = true;
	}

	// optional. send a preview packet if applied (rate - compress 25-30 frames per second)
	if (mSettings->OutputPreview)
	{
		SendPreview(buffers, systime);
	}

	// unbind additional texture slots (from depth, masks)

	if (isDepthSamplerBinded)
	{
		glActiveTexture(GL_TEXTURE0 + CommonEffectUniforms::GetDepthSamplerSlot());
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	if (isLinearDepthSamplerBinded)
	{
		glActiveTexture(GL_TEXTURE0 + CommonEffectUniforms::GetLinearDepthSamplerSlot());
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

PostEffectBase* PostEffectChain::ShaderFactory(const BuildInEffect effectType, const char *shadersLocation, bool immediatelyLoad)
{

	PostEffectBase *newEffect = nullptr;

	switch (effectType)
	{
	//case SHADER_TYPE_FISHEYE:
	//	newEffect = new PostEffectFishEye();
	//	break;
	//case SHADER_TYPE_COLOR:
	//	newEffect = new PostEffectColor();
	//	break;
	//case SHADER_TYPE_VIGNETTE:
	//	newEffect = new PostEffectVignetting();
	//	break;
	//case SHADER_TYPE_FILMGRAIN:
	//	newEffect = new PostEffectFilmGrain();
	//	break;
	//case SHADER_TYPE_LENSFLARE:
	//	newEffect = new PostEffectLensFlare();
	//	break;
	//case SHADER_TYPE_SSAO:
	//	newEffect = new PostEffectSSAO();
	//	break;
	//case SHADER_TYPE_DOF:
	//	newEffect = new PostEffectDOF();
	//	break;
	case BuildInEffect::DISPLACEMENT:
		newEffect = new PostEffectDisplacement();
		break;
	//case SHADER_TYPE_MOTIONBLUR:
	//	newEffect = new PostEffectMotionBlur();
	//	break;
	}

	if (immediatelyLoad)
	{
		if (!newEffect->Load(shadersLocation))
		{
			LOGE("Post Effect failed to Load\n");

			delete newEffect;
			newEffect = nullptr;
		}
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
		SHADER_DOWNSCALE_FRAGMENT,

		SHADER_SCENE_MASKED_VERTEX,
		SHADER_SCENE_MASKED_FRAGMENT
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

		std::unique_ptr<GLSLShaderProgram> pNewShader(new GLSLShaderProgram);

		FBString vertex_path(shadersPath, SHADER_DEPTH_LINEARIZE_VERTEX);
		FBString fragment_path(shadersPath, SHADER_DEPTH_LINEARIZE_FRAGMENT);

		if (!pNewShader->LoadShaders(vertex_path, fragment_path))
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

		mShaderDepthLinearize.reset(pNewShader.release());

		//
		// BLUR (for SSAO)

		pNewShader.reset(new GLSLShaderProgram);

		vertex_path = FBString(shadersPath, SHADER_BLUR_VERTEX);
		fragment_path = FBString(shadersPath, SHADER_BLUR_FRAGMENT);

		if (!pNewShader->LoadShaders(vertex_path, fragment_path))
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

		mShaderBlur.reset(pNewShader.release());

		//
		// IMAGE BLUR, simple bilateral blur

		mEffectBilateralBlur.reset(new PostEffectBilateralBlur());
		if (!mEffectBilateralBlur->Load(shadersPath))
		{
			throw std::exception("failed to load and prepare image blur shader");
		}

		/*
		pNewShader.reset(new GLSLShaderProgram);

		vertex_path = FBString(shadersPath, SHADER_IMAGE_BLUR_VERTEX);
		fragment_path = FBString(shadersPath, SHADER_IMAGE_BLUR_FRAGMENT);

		if (!pNewShader->LoadShaders(vertex_path, fragment_path))
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

		mShaderImageBlur.reset(pNewShader.release());
		*/
		//
		// MIX

		pNewShader.reset(new GLSLShaderProgram);

		vertex_path = FBString(shadersPath, SHADER_MIX_VERTEX);
		fragment_path = FBString(shadersPath, SHADER_MIX_FRAGMENT);

		if (!pNewShader->LoadShaders(vertex_path, fragment_path))
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

		mShaderMix.reset(pNewShader.release());

		//
		// DOWNSCALE

		pNewShader.reset(new GLSLShaderProgram);

		vertex_path = FBString(shadersPath, SHADER_DOWNSCALE_VERTEX);
		fragment_path = FBString(shadersPath, SHADER_DOWNSCALE_FRAGMENT);

		if (!pNewShader->LoadShaders(vertex_path, fragment_path))
		{
			throw std::exception("failed to load and prepare downscale shader");
		}

		// samplers and locations
		pNewShader->Bind();

		loc = pNewShader->findLocation("sampler");
		if (loc >= 0)
			glUniform1i(loc, 0);
		
		pNewShader->UnBind();

		mShaderDownscale.reset(pNewShader.release());

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
		FBTrace("Post Effect Chain ERROR: %s\n", e.what());
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