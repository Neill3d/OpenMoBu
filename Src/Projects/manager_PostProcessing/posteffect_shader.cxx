
/**	\file	posteffect_shader.cxx

Sergei <Neill3d> Solokhin 2025-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "postpersistentdata.h"
#include "posteffect_buffers.h"
#include "posteffect_context.h"
#include "shaderproperty_storage.h"
#include "mobu_logging.h"
#include "hashUtils.h"
#include "FileUtils.h"
#include "ShaderToyShaderProgram.h"
#include "posteffect_shader_userobject.h"

/////////////////////////////////////////////////////////////////////////
// PostEffectBufferShader

PostEffectBufferShader::PostEffectBufferShader(FBComponent* ownerIn)
	: mOwner(ownerIn)
{
	mRenderPropertyScheme = std::make_unique<const ShaderPropertyScheme>();
}

PostEffectBufferShader::~PostEffectBufferShader()
{
	FreeShaders();
}

void PostEffectBufferShader::MakeCommonProperties(ShaderPropertyScheme* scheme)
{
	if (const char* maskingPropName = GetUseMaskingPropertyName())
	{
		scheme->AddProperty(ShaderProperty("Mask Texture", "maskSampler", nullptr))
			.SetFlag(PropertyFlag::SYSTEM, true)
			.SetType(EPropertyType::TEXTURE)
			.SetRequired(false)
			.SetDefaultValue(CommonEffect::MaskSamplerSlot);

		UseMaskingProperty = scheme->AddProperty(ShaderProperty(maskingPropName, "useMasking", nullptr))
			.SetFlag(PropertyFlag::SYSTEM, true)
			.SetFlag(PropertyFlag::IsFlag, true)
			.SetRequired(false)
			.SetType(EPropertyType::FLOAT)
			.GetProxy();
	}
	
	scheme->AddProperty(ShaderProperty(PostPersistentData::UPPER_CLIP, "upperClip", nullptr))
		.SetFlag(PropertyFlag::SYSTEM, true)
		.SetType(EPropertyType::FLOAT)
		.SetRequired(false)
		.SetScale(0.01f);

	scheme->AddProperty(ShaderProperty(PostPersistentData::LOWER_CLIP, "lowerClip", nullptr))
		.SetFlag(PropertyFlag::SYSTEM, true)
		.SetType(EPropertyType::FLOAT)
		.SetRequired(false)
		.SetFlag(PropertyFlag::INVERT_VALUE, true)
		.SetScale(0.01f);

}

const char* PostEffectBufferShader::GetName() const
{
	return "empty";
}

// load and initialize shader from a specified location

void PostEffectBufferShader::SetCurrentVariation(const int index)
{
	if (index < 0 || index >= static_cast<int>(mVariations.size()))
	{
		LOGE("PostEffectBufferShader::SetCurrentShader: index %d is out of range\n", index);
		return;
	}
	if (mCurrentVariation != index)
	{
		bIsNeedToUpdatePropertyScheme = true;
	}
	mCurrentVariation = index;
}
void PostEffectBufferShader::FreeShaders()
{
	mVariations.clear();
}


bool PostEffectBufferShader::Load(const int variationIndex, const char* vname, const char* fname, bool useShaderToyCompatibility)
{
	if (variationIndex < 0 || !vname || !fname)
	{
		LOGE("[PostEffectBufferShader %s]: load shader with a provided wrong index or vertex / fragment path \n", GetName());
		SetActive(false);
		return false;
	}

	const bool isIndexExisting = (variationIndex < static_cast<int>(mVariations.size()));
	if (isIndexExisting)
	{
		mVariations[variationIndex].reset();
	}

	std::unique_ptr<GLSLShaderProgram> shader = (useShaderToyCompatibility) 
		? std::make_unique<ShaderToyShaderProgram>()
		: std::make_unique<GLSLShaderProgram>();

	if (!shader->LoadShaders(vname, fname))
	{
		LOGE("[PostEffectBufferShader %s]: failed to load variance %d (%s, %s)\n", GetName(), variationIndex, vname, fname);
		SetActive(false);
		return false;
	}

	if (isIndexExisting)
	{
		mVariations[variationIndex].swap(shader);
		// samplers and locations
		//InitializeUniforms(shaderIndex);
	}
	else
	{
		mVariations.push_back(std::move(shader));
		// samplers and locations
		//InitializeUniforms(static_cast<int>(mShaders.size())-1);
	}

	bIsNeedToUpdatePropertyScheme = true;
	SetActive(true);
	return true;
}

bool PostEffectBufferShader::Load(const char* shadersLocation, bool useShaderToyCompatibility)
{
	for (int i = 0; i < GetNumberOfVariations(); ++i)
	{
		FBString vertex_path(shadersLocation, GetVertexFname(i));
		FBString fragment_path(shadersLocation, GetFragmentFname(i));

		if (!Load(i, vertex_path, fragment_path, useShaderToyCompatibility))
			return false;
	}
	return true;
}

bool PostEffectBufferShader::CheckShadersPath(const char* path) const
{
	if (GetNumberOfVariations() <= 0)
	{
		return false;
	}

	const char* test_shaders[] = {
		GetVertexFname(0),
		GetFragmentFname(0)
	};
	LOGV("[CheckShadersPath] testing path %s\n", path);
	for (const char* shader_path : test_shaders)
	{
		FBString full_path(path, shader_path);

		if (!IsFileExists(full_path))
		{
			LOGV("[CheckShadersPath] %s is not found in the %s shader \n", shader_path, GetName());
			return false;
		}
	}

	return true;
}

bool PostEffectBufferShader::Load()
{
	if (GetNumberOfVariations() <= 0)
	{
		return false;
	}

	constexpr int PATH_LENGTH = 260;
	char shadersPath[PATH_LENGTH]{ 0 };
	if (!FindEffectLocation(std::bind(&PostEffectBufferShader::CheckShadersPath, this, std::placeholders::_1), 
		shadersPath, PATH_LENGTH))
	{
		LOGE("[PostProcessing] Failed to find shaders location!\n");
		return false;
	}

	for (int i = 0; i < GetNumberOfVariations(); ++i)
	{
		FBString vertex_path(shadersPath, GetVertexFname(i));
		FBString fragment_path(shadersPath, GetFragmentFname(i));

		if (!Load(i, vertex_path, fragment_path))
			return false;
	}
	return true;
}

bool EffectShaderPropertyProcessor::CollectUIValues(FBComponent* component, PostEffectContextProxy* effectContext, const PostEffectBufferShader* effectShader, int maskIndex)
{
	const ShaderPropertyScheme* propertyScheme = effectShader->GetPropertySchemePtr();

	if (!component || propertyScheme->IsEmpty() || !effectContext)
		return false;

	ShaderPropertyStorage::EffectMap* effectMap = effectContext->GetEffectPropertyMap();
	if (!effectMap)
		return false;

	const uint32_t nameHash = effectShader->GetNameHash();
	ShaderPropertyStorage::PropertyValueMap& writeMap = (*effectMap)[nameHash];
	
	writeMap.clear();
	writeMap.reserve(propertyScheme->GetNumberOfProperties());
	
	for (const auto& shaderProperty : propertyScheme->GetProperties())
	{
		ShaderPropertyValue value(shaderProperty.GetDefaultValue());
		
		if (!shaderProperty.HasFlag(PropertyFlag::SKIP))
		{
			FBProperty* fbProperty = shaderProperty.GetFBProperty();
			
			//if (!fbProperty || component != shaderProperty.GetFBComponent())
			//{
			//	const char* shaderPropertyName = shaderProperty.GetName();
			//	fbProperty = component->PropertyList.Find(shaderPropertyName);
			//}

			if (fbProperty)
			{
				const FBPropertyType fbType = fbProperty->GetPropertyType();
				if (fbType == FBPropertyType::kFBPT_object)
				{
					shaderProperty.ReadTextureConnections(value, fbProperty);
					if (value.GetType() == EPropertyType::SHADER_USER_OBJECT
						&& value.shaderUserObject)
					{
						EffectShaderUserObject* shaderUserObject = value.shaderUserObject;

						if (shaderUserObject && shaderUserObject->GetUserShaderPtr())
						{
							CollectUIValues(shaderUserObject, effectContext, shaderUserObject->GetUserShaderPtr(), maskIndex);
						}
					}
				}

				ShaderProperty::ReadFBPropertyValue(fbProperty, value, shaderProperty, effectContext, maskIndex);
			}
		}

		VERIFY(value.GetType() != EPropertyType::NONE);
		//VERIFY(ShaderPropertyStorage::CheckPropertyExists(writeMap, value.GetNameHash()) == false);
		writeMap.emplace_back(std::move(value));
	}

	// TODO:
	/*
	if (UseMaskingProperty && effectContext->GetPostProcessData()
		&& effectContext->GetPostProcessData()->EnableMaskingForAllEffects)
	{
		ShaderPropertyValue value(UseMaskingProperty->GetDefaultValue());
		VERIFY(value.GetNameHash() != 0);
		value.SetValue(true);
		VERIFY(ShaderPropertyStorage::CheckPropertyExists(writeMap, value.GetNameHash()) == false);
		writeMap.emplace_back(std::move(value));
	}
	*/

	return effectShader->OnCollectUI(effectContext, maskIndex);
}

int PostEffectBufferShader::GetNumberOfSourceShaders(const PostEffectContextProxy* effectContext) const
{
	int count = 0;
	const uint32_t effectNameHash = GetNameHash();
	if (const ShaderPropertyStorage::PropertyValueMap* readMap = effectContext->GetEffectPropertyValueMap(effectNameHash))
	{
		for (const ShaderPropertyValue& value : *readMap)
		{
			if (value.GetType() == EPropertyType::SHADER_USER_OBJECT
				&& value.shaderUserObject)
			{
				++count;
			}
		}
	}
	
	return count;
}

bool PostEffectBufferShader::HasAnySourceShaders(const PostEffectContextProxy* effectContext) const
{
	const uint32_t nameHash = GetNameHash();// nameContext.GetNameHash();
	if (const ShaderPropertyStorage::PropertyValueMap* readMap = effectContext->GetEffectPropertyValueMap(nameHash))
	{
		for (const ShaderPropertyValue& value : *readMap)
		{
			if (value.GetType() == EPropertyType::SHADER_USER_OBJECT
				&& value.shaderUserObject)
			{
				return true;
			}
		}
	}

	return false;
}

bool PostEffectBufferShader::HasAnySourceTextures(const PostEffectContextProxy* effectContext) const
{
	const uint32_t nameHash = GetNameHash();
	if (const ShaderPropertyStorage::PropertyValueMap* readMap = effectContext->GetEffectPropertyValueMap(nameHash))
	{
		for (const ShaderPropertyValue& value : *readMap)
		{
			if (value.GetType() == EPropertyType::TEXTURE
				&& value.texture)
			{
				return true;
			}
		}
	}

	return false;
}

const PostEffectBufferShader::SourceShadersMapConst PostEffectBufferShader::GetSourceShadersConst(PostEffectContextProxy* effectContext) const
{
	SourceShadersMapConst sourceShaders;

	const uint32_t nameHash = GetNameHash();
	if (const ShaderPropertyStorage::PropertyValueMap* readMap = effectContext->GetEffectPropertyValueMap(nameHash))
	{
		for (const ShaderPropertyValue& value : *readMap)
		{
			if (value.GetType() == EPropertyType::SHADER_USER_OBJECT
				&& value.shaderUserObject)
			{
				sourceShaders.emplace_back(&value);
			}
		}
	}
	
	return sourceShaders;
}

PostEffectBufferShader::SourceShadersMap PostEffectBufferShader::GetSourceShaders(PostEffectContextProxy* effectContext) const
{
	SourceShadersMap sourceShaders;

	const uint32_t nameHash = GetNameHash();
	if (ShaderPropertyStorage::PropertyValueMap* readMap = effectContext->GetEffectPropertyValueMap(nameHash))
	{
		for (ShaderPropertyValue& value : *readMap)
		{
			if (value.GetType() == EPropertyType::SHADER_USER_OBJECT
				&& value.shaderUserObject)
			{
				sourceShaders.emplace_back(&value);
			}
		}
	}

	return sourceShaders;
}

PostEffectBufferShader::SourceTexturesMap PostEffectBufferShader::GetSourceTextures(PostEffectContextProxy* effectContext) const
{
	SourceTexturesMap sourceTextures;

	const uint32_t nameHash = GetNameHash();
	if (ShaderPropertyStorage::PropertyValueMap* readMap = effectContext->GetEffectPropertyValueMap(nameHash))
	{
		for (ShaderPropertyValue& value : *readMap)
		{
			if (value.GetType() == EPropertyType::TEXTURE
				&& value.texture)
			{
				sourceTextures.emplace_back(&value);
			}
		}
	}

	return sourceTextures;
}

GLSLShaderProgram* PostEffectBufferShader::GetShaderPtr() {
	if (mCurrentVariation >= 0 && mCurrentVariation < static_cast<int>(mVariations.size()))
		return mVariations[mCurrentVariation].get();
	return nullptr;
}

const GLSLShaderProgram* PostEffectBufferShader::GetShaderPtr() const {
	if(mCurrentVariation >= 0 && mCurrentVariation < static_cast<int>(mVariations.size()))
		return mVariations[mCurrentVariation].get();
	return nullptr;
}

void PostEffectBufferShader::RenderPass(int passIndex, PostEffectRenderContext& renderContext, PostEffectContextProxy* effectContext)
{
	// bind an input source image for processing by the effect

	glBindTexture(GL_TEXTURE_2D, renderContext.srcTextureId);

	if (renderContext.generateMips)
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
	renderContext.targetFramebuffer->Bind(renderContext.colorAttachment);

	drawOrthoQuad2d(renderContext.width, renderContext.height);
	
	renderContext.targetFramebuffer->UnBind(renderContext.generateMips);
}

void PostEffectBufferShader::PreRender(PostEffectRenderContext& renderContext, PostEffectContextProxy* effectContext) const
{
	//
	// pre-process source textures

	PostEffectBufferShader::SourceTexturesMap sourceTextures = GetSourceTextures(effectContext);

	// bind sampler from a media resource texture
	for (ShaderPropertyValue* propValue : sourceTextures)
	{
		FBTexture* texture = propValue->texture;
		if (!texture)
			continue;

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
			
			// TODO: this value is probably not used!
			propValue->SetValue(userTextureSlot);
			renderContext.OverrideUniform(GetPropertySchemePtr(), { -1, propValue->GetNameHash() }, static_cast<float>(userTextureSlot));

			glActiveTexture(GL_TEXTURE0 + userTextureSlot);
			glBindTexture(GL_TEXTURE_2D, textureId);
			glActiveTexture(GL_TEXTURE0);

			// update index of a next free slot
			renderContext.userTextureSlot = userTextureSlot + 1;
		}
	}

	//
	// pre-process source shaders

	PostEffectBufferShader::SourceShadersMap sourceShaders = GetSourceShaders(effectContext);

	for (ShaderPropertyValue* propValue : sourceShaders)
	{
		EffectShaderUserObject* userObject = propValue->shaderUserObject;
		PostEffectBufferShader* bufferShader = userObject->GetUserShaderPtr();
		if (!bufferShader)
			continue;

		if (userObject->IsNeedToReloadShaders())
		{
			if (!userObject->DoReloadShaders())
			{
				continue;
			}
		}

		// bind sampler from another rendered buffer shader
		const std::string bufferName = std::string(GetName()) + "_" + std::string(userObject->LongName);
		const uint32_t bufferNameKey = xxhash32(bufferName);

		int effectW = renderContext.width;
		int effectH = renderContext.height;
		userObject->RecalculateWidthAndHeight(effectW, effectH);

		PostEffectBuffers* buffers = renderContext.buffers;
		constexpr bool autoResize = false;
		FrameBuffer* buffer = buffers->RequestFramebuffer(bufferNameKey, effectW, effectH, PostEffectBuffers::GetFlagsForSingleColorBuffer(), 1, autoResize);

		PostEffectRenderContext renderContext(renderContext);

		renderContext.width = effectW;
		renderContext.height = effectH;
		renderContext.targetFramebuffer = buffer;
		renderContext.colorAttachment = 0;

		bufferShader->Render(renderContext, effectContext);
		
		const GLuint bufferTextureId = buffer->GetColorObject();
		//buffers->ReleaseFramebuffer(bufferNameKey);

		// DONE: write value to the associated shader property
		GLint userTextureSlot = renderContext.userTextureSlot;
		
		// TODO: this value is probably not used!
		propValue->SetValue(userTextureSlot);
		renderContext.OverrideUniform(GetPropertySchemePtr(), { -1, propValue->GetNameHash() }, static_cast<float>(userTextureSlot));
		// bind input buffers
		glActiveTexture(GL_TEXTURE0 + userTextureSlot);
		glBindTexture(GL_TEXTURE_2D, bufferTextureId);
		glActiveTexture(GL_TEXTURE0);

		// update index of a next free slot
		renderContext.userTextureSlot = userTextureSlot + 1;
	}
}

// dstBuffer - main effects chain target and it's current target colorAttachment
void PostEffectBufferShader::Render(PostEffectRenderContext& renderContext, PostEffectContextProxy* effectContext)
{
	if (!GetShaderPtr() || !GetShaderPtr()->IsValid())
		return;

	if (GetNumberOfPasses() == 0)
		return;
	
	OnRenderBegin(renderContext, effectContext);
	
	if (bIsNeedToUpdatePropertyScheme)
	{
		// change shader, change context, reload shader, we have to re-initialize property scheme with a new one
		std::unique_ptr<ShaderPropertyScheme> newPropertyScheme = std::make_unique<ShaderPropertyScheme>();
		MakeCommonProperties(newPropertyScheme.get());
		OnPopulateProperties(newPropertyScheme.get());
		InitializeUniforms(newPropertyScheme.get(), GetCurrentVariation());

		OnPropertySchemeUpdated(newPropertyScheme.get(), mRenderPropertyScheme.get());
		FBComponent* effectComponent = GetOwner() ? GetOwner() : effectContext->GetPostProcessData();
		newPropertyScheme->AssociateFBProperties(effectComponent);

		mRenderPropertyScheme.reset(newPropertyScheme.release());
		bIsNeedToUpdatePropertyScheme = false;
	}
	
	PreRender(renderContext, effectContext);

	if (!Bind())
	{
		return;
	}

	PostEffectRenderContext renderContextPass = renderContext;
	PostEffectBuffers* buffers = renderContext.buffers;
	
	// system uniforms, properties uniforms, could trigger other effects to render
	BindSystemUniforms(effectContext);

	if (GetNumberOfPasses() == 1)
	{
		constexpr int passIndex{ 0 };
		OnRenderPassBegin(passIndex, renderContextPass, effectContext); // override uniforms could be updated here

		constexpr bool skipTextureUniforms = false;
		AutoUploadUniforms(renderContextPass, effectContext, skipTextureUniforms);
		TryUseGlobalMasking(effectContext);
		OnUniformsUploaded(passIndex);

		// last one goes into dst buffer
		const int finalPassIndex = GetNumberOfPasses() - 1;
		RenderPass(finalPassIndex, renderContextPass, effectContext);
	}
	else
	{
		const int finalPassIndex = GetNumberOfPasses() - 1;

		// bind sampler from another rendered buffer shader
		const std::string bufferName = std::string(GetName()) + "_passes";
		const uint32_t bufferNameKey = xxhash32(bufferName);

		constexpr int numberOfColorAttachments = 2;
		constexpr bool autoResize = false;
		FrameBuffer* buffer = buffers->RequestFramebuffer(bufferNameKey, renderContext.width, renderContext.height, PostEffectBuffers::GetFlagsForSingleColorBuffer(), numberOfColorAttachments, autoResize);
		PingPongData pingPongData;
		FramebufferPingPongHelper pingPongHelper(*buffer, pingPongData);
		GLuint srcTextureId = renderContext.srcTextureId;

		for (int passIndex = 0; passIndex < finalPassIndex; ++passIndex)
		{
			renderContextPass = renderContext;
			renderContextPass.srcTextureId = srcTextureId;
			renderContextPass.targetFramebuffer = pingPongHelper.GetPtr();
			renderContextPass.colorAttachment = pingPongHelper.GetWriteAttachment();

			// here the derived class could update some property values for the given pass
			OnRenderPassBegin(passIndex, renderContextPass, effectContext);

			const bool skipTextureUniforms = (passIndex > 0); // only for the first pass we use the original input texture
			AutoUploadUniforms(renderContextPass, effectContext, skipTextureUniforms);
			TryUseGlobalMasking(effectContext);
			OnUniformsUploaded(passIndex);

			RenderPass(passIndex, renderContextPass, effectContext);

			//
			pingPongHelper.Swap();

			// the input for the next pass
			srcTextureId = pingPongHelper.GetPtr()->GetColorObject(pingPongHelper.GetReadAttachment());
		}

		// final pass into the destination buffer
		renderContextPass = renderContext;
		renderContextPass.srcTextureId = srcTextureId;

		OnRenderPassBegin(finalPassIndex, renderContextPass, effectContext);
		AutoUploadUniforms(renderContextPass, effectContext, false);
		TryUseGlobalMasking(effectContext);
		OnUniformsUploaded(finalPassIndex);

		RenderPass(finalPassIndex, renderContextPass, effectContext);
	}

	UnBind();

	OnRenderEnd(renderContext, effectContext);
}

bool PostEffectBufferShader::Bind()
{
	return (GetShaderPtr()) ? GetShaderPtr()->Bind() : false;
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

int PostEffectBufferShader::GetNumberOfProperties() const
{
	return static_cast<int>(mRenderPropertyScheme->GetNumberOfProperties());
}

const ShaderProperty& PostEffectBufferShader::GetProperty(int index) const
{
	return mRenderPropertyScheme->GetProperties()[index];
}

const ShaderProperty* PostEffectBufferShader::FindProperty(const std::string_view name) const
{
	return mRenderPropertyScheme->FindProperty(name);
}

const ShaderProperty* PostEffectBufferShader::FindPropertyByUniformName(const std::string_view name) const
{
	return mRenderPropertyScheme->FindPropertyByUniform(name);
}

bool PostEffectBufferShader::InitializeUniforms(ShaderPropertyScheme* scheme, int varianceIndex)
{
	if (!GetShaderPtr())
		return false;

	scheme->ReflectUniforms(GetShaderPtr()->GetProgramObj(), DoPopulatePropertiesFromUniforms());
	UploadDefaultValues(scheme);

	return true;
}

void PostEffectBufferShader::UploadDefaultValues(ShaderPropertyScheme* scheme)
{
	if (!Bind())
		return;

	for (const auto& shaderProperty : scheme->GetProperties())
	{
		if (shaderProperty.IsGeneratedByUniform())
			continue;

		constexpr bool skipTextureProperties = false;
		const GLuint programId = GetShaderPtr()->GetProgramObj();
		PostEffectRenderContext::UploadUniformValue(programId, shaderProperty.GetDefaultValue(), skipTextureProperties);
	}
}

void PostEffectBufferShader::AutoUploadUniforms(const PostEffectRenderContext& renderContext, 
	const PostEffectContextProxy* effectContext, bool skipTextureProperties)
{
	if (!effectContext)
		return;

	const uint32_t nameHash = GetNameHash();
	const ShaderPropertyStorage::PropertyValueMap* readMap = effectContext->GetEffectPropertyValueMap(nameHash);
	const GLuint programId = GetShaderPtr()->GetProgramObj();
	renderContext.UploadUniforms(programId, readMap, skipTextureProperties);
}

void PostEffectBufferShader::BindSystemUniforms(const PostEffectContextProxy* effectContext) const
{
	if (!GetShaderPtr())
		return;

	// prepare use masking value
	/*
	bool useMasking = false;

	auto fn_lookForMaskingFlag = [](FBComponent* component, const char* propertyName) -> bool {
		if (FBProperty* prop = component->PropertyList.Find(propertyName))
		{
			bool bvalue{ false };
			prop->GetData(&bvalue, sizeof(bool));
			return bvalue;
		}
		return false;
	};
	
	if (GetOwner())
	{
		useMasking |= fn_lookForMaskingFlag(GetOwner(), GetUseMaskingPropertyName());
	}
	else if (effectContext->GetPostProcessData())
	{
		if (effectContext->GetPostProcessData()->EnableMaskingForAllEffects)
		{
			useMasking = true;
		}
		else
		{
			useMasking |= fn_lookForMaskingFlag(effectContext->GetPostProcessData(), GetUseMaskingPropertyName());
		}
	}
	*/
	// bind uniforms
	// TODO: should we replace glProgramUniform with glUniform ?!

	const GLuint programId = GetShaderPtr()->GetProgramObj();

	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::INPUT_COLOR_SAMPLER_2D); loc >= 0)
	{
		glProgramUniform1i(programId, loc, 0);
	}
	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::iCHANNEL0); loc >= 0)
	{
		glProgramUniform1i(programId, loc, 0);
	}
	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::INPUT_DEPTH_SAMPLER_2D); loc >= 0)
	{
		glProgramUniform1i(programId, loc, CommonEffect::DepthSamplerSlot);
	}
	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::LINEAR_DEPTH_SAMPLER_2D); loc >= 0)
	{
		glProgramUniform1i(programId, loc, CommonEffect::LinearDepthSamplerSlot);
	}
	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::WORLD_NORMAL_SAMPLER_2D); loc >= 0)
	{
		glProgramUniform1i(programId, loc, CommonEffect::WorldNormalSamplerSlot);
	}
	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::INPUT_MASK_SAMPLER_2D); loc >= 0)
	{
		glProgramUniform1i(programId, loc, CommonEffect::MaskSamplerSlot);
	}
	/*
	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::USE_MASKING); loc >= 0)
	{
		glProgramUniform1f(programId, loc, (useMasking) ? 1.0f : 0.0f);
	}
	*/
	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::UPPER_CLIP); loc >= 0)
	{
		const double value = effectContext->GetPostProcessData()->UpperClip;
		glProgramUniform1f(programId, loc, 0.01f * static_cast<float>(value));
	}

	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::LOWER_CLIP); loc >= 0)
	{
		const double value = effectContext->GetPostProcessData()->LowerClip;
		glProgramUniform1f(programId, loc, 1.0f - 0.01f * static_cast<float>(value));
	}

	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::RESOLUTION); loc >= 0)
	{
		glProgramUniform2f(programId, loc, static_cast<float>(effectContext->GetViewWidth()), static_cast<float>(effectContext->GetViewHeight()));
	}
	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::iRESOLUTION); loc >= 0)
	{
		glProgramUniform2f(programId, loc, static_cast<float>(effectContext->GetViewWidth()), static_cast<float>(effectContext->GetViewHeight()));
	}
	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::INV_RESOLUTION); loc >= 0)
	{
		glProgramUniform2f(programId, loc, 1.0f / static_cast<float>(effectContext->GetViewWidth()), 1.0f / static_cast<float>(effectContext->GetViewHeight()));
	}
	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::TEXEL_SIZE); loc >= 0)
	{
		glProgramUniform2f(programId, loc, 1.0f / static_cast<float>(effectContext->GetViewWidth()), 1.0f / static_cast<float>(effectContext->GetViewHeight()));
	}

	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::iTIME); loc >= 0)
	{
		glProgramUniform1f(programId, loc, static_cast<float>(effectContext->GetSystemTime()));
	}
	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::iDATE); loc >= 0)
	{
		glProgramUniform4fv(programId, loc, 1, effectContext->GetIDate());
	}
	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::CAMERA_POSITION); loc >= 0)
	{
		glProgramUniform3fv(programId, loc, 1, effectContext->GetCameraPositionF());
	}
	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::MODELVIEW); loc >= 0)
	{
		glProgramUniformMatrix4fv(programId, loc, 1, GL_FALSE, effectContext->GetModelViewMatrixF());
	}
	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::PROJ); loc >= 0)
	{
		glProgramUniformMatrix4fv(programId, loc, 1, GL_FALSE, effectContext->GetProjectionMatrixF());
	}
	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::MODELVIEWPROJ); loc >= 0)
	{
		glProgramUniformMatrix4fv(programId, loc, 1, GL_FALSE, effectContext->GetModelViewProjMatrixF());
	}
	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::INV_MODELVIEWPROJ); loc >= 0)
	{
		glProgramUniformMatrix4fv(programId, loc, 1, GL_FALSE, effectContext->GetInvModelViewProjMatrixF());
	}
	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::PREV_MODELVIEWPROJ); loc >= 0)
	{
		glProgramUniformMatrix4fv(programId, loc, 1, GL_FALSE, effectContext->GetPrevModelViewProjMatrixF());
	}
	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::ZNEAR); loc >= 0)
	{
		glProgramUniform1f(programId, loc, effectContext->GetCameraNearDistance());
	}
	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::ZFAR); loc >= 0)
	{
		glProgramUniform1f(programId, loc, effectContext->GetCameraFarDistance());
	}
}

void PostEffectBufferShader::TryUseGlobalMasking(const PostEffectContextProxy* effectContext) const
{
	if (!GetShaderPtr())
		return;
	const GLuint programId = GetShaderPtr()->GetProgramObj();

	if (effectContext->GetPostProcessData()
		&& effectContext->GetPostProcessData()->EnableMaskingForAllEffects)
	{
		if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::USE_MASKING); loc >= 0)
		{
			glProgramUniform1f(programId, loc, 1.0f);
		}
	}
}

bool PostEffectBufferShader::IsDepthSamplerUsed() const
{
	return mRenderPropertyScheme->GetSystemUniformLoc(ShaderSystemUniform::INPUT_DEPTH_SAMPLER_2D) >= 0;
}
bool PostEffectBufferShader::IsLinearDepthSamplerUsed() const
{
	return mRenderPropertyScheme->GetSystemUniformLoc(ShaderSystemUniform::LINEAR_DEPTH_SAMPLER_2D) >= 0;
}

bool PostEffectBufferShader::IsMaskSamplerUsed() const
{
	return mRenderPropertyScheme->GetSystemUniformLoc(ShaderSystemUniform::INPUT_MASK_SAMPLER_2D) >= 0;
}
bool PostEffectBufferShader::IsWorldNormalSamplerUsed() const
{
	return mRenderPropertyScheme->GetSystemUniformLoc(ShaderSystemUniform::WORLD_NORMAL_SAMPLER_2D) >= 0;
}