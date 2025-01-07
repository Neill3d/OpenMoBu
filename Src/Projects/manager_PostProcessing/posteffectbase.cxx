
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
		.SetValue(CommonEffectUniforms::GetMaskSamplerSlot());

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

/////////////////////////////////////////////////////////////////////////
// IEffectShaderConnections

FBPropertyType IEffectShaderConnections::ShaderPropertyToFBPropertyType(const IEffectShaderConnections::ShaderProperty& prop)
{
	switch (prop.type)
	{
	case IEffectShaderConnections::EPropertyType::FLOAT:
		return (prop.HasFlag(IEffectShaderConnections::PropertyFlag::IsFlag)) ? FBPropertyType::kFBPT_bool : FBPropertyType::kFBPT_double;
	case IEffectShaderConnections::EPropertyType::INT:
		return FBPropertyType::kFBPT_int;
	case IEffectShaderConnections::EPropertyType::BOOL:
		return FBPropertyType::kFBPT_bool;
	case IEffectShaderConnections::EPropertyType::VEC2:
		return (prop.HasFlag(IEffectShaderConnections::PropertyFlag::ConvertWorldToScreenSpace)) ? FBPropertyType::kFBPT_Vector3D : FBPropertyType::kFBPT_Vector2D;
	case IEffectShaderConnections::EPropertyType::VEC3:
		return (prop.HasFlag(IEffectShaderConnections::PropertyFlag::IsColor)) ? FBPropertyType::kFBPT_ColorRGB : FBPropertyType::kFBPT_Vector3D;
	case IEffectShaderConnections::EPropertyType::VEC4:
		return (prop.HasFlag(IEffectShaderConnections::PropertyFlag::IsColor)) ? FBPropertyType::kFBPT_ColorRGBA : FBPropertyType::kFBPT_Vector4D;
	case IEffectShaderConnections::EPropertyType::MAT4:
		return FBPropertyType::kFBPT_Vector4D; // TODO:
	case IEffectShaderConnections::EPropertyType::TEXTURE:
		return FBPropertyType::kFBPT_object; // reference to a texture object that we could bind to a property
	default:
		return FBPropertyType::kFBPT_double;
	}
}

IEffectShaderConnections::EPropertyType IEffectShaderConnections::FBPropertyToShaderPropertyType(const FBPropertyType& fbType)
{
	switch (fbType)
	{
	case FBPropertyType::kFBPT_int:
		return IEffectShaderConnections::EPropertyType::INT;
	case FBPropertyType::kFBPT_float:
		return IEffectShaderConnections::EPropertyType::FLOAT;
	case FBPropertyType::kFBPT_bool:
		return IEffectShaderConnections::EPropertyType::FLOAT;
	case FBPropertyType::kFBPT_Vector2D:
		return IEffectShaderConnections::EPropertyType::VEC2;
	case FBPropertyType::kFBPT_ColorRGB:
	case FBPropertyType::kFBPT_Vector3D:
		return IEffectShaderConnections::EPropertyType::VEC3;
	case FBPropertyType::kFBPT_ColorRGBA:
	case FBPropertyType::kFBPT_Vector4D:
		return IEffectShaderConnections::EPropertyType::VEC4;
	}
	return IEffectShaderConnections::EPropertyType::FLOAT;
}

IEffectShaderConnections::EPropertyType IEffectShaderConnections::UniformTypeToShaderPropertyType(GLenum type)
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

IEffectShaderConnections::ShaderProperty::ShaderProperty(const char* nameIn, const char* uniformNameIn, FBProperty* fbPropertyIn)
{
	if (nameIn)
		strcpy_s(name, sizeof(char) * 64, nameIn);
	if (uniformNameIn)
		strcpy_s(uniformName, sizeof(char) * 64, uniformNameIn);
	if (fbPropertyIn)
	{
		const auto propertyType = FBPropertyToShaderPropertyType(fbPropertyIn->GetPropertyType());
		SetType(propertyType);
		fbProperty = fbPropertyIn;
	}
}

IEffectShaderConnections::ShaderProperty::ShaderProperty(const char* nameIn, const char* uniformNameIn, IEffectShaderConnections::EPropertyType typeIn, FBProperty* fbPropertyIn)
{
	if (nameIn)
		strcpy_s(name, sizeof(char) * 64, nameIn);
	if (uniformNameIn)
		strcpy_s(uniformName, sizeof(char) * 64, uniformNameIn);

	SetType(typeIn);

	if (fbPropertyIn)
		fbProperty = fbPropertyIn;
}

IEffectShaderConnections::ShaderProperty& IEffectShaderConnections::ShaderProperty::SetType(IEffectShaderConnections::EPropertyType newType) {
	type = newType;
	switch (newType) {
	case IEffectShaderConnections::EPropertyType::INT:
	case IEffectShaderConnections::EPropertyType::FLOAT:
	case IEffectShaderConnections::EPropertyType::TEXTURE:
		value = std::array<float, 1>{ 0.0f };
		break;
	case IEffectShaderConnections::EPropertyType::VEC2:
		value = std::array<float, 2>{0.0f, 0.0f};
		break;
	case IEffectShaderConnections::EPropertyType::VEC3:
		value = std::array<float, 3>{0.0f, 0.0f, 0.0f};
		break;
	case IEffectShaderConnections::EPropertyType::VEC4:
		value = std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f};
		break;
	case IEffectShaderConnections::EPropertyType::MAT4:
		value = std::vector<float>(15, 0.0f);
		break;
	}
	return *this;
}

IEffectShaderConnections::ShaderProperty& IEffectShaderConnections::ShaderProperty::SetValue(int valueIn)
{
	assert((type == IEffectShaderConnections::EPropertyType::INT)
		|| (type == IEffectShaderConnections::EPropertyType::FLOAT)
		|| (type == IEffectShaderConnections::EPropertyType::TEXTURE));

	value = std::array<float, 1>{ static_cast<float>(valueIn) };
	return *this;
}

IEffectShaderConnections::ShaderProperty& IEffectShaderConnections::ShaderProperty::SetValue(float valueIn)
{
	assert(type == IEffectShaderConnections::EPropertyType::FLOAT);

	value = std::array<float, 1>{ valueIn };
	return *this;
}

IEffectShaderConnections::ShaderProperty& IEffectShaderConnections::ShaderProperty::SetValue(double valueIn)
{
	assert(type == IEffectShaderConnections::EPropertyType::FLOAT);

	value = std::array<float, 1>{ static_cast<float>(valueIn) };
	return *this;
}

IEffectShaderConnections::ShaderProperty& IEffectShaderConnections::ShaderProperty::SetFlag(PropertyFlag testFlag, bool setValue) {
	flags.set(static_cast<size_t>(testFlag), setValue);
	return *this;
}

IEffectShaderConnections::ShaderProperty& IEffectShaderConnections::ShaderProperty::SetScale(float scaleIn)
{
	scale = scaleIn;
	return *this;
}

float IEffectShaderConnections::ShaderProperty::GetScale() const
{
	return scale;
}

float* IEffectShaderConnections::ShaderProperty::GetFloatData() {
	return std::visit([](auto& data) -> float* {
		return data.data();
		}, value);
}

bool IEffectShaderConnections::ShaderProperty::HasFlag(PropertyFlag testFlag) const {
	return flags.test(static_cast<size_t>(testFlag));
}

void IEffectShaderConnections::ShaderProperty::ReadFBPropertyValue(const IPostEffectContext* effectContext, int maskIndex)
{
	if (fbProperty == nullptr)
		return;

	double v[4]{ 0.0 };

	switch (type)
	{
	case IEffectShaderConnections::EPropertyType::INT:
	{
		int ivalue = 0;
		fbProperty->GetData(&ivalue, sizeof(int));
		GetFloatData()[0] = static_cast<float>(ivalue);
	} break;

	case IEffectShaderConnections::EPropertyType::FLOAT:
	{
		if (HasFlag(IEffectShaderConnections::PropertyFlag::IsFlag))
		{
			bool bvalue = false;
			fbProperty->GetData(&bvalue, sizeof(bool));
			GetFloatData()[0] = bvalue ? 1.0f : 0.0f;
		}
		else
		{
			fbProperty->GetData(v, sizeof(double));
			GetFloatData()[0] = static_cast<float>(v[0]);
		}
	} break;

	case IEffectShaderConnections::EPropertyType::VEC2:
	{
		if (!HasFlag(IEffectShaderConnections::PropertyFlag::ConvertWorldToScreenSpace))
		{
			fbProperty->GetData(v, sizeof(double) * 2);
			float* value = GetFloatData();
			value[0] = static_cast<float>(v[0]);
			value[1] = static_cast<float>(v[1]);
		}
		else
		{
			// world space to screen space
			fbProperty->GetData(v, sizeof(double) * 3);

			FBMatrix mvp(effectContext->GetModelViewProjMatrix());

			FBVector4d v4;
			FBVectorMatrixMult(v4, mvp, FBVector4d(v[0], v[1], v[2], 1.0));

			v4[0] = effectContext->GetViewWidth() * 0.5 * (v4[0] + 1.0);
			v4[1] = effectContext->GetViewHeight() * 0.5 * (v4[1] + 1.0);

			float* value = GetFloatData();
			value[0] = static_cast<float>(v4[0]) / static_cast<float>(effectContext->GetViewWidth());
			value[1] = static_cast<float>(v4[1]) / static_cast<float>(effectContext->GetViewHeight());
			//value[2] = static_cast<float>(v4[2]);
		}
		
	} break;

	case IEffectShaderConnections::EPropertyType::VEC3:
	{
		fbProperty->GetData(v, sizeof(double) * 3);

		float* value = GetFloatData();
		value[0] = static_cast<float>(v[0]);
		value[1] = static_cast<float>(v[1]);
		value[2] = static_cast<float>(v[2]);

	} break;

	case IEffectShaderConnections::EPropertyType::VEC4:
	{
		fbProperty->GetData(v, sizeof(double) * 4);

		float* value = GetFloatData();
		value[0] = static_cast<float>(v[0]);
		value[1] = static_cast<float>(v[1]);
		value[2] = static_cast<float>(v[2]);
		value[3] = static_cast<float>(v[3]);
	} break;

	case IEffectShaderConnections::EPropertyType::TEXTURE:
		ReadTextureConnections();
		
		if (shaderUserObject)
		{
			FBComponent* tempComponent = effectContext->GetComponent();
			effectContext->OverrideComponent(shaderUserObject);
			shaderUserObject->GetUserShaderPtr()->CollectUIValues(effectContext, maskIndex);
			effectContext->OverrideComponent(tempComponent);
		}
		break;
	}
}

void IEffectShaderConnections::ShaderProperty::ReadTextureConnections()
{
	texture = nullptr;
	shaderUserObject = nullptr;

	if (FBIS(fbProperty, FBPropertyListObject))
	{
		if (FBPropertyListObject* listObjProp = FBCast<FBPropertyListObject>(fbProperty))
		{
			if (listObjProp->GetCount() > 0)
			{
				if (FBIS(listObjProp->GetAt(0), FBTexture))
				{
					FBTexture* textureObj = FBCast<FBTexture>(listObjProp->GetAt(0));
					assert(textureObj != nullptr);

					texture = textureObj;
				}
				else if (FBIS(listObjProp->GetAt(0), EffectShaderUserObject))
				{
					EffectShaderUserObject* shaderObject = FBCast<EffectShaderUserObject>(listObjProp->GetAt(0));
					assert(shaderObject != nullptr);

					shaderUserObject = shaderObject;
				}
			}
		}
	}
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

void PostEffectBufferShader::AutoUploadUniforms(PostEffectBuffers* buffers, const GLuint inputTextureId, int w, int h, bool generateMips, const IPostEffectContext* effectContext)
{
	GLint userTextureSlot = PostEffectBufferShader::GetUserSamplerId(); //!< start index to bind user textures
	GLSLShaderProgram* shader = GetShaderPtr();

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
		glProgramUniform1i(programId, sysLocations[static_cast<int>(ShaderSystemUniform::INPUT_DEPTH_SAMPLER_2D)], CommonEffectUniforms::GetDepthSamplerSlot());
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::LINEAR_DEPTH_SAMPLER_2D)] >= 0)
	{
		glProgramUniform1i(programId, sysLocations[static_cast<int>(ShaderSystemUniform::LINEAR_DEPTH_SAMPLER_2D)], CommonEffectUniforms::GetLinearDepthSamplerSlot());
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::WORLD_NORMAL_SAMPLER_2D)] >= 0)
	{
		glProgramUniform1i(programId, sysLocations[static_cast<int>(ShaderSystemUniform::WORLD_NORMAL_SAMPLER_2D)], CommonEffectUniforms::GetWorldNormalSamplerSlot());
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::INPUT_MASK_SAMPLER_2D)] >= 0)
	{
		glProgramUniform1i(programId, sysLocations[static_cast<int>(ShaderSystemUniform::INPUT_MASK_SAMPLER_2D)], CommonEffectUniforms::GetMaskSamplerSlot());
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
		glProgramUniform1f(programId, loc, 1.0f - 0.01f * static_cast<float>(value));
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
	if (sysLocations[static_cast<int>(ShaderSystemUniform::TEXEL_SIZE)] >= 0)
	{
		const GLint loc = sysLocations[static_cast<int>(ShaderSystemUniform::TEXEL_SIZE)];
		glProgramUniform2f(programId, loc, 1.0f/static_cast<float>(effectContext->GetViewWidth()), 1.0f/static_cast<float>(effectContext->GetViewHeight()));
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
		std::tm localTime;
		localtime_s(&localTime, &now);  // now should be of type std::time_t
		
		const float secondsSinceMidnight = static_cast<float>(localTime.tm_hour * 3600 + localTime.tm_min * 60 + localTime.tm_sec);
		const float dateTimeArray[4]{
			static_cast<float>(localTime.tm_year + 1900),
			static_cast<float>(localTime.tm_mon + 1),
			static_cast<float>(localTime.tm_mday),
			secondsSinceMidnight
		};

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