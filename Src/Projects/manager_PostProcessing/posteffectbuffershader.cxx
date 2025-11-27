
/**	\file	posteffectbuffershader.cxx

Sergei <Neill3d> Solokhin 2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectbase.h"
#include "postpersistentdata.h"
#include "posteffectbuffers.h"
#include "shaderpropertystorage.h"
#include "mobu_logging.h"
#include "hashUtils.h"
#include "posteffect_shader_userobject.h"

namespace _PostEffectBufferShaderInternal
{
	std::string RemovePostfix(const std::string& uniformName, const std::string& postfix) {
		const size_t pos = uniformName.rfind(postfix);
		if (pos != std::string::npos && pos == uniformName.size() - postfix.size()) {
			return uniformName.substr(0, pos);  // Remove postfix
		}
		return uniformName;
	}

	void ExtractNameAndFlagsFromUniformNameAndType(IEffectShaderConnections::ShaderProperty& prop, const char* uniformNameIn, GLenum type)
	{
		const std::string uniformName(uniformNameIn);

		switch (type)
		{
		case GL_FLOAT:
		{
			const std::string name = RemovePostfix(uniformName, "_flag");
			if (name.size() != uniformName.size())
			{
				prop.SetFlag(IEffectShaderConnections::PropertyFlag::IsFlag, true);
			}
			prop.SetName(name);
		} break;
		case GL_FLOAT_VEC2:
		{
			const std::string name = RemovePostfix(uniformName, "_wstoss");
			if (name.size() != uniformName.size())
			{
				prop.SetFlag(IEffectShaderConnections::PropertyFlag::ConvertWorldToScreenSpace, true);
			}
			prop.SetName(name);
		} break;
		case GL_FLOAT_VEC3:
		case GL_FLOAT_VEC4:
		{
			const std::string name = RemovePostfix(uniformName, "_color");
			if (name.size() != uniformName.size())
			{
				prop.SetFlag(IEffectShaderConnections::PropertyFlag::IsColor, true);
			}
			prop.SetName(name);
		} break;
		default:
			prop.SetName(uniformName);
		}
	}
};

const char* PostEffectBufferShader::gSystemUniformNames[static_cast<int>(ShaderSystemUniform::COUNT)] =
{
	"inputSampler", //!< this is an input image that we read from
	"iChannel0", //!< this is an input image, compatible with shadertoy
	"depthSampler", //!< this is a scene depth texture sampler in case shader will need it for processing
	"linearDepthSampler",
	"maskSampler", //!< binded mask for a shader processing (system run-time texture)
	"normalSampler", //!< binded World-space normals texture (system run-time texture)

	"useMasking", //!< float uniform [0; 1] to define if the mask have to be used
	"upperClip", //!< this is an upper clip image level. defined in a texture coord space to skip processing
	"lowerClip", //!< this is a lower clip image level. defined in a texture coord space to skip processing

	"gResolution", //!< vec2 that contains processing absolute resolution, like 1920x1080
	"iResolution", //!< vec2 image absolute resolution, compatible with shadertoy naming
	"uInvResolution", //!< inverse resolution
	"texelSize", //!< vec2 of a texel size, computed as 1/resolution

	"iTime", //!< compatible with shadertoy, float, shader playback time (in seconds)
	"iDate",  //!< compatible with shadertoy, vec4, (year, month, day, time in seconds)

	"cameraPosition", //!< world space camera position
	"modelView", //!< current camera modelview matrix
	"projection", //!< current camera projection matrix
	"modelViewProj", //!< current camera modelview-projection matrix
	"invModelViewProj",
	"prevModelViewProj",

	"zNear", //!< camera near plane
	"zFar"	//!< camera far plane
};

/////////////////////////////////////////////////////////////////////////
// PostEffectBufferShader

PostEffectBufferShader::PostEffectBufferShader(FBComponent* ownerIn)
	: mOwner(ownerIn)
{
	ResetSystemUniformLocations();
}

PostEffectBufferShader::~PostEffectBufferShader()
{
	FreeShaders();
}

void PostEffectBufferShader::MakeCommonProperties()
{
	AddProperty(IEffectShaderConnections::ShaderProperty("Mask Texture", "maskSampler", nullptr))
		.SetFlag(IEffectShaderConnections::PropertyFlag::SYSTEM, true)
		.SetType(IEffectShaderConnections::EPropertyType::TEXTURE)
		.SetDefaultValue(CommonEffect::MaskSamplerSlot);

	const char* maskingPropName = GetUseMaskingPropertyName();
	assert(maskingPropName != nullptr);
	UseMaskingProperty = &AddProperty(IEffectShaderConnections::ShaderProperty(maskingPropName, "useMasking", nullptr))
		.SetFlag(IEffectShaderConnections::PropertyFlag::SYSTEM, true)
		.SetType(IEffectShaderConnections::EPropertyType::BOOL);

	AddProperty(IEffectShaderConnections::ShaderProperty(PostPersistentData::UPPER_CLIP, "upperClip", nullptr))
		.SetFlag(IEffectShaderConnections::PropertyFlag::SYSTEM, true)
		.SetScale(0.01f);

	AddProperty(IEffectShaderConnections::ShaderProperty(PostPersistentData::LOWER_CLIP, "lowerClip", nullptr))
		.SetFlag(IEffectShaderConnections::PropertyFlag::SYSTEM, true)
		.SetFlag(IEffectShaderConnections::PropertyFlag::INVERT_VALUE, true)
		.SetScale(0.01f);

}

const char* PostEffectBufferShader::GetName() const
{
	return "empty";
}

// load and initialize shader from a specified location

void PostEffectBufferShader::SetCurrentShader(const int index)
{
	if (index < 0 || index >= mShaders.size())
	{
		LOGE("PostEffectBufferShader::SetCurrentShader: index %d is out of range\n", index);
		return;
	}
	if (mCurrentShader != index)
	{
		bHasShaderChanged = true;
	}
	mCurrentShader = index;
}
void PostEffectBufferShader::FreeShaders()
{
	mShaders.clear();
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
		InitializeUniforms(shaderIndex);
	}
	else
	{
		mShaders.push_back(std::move(shader));
		// samplers and locations
		InitializeUniforms(static_cast<int>(mShaders.size())-1);
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

bool PostEffectBufferShader::InitializeUniforms(const int varianceIndex)
{
	// existing registered properties got their shader uniform locations
	MakePropertyLocationsFromShaderUniforms();

	if (!DoPopulatePropertiesFromUniforms())
	{
		// have a look which system uniforms we are using in the shader
		MakeSystemLocationsFromShaderUniforms();
	}

	return OnPrepareUniforms(varianceIndex);
}

bool PostEffectBufferShader::CollectUIValues(FBComponent* component, IPostEffectContext* effectContext, int maskIndex)
{
	if (!component || mProperties.empty())
		return false;

	//const int variationIndex = GetCurrentShader();
	ShaderPropertyStorage* propertyStorage = effectContext->GetShaderPropertyStorage();
	if (!propertyStorage)
		return false;

	ShaderPropertyStorage::PropertyValueMap& writeMap = propertyStorage->GetWritePropertyMap(GetNameHash());

	writeMap.clear();
	writeMap.reserve(mProperties.size());
	
	for (auto& [key, shaderProperty] : mProperties)
	{
		if (shaderProperty.HasFlag(PropertyFlag::ShouldSkip))
		{
			// the shader will process the value on its own
			continue;
		}

		ShaderPropertyValue value(shaderProperty.GetDefaultValue());
		assert(value.GetNameHash() != 0);

		if (!shaderProperty.GetFBProperty())
		{
			if (strnlen(shaderProperty.GetName(), ShaderProperty::MAX_NAME_LENGTH) > 0)
			{
				shaderProperty.SetFBProperty(component->PropertyList.Find(shaderProperty.GetName()));
			}
		}

		if (shaderProperty.GetFBProperty())
		{
			ShaderProperty::ReadFBPropertyValue(value, shaderProperty, effectContext, maskIndex);
		}

		writeMap.emplace_back(std::move(value));
	}

	if (UseMaskingProperty && effectContext->GetPostProcessData()
		&& effectContext->GetPostProcessData()->EnableMaskingForAllEffects)
	{
		ShaderPropertyValue value(UseMaskingProperty->GetDefaultValue());
		value.SetValue(true);

		writeMap.emplace_back(std::move(value));
	}

	return true;
}

bool PostEffectBufferShader::CollectUIValues(IPostEffectContext* effectContext, int maskIndex)
{
	if (effectContext->GetPostProcessData())
	{
		CollectUIValues(effectContext->GetPostProcessData(), effectContext, maskIndex);
	}

	if (effectContext->GetComponent() && effectContext->GetComponent() != effectContext->GetPostProcessData())
	{
		CollectUIValues(effectContext->GetComponent(), effectContext, maskIndex);
	}

	return OnCollectUI(effectContext, maskIndex);
}

//void PostEffectBufferShader::UploadUniforms(PostEffectBuffers* buffers, FrameBuffer* dstBuffer, int colorAttachment, const GLuint inputTextureId, int w, int h, bool generateMips, const IPostEffectContext* effectContext)
//{
//	
//}

bool PostEffectBufferShader::ReloadPropertyShaders()
{
	// TODO: move this to evaluation thread and shader property storage access
	/*
	for (auto& prop : mProperties)
	{
		auto& shaderProperty = prop.second;

		if (shaderProperty.type == IEffectShaderConnections::EPropertyType::TEXTURE)
			shaderProperty.GetReadValue().ReadTextureConnections(shaderProperty.fbProperty);

		EffectShaderUserObject* shaderUserObject = shaderProperty.GetReadValue().shaderUserObject;
		if (shaderUserObject && !shaderUserObject->DoReloadShaders())
		{
			return false;
		}
	}
	*/
	return true;
}

GLSLShaderProgram* PostEffectBufferShader::GetShaderPtr() {
	if (mCurrentShader >= 0 && mCurrentShader < mShaders.size())
		return mShaders[mCurrentShader].get();
	return nullptr;
}

const GLSLShaderProgram* PostEffectBufferShader::GetShaderPtr() const {
	if(mCurrentShader >= 0 && mCurrentShader < mShaders.size())
		return mShaders[mCurrentShader].get();
	return nullptr;
}

void PostEffectBufferShader::RenderPass(int passIndex, const PostEffectRenderContext& renderContext)
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

// dstBuffer - main effects chain target and it's current target colorAttachment
void PostEffectBufferShader::Render(const PostEffectRenderContext& renderContext, const IPostEffectContext* effectContext)
{
	if (GetNumberOfPasses() == 0)
		return;

	

	if (bHasShaderChanged)
	{
		InitializeUniforms(GetCurrentShader());
		bHasShaderChanged = false;
	}

	if (!Bind())
	{
		return;
	}

	PostEffectRenderContext renderContextPass = renderContext;

	//GLuint texId = renderContext.srcTextureId;
	PostEffectBuffers* buffers = renderContext.buffers;
	//FrameBuffer* dstBuffer = renderContext.targetFramebuffer;
	//const int colorAttachment = renderContext.colorAttachment;
	//const int w = renderContext.width;
	//const int h = renderContext.height;
	//const bool generateMips = renderContext.generateMips;

	// system uniforms, properties uniforms, could trigger other effects to render
	BindSystemUniforms(effectContext);

	if (GetNumberOfPasses() == 1)
	{
		constexpr int passIndex{ 0 };
		OnRenderPassBegin(passIndex, renderContextPass); // override uniforms could be updated here

		constexpr bool skipTextureUniforms = false;
		AutoUploadUniforms(renderContextPass, effectContext, skipTextureUniforms);
		OnUniformsUploaded(passIndex);

		// last one goes into dst buffer
		const int finalPassIndex = GetNumberOfPasses() - 1;
		RenderPass(finalPassIndex, renderContextPass);
	}
	else
	{
		const int finalPassIndex = GetNumberOfPasses() - 1;

		// bind sampler from another rendered buffer shader
		const std::string bufferName = std::string(GetName()) + "_passes";
		const uint32_t bufferNameKey = xxhash32(bufferName);

		FrameBuffer* buffer = buffers->RequestFramebuffer(bufferNameKey, renderContext.width, renderContext.height, PostEffectBuffers::GetFlagsForSingleColorBuffer(), 2, false);
		PingPongData pingPongData;
		FramebufferPingPongHelper pingPongHelper(buffer, &pingPongData);
		
		for (int passIndex = 0; passIndex < finalPassIndex; ++passIndex)
		{
			renderContextPass.targetFramebuffer = pingPongHelper.GetPtr();
			renderContextPass.colorAttachment = pingPongHelper.GetWriteAttachment();

			// here the derived class could update some property values for the given pass
			OnRenderPassBegin(passIndex, renderContextPass);

			const bool skipTextureUniforms = (passIndex > 0); // only for the first pass we use the original input texture
			AutoUploadUniforms(renderContext, effectContext, skipTextureUniforms);
			OnUniformsUploaded(passIndex);

			RenderPass(passIndex, renderContextPass);

			//
			pingPongHelper.Swap();

			// the input for the next pass
			renderContextPass.srcTextureId = pingPongHelper.GetPtr()->GetColorObject(pingPongHelper.GetReadAttachment());
		}

		buffers->ReleaseFramebuffer(bufferNameKey);

		renderContextPass = renderContext;

		OnRenderPassBegin(finalPassIndex, renderContextPass);
		AutoUploadUniforms(renderContextPass, effectContext, false);
		OnUniformsUploaded(finalPassIndex);

		RenderPass(finalPassIndex, renderContext);
	}

	UnBind();
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

IEffectShaderConnections::ShaderProperty& PostEffectBufferShader::AddProperty(const ShaderProperty& property)
{
	const std::string uniformName(property.GetUniformName());
	const uint32_t nameHash = xxhash32(uniformName, ShaderProperty::HASH_SEED);
	assert(nameHash != 0);
	assert(mProperties.find(nameHash) == end(mProperties));

	mProperties.emplace(nameHash, property);
	return mProperties[nameHash];
}

IEffectShaderConnections::ShaderProperty& PostEffectBufferShader::AddProperty(ShaderProperty&& property)
{
	OnPropertyAdded(property);

	const std::string uniformName(property.GetUniformName());
	const uint32_t nameHash = xxhash32(uniformName, ShaderProperty::HASH_SEED);
	assert(nameHash != 0);
	assert(mProperties.find(nameHash) == end(mProperties));

	mProperties.emplace(nameHash, std::move(property));
	return mProperties[nameHash];
}

int PostEffectBufferShader::GetNumberOfProperties()
{
	return static_cast<int>(mProperties.size());
}

IEffectShaderConnections::ShaderProperty& PostEffectBufferShader::GetProperty(int index)
{
	assert(index < GetNumberOfProperties());
	auto it = std::next(begin(mProperties), index);
	return it->second;
}

IEffectShaderConnections::ShaderProperty* PostEffectBufferShader::FindProperty(const std::string & name)
{
	const uint32_t nameHash = xxhash32(name, ShaderProperty::HASH_SEED);
	auto it = mProperties.find(nameHash);
	return (it != end(mProperties)) ? &it->second : nullptr;
}

int PostEffectBufferShader::MakePropertyLocationsFromShaderUniforms()
{
	if (!GetShaderPtr())
		return 0;

	const GLuint programId = GetShaderPtr()->GetProgramObj();
	int count = 0;

	for (auto& prop : mProperties)
	{
		GLint location = -1;

		if (strnlen(prop.second.GetUniformName(), 64) > 0)
		{
			location = glGetUniformLocation(programId, prop.second.GetUniformName());
			count += 1;
		}
		prop.second.SetLocation(location);
	}
	return count;
}

int PostEffectBufferShader::MakeSystemLocationsFromShaderUniforms()
{
	ResetSystemUniformLocations();

	if (!GetShaderPtr())
		return 0;

	const GLuint programId = GetShaderPtr()->GetProgramObj();

	GLint numUniforms = 0;
	glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &numUniforms);

	int addedProperties = 0;
	for (GLint i = 0; i < numUniforms; ++i)
	{
		char uniformName[64]{ 0 };
		GLsizei length{ 0 };
		GLint size{ 0 };
		GLenum type{ 0 };
		glGetActiveUniform(programId, i, sizeof(uniformName), &length, &size, &type, uniformName);
		
		// NOTE: skip system properties, but track down the locations
		const int systemUniformId = IsSystemUniform(uniformName);
		if (systemUniformId >= 0)
		{
			mSystemUniformLocations[systemUniformId] = glGetUniformLocation(programId, uniformName);
			addedProperties += 1;
			continue;
		}
	}

	return addedProperties;
}

int PostEffectBufferShader::PopulatePropertiesFromShaderUniforms()
{
	using namespace _PostEffectBufferShaderInternal;

	ResetSystemUniformLocations();

	if (!GetShaderPtr())
		return 0;

	const GLuint programId = GetShaderPtr()->GetProgramObj();

	GLint numUniforms = 0;
	glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &numUniforms);

	GLsizei length{ 0 };
	GLint size{ 0 };
	GLenum type{ 0 };

	int addedProperties = 0;
	for (GLint i = 0; i < numUniforms; ++i)
	{
		ShaderProperty prop;
		glGetActiveUniform(programId, i, ShaderProperty::MAX_NAME_LENGTH, &length, &size, &type, prop.GetUniformNameAccess());
		prop.SetLocation(glGetUniformLocation(programId, prop.GetUniformName()));

		// NOTE: skip system properties, but track down the locations
		const int systemUniformId = IsSystemUniform(prop.GetUniformName());
		if (systemUniformId >= 0)
		{
			mSystemUniformLocations[systemUniformId] = prop.GetLocation();
			continue;
		}

		// skip GLSL internal uniforms
		if (IsInternalGLSLUniform(prop.GetUniformName()))
		{
			continue;
		}

		const auto shaderType = UniformTypeToShaderPropertyType(type);
		prop.SetType(shaderType);

		// from a uniform name, let's extract special postfix and convert it into a flag bit, prepare a clean property name
		// metadata
		ExtractNameAndFlagsFromUniformNameAndType(prop, prop.GetUniformName(), type);

		AddProperty(std::move(prop));
		addedProperties += 1;
	}

	return addedProperties;
}

void PostEffectBufferShader::AutoUploadUniforms(const PostEffectRenderContext& renderContext, 
	const IPostEffectContext* effectContext, bool skipTextureProperties)
{
	const ShaderPropertyStorage* propertyStorage = effectContext->GetShaderPropertyStorage();

	if (!propertyStorage)
		return;

	if (const ShaderPropertyStorage::PropertyValueMap* readMap = propertyStorage->GetReadPropertyMap(GetNameHash()))
	{
		renderContext.UploadUniforms(*readMap, skipTextureProperties);
	}
}

///////////////////////////////////////////////////////////////////////////
// System Uniforms

void PostEffectBufferShader::ResetSystemUniformLocations()
{
	for (int i = 0; i < static_cast<int>(ShaderSystemUniform::COUNT); ++i)
	{
		mSystemUniformLocations[i] = -1;
	}
}

int PostEffectBufferShader::IsSystemUniform(const char* uniformName)
{
	for (int i = 0; i < static_cast<int>(ShaderSystemUniform::COUNT); ++i)
	{
		if (strncmp(uniformName, gSystemUniformNames[i], 256) == 0)
			return i;
	}
	return -1;
}

bool PostEffectBufferShader::IsInternalGLSLUniform(const char* uniformName)
{
	if (strncmp(uniformName, "gl_ModelViewProjectionMatrix", 64) == 0)
		return true;

	return false;
}

void PostEffectBufferShader::BindSystemUniforms(const IPostEffectContext* effectContext) const
{
	if (!GetShaderPtr())
		return;

	// prepare use masking value

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
	
	if (effectContext->GetComponent())
	{
		useMasking |= fn_lookForMaskingFlag(effectContext->GetComponent(), GetUseMaskingPropertyName());
	}
	else if (effectContext->GetPostProcessData())
	{
		useMasking |= fn_lookForMaskingFlag(effectContext->GetPostProcessData(), GetUseMaskingPropertyName());
	}
	
	// bind uniforms
	// TODO: should we replace glProgramUniform with glUniform ?!

	const GLuint programId = GetShaderPtr()->GetProgramObj();

	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::INPUT_COLOR_SAMPLER_2D); loc >= 0)
	{
		glProgramUniform1f(programId, loc, 0);
	}
	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::iCHANNEL0); loc >= 0)
	{
		glProgramUniform1f(programId, loc, 0);
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
	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::USE_MASKING); loc >= 0)
	{
		glProgramUniform1f(programId, loc, (useMasking) ? 1.0f : 0.0f);
	}

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
		glProgramUniform2f(programId, loc, 1.0f/static_cast<float>(effectContext->GetViewWidth()), 1.0f/static_cast<float>(effectContext->GetViewHeight()));
	}
	if (const GLint loc = GetSystemUniformLoc(ShaderSystemUniform::TEXEL_SIZE); loc >= 0)
	{
		glProgramUniform2f(programId, loc, 1.0f/static_cast<float>(effectContext->GetViewWidth()), 1.0f/static_cast<float>(effectContext->GetViewHeight()));
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

bool PostEffectBufferShader::IsDepthSamplerUsed() const
{
	return mSystemUniformLocations[static_cast<int>(ShaderSystemUniform::INPUT_DEPTH_SAMPLER_2D)] >= 0;
}
bool PostEffectBufferShader::IsLinearDepthSamplerUsed() const
{
	return mSystemUniformLocations[static_cast<int>(ShaderSystemUniform::LINEAR_DEPTH_SAMPLER_2D)] >= 0;
}

bool PostEffectBufferShader::IsMaskSamplerUsed() const
{
	return mSystemUniformLocations[static_cast<int>(ShaderSystemUniform::INPUT_MASK_SAMPLER_2D)] >= 0;
}
bool PostEffectBufferShader::IsWorldNormalSamplerUsed() const
{
	return mSystemUniformLocations[static_cast<int>(ShaderSystemUniform::WORLD_NORMAL_SAMPLER_2D)] >= 0;
}