
/**	\file	postprocessing_effect.cxx

Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectchain.h"
#include "standardeffectcollection.h"
#include "postprocessing_helper.h"
#include "fxmaskingshader.h"
#include "posteffectbuffers.h"
#include "posteffect_userobject.h"
#include "posteffectbase.h"



#include "mobu_logging.h"
#include "hashUtils.h"

// define new task cycle index
FBProfiler_CreateTaskCycle(PostEffectChain, 0.5f, 0.1f, 0.1f);

uint32_t PostEffectChain::DOUBLE_BUFFER_NAME_KEY = xxhash32("doubleBuffer");
uint32_t PostEffectChain::MASK_BUFFER_NAME_KEY = xxhash32("mask");
uint32_t PostEffectChain::DEPTH_LINEAR_BUFFER_NAME_KEY = xxhash32("depthLinearize");

//! a constructor
PostEffectChain::PostEffectChain()
{
	//
	// Register task cycle index in profiler.
	//
	FBProfiling_SetupTaskCycle(PostEffectChain);

	mIsCompressedDataReady = false;
	
	//mLocDepthLinearizeClipInfo = -1;
	mLastCompressTime = 0.0;
}

void PostEffectChain::ChangeContext()
{
	mIsCompressedDataReady = false;
	mLastCompressTime = 0.0;

	mRenderData[0].isReady = false;
	mRenderData[1].isReady = false;
}

bool PostEffectChain::Prep(PostPersistentData *pData, const PostEffectContextMoBu& effectContext)
{
	bool lSuccess = true;

	mSettings = pData;
	mLastCamera = effectContext.GetCamera();

	if (!mSettings.Ok() || !mLastCamera)
		return false;
	
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
	FBScene* scene = FBSystem::TheOne().Scene;

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
	FBScene* scene = FBSystem::TheOne().Scene;

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

void PostEffectChain::RenderSceneMaskToTexture(GLSLShaderProgram* shader, const int maskIndex, PostPersistentData::SMaskProperties& maskProps, PostEffectBuffers* buffers)
{
	if (!shader || !buffers)
		return;

	FrameBuffer* maskBuffer = buffers->RequestFramebuffer(MASK_BUFFER_NAME_KEY);
	if (!maskBuffer)
	{
		LOGE("Failed to request a framebuffer to render scene mask to texture");
		return;
	}
	
	maskBuffer->Bind(maskIndex);

	glViewport(0, 0, maskBuffer->GetWidth(), maskBuffer->GetHeight());

	const bool invert = maskProps.InvertMask;

	const float clearValue = invert ? 1.0f : 0.0f;
	glClearColor(clearValue, clearValue, clearValue, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	static const nv::vec4 white(1.0f, 1.0f, 1.0f, 1.0f);
	static const nv::vec4 black(0.0f, 0.0f, 0.0f, 1.0f);

	const nv::vec4& baseColor = invert ? black : white;
	const nv::vec4& rimColor = invert ? white : black;

	shader->Bind();

	constexpr float PERCENT_TO_FACTOR = 0.01f;
	const float rimFactor = static_cast<float>(maskProps.UseRimForMask) * PERCENT_TO_FACTOR;
	const float rimPower = static_cast<float>(maskProps.MaskRimPower) * PERCENT_TO_FACTOR;

	shader->setUniformVector("baseColor", baseColor.x, baseColor.y, baseColor.z, baseColor.w);
	shader->setUniformVector("rimOptions", rimFactor, rimPower, 0.0f, 0.0f);
	shader->setUniformVector("rimColor", rimColor.x, rimColor.y, rimColor.z, rimColor.w);

	RenderMaskedModels(maskIndex, mLastCamera);

	shader->UnBind();

	maskBuffer->UnBind();

	buffers->ReleaseFramebuffer(MASK_BUFFER_NAME_KEY);
}

bool PostEffectChain::PrepareChainOrder(std::vector<PostEffectBase*>& chain, StandardEffectCollection* collection)
{
	if (!mSettings.Ok() || !collection)
	{
		chain.clear();
		return false;
	}

	chain.clear();
	// 9 built-in effects + user-defined ones
	chain.reserve(9 + static_cast<size_t>(mSettings->GetNumberOfActiveUserEffects()));

	auto addEffect = [&](bool enabled,
		PostEffectBase* effect,
		bool useMask,
		int maskChannel)
		{
			if (!enabled || !effect)
				return;

			effect->SetMaskIndex(useMask ? maskChannel : -1);
			chain.push_back(effect);
		};

	// Ordering HERE
	addEffect(mSettings->SSAO,
		collection->GetSSAOEffect(),
		mSettings->SSAO_UseMasking,
		static_cast<int>(mSettings->SSAO_MaskingChannel));

	addEffect(mSettings->MotionBlur,
		collection->GetMotionBlurEffect(),
		mSettings->MotionBlur_UseMasking,
		static_cast<int>(mSettings->MotionBlur_MaskingChannel));

	addEffect(mSettings->DepthOfField,
		collection->GetDOFEffect(),
		mSettings->DOF_UseMasking,
		static_cast<int>(mSettings->DOF_MaskingChannel));

	addEffect(mSettings->ColorCorrection,
		collection->GetColorEffect(),
		mSettings->ColorCorrection_UseMasking,
		static_cast<int>(mSettings->ColorCorrection_MaskingChannel));

	addEffect(mSettings->LensFlare,
		collection->GetLensFlareEffect(),
		mSettings->LensFlare_UseMasking,
		static_cast<int>(mSettings->LensFlare_MaskingChannel));

	addEffect(mSettings->Displacement,
		collection->GetDisplacementEffect(),
		mSettings->Disp_UseMasking,
		static_cast<int>(mSettings->Disp_MaskingChannel));

	addEffect(mSettings->FishEye,
		collection->GetFishEyeEffect(),
		mSettings->FishEye_UseMasking,
		static_cast<int>(mSettings->FishEye_MaskingChannel));

	addEffect(mSettings->FilmGrain,
		collection->GetFilmGrainEffect(),
		mSettings->FilmGrain_UseMasking,
		static_cast<int>(mSettings->FilmGrain_MaskingChannel));

	addEffect(mSettings->Vignetting,
		collection->GetVignettingEffect(),
		mSettings->Vign_UseMasking,
		static_cast<int>(mSettings->Vign_MaskingChannel));

	// User-defined effects
	const int userCount = mSettings->GetNumberOfActiveUserEffects();
	for (int i = 0; i < userCount; ++i)
	{
		if (auto* effect = mSettings->GetActiveUserEffect(i))
		{
			chain.push_back(effect);
		}
	}

	if (chain.empty() && mSettings->OutputPreview.AsInt() == 0)
		return false;

	return true;
}


void PostEffectChain::SendPreview(PostEffectBuffers* buffers, double systime)
{
	/*
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
	*/
}

void PostEffectChain::RenderLinearDepth(PostEffectBuffers* buffers, PostEffectBase* effect, const GLuint depthId, bool makeDownscale, const PostEffectContextMoBu& effectContext)
{
	if (!buffers || !effect || depthId == 0)
		return;

	int outWidth = buffers->GetWidth();
	int outHeight = buffers->GetHeight();

	if (makeDownscale)
	{
		outWidth = std::max(1, outWidth / 2);
		outHeight = std::max(1, outHeight / 2);
	}

	FrameBuffer* pBufferDepth = buffers->RequestFramebuffer(DEPTH_LINEAR_BUFFER_NAME_KEY,
		outWidth, outHeight, PostEffectBuffers::GetFlagsForSingleColorBuffer(),
		1, false, [](FrameBuffer* frameBuffer) {
			PostEffectBuffers::SetParametersForMainDepthBuffer(frameBuffer);
	});
	if (!pBufferDepth)
	{
		LOGE("Failed to request a framebuffer to render a linear depth");
		return;
	}
	
	PostEffectBase::RenderEffectContext renderContext;
	renderContext.depthTextureId = depthId;
	renderContext.width = outWidth;
	renderContext.height = outHeight;
	renderContext.targetFramebuffer = pBufferDepth;

	glActiveTexture(GL_TEXTURE0 + CommonEffect::DepthSamplerSlot);
	glBindTexture(GL_TEXTURE_2D, depthId);
	glActiveTexture(GL_TEXTURE0);

	effect->Process(renderContext, &effectContext);

	// DONE: bind a depth texture
	const GLuint linearDepthId = pBufferDepth->GetColorObject();

	glActiveTexture(GL_TEXTURE0 + CommonEffect::LinearDepthSamplerSlot);
	glBindTexture(GL_TEXTURE_2D, linearDepthId);
	glActiveTexture(GL_TEXTURE0);
	
	buffers->ReleaseFramebuffer(DEPTH_LINEAR_BUFFER_NAME_KEY);
}

void PostEffectChain::RenderWorldNormals(PostEffectBuffers* buffers)
{
	//TODO:
}

FrameBuffer* PostEffectChain::RequestMaskFrameBuffer(PostEffectBuffers* buffers)
{
	FrameBuffer* maskfb = buffers->RequestFramebuffer(MASK_BUFFER_NAME_KEY, 
		buffers->GetWidth(), buffers->GetHeight(), 
		PostEffectBuffers::GetFlagsForMainColorBuffer(),
		PostPersistentData::NUMBER_OF_MASKS + 1,
		false); // NOTE: 4 masks and 1 mask for processing

	return maskfb;
}

void PostEffectChain::ReleaseMaskFrameBuffer(PostEffectBuffers* buffers)
{
	buffers->ReleaseFramebuffer(MASK_BUFFER_NAME_KEY);
}

FrameBuffer* PostEffectChain::RequestDoubleFrameBuffer(PostEffectBuffers* buffers)
{
	FrameBuffer* buffer = buffers->RequestFramebuffer(DOUBLE_BUFFER_NAME_KEY,
		buffers->GetWidth(), buffers->GetHeight(),
		PostEffectBuffers::GetFlagsForMainColorBuffer(),
		2,
		true, // is auto resize
		[](FrameBuffer* framebuffer) {
			PostEffectBuffers::SetParametersForMainColorBuffer(framebuffer, false);
		});

	return buffer;
}

void PostEffectChain::ReleaseDoubleFrameBuffer(PostEffectBuffers* buffers)
{
	buffers->ReleaseFramebuffer(DOUBLE_BUFFER_NAME_KEY);
}

void PostEffectChain::BlurMasksPass(const int maskIndex, PostEffectBuffers* buffers, PostEffectBilateralBlur* effect, const PostEffectContextMoBu& effectContext)
{
	assert(maskIndex < PostPersistentData::NUMBER_OF_MASKS);
	if (!effect)
		return;

	// Bilateral Blur Pass

	MaskFramebufferRequestScope maskRequest(this, buffers);

	const GLuint texid = maskRequest->GetColorObject(maskIndex);
	const int w = maskRequest->GetWidth();
	const int h = maskRequest->GetHeight();

	PostEffectBase::RenderEffectContext renderContext;
	renderContext.srcTextureId = texid;
	renderContext.width = w;
	renderContext.height = h;
	renderContext.targetFramebuffer = maskRequest.GetFrameBufferPtr();
	renderContext.colorAttachment = PostPersistentData::NUMBER_OF_MASKS; // last color attachment for processing

	effect->SetMaskIndex(maskIndex);
	
	const FBVector2d value = mSettings->GetMaskScale(maskIndex);
	effect->GetBufferShaderTypedPtr()->BlurScale->SetValue(static_cast<float>(value[0]), static_cast<float>(value[1]));

	effect->Process(renderContext, &effectContext);

	BlitFBOToFBOCustomAttachment(maskRequest->GetFrameBuffer(), w, h, PostPersistentData::NUMBER_OF_MASKS,
		maskRequest->GetFrameBuffer(), w, h, maskIndex);
}

void PostEffectChain::MixMasksPass(GLSLShaderProgram* shader, const int maskIndex, const int maskIndex2, PostEffectBuffers* buffers)
{
	MaskFramebufferRequestScope maskRequest(this, buffers);
	
	// Mix masks = Mask A * Mask B

	const GLuint texid = maskRequest->GetColorObject(maskIndex);
	const GLuint texid2 = maskRequest->GetColorObject(maskIndex2);
	const int w = maskRequest->GetWidth();
	const int h = maskRequest->GetHeight();

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texid2);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texid);

	maskRequest->Bind(PostPersistentData::NUMBER_OF_MASKS);
	shader->Bind();

	shader->setUniformVector("gBloom", 0.0f, 0.0f, 1.0f, 0.0f);

	drawOrthoQuad2d(w, h);

	shader->UnBind();
	maskRequest->UnBind();

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	BlitFBOToFBOCustomAttachment(maskRequest->GetFrameBuffer(), w, h, PostPersistentData::NUMBER_OF_MASKS,
		maskRequest->GetFrameBuffer(), w, h, maskIndex);
}

void PostEffectChain::Evaluate(const PostEffectContextMoBu& effectContext, StandardEffectCollection& effectCollection)
{
	constexpr std::uint8_t kBufferCount = 2;

	const uint8_t activeIndex = gActiveData.load(std::memory_order_acquire);
	const uint8_t writeIndex = (activeIndex + 1) % kBufferCount;

	RenderData& data = mRenderData[writeIndex];

	std::vector<PostEffectBase*>& effectChain = data.mChain;

	data.isReady = false;

	if (!mSettings.Ok())
	{
		return;
	}
	
	if (!PrepareChainOrder(effectChain, &effectCollection))
	{	
		return;
	}

	data.isReady = true;
	data.isMaskTextureBinded = false;
	data.isMaskBlurRequested = false;
	data.isMaskMixRequested = false;

	for (int i = 0; i < PostPersistentData::NUMBER_OF_MASKS; ++i)
	{
		data.maskRenderFlags[i] = false;
	}
	
	if (HasAnyMaskUsedByEffect() && HasAnyObjectMasked() && mLastCamera)
	{
		for (int i = 0; i < PostPersistentData::NUMBER_OF_MASKS; ++i)
		{
			const EMaskingChannel maskId = static_cast<EMaskingChannel>(i);
			if (IsMaskUsedByEffect(maskId) && IsAnyObjectMaskedByMaskId(maskId))
			{
				data.isMaskBlurRequested |= mSettings->Masks[i].BlurMask;

				if (mSettings->Masks[i].UseMixWithMask && mSettings->Masks[i].MixWithMask != maskId)
				{
					const int maskIndex2 = static_cast<int>(mSettings->Masks[i].MixWithMask);
					if (maskIndex2 >= 0 && maskIndex2 < PostPersistentData::NUMBER_OF_MASKS)
					{
						data.isMaskMixRequested = true;
						data.maskRenderFlags[maskIndex2] = true;
					}
				}

				data.maskRenderFlags[i] = true;
				data.isMaskTextureBinded = true;
			}
		}
	}

	data.isDepthSamplerBinded = false;
	data.isLinearDepthSamplerBinded = false;
	data.isWorldNormalSamplerBinded = false;

	for (const auto* effect : effectChain)
	{
		if (!effect || !effect->IsActive())
			continue;

		data.isDepthSamplerBinded |= effect->IsDepthSamplerUsed();
		data.isLinearDepthSamplerBinded |= effect->IsLinearDepthSamplerUsed();
		data.isWorldNormalSamplerBinded |= effect->IsWorldNormalSamplerUsed();

		if (data.isDepthSamplerBinded &&
			data.isLinearDepthSamplerBinded &&
			data.isWorldNormalSamplerBinded)
		{
			break; // nothing more to gain
		}
	}

	for (const auto& effect : effectChain)
	{
		if (!effect || !effect->IsActive())
		{
			continue;
		}

		effect->CollectUIValues(&effectContext);
	}
}

void PostEffectChain::Synchronize()
{
	// swap
	// uint8_t oldIndex =
	gActiveData.fetch_xor(1, std::memory_order_acq_rel);
}

bool PostEffectChain::Process(PostEffectBuffers* buffers, double systime, 
	const PostEffectContextMoBu& effectContext, const StandardEffectCollection& effectCollection)
{
	const uint8_t activeIndex = gActiveData.load(std::memory_order_acquire);
	RenderData& data = mRenderData[activeIndex];
	std::vector<PostEffectBase*>& effectChain = data.mChain;

	//
	// Start PostEffectChain task cycle profiling, 
	//
	FBProfilerHelper lProfiling(FBProfiling_TaskCycleIndex(PostEffectChain), FBGetDisplayInfo(), FBGetRenderingTaskCycle());

	mIsCompressedDataReady = false;

	if (!buffers || !buffers->Ok() || !mSettings.Ok() || !data.isReady)
		return false;

	// 1. prepare chain count and order
	// NOTE: done at evaluate stage
	//if (!PrepareChainOrder(effectChain, effectCollection))
	//	return false;

	//
	DoubleFramebufferRequestScope doubleBufferRequest(this, buffers);
	MaskFramebufferRequestScope maskRequest(this, buffers);

	// 2. prepare and render into masks

	for (int i = 0; i < PostPersistentData::NUMBER_OF_MASKS; ++i)
	{
		if (data.maskRenderFlags[i])
		{
			RenderSceneMaskToTexture(effectCollection.mShaderSceneMasked.get(), i, mSettings->Masks[i], buffers);
		}
	}
	
	// 4a. blur masks (if applied)

	if (data.isMaskBlurRequested)
	{
		for (int i = 0; i < PostPersistentData::NUMBER_OF_MASKS; ++i)
		{
			if (data.maskRenderFlags[i] && mSettings->Masks[i].BlurMask)
			{
				BlurMasksPass(i, buffers, effectCollection.mEffectBilateralBlur.get(), effectContext);
			}
		}
	}

	// 4b. mix masks (if applied)
	if (data.isMaskMixRequested)
	{
		for (int i = 0; i < PostPersistentData::NUMBER_OF_MASKS; ++i)
		{
			if (!data.maskRenderFlags[i] || !mSettings->Masks[i].UseMixWithMask)
			{
				continue;
			}

			const int mask2 = static_cast<int>(mSettings->Masks[i].MixWithMask);

			if (mask2 >= 0 && mask2 < PostPersistentData::NUMBER_OF_MASKS
				&& i != mask2
				&& data.maskRenderFlags[mask2])
			{
				MixMasksPass(effectCollection.mShaderMix.get(), i, mask2, buffers);
			}
		}
	}

	// user option to show only mask result on a screen
	const unsigned int globalMaskingIndex = static_cast<unsigned int>(mSettings->GetGlobalMaskIndex());

	if (mSettings->DebugDisplyMasking)
	{
		FrameBuffer* doubleBuffer = doubleBufferRequest->GetPtr();

		BlitFBOToFBOCustomAttachment(maskRequest->GetFrameBuffer(), maskRequest->GetWidth(), maskRequest->GetHeight(), globalMaskingIndex,
			doubleBuffer->GetFrameBuffer(), doubleBuffer->GetWidth(), doubleBuffer->GetHeight(), doubleBufferRequest->GetWriteAttachment());
		doubleBufferRequest->Swap();
		return true;
	}

	// 5. bind textures of mask and depth for effects

	if (data.isMaskTextureBinded)
	{
		const GLuint maskTextureId = maskRequest->GetColorObject(globalMaskingIndex);
		glActiveTexture(GL_TEXTURE0 + CommonEffect::MaskSamplerSlot);
		glBindTexture(GL_TEXTURE_2D, maskTextureId);
		glActiveTexture(GL_TEXTURE0);
	}

	if (data.isDepthSamplerBinded)
	{
		const GLuint depthId = doubleBufferRequest->GetPtr()->GetDepthObject();

		glActiveTexture(GL_TEXTURE0 + CommonEffect::DepthSamplerSlot);
		glBindTexture(GL_TEXTURE_2D, depthId);
		glActiveTexture(GL_TEXTURE0);
	}

	// 3. in case of SSAO active, render a linear depth texture

	if (data.isLinearDepthSamplerBinded)
	{
		auto* doubleBuffer = doubleBufferRequest->GetPtr();
		auto* depthEffect = effectCollection.mEffectDepthLinearize.get();
		if (doubleBuffer && depthEffect)
		{
			const GLuint depthId = doubleBuffer->GetDepthObject();
			if (depthId != 0)
			{
				constexpr bool makeDownscale = false;
				RenderLinearDepth(buffers, depthEffect, depthId, makeDownscale, effectContext);
			}
		}
	}

	if (data.isWorldNormalSamplerBinded)
	{
		RenderWorldNormals(buffers);
	}

	// compute effect chain with double buffer

	// DONE: when buffer is attached, buffer is used itself !
	bool lSuccess = false;
	const bool generateMips = mSettings->GenerateMipMaps;

	if (!effectChain.empty())
	{
		auto* doubleBuffer = doubleBufferRequest->GetPtr();

		// optional. generate mipmaps for the first target
		GLuint texid = doubleBuffer->GetColorObject(doubleBufferRequest->GetReadAttachment());
		const int w = doubleBuffer->GetWidth();
		const int h = doubleBuffer->GetHeight();

		if (generateMips && texid > 0)
		{
			glBindTexture(GL_TEXTURE_2D, texid);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		// 6. render each effect in order
		
		for (const auto& effect : effectChain)
		{
			if (!effect || !effect->IsActive())
			{
				continue;
			}
			
			// activate local mask for the effect if applied
			bool customMaskBinded = false;
			const int maskIndex = effect->GetMaskIndex();
			if (maskIndex >= 0 && data.maskRenderFlags[maskIndex] && maskIndex != static_cast<int>(globalMaskingIndex))
			{
				const GLuint maskTextureId = maskRequest->GetColorObject(maskIndex);
				glActiveTexture(GL_TEXTURE0 + CommonEffect::MaskSamplerSlot);
				glBindTexture(GL_TEXTURE_2D, maskTextureId);
				glActiveTexture(GL_TEXTURE0);
				customMaskBinded = true;
			}
			
			doubleBufferRequest->Swap(); // current written goes to read and we are ready to write from a shader

			const GLuint srcTex = doubleBuffer->GetColorObject(doubleBufferRequest->GetReadAttachment());
			const GLuint depthTex = doubleBuffer->GetDepthObject();
			
			const PostEffectBase::RenderEffectContext renderContext{
				buffers,
				// INPUT
				srcTex, // src texture id
				depthTex, // depth texture id
				w,
				h,
				// OUTPUT
				doubleBuffer, // dst frame buffer
				doubleBufferRequest->GetWriteAttachment(),
				generateMips
			};

			effect->Process(renderContext, &effectContext);

			// if local masking index was used, switch back to global mask for next effect
			if (customMaskBinded)
			{
				const GLuint maskTextureId = maskRequest->GetColorObject(globalMaskingIndex);
				glActiveTexture(GL_TEXTURE0 + CommonEffect::MaskSamplerSlot);
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

	if (data.isDepthSamplerBinded)
	{
		glActiveTexture(GL_TEXTURE0 + CommonEffect::DepthSamplerSlot);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	if (data.isLinearDepthSamplerBinded)
	{
		glActiveTexture(GL_TEXTURE0 + CommonEffect::LinearDepthSamplerSlot);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	if (data.isWorldNormalSamplerBinded)
	{
		glActiveTexture(GL_TEXTURE0 + CommonEffect::WorldNormalSamplerSlot);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	if (data.isMaskTextureBinded)
	{
		glActiveTexture(GL_TEXTURE0 + CommonEffect::MaskSamplerSlot);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glActiveTexture(GL_TEXTURE0);
	return lSuccess;
}