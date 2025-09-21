
/**	\file	posteffectbuffershader.cxx

Sergei <Neill3d> Solokhin 2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectbase.h"
#include "postpersistentdata.h"
#include "posteffectbuffers.h"
#include "mobu_logging.h"

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
				prop.flags.set(static_cast<size_t>(IEffectShaderConnections::PropertyFlag::IsFlag), true);
			}
			strncpy_s(prop.name, name.c_str(), name.size());
		} break;
		case GL_FLOAT_VEC2:
		{
			const std::string name = RemovePostfix(uniformName, "_wstoss");
			if (name.size() != uniformName.size())
			{
				prop.flags.set(static_cast<size_t>(IEffectShaderConnections::PropertyFlag::ConvertWorldToScreenSpace), true);
			}
			strncpy_s(prop.name, name.c_str(), name.size());
		} break;
		case GL_FLOAT_VEC3:
		case GL_FLOAT_VEC4:
		{
			const std::string name = RemovePostfix(uniformName, "_color");
			if (name.size() != uniformName.size())
			{
				prop.flags.set(static_cast<size_t>(IEffectShaderConnections::PropertyFlag::IsColor), true);
			}
			strncpy_s(prop.name, name.c_str(), name.size());
		} break;
		default:
			strncpy_s(prop.name, uniformName.c_str(), uniformName.size());
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
		.SetValue(CommonEffect::MaskSamplerSlot);

	const char* maskingPropName = GetUseMaskingPropertyName();
	assert(maskingPropName != nullptr);
	AddProperty(IEffectShaderConnections::ShaderProperty(maskingPropName, "useMasking", nullptr))
		.SetFlag(IEffectShaderConnections::PropertyFlag::SYSTEM, true);

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

bool PostEffectBufferShader::PrepUniforms(const int varianceIndex)
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

bool PostEffectBufferShader::CollectUIValues(FBComponent* component, const IPostEffectContext* effectContext, int maskIndex)
{
	if (!component)
		return false;

	for (auto& prop : mProperties)
	{
		if (strnlen(prop.second.name, 64) > 0)
		{
			if (FBProperty* fbProperty = component->PropertyList.Find(prop.second.name))
			{
				prop.second.fbProperty = fbProperty;
				prop.second.ReadFBPropertyValue(effectContext, maskIndex);
			}
		}
	}

	return true;
}

bool PostEffectBufferShader::CollectUIValues(const IPostEffectContext* effectContext, int maskIndex)
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

void PostEffectBufferShader::UploadUniforms(PostEffectBuffers* buffers, FrameBuffer* dstBuffer, int colorAttachment, const GLuint inputTextureId, int w, int h, bool generateMips, const IPostEffectContext* effectContext)
{
	// system uniforms
	BindSystemUniforms(effectContext);
	
	AutoUploadUniforms(buffers, inputTextureId, w, h, generateMips, effectContext);

	OnUploadUniforms(buffers, dstBuffer, colorAttachment, inputTextureId, w, h, generateMips, effectContext);
}

bool PostEffectBufferShader::ReloadPropertyShaders()
{
	for (auto& prop : mProperties)
	{
		if (prop.second.type == IEffectShaderConnections::EPropertyType::TEXTURE)
			prop.second.ReadTextureConnections();

		if (prop.second.shaderUserObject)
		{
			if (!prop.second.shaderUserObject->DoReloadShaders())
				return false;
		}
	}
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
void PostEffectBufferShader::Render(PostEffectBuffers* buffers, FrameBuffer* dstBuffer, int colorAttachment, const GLuint inputTextureId, int w, int h, bool generateMips, const IPostEffectContext* effectContext)
{
	if (GetNumberOfPasses() == 0)
		return;

	GLuint texId = inputTextureId;

	Bind();
	UploadUniforms(buffers, dstBuffer, colorAttachment, inputTextureId, w, h, generateMips, effectContext);

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

IEffectShaderConnections::ShaderProperty& PostEffectBufferShader::AddProperty(const ShaderProperty& property)
{
	const std::string uniformName(property.uniformName);
	assert(mProperties.find(uniformName) == end(mProperties));

	mProperties.emplace(uniformName, property);
	return mProperties[uniformName];
}

IEffectShaderConnections::ShaderProperty& PostEffectBufferShader::AddProperty(ShaderProperty&& property)
{
	const std::string uniformName(property.uniformName);
	assert(mProperties.find(uniformName) == end(mProperties));

	mProperties.emplace(uniformName, std::move(property));
	return mProperties[uniformName];
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
	auto it = mProperties.find(name);
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
		prop.second.location = -1;

		if (strnlen(prop.second.uniformName, 64) > 0)
		{
			prop.second.location = glGetUniformLocation(programId, prop.second.uniformName);
			count += 1;
		}
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

	int addedProperties = 0;
	for (GLint i = 0; i < numUniforms; ++i)
	{
		ShaderProperty prop;
		GLsizei length{ 0 };
		GLint size{ 0 };
		GLenum type{ 0 };
		glGetActiveUniform(programId, i, sizeof(prop.uniformName), &length, &size, &type, prop.uniformName);
		prop.location = glGetUniformLocation(programId, prop.uniformName);

		// NOTE: skip system properties, but track down the locations
		const int systemUniformId = IsSystemUniform(prop.uniformName);
		if (systemUniformId >= 0)
		{
			mSystemUniformLocations[systemUniformId] = prop.location;
			continue;
		}

		// skip GLSL internal uniforms
		if (IsInternalGLSLUniform(prop.uniformName))
		{
			continue;
		}

		auto shaderType = UniformTypeToShaderPropertyType(type);
		prop.SetType(shaderType);

		// from a uniform name, let's extract special postfix and convert it into a flag bit, prepare a clean property name
		ExtractNameAndFlagsFromUniformNameAndType(prop, prop.uniformName, type);

		//prop.property = mUserObject->GetOrMakeProperty(prop);

		OnPropertyAdded(prop);

		mProperties.emplace(prop.uniformName, std::move(prop));
		addedProperties += 1;
	}

	return addedProperties;
}

void PostEffectBufferShader::AutoUploadUniforms(PostEffectBuffers* buffers, const GLuint inputTextureId, int w, int h, bool generateMips, const IPostEffectContext* effectContext)
{
	GLint userTextureSlot = PostEffectBufferShader::GetUserSamplerId(); //!< start index to bind user textures
	//GLSLShaderProgram* shader = GetShaderPtr();

	//std::vector<EffectShaderUserObject*> shadersChain;

	for (auto& prop : mProperties)
	{
		if (prop.second.location < 0)
			continue;

		switch (prop.second.type)
		{
		case IEffectShaderConnections::EPropertyType::INT:
			glUniform1i(prop.second.location, static_cast<int>(prop.second.GetFloatData()[0]));
			break;

		case IEffectShaderConnections::EPropertyType::BOOL:
			glUniform1f(prop.second.location, prop.second.GetFloatData()[0]);
			break;

		case IEffectShaderConnections::EPropertyType::FLOAT:
			if (!prop.second.HasFlag(IEffectShaderConnections::PropertyFlag::INVERT_VALUE))
			{
				glUniform1f(prop.second.location, prop.second.GetScale() * prop.second.GetFloatData()[0]);
			}
			else
			{
				glUniform1f(prop.second.location, 1.0f - prop.second.GetScale() * prop.second.GetFloatData()[0]);
			}
			break;

		case IEffectShaderConnections::EPropertyType::VEC2:
			glUniform2fv(prop.second.location, 1, prop.second.GetFloatData());
			break;

		case IEffectShaderConnections::EPropertyType::VEC3:
			glUniform3fv(prop.second.location, 1, prop.second.GetFloatData());
			break;

		case IEffectShaderConnections::EPropertyType::VEC4:
			glUniform4fv(prop.second.location, 1, prop.second.GetFloatData());
			break;

		case IEffectShaderConnections::EPropertyType::TEXTURE:
			if (FBTexture* texture = prop.second.texture)
			{
				// bind sampler from a media resource texture

				int textureId = texture->TextureOGLId;
				if (textureId == 0)
				{
					texture->OGLInit();
					textureId = texture->TextureOGLId;
				}

				if (textureId > 0)
				{
					//const GLint loc = glGetUniformLocation(shader->GetProgramObj(), prop.first.c_str());
					glUniform1i(prop.second.location, userTextureSlot);

					glActiveTexture(GL_TEXTURE0 + userTextureSlot);
					glBindTexture(GL_TEXTURE_2D, textureId);
					glActiveTexture(GL_TEXTURE0);

					userTextureSlot += 1;
				}
			}
			else if (EffectShaderUserObject* userObject = prop.second.shaderUserObject)
			{
				PostEffectBufferShader* bufferShader = userObject->GetUserShaderPtr();

				UnBind();

				// bind sampler from another rendered buffer shader
				const std::string bufferName = std::string(GetName()) + "_" + std::string(userObject->Name);

				int effectW = w;
				int effectH = h;
				userObject->RecalculateWidthAndHeight(effectW, effectH);

				FrameBuffer* buffer = buffers->RequestFramebuffer(bufferName, effectW, effectH, PostEffectBuffers::GetFlagsForSingleColorBuffer(), 1, false);

				bufferShader->Render(buffers, buffer, 0, inputTextureId, effectW, effectH, generateMips, effectContext);

				const GLuint bufferTextureId = buffer->GetColorObject();
				buffers->ReleaseFramebuffer(bufferName);

				// bind input buffers
				glActiveTexture(GL_TEXTURE0 + userTextureSlot);
				glBindTexture(GL_TEXTURE_2D, bufferTextureId);
				glActiveTexture(GL_TEXTURE0);

				Bind();

				//const GLint loc = glGetUniformLocation(shader->GetProgramObj(), prop.first.c_str());
				glUniform1i(prop.second.location, userTextureSlot);

				userTextureSlot += 1;
			}
			else
			{
				const int textureSlot = static_cast<int>(prop.second.GetFloatData()[0]);
				if (textureSlot >= 0 && textureSlot < 16)
				{
					glUniform1i(prop.second.location, textureSlot);
				}
			}
			break;

		default:
			LOGE("not supported property for auto upload into uniform %s", prop.second.name);
		}
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

	const GLuint programId = GetShaderPtr()->GetProgramObj();
	const GLint* sysLocations = mSystemUniformLocations;
	bool useMasking = false;

	if (effectContext->GetComponent())
	{
		if (FBProperty* prop = effectContext->GetComponent()->PropertyList.Find(GetUseMaskingPropertyName()))
		{
			bool bvalue{ false };
			prop->GetData(&bvalue, sizeof(bool));
			useMasking = bvalue;
		}
	}

	auto IsUniformBound = [sysLocations](ShaderSystemUniform systemUniform) {
		return sysLocations[static_cast<int>(systemUniform)] >= 0;
		};
	auto GetUniformLocation = [sysLocations](ShaderSystemUniform systemUniform) {
		return sysLocations[static_cast<int>(systemUniform)];
		};

	if (IsUniformBound(ShaderSystemUniform::INPUT_COLOR_SAMPLER_2D))
	{
		glProgramUniform1f(programId, GetUniformLocation(ShaderSystemUniform::INPUT_COLOR_SAMPLER_2D), 0);
	}
	if (IsUniformBound(ShaderSystemUniform::iCHANNEL0))
	{
		glProgramUniform1f(programId, GetUniformLocation(ShaderSystemUniform::iCHANNEL0), 0);
	}
	if (IsUniformBound(ShaderSystemUniform::INPUT_DEPTH_SAMPLER_2D))
	{
		glProgramUniform1i(programId, GetUniformLocation(ShaderSystemUniform::INPUT_DEPTH_SAMPLER_2D), CommonEffect::DepthSamplerSlot);
	}
	if (IsUniformBound(ShaderSystemUniform::LINEAR_DEPTH_SAMPLER_2D))
	{
		glProgramUniform1i(programId, GetUniformLocation(ShaderSystemUniform::LINEAR_DEPTH_SAMPLER_2D), CommonEffect::LinearDepthSamplerSlot);
	}
	if (IsUniformBound(ShaderSystemUniform::WORLD_NORMAL_SAMPLER_2D))
	{
		glProgramUniform1i(programId, GetUniformLocation(ShaderSystemUniform::WORLD_NORMAL_SAMPLER_2D), CommonEffect::WorldNormalSamplerSlot);
	}
	if (IsUniformBound(ShaderSystemUniform::INPUT_MASK_SAMPLER_2D))
	{
		glProgramUniform1i(programId, GetUniformLocation(ShaderSystemUniform::INPUT_MASK_SAMPLER_2D), CommonEffect::MaskSamplerSlot);
	}
	if (IsUniformBound(ShaderSystemUniform::USE_MASKING))
	{
		const GLint loc = GetUniformLocation(ShaderSystemUniform::USE_MASKING);
		glProgramUniform1f(programId, loc, (useMasking) ? 1.0f : 0.0f);
	}

	if (IsUniformBound(ShaderSystemUniform::UPPER_CLIP))
	{
		const GLint loc = GetUniformLocation(ShaderSystemUniform::UPPER_CLIP);
		const double value = effectContext->GetPostProcessData()->UpperClip;
		glProgramUniform1f(programId, loc, 0.01f * static_cast<float>(value));
	}

	if (IsUniformBound(ShaderSystemUniform::LOWER_CLIP))
	{
		const GLint loc = GetUniformLocation(ShaderSystemUniform::LOWER_CLIP);
		const double value = effectContext->GetPostProcessData()->LowerClip;
		glProgramUniform1f(programId, loc, 1.0f - 0.01f * static_cast<float>(value));
	}

	if (IsUniformBound(ShaderSystemUniform::RESOLUTION))
	{
		const GLint loc = GetUniformLocation(ShaderSystemUniform::RESOLUTION);
		glProgramUniform2f(programId, loc, static_cast<float>(effectContext->GetViewWidth()), static_cast<float>(effectContext->GetViewHeight()));
	}
	if (IsUniformBound(ShaderSystemUniform::iRESOLUTION))
	{
		const GLint loc = GetUniformLocation(ShaderSystemUniform::iRESOLUTION);
		glProgramUniform2f(programId, loc, static_cast<float>(effectContext->GetViewWidth()), static_cast<float>(effectContext->GetViewHeight()));
	}
	if (IsUniformBound(ShaderSystemUniform::TEXEL_SIZE))
	{
		const GLint loc = GetUniformLocation(ShaderSystemUniform::TEXEL_SIZE);
		glProgramUniform2f(programId, loc, 1.0f/static_cast<float>(effectContext->GetViewWidth()), 1.0f/static_cast<float>(effectContext->GetViewHeight()));
	}

	if (IsUniformBound(ShaderSystemUniform::iTIME))
	{
		const GLint loc = GetUniformLocation(ShaderSystemUniform::iTIME);
		glProgramUniform1f(programId, loc, static_cast<float>(effectContext->GetSystemTime()));
	}
	if (IsUniformBound(ShaderSystemUniform::iDATE))
	{
		const GLint loc = GetUniformLocation(ShaderSystemUniform::iDATE);
		glProgramUniform4fv(programId, loc, 1, effectContext->GetIDate());
	}
	if (IsUniformBound(ShaderSystemUniform::CAMERA_POSITION))
	{
		const GLint loc = GetUniformLocation(ShaderSystemUniform::CAMERA_POSITION);
		glProgramUniform3fv(programId, loc, 1, effectContext->GetCameraPositionF());
	}
	if (IsUniformBound(ShaderSystemUniform::MODELVIEW))
	{
		const GLint loc = GetUniformLocation(ShaderSystemUniform::MODELVIEW);
		glProgramUniformMatrix4fv(programId, loc, 1, GL_FALSE, effectContext->GetModelViewMatrixF());
	}
	if (IsUniformBound(ShaderSystemUniform::PROJ))
	{
		const GLint loc = GetUniformLocation(ShaderSystemUniform::PROJ);
		glProgramUniformMatrix4fv(programId, loc, 1, GL_FALSE, effectContext->GetProjectionMatrixF());
	}
	if (IsUniformBound(ShaderSystemUniform::MODELVIEWPROJ))
	{
		const GLint loc = GetUniformLocation(ShaderSystemUniform::MODELVIEWPROJ);
		glProgramUniformMatrix4fv(programId, loc, 1, GL_FALSE, effectContext->GetModelViewProjMatrixF());
	}
	if (IsUniformBound(ShaderSystemUniform::INV_MODELVIEWPROJ))
	{
		const GLint loc = GetUniformLocation(ShaderSystemUniform::INV_MODELVIEWPROJ);
		glProgramUniformMatrix4fv(programId, loc, 1, GL_FALSE, effectContext->GetInvModelViewProjMatrixF());
	}
	if (IsUniformBound(ShaderSystemUniform::PREV_MODELVIEWPROJ))
	{
		const GLint loc = GetUniformLocation(ShaderSystemUniform::PREV_MODELVIEWPROJ);
		glProgramUniformMatrix4fv(programId, loc, 1, GL_FALSE, effectContext->GetPrevModelViewProjMatrixF());
	}
	if (IsUniformBound(ShaderSystemUniform::ZNEAR))
	{
		const GLint loc = GetUniformLocation(ShaderSystemUniform::ZNEAR);
		glProgramUniform1f(programId, loc, effectContext->GetCameraNearDistance());
	}
	if (IsUniformBound(ShaderSystemUniform::ZFAR))
	{
		const GLint loc = GetUniformLocation(ShaderSystemUniform::ZFAR);
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