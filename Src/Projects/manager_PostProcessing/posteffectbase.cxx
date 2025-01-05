
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
// PostEffectBufferShader

PostEffectBufferShader::PostEffectBufferShader()
{
	ResetSystemUniformLocations();
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

void CommonEffectUniforms::PrepareCommonLocations(GLSLShaderProgram* shader)
{
	// TODO: move into shader properties ?!
	const GLint loc = shader->findLocation("maskSampler");
	if (loc >= 0)
		glUniform1i(loc, GetMaskSamplerSlot());

	useMaskLoc = shader->findLocation("useMasking");
	upperClipLoc = shader->findLocation("upperClip");
	lowerClipLoc = shader->findLocation("lowerClip");
}

void CommonEffectUniforms::CollectCommonData(PostPersistentData* data, FBComponent* userObject, const char* enableMaskingPropertyName)
{
	bool isEnabledEffectMasking = false;
	if (enableMaskingPropertyName && userObject)
	{
		if (FBProperty* prop = userObject->PropertyList.Find(enableMaskingPropertyName))
		{
			isEnabledEffectMasking = prop->AsInt() > 0;
		}
	}
	
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

bool PostEffectBufferShader::PrepUniforms(const int varianceIndex)
{
	PrepareCommonLocations(GetShaderPtr());
	return OnPrepareUniforms(varianceIndex);
}

bool PostEffectBufferShader::CollectUIValues(const IPostEffectContext* effectContext, int maskIndex)
{
	CollectCommonData(nullptr, nullptr, GetUseMaskingPropertyName());
	return OnCollectUI(effectContext, maskIndex);
}

void PostEffectBufferShader::UploadUniforms(PostEffectBuffers* buffers, FrameBuffer* dstBuffer, int colorAttachment, const GLuint inputTextureId, int w, int h, bool generateMips)
{
	UploadCommonData();
	OnUploadUniforms(buffers, dstBuffer, colorAttachment, inputTextureId, w, h, generateMips);
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
void PostEffectBufferShader::Render(PostEffectBuffers* buffers, FrameBuffer* dstBuffer, int colorAttachment, const GLuint inputTextureId, int w, int h, bool generateMips)
{
	if (GetNumberOfPasses() == 0)
		return;

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

/////////////////////////////////////////////////////////////////////////
// IEffectShaderConnections

int PostEffectBufferShader::AddProperty(const ShaderProperty& property)
{
	mProperties.emplace(property.uniformName, property);
}

int PostEffectBufferShader::AddProperty(ShaderProperty&& property)
{
	mProperties.emplace(property.uniformName, std::move(property));
}

int PostEffectBufferShader::GetNumberOfProperties()
{
	return static_cast<int>(mProperties.size());
}

IEffectShaderConnections::ShaderProperty& PostEffectBufferShader::GetProperty(int index)
{
	auto it = begin(mProperties);
	for (int i = 0; i < index; ++i)
	{
		++it;
	}
	return it->second;
}

IEffectShaderConnections::ShaderProperty* PostEffectBufferShader::FindProperty(const std::string & name)
{
	auto it = mProperties.find(name);
	return (it != end(mProperties)) ? &it->second : nullptr;
}

IEffectShaderConnections::EPropertyType UniformTypeToShaderPropertyType(GLenum type)
{
	switch (type)
	{
	case GL_FLOAT:
		return IEffectShaderConnections::EPropertyType::FLOAT;
	case GL_INT:
		return IEffectShaderConnections::EPropertyType::INT;
	case GL_BOOL:
		return IEffectShaderConnections::EPropertyType::INT;
	case GL_FLOAT_VEC2:
		return IEffectShaderConnections::EPropertyType::VEC2;
	case GL_FLOAT_VEC3:
		return IEffectShaderConnections::EPropertyType::VEC3;
	case GL_FLOAT_VEC4:
		return IEffectShaderConnections::EPropertyType::VEC4;
	case GL_FLOAT_MAT4:
		return IEffectShaderConnections::EPropertyType::MAT4;
	case GL_SAMPLER_2D:
		return IEffectShaderConnections::EPropertyType::TEXTURE;
	default:
		LOGE("unsupported uniform type %d", type);
		return IEffectShaderConnections::EPropertyType::FLOAT;
	}
}

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
		memcpy(prop.name, name.c_str(), sizeof(char) * name.size());
	} break;
	case GL_FLOAT_VEC2:
	{
		const std::string name = RemovePostfix(uniformName, "_wstoss");
		if (name.size() != uniformName.size())
		{
			prop.flags.set(static_cast<size_t>(IEffectShaderConnections::PropertyFlag::ConvertWorldToScreenSpace), true);
		}
		memcpy(prop.name, name.c_str(), sizeof(char) * name.size());
	} break;
	case GL_FLOAT_VEC3:
	case GL_FLOAT_VEC4:
	{
		const std::string name = RemovePostfix(uniformName, "_color");
		if (name.size() != uniformName.size())
		{
			prop.flags.set(static_cast<size_t>(IEffectShaderConnections::PropertyFlag::IsColor), true);
		}
		memcpy(prop.name, name.c_str(), sizeof(char) * name.size());
	} break;
	default:
		memcpy(prop.name, uniformName.c_str(), sizeof(char) * uniformName.size());
	}
}

int PostEffectBufferShader::PopulatePropertiesFromShaderUniforms()
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

void PostEffectBufferShader::AutoUploadUniforms(PostEffectBuffers* buffers, const GLuint inputTextureId, int w, int h, bool generateMips)
{
	GLint userTextureSlot = PostEffectBufferShader::GetUserSamplerId(); //!< start index to bind user textures
	GLSLShaderProgram* shader = GetShaderPtr();

	std::vector<EffectShaderUserObject*> shadersChain;

	for (auto& prop : mProperties)
	{
		switch (prop.second.type)
		{
		case IEffectShaderConnections::EPropertyType::INT:
			glUniform1i(prop.second.location, static_cast<int>(prop.second.GetFloatData()[0]));
			break;

		case IEffectShaderConnections::EPropertyType::FLOAT:
			glUniform1f(prop.second.location, prop.second.GetFloatData()[0]);
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
					const GLint loc = glGetUniformLocation(shader->GetProgramObj(), prop.first.c_str());
					glUniform1i(loc, userTextureSlot);

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

				bufferShader->Render(buffers, buffer, 0, inputTextureId, effectW, effectH, generateMips);

				const GLuint bufferTextureId = buffer->GetColorObject();
				buffers->ReleaseFramebuffer(bufferName);

				// bind input buffers
				glActiveTexture(GL_TEXTURE0 + userTextureSlot);
				glBindTexture(GL_TEXTURE_2D, bufferTextureId);
				glActiveTexture(GL_TEXTURE0);

				Bind();

				const GLint loc = glGetUniformLocation(shader->GetProgramObj(), prop.first.c_str());
				glUniform1i(loc, userTextureSlot);

				userTextureSlot += 1;
			}
			break;

		default:
			LOGE("not supported property for auto upload into uniform %s", prop.second.name);
		}
	}
}

///////////////////////////////////////////////////////////////////////////
// System Uniforms

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

	"iTime", //!< compatible with shadertoy, float, shader playback time (in seconds)
	"iDate",  //!< compatible with shadertoy, vec4, (year, month, day, time in seconds)

	"cameraPosition", //!< world space camera position
	"modelView", //!< current camera modelview matrix
	"projection", //!< current camera projection matrix
	"modelViewProj" //!< current camera modelview-projection matrix
};

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


void PostEffectBufferShader::BindSystemUniforms(const IPostEffectContext* effectContextIn) const
{
	const PostEffectContextMoBu* effectContext = static_cast<const PostEffectContextMoBu*>(effectContextIn);

	const GLuint programId = GetShaderPtr()->GetProgramObj();
	const GLint* sysLocations = mSystemUniformLocations;
	bool useMasking = false;
	if (effectContext->GetComponent())
	{
		if (FBProperty* prop = effectContext->GetComponent()->PropertyList.Find("Use Masking"))
		{
			bool bvalue{ false };
			prop->GetData(&bvalue, sizeof(bool));
			useMasking = bvalue;
		}
	}

	if (sysLocations[static_cast<int>(ShaderSystemUniform::INPUT_COLOR_SAMPLER_2D)] >= 0)
	{
		glProgramUniform1f(programId, sysLocations[static_cast<int>(ShaderSystemUniform::INPUT_COLOR_SAMPLER_2D)], 0);
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::iCHANNEL0)] >= 0)
	{
		glProgramUniform1f(programId, sysLocations[static_cast<int>(ShaderSystemUniform::iCHANNEL0)], 0);
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::INPUT_DEPTH_SAMPLER_2D)] >= 0)
	{
		glProgramUniform1i(programId, sysLocations[static_cast<int>(ShaderSystemUniform::INPUT_DEPTH_SAMPLER_2D)], GetDepthSamplerSlot());
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::LINEAR_DEPTH_SAMPLER_2D)] >= 0)
	{
		glProgramUniform1i(programId, sysLocations[static_cast<int>(ShaderSystemUniform::LINEAR_DEPTH_SAMPLER_2D)], GetLinearDepthSamplerSlot());
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::WORLD_NORMAL_SAMPLER_2D)] >= 0)
	{
		glProgramUniform1i(programId, sysLocations[static_cast<int>(ShaderSystemUniform::WORLD_NORMAL_SAMPLER_2D)], GetWorldNormalSamplerSlot());
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::INPUT_MASK_SAMPLER_2D)] >= 0)
	{
		glProgramUniform1i(programId, sysLocations[static_cast<int>(ShaderSystemUniform::INPUT_MASK_SAMPLER_2D)], GetMaskSamplerSlot());
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::USE_MASKING)] >= 0)
	{
		const GLint loc = sysLocations[static_cast<int>(ShaderSystemUniform::USE_MASKING)];
		glProgramUniform1f(programId, loc, (useMasking) ? 1.0f : 0.0f);
	}

	if (sysLocations[static_cast<int>(ShaderSystemUniform::UPPER_CLIP)] >= 0)
	{
		const GLint loc = sysLocations[static_cast<int>(ShaderSystemUniform::UPPER_CLIP)];
		const double value = effectContext->GetPostProcessData()->UpperClip;
		glProgramUniform1f(programId, loc, 0.01f * static_cast<float>(value));
	}

	if (sysLocations[static_cast<int>(ShaderSystemUniform::LOWER_CLIP)] >= 0)
	{
		const GLint loc = sysLocations[static_cast<int>(ShaderSystemUniform::LOWER_CLIP)];
		const double value = effectContext->GetPostProcessData()->LowerClip;
		glProgramUniform1f(programId, loc, 0.01f * static_cast<float>(value));
	}

	if (sysLocations[static_cast<int>(ShaderSystemUniform::RESOLUTION)] >= 0)
	{
		const GLint loc = sysLocations[static_cast<int>(ShaderSystemUniform::RESOLUTION)];
		glProgramUniform2f(programId, loc, static_cast<float>(effectContext->GetViewWidth()), static_cast<float>(effectContext->GetViewHeight()));
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::iRESOLUTION)] >= 0)
	{
		const GLint loc = sysLocations[static_cast<int>(ShaderSystemUniform::iRESOLUTION)];
		glProgramUniform2f(programId, loc, static_cast<float>(effectContext->GetViewWidth()), static_cast<float>(effectContext->GetViewHeight()));
	}

	if (sysLocations[static_cast<int>(ShaderSystemUniform::iTIME)] >= 0)
	{
		const GLint loc = sysLocations[static_cast<int>(ShaderSystemUniform::iTIME)];
		glProgramUniform1f(programId, loc, static_cast<float>(effectContext->GetSystemTime()));
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::iDATE)] >= 0)
	{
		const GLint loc = sysLocations[static_cast<int>(ShaderSystemUniform::iDATE)];

		std::time_t now = std::time(nullptr);
		std::tm* localTime = std::localtime(&now);
		
		float dateTimeArray[4];

		dateTimeArray[0] = static_cast<float>(localTime->tm_year + 1900);
		dateTimeArray[1] = static_cast<float>(localTime->tm_mon + 1);
		dateTimeArray[2] = static_cast<float>(localTime->tm_mday);

		float secondsSinceMidnight = static_cast<float>(localTime->tm_hour * 3600 + localTime->tm_min * 60 + localTime->tm_sec);
		dateTimeArray[3] = secondsSinceMidnight;

		glProgramUniform4fv(programId, loc, 1, dateTimeArray);
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::CAMERA_POSITION)] >= 0)
	{
		const GLint loc = sysLocations[static_cast<int>(ShaderSystemUniform::CAMERA_POSITION)];
		const double* pos = effectContext->GetCameraPosition();
		const float fpos[3]{ static_cast<float>(pos[0]), static_cast<float>(pos[1]), static_cast<float>(pos[2]) };
		glProgramUniform3fv(programId, loc, 1, fpos);
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::MODELVIEW)] >= 0)
	{
		const GLint loc = sysLocations[static_cast<int>(ShaderSystemUniform::MODELVIEW)];
		const double* tm = effectContext->GetModelViewMatrix();
		float fModelView[16];
		for (int i = 0; i < 16; ++i)
		{
			fModelView[i] = static_cast<float>(tm[i]);
		}
		glProgramUniformMatrix4fv(programId, loc, 1, GL_FALSE, fModelView);
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::PROJ)] >= 0)
	{
		const GLint loc = sysLocations[static_cast<int>(ShaderSystemUniform::PROJ)];
		const double* tm = effectContext->GetProjectionMatrix();
		float fMatrix[16];
		for (int i = 0; i < 16; ++i)
		{
			fMatrix[i] = static_cast<float>(tm[i]);
		}
		glProgramUniformMatrix4fv(programId, loc, 1, GL_FALSE, fMatrix);
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::MODELVIEWPROJ)] >= 0)
	{
		const GLint loc = sysLocations[static_cast<int>(ShaderSystemUniform::MODELVIEWPROJ)];
		const double* tm = effectContext->GetModelViewProjMatrix();
		float fMatrix[16];
		for (int i = 0; i < 16; ++i)
		{
			fMatrix[i] = static_cast<float>(tm[i]);
		}
		glProgramUniformMatrix4fv(programId, loc, 1, GL_FALSE, fMatrix);
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
			if (!bufferShader->CollectUIValues(effectContext, GetMaskIndex()))
				return false;
		}
	}

	return true;
}


void PostEffectBase::Process(const RenderEffectContext& context)
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