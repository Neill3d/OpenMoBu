
/**	\file	posteffectbase.cxx

Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectbase.h"
#include "postpersistentdata.h"
#include "posteffectbuffers.h"
#include "shaderpropertystorage.h"
#include "mobu_logging.h"

#include "posteffect_shader_userobject.h"

#include <limits>
#include <ctime>
#include <hashUtils.h>

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

bool PostEffectBase::CollectUIValues(IPostEffectContext* effectContext)
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

bool PostEffectBase::HasAnySourceShaders(const IPostEffectContext* effectContext) const
{
	if (GetNumberOfBufferShaders() == 0)
		return false;

	const int mainBufferShader = GetNumberOfBufferShaders() - 1;
	if (const PostEffectBufferShader* bufferShader = GetBufferShaderPtr(mainBufferShader))
	{
		return bufferShader->HasAnySourceShaders(effectContext);
	}
	return false;
}

bool PostEffectBase::HasAnySourceTextures(const IPostEffectContext* effectContext) const
{
	if (GetNumberOfBufferShaders() == 0)
		return false;

	const int mainBufferShader = GetNumberOfBufferShaders() - 1;
	if (const PostEffectBufferShader* bufferShader = GetBufferShaderPtr(mainBufferShader))
	{
		return bufferShader->HasAnySourceTextures(effectContext);
	}
	return false;
}

bool PostEffectBase::PreProcessSourceShaders(PostEffectRenderContext& renderContextParent, IPostEffectContext* effectContext) const
{
	const int mainBufferShader = GetNumberOfBufferShaders() - 1;
	const PostEffectBufferShader* bufferShader = GetBufferShaderPtr(mainBufferShader);
	if (!bufferShader)
		return false;

	PostEffectBufferShader::SourceShadersMap sourceShaders = bufferShader->GetSourceShaders(effectContext);

	for (IEffectShaderConnections::ShaderPropertyValue* propValue : sourceShaders)
	{
		EffectShaderUserObject* userObject = propValue->shaderUserObject;
		PostEffectBufferShader* bufferShader = userObject->GetUserShaderPtr();
		if (!bufferShader)
			continue;

		// bind sampler from another rendered buffer shader
		const std::string bufferName = std::string(GetName()) + "_" + std::string(userObject->Name);
		const uint32_t bufferNameKey = xxhash32(bufferName);

		int effectW = renderContextParent.width;
		int effectH = renderContextParent.height;
		userObject->RecalculateWidthAndHeight(effectW, effectH);

		PostEffectBuffers* buffers = renderContextParent.buffers;
		FrameBuffer* buffer = buffers->RequestFramebuffer(bufferNameKey, effectW, effectH, PostEffectBuffers::GetFlagsForSingleColorBuffer(), 1, false);

		PostEffectRenderContext renderContext(renderContextParent);
		
		renderContext.width = effectW;
		renderContext.height = effectH;
		renderContext.targetFramebuffer = buffer;
		renderContext.colorAttachment = 0;
		
		
		bufferShader->Render(renderContext, effectContext);

		const GLuint bufferTextureId = buffer->GetColorObject();
		buffers->ReleaseFramebuffer(bufferNameKey);

		// DONE: write value to the associated shader property
		GLint userTextureSlot = renderContext.userTextureSlot;
		propValue->SetValue(userTextureSlot);

		// bind input buffers
		glActiveTexture(GL_TEXTURE0 + userTextureSlot);
		glBindTexture(GL_TEXTURE_2D, bufferTextureId);
		glActiveTexture(GL_TEXTURE0);

		// update index of a next free slot
		renderContextParent.userTextureSlot = userTextureSlot + 1;
	}
	return true;
}

bool PostEffectBase::PreProcessSourceTextures(PostEffectRenderContext& renderContext, IPostEffectContext* effectContext) const
{
	//GLint userTextureSlot = CommonEffect::UserSamplerSlot; //!< start index to bind user textures

	const int mainBufferShader = GetNumberOfBufferShaders() - 1;
	const PostEffectBufferShader* bufferShader = GetBufferShaderPtr(mainBufferShader);
	if (!bufferShader)
		return false;

	PostEffectBufferShader::SourceTexturesMap sourceTextures = bufferShader->GetSourceTextures(effectContext);

	// bind sampler from a media resource texture
	for (IEffectShaderConnections::ShaderPropertyValue* propValue : sourceTextures)
	{
		FBTexture* texture = propValue->texture;

		int textureId = texture->TextureOGLId;
		if (textureId == 0)
		{
			texture->OGLInit();
			textureId = texture->TextureOGLId;
		}

		if (textureId > 0)
		{
			// DONE: write value to the associated shader property
			GLint userTextureSlot = renderContext.userTextureSlot;
			propValue->SetValue(userTextureSlot);
			
			glActiveTexture(GL_TEXTURE0 + userTextureSlot);
			glBindTexture(GL_TEXTURE_2D, textureId);
			glActiveTexture(GL_TEXTURE0);

			// update index of a next free slot
			renderContext.userTextureSlot = userTextureSlot + 1;
		}
	}
		
	return true;
}

void PostEffectBase::Process(PostEffectRenderContext& renderContext, IPostEffectContext* effectContext)
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

	if (HasAnySourceShaders(effectContext))
	{
		PreProcessSourceShaders(renderContext, effectContext);
	}

	if (HasAnySourceTextures(effectContext))
	{
		PreProcessSourceTextures(renderContext, effectContext);
	}

	// main buffer shader is a last shader in the list, it have to output directly to the chain effects
	const int mainBufferShader = GetNumberOfBufferShaders() - 1;
	if (PostEffectBufferShader* bufferShader = GetBufferShaderPtr(mainBufferShader))
	{
		bufferShader->Render(renderContext, effectContext);
	}	
}