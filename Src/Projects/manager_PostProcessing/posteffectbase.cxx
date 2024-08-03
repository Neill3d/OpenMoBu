
/**	\file	posteffectbase.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectbase.h"
#include "postpersistentdata.h"

//
extern void LOGE(const char* pFormatString, ...);

/////////////////////////////////////////////////////////////////////////
// EffectBase

PostEffectBase::PostEffectBase()
{
}

PostEffectBase::~PostEffectBase()
{
	FreeShaders();
}

const char *PostEffectBase::GetName() const
{
	return "empty";
}

// load and initialize shader from a specified location

void PostEffectBase::FreeShaders()
{
	for (auto shaderptr : mShaders)
	{
		delete shaderptr;
	}
	mShaders.clear();
}

void CommonEffectUniforms::PrepareUniformLocations(GLSLShader* shader)
{
	const GLint loc = shader->findLocation("maskSampler");
	if (loc >= 0)
		glUniform1i(loc, GetMaskSamplerSlot());
	
	useMaskLoc = shader->findLocation("useMasking");
	upperClipLoc = shader->findLocation("upperClip");
	lowerClipLoc = shader->findLocation("lowerClip");
}

void CommonEffectUniforms::UpdateUniforms(PostPersistentData* data)
{
	const bool isEnabledEffectMasking = data->PropertyList.Find(GetEnableMaskPropertyName())->AsInt() > 0;
	const bool useMasking = data->UseCompositeMasking && (data->EnableMaskingForAllEffects || isEnabledEffectMasking);
	const double _upperClip = data->UpperClip;
	const double _lowerClip = data->LowerClip;

	if (useMaskLoc >= 0)
		glUniform1f(useMaskLoc, (useMasking) ? 1.0f : 0.0f);

	if (upperClipLoc >= 0)
	{
		glUniform1f(upperClipLoc, 0.01f * (float)_upperClip);
	}

	if (lowerClipLoc >= 0)
		glUniform1f(lowerClipLoc, 1.0f - 0.01f * (float)_lowerClip);
}

bool PostEffectBase::Load(const int shaderIndex, const char *vname, const char *fname)
{
	if (mShaders.size() > shaderIndex)
	{
		delete mShaders[shaderIndex];
		mShaders[shaderIndex] = nullptr;
	}

	bool lSuccess = true;
	GLSLShader* shader = new GLSLShader();

	try
	{
		if (nullptr == shader)
		{
			throw std::exception("failed to allocate memory for the shader object");
		}

		if (false == shader->LoadShaders(vname, fname))
		{
			throw std::exception("failed to locate or load shader files");
		}

	}
	catch (const std::exception &e)
	{
		LOGE("Post Effect Chain (%s, %s) ERROR: %s\n", vname, fname, e.what());

		delete shader;
		shader = nullptr;

		lSuccess = false;
	}

	if (mShaders.size() > shaderIndex)
	{
		mShaders[shaderIndex] = shader;
	}
	else
	{
		mShaders.push_back(shader);
	}

	return lSuccess;
}

bool PostEffectBase::PrepUniforms(const int)
{
	return false;
}

bool PostEffectBase::CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera)
{
	return false;
}

const int PostEffectBase::GetNumberOfPasses() const
{
	return 1;
}
bool PostEffectBase::PrepPass(const int pass)
{
	return true;
}

void PostEffectBase::Bind()
{
	if (nullptr != GetShaderPtr())
	{
		GetShaderPtr()->Bind();
	}
}
void PostEffectBase::UnBind()
{
	if (nullptr != GetShaderPtr())
	{
		GetShaderPtr()->UnBind();
	}
}

GLSLShader *PostEffectBase::GetShaderPtr() {
	return mShaders[mCurrentShader];
}