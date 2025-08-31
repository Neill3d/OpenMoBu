
/**	\file	posteffectbase.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectbase.h"
#include "postpersistentdata.h"
#include "posteffectbuffers.h"
#include "mobu_logging.h"

#include "posteffect_shader_userobject.h"

#include <limits>
#include <ctime>

/////////////////////////////////////////////////////////////////////////
// EffectBase

PostEffectBase::PostEffectBase()
{}

PostEffectBase::~PostEffectBase()
{}

bool PostEffectBase::Load(const char* shadersLocation)
{
	for (int i = 0; i < GetNumberOfBufferShaders(); ++i)
	{
		if (PostEffectBufferShader* bufferShader = GetBufferShaderPtr(i))
		{
			if (!bufferShader->Load(shadersLocation))
				return false;
		}
	}

	return true;
}

bool PostEffectBase::IsReadyAndActive() const
{
	return true;
}

bool PostEffectBase::IsDepthSamplerUsed() const
{
	for (int i = 0; i < GetNumberOfBufferShaders(); ++i)
	{
		if (const PostEffectBufferShader* bufferShader = GetBufferShaderPtr(i))
		{
			if (bufferShader->IsDepthSamplerUsed())
				return true;
		}
	}
	return false;
}
bool PostEffectBase::IsLinearDepthSamplerUsed() const
{
	for (int i = 0; i < GetNumberOfBufferShaders(); ++i)
	{
		if (const PostEffectBufferShader* bufferShader = GetBufferShaderPtr(i))
		{
			if (bufferShader->IsLinearDepthSamplerUsed())
				return true;
		}
	}
	return false;
}

bool PostEffectBase::IsMaskSamplerUsed() const
{
	for (int i = 0; i < GetNumberOfBufferShaders(); ++i)
	{
		if (const PostEffectBufferShader* bufferShader = GetBufferShaderPtr(i))
		{
			if (bufferShader->IsMaskSamplerUsed())
				return true;
		}
	}
	return false;
}
bool PostEffectBase::IsWorldNormalSamplerUsed() const
{
	for (int i = 0; i < GetNumberOfBufferShaders(); ++i)
	{
		if (const PostEffectBufferShader* bufferShader = GetBufferShaderPtr(i))
		{
			if (bufferShader->IsWorldNormalSamplerUsed())
				return true;
		}
	}
	return false;
}

bool PostEffectBase::CollectUIValues(const IPostEffectContext* effectContext)
{
	for (int i = 0; i < GetNumberOfBufferShaders(); ++i)
	{
		if (PostEffectBufferShader* bufferShader = GetBufferShaderPtr(i))
		{
			FBComponent* tempComponent = effectContext->GetComponent();
			effectContext->OverrideComponent(bufferShader->GetOwner());

			bool status = bufferShader->CollectUIValues(effectContext, GetMaskIndex());

			effectContext->OverrideComponent(tempComponent);

			if (!status)
				return false;
		}
	}

	return true;
}


void PostEffectBase::Process(const RenderEffectContext& renderContext, const IPostEffectContext* effectContext)
{
	if (GetNumberOfBufferShaders() == 0)
		return;

	// if buffer shader is original size, we can use chain intermediate buffers for processing
	//  we also have to allocate the output of each buffer shader, so that we can use them as input textures for every next buffer shader
	//  the order is  input->first buffer shader processing->second buffer shader could use result of first buffer shader
	// ->main buffer shader mix initial input and result of second buffer shader
	/*
	* TODO: we have a scheme that post effect has one main buffer shader
	*  main buffer shader could contain passes
	*  but it also could contain connected bufferA, bufferB, etc.
	*  each buffer is a separate buffer shader, that is going to be evaluated and passed as additional sampler
	* 
	// get a framebuffer for every intermediate step
	InitializeFrameBuffers(context.viewWidth, context.viewHeight);

	const bool needBuffers = DoNeedIntermediateBuffers();

	for (int i = 0; i < GetNumberOfBufferShaders() - 1; ++i)
	{
		if (PostEffectBufferShader* bufferShader = GetBufferShaderPtr(i))
		{
			FrameBuffer* bufferShaderFrameBuffer = GetFrameBufferForBufferShader(i);
			bufferShader->Render(context.buffers, bufferShaderFrameBuffer, 0, context.srcTextureId, context.viewWidth, context.viewHeight, context.generateMips);

			// TODO: bind result of buffer shader for next buffer shaders !
		}
	}
	*/
	// main buffer shader is a last shader in the list, it have to output directly to the chain effects
	const int mainBufferShader = GetNumberOfBufferShaders() - 1;
	if (PostEffectBufferShader* bufferShader = GetBufferShaderPtr(mainBufferShader))
	{
		bufferShader->Render(renderContext.buffers, renderContext.dstFrameBuffer, renderContext.colorAttachment, renderContext.srcTextureId, 
			renderContext.viewWidth, renderContext.viewHeight, renderContext.generateMips, effectContext);
	}	
}

void PostEffectBase::InitializeFrameBuffers(int w, int h)
{
	if (GetNumberOfBufferShaders() <= 1)
	{
		mFrameBuffers.clear();
		return;
	}

	// we need framebuffer for all buffer shaders except the last one which goes into effect chain directly

	const int count = GetNumberOfBufferShaders() - 1;
	if (count != static_cast<int>(mFrameBuffers.size()))
	{
		mFrameBuffers.resize(count);
		mBufferShaderVersions.resize(count, 0);
	}

	const int downscaleWidth = w / 4;
	const int downscaleHeight = h / 4;

	for (int i = 0; i < count; ++i)
	{
		PostEffectBufferShader* bufferShader = GetBufferShaderPtr(i);
		if (bufferShader == nullptr)
			continue;

		const int version = bufferShader->GetVersion();
		const bool isDownscaleMode = bufferShader->IsDownscaleMode();

		if (mBufferShaderVersions[i] != version)
		{
			mFrameBuffers[i].reset(!isDownscaleMode
				? new FrameBuffer(w, h, 72, 1)
				: new FrameBuffer(downscaleWidth, downscaleHeight, 72, 1)
			);
		}
	}
}

FrameBuffer* PostEffectBase::GetFrameBufferForBufferShader(const int shaderIndex)
{
	return mFrameBuffers[shaderIndex].get();
}

bool PostEffectBase::DoNeedIntermediateBuffers()
{
	return false;
}

void PostEffectBase::BindFrameBuffer(int bufferIndex)
{
	mFrameBuffers[bufferIndex]->Bind();
}

void PostEffectBase::UnBindFrameBuffer(int bufferIndex, bool generateMips)
{
	mFrameBuffers[bufferIndex]->UnBind(generateMips);
}

GLuint PostEffectBase::GetTextureTextureId(int bufferIndex) const
{
	return mFrameBuffers[bufferIndex]->GetColorObject();
}