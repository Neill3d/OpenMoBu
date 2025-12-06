
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
	inline std::string_view RemovePostfix(std::string_view name, std::string_view postfix) 
	{
		if (postfix.empty()) {
			return name;  // No postfix to remove
		}

		if (name.size() >= postfix.size() &&
			name.compare(name.size() - postfix.size(), postfix.size(), postfix) == 0)
		{
			return name.substr(0, name.size() - postfix.size());  // Remove postfix
		}

		return name;
	}

	void SetNameAndFlagsFromUniformNameAndType(IEffectShaderConnections::ShaderProperty& prop, const char* uniformNameIn, GLenum type)
	{
		std::string_view name(uniformNameIn);

		// TABLE OF POSTFIX RULES
		struct Rule
		{
			GLenum glType;
			std::string_view postfix;
			IEffectShaderConnections::PropertyFlag flag;
		};

		static constexpr Rule rules[] = {
			{ GL_FLOAT,       "_flag",  IEffectShaderConnections::PropertyFlag::IsFlag },
			{ GL_FLOAT_VEC2,  "_wstoss", IEffectShaderConnections::PropertyFlag::ConvertWorldToScreenSpace },
			{ GL_FLOAT_VEC3,  "_color", IEffectShaderConnections::PropertyFlag::IsColor },
			{ GL_FLOAT_VEC4,  "_color", IEffectShaderConnections::PropertyFlag::IsColor }
		};

		// default: no change
		std::string_view finalName = name;

		for (const Rule& r : rules)
		{
			if (r.glType == type)
			{
				if (name.size() > r.postfix.size() &&
					name.compare(name.size() - r.postfix.size(), r.postfix.size(), r.postfix) == 0)
				{
					prop.SetFlag(r.flag, true);
					finalName = name.substr(0, name.size() - r.postfix.size());
				}
				break;
			}
		}

		prop.SetName(finalName);
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
		.SetRequired(false)
		.SetDefaultValue(CommonEffect::MaskSamplerSlot);

	const char* maskingPropName = GetUseMaskingPropertyName();
	assert(maskingPropName != nullptr);
	UseMaskingProperty = &AddProperty(IEffectShaderConnections::ShaderProperty(maskingPropName, "useMasking", nullptr))
		.SetFlag(IEffectShaderConnections::PropertyFlag::SYSTEM, true)
		.SetRequired(false)
		.SetType(IEffectShaderConnections::EPropertyType::BOOL);

	AddProperty(IEffectShaderConnections::ShaderProperty(PostPersistentData::UPPER_CLIP, "upperClip", nullptr))
		.SetFlag(IEffectShaderConnections::PropertyFlag::SYSTEM, true)
		.SetRequired(false)
		.SetScale(0.01f);

	AddProperty(IEffectShaderConnections::ShaderProperty(PostPersistentData::LOWER_CLIP, "lowerClip", nullptr))
		.SetFlag(IEffectShaderConnections::PropertyFlag::SYSTEM, true)
		.SetRequired(false)
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
		VERIFY(value.GetNameHash() != 0);

		FBComponent* shaderPropertyComp = shaderProperty.GetFBComponent();
		if (!shaderProperty.GetFBProperty() || component != shaderPropertyComp)
		{
			if (strnlen(shaderProperty.GetName(), ShaderProperty::MAX_NAME_LENGTH) > 0)
			{
				shaderProperty.SetFBComponent(component);
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
		VERIFY(value.GetNameHash() != 0);
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
	PostEffectBuffers* buffers = renderContext.buffers;
	
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
		GLuint srcTextureId = renderContext.srcTextureId;

		for (int passIndex = 0; passIndex < finalPassIndex; ++passIndex)
		{
			renderContextPass = renderContext;
			renderContextPass.srcTextureId = srcTextureId;
			renderContextPass.targetFramebuffer = pingPongHelper.GetPtr();
			renderContextPass.colorAttachment = pingPongHelper.GetWriteAttachment();

			// here the derived class could update some property values for the given pass
			OnRenderPassBegin(passIndex, renderContextPass);

			const bool skipTextureUniforms = (passIndex > 0); // only for the first pass we use the original input texture
			AutoUploadUniforms(renderContextPass, effectContext, skipTextureUniforms);
			OnUniformsUploaded(passIndex);

			RenderPass(passIndex, renderContextPass);

			//
			pingPongHelper.Swap();

			// the input for the next pass
			srcTextureId = pingPongHelper.GetPtr()->GetColorObject(pingPongHelper.GetReadAttachment());
		}

		buffers->ReleaseFramebuffer(bufferNameKey);

		// final pass into the destination buffer
		renderContextPass = renderContext;
		renderContextPass.srcTextureId = srcTextureId;

		OnRenderPassBegin(finalPassIndex, renderContextPass);
		AutoUploadUniforms(renderContextPass, effectContext, false);
		OnUniformsUploaded(finalPassIndex);

		RenderPass(finalPassIndex, renderContextPass);
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
	const uint32_t nameHash = property.GetNameHash();
	assert(nameHash != 0);

	auto [it, inserted] = mProperties.emplace(nameHash, property);
	assert(inserted);
	mPropertyOrder.push_back(nameHash);
	OnPropertyAdded(it->second);

	return it->second;
}

IEffectShaderConnections::ShaderProperty& PostEffectBufferShader::AddProperty(ShaderProperty&& property)
{
	const uint32_t nameHash = property.GetNameHash();
	assert(nameHash != 0);

	auto [it, inserted] = mProperties.emplace(nameHash, std::move(property));
	assert(inserted);
	mPropertyOrder.push_back(nameHash);
	OnPropertyAdded(it->second);

	return it->second;
}

int PostEffectBufferShader::GetNumberOfProperties() const
{
	return static_cast<int>(mProperties.size());
}

IEffectShaderConnections::ShaderProperty& PostEffectBufferShader::GetProperty(int index)
{
	VERIFY(index < GetNumberOfProperties());
	VERIFY(mPropertyOrder.size() == mProperties.size());
	return mProperties[mPropertyOrder[index]];
	//auto it = std::next(begin(mProperties), index);
	//return it->second;
}

IEffectShaderConnections::ShaderProperty* PostEffectBufferShader::FindProperty(const std::string & name)
{
	const uint32_t nameHash = xxhash32(name, ShaderProperty::HASH_SEED);
	auto it = mProperties.find(nameHash);
	return (it != end(mProperties)) ? &it->second : nullptr;
}

IEffectShaderConnections::ShaderProperty* PostEffectBufferShader::FindPropertyByUniformName(const char* name) const
{
	for (auto& kv : mProperties)
	{
		if (strcmp(kv.second.GetUniformName(), name) == 0)
		{
			return const_cast<ShaderProperty*>(&kv.second);
		}
	}
	return nullptr;
}

void PostEffectBufferShader::ClearGeneratedByUniformProperties()
{
	for (auto it = mProperties.begin(); it != mProperties.end(); )
	{
		if (it->second.IsGeneratedByUniform())
		{
			it = mProperties.erase(it);
		}
		else
		{
			++it;
		}
	}

	mPropertyOrder.clear();
	mPropertyOrder.reserve(mProperties.size());

	for (auto& kv : mProperties)
		mPropertyOrder.push_back(kv.first);
}

int PostEffectBufferShader::ReflectUniforms()
{
	using namespace _PostEffectBufferShaderInternal;

	ResetSystemUniformLocations();
	// properties could contain manually initialized ones
	ClearGeneratedByUniformProperties();
	if (!GetShaderPtr())
		return 0;

	const GLuint programId = GetShaderPtr()->GetProgramObj();

	GLint count = 0;
	glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &count);

	GLint maxNameLen = 0;
	glGetProgramiv(programId, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLen);

	std::vector<char> name(maxNameLen);

	int added = 0;

	for (int i = 0; i < count; i++)
	{
		GLsizei length;
		GLint size;
		GLenum type;

		glGetActiveUniform(programId, i, maxNameLen, &length, &size, &type, name.data());
		const char* uniformName = name.data();

		// Skip GLSL internal
		if (IsInternalGLSLUniform(uniformName))
			continue;

		const GLint location = glGetUniformLocation(programId, name.data());
		VERIFY(location >= 0);
		
		// System uniform?
		const int sysId = FindSystemUniform(uniformName);
		const bool isSystemUniform = (sysId >= 0);
		if (isSystemUniform)
		{
			mSystemUniformLocations[sysId] = location;
		}

		const auto shaderType = UniformTypeToShaderPropertyType(type);

		// 3) Create ShaderProperty
		if (auto prop = FindPropertyByUniformName(uniformName))
		{
			// already exists, update location
			
			assert(prop->GetType() == shaderType);
			prop->SetLocation(location);
		}
		else if (!isSystemUniform)
		{
			ShaderProperty newProp;
			newProp.SetGeneratedByUniform(true);
			newProp.SetUniformName(uniformName);
			newProp.SetLocation(location);
			newProp.SetType(shaderType);

			// from a uniform name, let's extract special postfix and convert it into a flag bit, prepare a clean property name
			// metadata
			SetNameAndFlagsFromUniformNameAndType(newProp, newProp.GetUniformName(), type);
			AddProperty(std::move(newProp));
			added++;
		}
	}

	return added;
}

bool PostEffectBufferShader::InitializeUniforms(const int varianceIndex)
{
	ReflectUniforms();
	UploadDefaultValues();

	return OnPrepareUniforms(varianceIndex);
}

void PostEffectBufferShader::UploadDefaultValues()
{
	if (!Bind())
		return;

	for (auto& [key, shaderProperty] : mProperties)
	{
		if (shaderProperty.IsGeneratedByUniform())
			continue;

		constexpr bool skipTextureProperties = false;
		PostEffectRenderContext::UploadUniformValue(shaderProperty.GetDefaultValue(), skipTextureProperties);
	}
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
	mSystemUniformLocations.fill(-1);
}

int PostEffectBufferShader::FindSystemUniform(const char* uniformName)
{
	for (int i = 0; i < static_cast<int>(ShaderSystemUniform::COUNT); ++i)
	{
		if (std::strcmp(uniformName, gSystemUniformNames[i]) == 0)
			return i;
	}
	return -1;
}

bool PostEffectBufferShader::IsInternalGLSLUniform(const char* uniformName)
{
	return std::strncmp(uniformName, "gl_", 3) == 0;
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