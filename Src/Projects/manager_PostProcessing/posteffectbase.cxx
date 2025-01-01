
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

/////////////////////////////////////////////////////////////////////////
// PostEffectBufferShader

PostEffectBufferShader::PostEffectBufferShader()
{
}

PostEffectBufferShader::~PostEffectBufferShader()
{
	FreeShaders();
}

const char* PostEffectBufferShader::GetName() const
{
	return "empty";
}

// load and initialize shader from a specified location

void PostEffectBufferShader::FreeShaders()
{
	mShaders.clear();
}

void CommonEffectUniforms::PrepareUniformLocations(GLSLShaderProgram* shader)
{
	const GLint loc = shader->findLocation("maskSampler");
	if (loc >= 0)
		glUniform1i(loc, GetMaskSamplerSlot());

	useMaskLoc = shader->findLocation("useMasking");
	upperClipLoc = shader->findLocation("upperClip");
	lowerClipLoc = shader->findLocation("lowerClip");
}

void CommonEffectUniforms::CollectCommonData(PostPersistentData* data, const char* enableMaskingPropertyName)
{
	const bool isEnabledEffectMasking = (enableMaskingPropertyName) ? data->PropertyList.Find(enableMaskingPropertyName)->AsInt() > 0 : false;
	const bool useMasking = data->UseCompositeMasking && (data->EnableMaskingForAllEffects || isEnabledEffectMasking);
	const double _upperClip = data->UpperClip;
	const double _lowerClip = data->LowerClip;

	mCommonShaderData.useMasking = (useMasking) ? 1.0f : 0.0f;
	mCommonShaderData.upperClip = 0.01f * static_cast<float>(_upperClip);
	mCommonShaderData.lowerClip = 1.0f - 0.01f * static_cast<float>(_lowerClip);
}

void CommonEffectUniforms::UploadCommonData()
{
	if (useMaskLoc >= 0)
		glUniform1f(useMaskLoc, mCommonShaderData.useMasking);

	if (upperClipLoc >= 0)
		glUniform1f(upperClipLoc, mCommonShaderData.upperClip);

	if (lowerClipLoc >= 0)
		glUniform1f(lowerClipLoc, mCommonShaderData.lowerClip);
}

bool PostEffectBufferShader::Load(const int shaderIndex, const char* vname, const char* fname)
{
	if (mShaders.size() > shaderIndex)
	{
		mShaders[shaderIndex].reset();
	}

	std::unique_ptr<GLSLShaderProgram> shader = std::make_unique<GLSLShaderProgram>();

	try
	{
		if (!shader)
		{
			throw std::exception("failed to allocate memory for the shader object");
		}

		if (!shader->LoadShaders(vname, fname))
		{
			throw std::exception("failed to locate or load shader files");
		}

	}
	catch (const std::exception& e)
	{
		LOGE("Post Effect Chain (%s, %s) ERROR: %s\n", vname, fname, e.what());
		return false;
	}

	if (mShaders.size() > shaderIndex)
	{
		mShaders[shaderIndex].swap(shader);
		// samplers and locations
		PrepUniforms(shaderIndex);
	}
	else
	{
		mShaders.push_back(std::move(shader));
		// samplers and locations
		PrepUniforms(static_cast<int>(mShaders.size())-1);
	}

	return true;
}

bool PostEffectBufferShader::Load(const char* shadersLocation)
{
	for (int i = 0; i < GetNumberOfVariations(); ++i)
	{
		FBString vertex_path(shadersLocation, GetVertexFname(i));
		FBString fragment_path(shadersLocation, GetFragmentFname(i));

		if (!Load(i, vertex_path, fragment_path))
			return false;
	}
	return true;
}

bool PostEffectBufferShader::PrepUniforms(const int)
{
	return false;
}

bool PostEffectBufferShader::CollectUIValues(PostPersistentData* pData, const PostEffectContext& effectContext, int maskIndex)
{
	return false;
}

const int PostEffectBufferShader::GetNumberOfPasses() const
{
	return 1;
}
bool PostEffectBufferShader::PrepPass(const int pass, int w, int h)
{
	return true;
}

GLSLShaderProgram* PostEffectBufferShader::GetShaderPtr() {
	assert(mCurrentShader >= 0 && mCurrentShader < mShaders.size());
	return mShaders[mCurrentShader].get();
}

const GLSLShaderProgram* PostEffectBufferShader::GetShaderPtr() const {
	assert(mCurrentShader >= 0 && mCurrentShader < mShaders.size());
	return mShaders[mCurrentShader].get();
}

void PostEffectBufferShader::RenderPass(int passIndex, FrameBuffer* dstBuffer, int colorAttachment, const GLuint inputTextureId, int w, int h, bool generateMips)
{
	PrepPass(passIndex, w, h);

	// bind an input source image for processing by the effect

	glBindTexture(GL_TEXTURE_2D, inputTextureId);

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
	dstBuffer->Bind(colorAttachment);

	drawOrthoQuad2d(w, h);
	
	dstBuffer->UnBind(generateMips);
}

// dstBuffer - main effects chain target and it's current target colorAttachment
void PostEffectBufferShader::Render(PostEffectBuffers* buffers, FrameBuffer* dstBuffer, int colorAttachment, const GLuint inputTextureId, int w, int h, bool generateMips)
{
	if (GetNumberOfPasses() == 0)
		return;

	/*
	// render dependent buffer shaders ?!
	if (BufferAShader)
	{
		const std::string bufferAName = std::string(GetName()) + "_bufferA";

		// dst should be buffer textures ?!

		FrameBuffer* bufferA = buffers->RequestFramebuffer(bufferAName);

		BufferAShader->Render(buffers, bufferA, 0, inputTextureId, w, h, generateMips);

		const GLuint bufferATextureId = bufferA->GetColorObject();
		buffers->ReleaseFramebuffer(bufferAName);

		// TODO: bind input buffers
		glActiveTexture(GL_TEXTURE0 + GetBufferSamplerId());
		glBindTexture(GL_TEXTURE_2D, bufferATextureId);
		glActiveTexture(GL_TEXTURE0);
	}
	*/
	GLuint texId = inputTextureId;

	Bind();
	UploadUniforms(buffers, dstBuffer, colorAttachment, inputTextureId, w, h, generateMips);

	//OnPreRender(buffers, dstBuffer, colorAttachment, inputTextureId, w, h, generateMips);

	// render all passes except last one
	// we can't do intermediate passes without buffers
	// TODO:
	/*
	if (buffers)
	{
		for (int j = 0; j < GetNumberOfPasses() - 1; ++j)
		{
			RenderPass(j, buffers->GetDstBufferPtr(), texId, w, h, generateMips);

			//
			buffers->SwapBuffers();

			// the input for the next pass
			texId = buffers->GetSrcBufferPtr()->GetColorObject();
		}
	}
	*/
	// last one goes into dst buffer

	const int finalPassIndex = GetNumberOfPasses() - 1;
	RenderPass(finalPassIndex, dstBuffer, colorAttachment, texId, w, h, generateMips);

	UnBind();
}

void PostEffectBufferShader::Bind()
{
	if (GetShaderPtr())
	{
		GetShaderPtr()->Bind();
	}
}
void PostEffectBufferShader::UnBind()
{
	if (GetShaderPtr())
	{
		GetShaderPtr()->UnBind();
	}
}

void PostEffectBufferShader::SetDownscaleMode(const bool value)
{
	isDownscale = value;
	version += 1;
}
/*
void PostEffectBufferShader::SetBufferA(PostEffectBufferShader* bufferShaderIn)
{
	BufferAShader = bufferShaderIn;
}
*/
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

bool PostEffectBase::CollectUIValues(PostPersistentData* pData, const PostEffectContext& effectContext)
{
	for (int i = 0; i < GetNumberOfBufferShaders(); ++i)
	{
		if (PostEffectBufferShader* bufferShader = GetBufferShaderPtr(i))
		{
			if (!bufferShader->CollectUIValues(pData, effectContext, GetMaskIndex()))
				return false;
		}
	}

	return true;
}


void PostEffectBase::Process(const EffectContext& context)
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
		bufferShader->Render(context.buffers, context.dstFrameBuffer, context.colorAttachment, context.srcTextureId, context.viewWidth, context.viewHeight, context.generateMips);
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