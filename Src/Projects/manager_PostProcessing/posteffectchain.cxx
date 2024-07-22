
/**	\file	postprocessing_effect.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectchain.h"
#include "posteffectssao.h"
#include "posteffectdisplacement.h"
#include "posteffectmotionblur.h"
#include "posteffectlensflare.h"
#include "posteffectcolor.h"
#include "posteffectdof.h"
#include "posteffectfilmgrain.h"
#include "posteffectfisheye.h"
#include "posteffectvignetting.h"
#include "posteffectdof.h"
#include "postprocessing_helper.h"
#include "fxmaskingshader.h"
#include "posteffectbuffers.h"

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

	if (true == mSettings->FishEye && mFishEye.get())
		mFishEye->CollectUIValues(mSettings, w,h, pCamera);
	
	if (true == mSettings->ColorCorrection && mColor.get())
		mColor->CollectUIValues(mSettings, w,h, pCamera);
	
	if (true == mSettings->Vignetting && mVignetting.get())
		mVignetting->CollectUIValues(mSettings, w,h, pCamera);
	
	if (true == mSettings->FilmGrain && mFilmGrain.get())
		mFilmGrain->CollectUIValues(mSettings, w,h, pCamera);

	if (true == mSettings->LensFlare && mLensFlare.get())
		mLensFlare->CollectUIValues(mSettings, w,h, pCamera);

	if (true == mSettings->SSAO && mSSAO.get())
		mSSAO->CollectUIValues(mSettings, w,h, pCamera);

	if (true == mSettings->DepthOfField && mDOF.get())
		mDOF->CollectUIValues(mSettings, w, h, pCamera);

	if (true == mSettings->Displacement && mDisplacement.get())
		mDisplacement->CollectUIValues(mSettings, w, h, pCamera);

	if (true == mSettings->MotionBlur && mMotionBlur.get())
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