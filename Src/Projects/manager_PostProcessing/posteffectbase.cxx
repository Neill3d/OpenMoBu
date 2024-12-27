
/**	\file	posteffectbase.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectbase.h"
#include "postpersistentdata.h"
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
	const bool isEnabledEffectMasking = data->PropertyList.Find(enableMaskingPropertyName)->AsInt() > 0;
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

	bool lSuccess = true;
	GLSLShaderProgram* shader = new GLSLShaderProgram();

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

		delete shader;
		shader = nullptr;

		lSuccess = false;
	}

	if (mShaders.size() > shaderIndex)
	{
		mShaders[shaderIndex].reset(shader);
	}
	else
	{
		mShaders.push_back(std::make_unique<GLSLShaderProgram>(shader));
	}

	return lSuccess;
}

bool PostEffectBufferShader::Load(const char* shadersLocation)
{
	for (int i = 0; i < GetNumberOfVariations(); ++i)
	{
		FBString vertex_path(shadersLocation, GetVertexFname(i));
		FBString fragment_path(shadersLocation, GetFragmentFname(i));

		if (!Load(i, vertex_path, fragment_path))
			return false;
		
		// samplers and locations
		PrepUniforms(i);
	}
	return true;
}

bool PostEffectBufferShader::PrepUniforms(const int)
{
	return false;
}

bool PostEffectBufferShader::CollectUIValues(PostPersistentData* pData, PostEffectContext& effectContext)
{
	return false;
}

const int PostEffectBufferShader::GetNumberOfPasses() const
{
	return 1;
}
bool PostEffectBufferShader::PrepPass(const int pass)
{
	return true;
}

GLSLShaderProgram* PostEffectBufferShader::GetShaderPtr() {
	assert(mCurrentShader >= 0 && mCurrentShader < mShaders.size());
	return mShaders[mCurrentShader].get();
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


/////////////////////////////////////////////////////////////////////////
// EffectBase

PostEffectBase::PostEffectBase()
{
}

PostEffectBase::~PostEffectBase()
{
	//mBufferShaders.clear();
}

bool PostEffectBase::Load(const char* shadersLocation)
{
	for (int i = 0; i < GetNumberOfBufferShaders(); ++i)
	{
		if (!GetBufferShaderPtr(i)->Load(shadersLocation))
			return false;
	}

	return true;
}

bool PostEffectBase::IsDepthSamplerUsed() const
{
	for (int i = 0; i < GetNumberOfBufferShaders(); ++i)
	{
		if (GetBufferShaderPtr(i)->IsDepthSamplerUsed())
			return true;
	}
	return false;
}
bool PostEffectBase::IsLinearDepthSamplerUsed() const
{
	for (int i = 0; i < GetNumberOfBufferShaders(); ++i)
	{
		if (GetBufferShaderPtr(i)->IsLinearDepthSamplerUsed())
			return true;
	}
	return false;
}

bool PostEffectBase::CollectUIValues(PostPersistentData* pData, PostEffectContext& effectContext)
{
	for (int i = 0; i < GetNumberOfBufferShaders(); ++i)
	{
		if (!GetBufferShaderPtr(i)->CollectUIValues(pData, effectContext))
			return false;
	}

	return true;
}


void PostEffectBase::Process(const EffectContext& context)
{
	if (GetNumberOfBufferShaders() == 1)
	{
		PostEffectBufferShader* bufferShader = GetBufferShaderPtr(0);

		bufferShader->Bind();
	
		// process each buffer effect and write to dst framebuffer at the end
		// apply effect into dst buffer
		context.dstFrameBuffer->Bind();

		drawOrthoQuad2d(context.viewWidth, context.viewHeight);

		context.dstFrameBuffer->UnBind(context.generateMips);

		bufferShader->UnBind();
	}	
}
