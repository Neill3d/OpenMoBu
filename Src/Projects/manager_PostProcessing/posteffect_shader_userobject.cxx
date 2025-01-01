
/** \file posteffect_userobject.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

// Class declaration
#include "posteffect_shader_userobject.h"
#include "postprocessing_ini.h"
#include <vector>
#include <limits>
#include <ctime>

#include "FileUtils.h"
#include "mobu_logging.h"

#include "postpersistentdata.h"
#include "posteffectbuffers.h"

// custom assets inserting

/** Element Class implementation. (Asset system)
*	This should be placed in the source code file for a class.
*/
#define PostEffectFBElementClassImplementation(ClassName,AssetName,IconFileName)\
	HIObject RegisterElement##ClassName##Create(HIObject /*pOwner*/, const char* pName, void* /*pData*/){\
	ClassName* Class = new ClassName(pName); \
	Class->mAllocated = true; \
if (Class->FBCreate()){\
	__FBRemoveModelFromScene(Class->GetHIObject()); /* Hack in MoBu2013, we shouldn't add object to the scene/entity automatically*/\
	return Class->GetHIObject(); \
}\
else {\
	delete Class; \
	return NULL;\
}\
	}\
		FBLibraryModule(ClassName##Element){\
		FBRegisterObject(ClassName##R2, "Browsing/Templates/Shading Elements", AssetName, "", RegisterElement##ClassName##Create, true, IconFileName); \
		}

//--- FiLMBOX Registration & Implementation.
FBClassImplementation(EffectShaderUserObject);
FBUserObjectImplement(EffectShaderUserObject,
                        "User Object used to store a persistance data for one effect shader.",
						"cam_switcher_toggle.png");                                          //Register UserObject class
PostEffectFBElementClassImplementation(EffectShaderUserObject, "Effect Shader", "cam_switcher_toggle.png");                  //Register to the asset system

////////////////////////////////////////////////////////////////////////////////
//

const char* UserBufferShader::gSystemUniformNames[static_cast<int>(ShaderSystemUniform::COUNT)] =
{
	"inputSampler", //!< this is an input image that we read from
	"iChannel0", //!< this is an input image, compatible with shadertoy
	"depthSampler", //!< this is a scene depth texture sampler in case shader will need it for processing
	"linearDepthSampler",
	"maskSampler", //!< binded mask for a shader processing

	"bufferSampler", //!< buffer A sampler

	"useMasking", //!< float uniform [0; 1] to define if the mask have to be used
	"upperClip", //!< this is an upper clip image level. defined in a texture coord space to skip processing
	"lowerClip", //!< this is a lower clip image level. defined in a texture coord space to skip processing

	"gResolution", //!< vec2 that contains processing absolute resolution, like 1920x1080
	"iResolution", //!< vec2 image absolute resolution, compatible with shadertoy naming

	"iTime", //!< compatible with shadertoy, float, shader playback time (in seconds)
	"iDate",  //!< compatible with shadertoy, vec4, (year, month, day, time in seconds)

	"modelView", //!< current camera modelview matrix
	"projection", //!< current camera projection matrix
	"modelViewProj" //!< current camera modelview-projection matrix
};

// use uniformName to track down some type casts
FBPropertyType UniformTypeToFBPropertyType(GLenum type, const char* uniformName)
{
	switch (type)
	{
	case GL_FLOAT:
		return (strstr(uniformName, "_flag") != nullptr) ? FBPropertyType::kFBPT_bool : FBPropertyType::kFBPT_double;
	case GL_INT:
		return FBPropertyType::kFBPT_int;
	case GL_BOOL:
		return FBPropertyType::kFBPT_bool;
	case GL_FLOAT_VEC2:
		return FBPropertyType::kFBPT_Vector2D;
	case GL_FLOAT_VEC3:
		return (strstr(uniformName, "_color") != nullptr) ? FBPropertyType::kFBPT_ColorRGB : FBPropertyType::kFBPT_Vector3D;
	case GL_FLOAT_VEC4:
		return (strstr(uniformName, "_color") != nullptr) ? FBPropertyType::kFBPT_ColorRGBA : FBPropertyType::kFBPT_Vector4D;
	case GL_FLOAT_MAT4:
		return FBPropertyType::kFBPT_Vector4D; // TODO:
	case GL_SAMPLER_2D:
		return FBPropertyType::kFBPT_object; // reference to a texture object that we could bind to a property
	default:
		return FBPropertyType::kFBPT_double;
	}
}

/************************************************
 *  Constructor.
 ************************************************/
EffectShaderUserObject::EffectShaderUserObject(const char* pName, HIObject pObject)
	: FBUserObject(pName, pObject)
	, mText("")
{
    FBClassInit;

	mReloadShaders = false;
}

void EffectShaderUserObject::ActionReloadShaders(HIObject pObject, bool value)
{
	EffectShaderUserObject* p = FBCast<EffectShaderUserObject>(pObject);
	if (p && value)
	{
		p->DoReloadShaders();
	}
}

/************************************************
 *  FiLMBOX Constructor.
 ************************************************/
bool EffectShaderUserObject::FBCreate()
{
	// modify system behavoiur
	DisableObjectFlags(kFBFlagClonable);

	FBPropertyPublish(this, UniqueClassId, "UniqueClassId", nullptr, nullptr);
	FBPropertyPublish(this, Active, "Active", nullptr, nullptr);
	
	//FBPropertyPublish(this, RenderToTexture, "Render To Texture", nullptr, nullptr);
	FBPropertyPublish(this, OutputVideo, "Output Video", nullptr, nullptr);

	FBPropertyPublish(this, ShaderFile, "Shader File", nullptr, nullptr);
	FBPropertyPublish(this, ReloadShaders, "Reload Shader", nullptr, ActionReloadShaders);

	FBPropertyPublish(this, NumberOfPasses, "Number Of Passes", nullptr, nullptr);

	FBPropertyPublish(this, UseMasking, "Use Masking", nullptr, nullptr);
	FBPropertyPublish(this, MaskingChannel, "Masking Channel", nullptr, nullptr);

	// sibling buffer shaders
	//FBPropertyPublish(this, BufferA, "BufferA", nullptr, nullptr);
	//FBPropertyPublish(this, BufferB, "BufferB", nullptr, nullptr);
	//FBPropertyPublish(this, BufferC, "BufferC", nullptr, nullptr);
	//FBPropertyPublish(this, BufferD, "BufferD", nullptr, nullptr);

	ShaderFile = "//GLSL//test.glslf";
	NumberOfPasses = 1;
	NumberOfPasses.SetMinMax(1.0, 12.0, true, true);

	//
	UniqueClassId.ModifyPropertyFlag(kFBPropertyFlagHideProperty, true);
	UniqueClassId.ModifyPropertyFlag(kFBPropertyFlagNotSavable, true);
	UniqueClassId.ModifyPropertyFlag(kFBPropertyFlagReadOnly, true);
	UniqueClassId = 57;
	/*
	//
	BufferA.SetFilter(EffectShaderUserObject::GetInternalClassId());
	BufferA.SetSingleConnect(true);
	BufferB.SetFilter(EffectShaderUserObject::GetInternalClassId());
	BufferB.SetSingleConnect(true);
	BufferC.SetFilter(EffectShaderUserObject::GetInternalClassId());
	BufferC.SetSingleConnect(true);
	BufferD.SetFilter(EffectShaderUserObject::GetInternalClassId());
	BufferD.SetSingleConnect(true);
	*/
	// DONE: READ default values from config file !
	DefaultValues();

    return true;
}

/************************************************
 *  FiLMBOX Destructor.
 ************************************************/
void EffectShaderUserObject::FBDestroy()
{
	mUserShader.reset(nullptr);
}

bool EffectShaderUserObject::DoReloadShaders()
{
	// load a fragment shader from a given path and try to validate the shader and the program

	const char* fragment_shader_rpath = ShaderFile;
	if (!fragment_shader_rpath || strlen(fragment_shader_rpath) < 2)
	{
		LOGE("[PostEffectUserObject] Fragment shader relative path is not defined!\n");
		return false;
	}

	mUserShader.reset(new UserBufferShader(this));

	constexpr const char* vertex_shader_rpath = "/GLSL/simple.vsh";

	char vertex_abs_path_only[MAX_PATH];
	char fragment_abs_path_only[MAX_PATH];
	if (!FindEffectLocation(vertex_shader_rpath, vertex_abs_path_only, MAX_PATH)
		|| !FindEffectLocation(fragment_shader_rpath, fragment_abs_path_only, MAX_PATH))
	{
		LOGE("[PostEffectUserObject] Failed to find shaders location!\n");
		return false;
	}

	LOGI("[PostEffectUserObject] Vertex shader Location - %s\n", vertex_abs_path_only);
	LOGI("[PostEffectUserObject] Fragment shader Location - %s\n", fragment_abs_path_only);

	FBString vertex_path(vertex_abs_path_only, vertex_shader_rpath);
	FBString fragment_path(fragment_abs_path_only, fragment_shader_rpath);

	// NOTE: prep uniforms when load is succesfull
	if (!mUserShader->Load(0, vertex_path, fragment_path))
	{
		LOGE("[PostEffectUserObject] Failed to load shaders!\n");

		mUserShader.reset(nullptr);
		return false;
	}

	// reload connected input buffers
	if (GetUserShaderPtr())
	{
		for (auto& prop : GetUserShaderPtr()->mShaderProperties)
		{
			if (prop.second.shaderUserObject)
			{
				if (!prop.second.shaderUserObject->DoReloadShaders())
					return false;
			}
		}
	}
	
	return true;
}


void EffectShaderUserObject::DefaultValues()
{}


FBProperty* EffectShaderUserObject::MakePropertyFloat(const UserBufferShader::ShaderProperty& prop)
{
	FBProperty* newProp = nullptr;

	if (strstr(prop.uniformName, "_flag") != nullptr)
	{
		newProp = PropertyCreate(prop.uniformName, FBPropertyType::kFBPT_bool, ANIMATIONNODE_TYPE_BOOL, true, false, nullptr);
	}
	else
	{
		newProp = PropertyCreate(prop.uniformName, FBPropertyType::kFBPT_double, ANIMATIONNODE_TYPE_NUMBER, true, false, nullptr);

		if (strstr(prop.uniformName, "_slider") != nullptr)
		{
			newProp->SetMinMax(0.0, 100.0);
		}
	}

	PropertyAdd(newProp);
	return newProp;
}

FBProperty* EffectShaderUserObject::MakePropertyVec2(const UserBufferShader::ShaderProperty& prop)
{
	FBProperty* newProp = PropertyCreate(prop.uniformName, FBPropertyType::kFBPT_Vector2D, ANIMATIONNODE_TYPE_VECTOR, true, false, nullptr);
	PropertyAdd(newProp);
	return newProp;
}

FBProperty* EffectShaderUserObject::MakePropertyVec3(const UserBufferShader::ShaderProperty& prop)
{
	FBProperty* newProp = nullptr;

	if (strstr(prop.uniformName, "_color") != nullptr)
	{
		newProp = PropertyCreate(prop.uniformName, FBPropertyType::kFBPT_ColorRGB, ANIMATIONNODE_TYPE_COLOR, true, false, nullptr);
	}
	else
	{
		newProp = PropertyCreate(prop.uniformName, FBPropertyType::kFBPT_Vector3D, ANIMATIONNODE_TYPE_VECTOR, true, false, nullptr);
	}

	PropertyAdd(newProp);
	return newProp;
}

FBProperty* EffectShaderUserObject::MakePropertyVec4(const UserBufferShader::ShaderProperty& prop)
{
	FBProperty* newProp = nullptr;

	if (strstr(prop.uniformName, "_color") != nullptr)
	{
		newProp = PropertyCreate(prop.uniformName, FBPropertyType::kFBPT_ColorRGBA, ANIMATIONNODE_TYPE_COLOR_RGBA, true, false, nullptr);
	}
	else
	{
		newProp = PropertyCreate(prop.uniformName, FBPropertyType::kFBPT_Vector4D, ANIMATIONNODE_TYPE_VECTOR_4, true, false, nullptr);
	}

	PropertyAdd(newProp);
	return newProp;
}

FBProperty* EffectShaderUserObject::MakePropertySampler(const UserBufferShader::ShaderProperty& prop)
{
	FBProperty* newProp = PropertyCreate(prop.uniformName, FBPropertyType::kFBPT_object, ANIMATIONNODE_TYPE_OBJECT, false, false, nullptr);
	if (FBPropertyListObject* listObjProp = FBCast<FBPropertyListObject>(newProp))
	{
		listObjProp->SetFilter(FBTexture::GetInternalClassId());
		listObjProp->SetSingleConnect(true);

		PropertyAdd(newProp);
		return newProp;
	}
	return nullptr;
}

FBProperty* EffectShaderUserObject::GetOrMakeProperty(const UserBufferShader::ShaderProperty& prop)
{
	FBProperty* fbProperty = PropertyList.Find(prop.uniformName);
	const FBPropertyType fbPropertyType = UniformTypeToFBPropertyType(prop.type, prop.uniformName);

	// NOTE: check not only user property, but also a property type !
	if (fbProperty && fbProperty->IsUserProperty() && fbProperty->GetPropertyType() == fbPropertyType)
	{
		//return fbProperty;
		//prop.property = fbProperty;
	}
	else
	{
		// base on type, make a custom property
		fbProperty = nullptr;

		switch (prop.type)
		{
		case GL_FLOAT:
			fbProperty = MakePropertyFloat(prop);
			break;
		case GL_FLOAT_VEC2:
			fbProperty = MakePropertyVec2(prop);
			break;
		case GL_FLOAT_VEC3:
			fbProperty = MakePropertyVec3(prop);
			break;
		case GL_FLOAT_VEC4:
			fbProperty = MakePropertyVec4(prop);
			break;
		case GL_SAMPLER_2D:
			fbProperty = MakePropertySampler(prop);
			break;
		default:
			LOGE("[PostEffectUserObject] not supported prop type for %s uniform\n", prop.uniformName);
		}
	}
	return fbProperty;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UserBufferShader

UserBufferShader::UserBufferShader(EffectShaderUserObject* UserObject)
	: PostEffectBufferShader()
	, mUserObject(UserObject)
{
	for (int i = 0; i < static_cast<int>(ShaderSystemUniform::COUNT); ++i)
	{
		mSystemUniformLocations[i] = -1;
	}
}

bool UserBufferShader::IsDepthSamplerUsed() const 
{ 
	return mSystemUniformLocations[static_cast<int>(ShaderSystemUniform::INPUT_DEPTH_SAMPLER_2D)] >= 0; 
}
bool UserBufferShader::IsLinearDepthSamplerUsed() const
{ 
	return mSystemUniformLocations[static_cast<int>(ShaderSystemUniform::LINEAR_DEPTH_SAMPLER_2D)] >= 0; 
}

/// new feature to have several passes for a specified effect
const int UserBufferShader::GetNumberOfPasses() const
{
	return mUserObject->NumberOfPasses;
}
//! initialize a specific path for drawing
bool UserBufferShader::PrepPass(const int pass, int w, int h)
{
	// TODO: define iPass value for the shader
	
	return true;
}


void UserBufferShader::RemoveShaderProperties()
{
	for (auto& shaderProperty : mShaderProperties)
	{
		if (shaderProperty.second.property != nullptr)
		{
			mUserObject->PropertyRemove(shaderProperty.second.property);
			shaderProperty.second.property = nullptr;
		}
	}

	mShaderProperties.clear();
}


void UserBufferShader::ResetSystemUniformLocations()
{
	for (int i = 0; i < static_cast<int>(ShaderSystemUniform::COUNT); ++i)
	{
		mSystemUniformLocations[i] = -1;
	}
}

int UserBufferShader::IsSystemUniform(const char* uniformName)
{
	for (int i = 0; i < static_cast<int>(ShaderSystemUniform::COUNT); ++i)
	{
		if (strncmp(uniformName, gSystemUniformNames[i], 256) == 0)
			return i;
	}
	return -1;
}


void UserBufferShader::BindSystemUniforms(PostPersistentData* pData, const PostEffectContext& effectContext) const
{
	const GLuint programId = GetShaderPtr()->GetProgramObj();
	const GLint* sysLocations = mSystemUniformLocations;
	const bool useMasking = mUserObject->UseMasking;

	if (sysLocations[static_cast<int>(ShaderSystemUniform::INPUT_COLOR_SAMPLER_2D)] >= 0)
	{
		glProgramUniform1f(programId, sysLocations[static_cast<int>(ShaderSystemUniform::INPUT_COLOR_SAMPLER_2D)], 0);
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::iCHANNEL0)] >= 0)
	{
		glProgramUniform1f(programId, sysLocations[static_cast<int>(ShaderSystemUniform::iCHANNEL0)], 0);
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::BUFFERA_SAMPLER_2D)] >= 0)
	{
		glProgramUniform1i(programId, sysLocations[static_cast<int>(ShaderSystemUniform::BUFFERA_SAMPLER_2D)], PostEffectBufferShader::GetBufferSamplerId());
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::INPUT_DEPTH_SAMPLER_2D)] >= 0)
	{
		glProgramUniform1i(programId, sysLocations[static_cast<int>(ShaderSystemUniform::INPUT_DEPTH_SAMPLER_2D)], 1);
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::LINEAR_DEPTH_SAMPLER_2D)] >= 0)
	{
		glProgramUniform1i(programId, sysLocations[static_cast<int>(ShaderSystemUniform::LINEAR_DEPTH_SAMPLER_2D)], 2);
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::INPUT_MASK_SAMPLER_2D)] >= 0)
	{
		const GLint maskSlot = CommonEffectUniforms::GetMaskSamplerSlot();
		glProgramUniform1i(programId, sysLocations[static_cast<int>(ShaderSystemUniform::INPUT_MASK_SAMPLER_2D)], maskSlot);
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::USE_MASKING)] >= 0)
	{
		const GLint loc = sysLocations[static_cast<int>(ShaderSystemUniform::USE_MASKING)];
		glProgramUniform1f(programId, loc, (useMasking) ? 1.0f : 0.0f);
	}

	if (sysLocations[static_cast<int>(ShaderSystemUniform::UPPER_CLIP)] >= 0)
	{
		const GLint loc = sysLocations[static_cast<int>(ShaderSystemUniform::UPPER_CLIP)];
		const double value = pData->UpperClip;
		glProgramUniform1f(programId, loc, 0.01f * static_cast<float>(value));
	}

	if (sysLocations[static_cast<int>(ShaderSystemUniform::LOWER_CLIP)] >= 0)
	{
		const GLint loc = sysLocations[static_cast<int>(ShaderSystemUniform::LOWER_CLIP)];
		const double value = pData->LowerClip;
		glProgramUniform1f(programId, loc, 0.01f * static_cast<float>(value));
	}

	if (sysLocations[static_cast<int>(ShaderSystemUniform::RESOLUTION)] >= 0)
	{
		const GLint loc = sysLocations[static_cast<int>(ShaderSystemUniform::RESOLUTION)];
		glProgramUniform2f(programId, loc, static_cast<float>(effectContext.w), static_cast<float>(effectContext.h));
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::iRESOLUTION)] >= 0)
	{
		const GLint loc = sysLocations[static_cast<int>(ShaderSystemUniform::iRESOLUTION)];
		glProgramUniform2f(programId, loc, static_cast<float>(effectContext.w), static_cast<float>(effectContext.h));
	}

	if (sysLocations[static_cast<int>(ShaderSystemUniform::iTIME)] >= 0)
	{
		const GLint loc = sysLocations[static_cast<int>(ShaderSystemUniform::iTIME)];
		glProgramUniform1f(programId, loc, static_cast<float>(effectContext.sysTime));
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

	if (sysLocations[static_cast<int>(ShaderSystemUniform::MODELVIEW)] >= 0)
	{
		const GLint loc = sysLocations[static_cast<int>(ShaderSystemUniform::MODELVIEW)];

		FBMatrix tm;
		effectContext.camera->GetCameraMatrix(tm, kFBModelView);
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
		FBMatrix tm;
		effectContext.camera->GetCameraMatrix(tm, kFBProjection);

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
		FBMatrix tm;
		effectContext.camera->GetCameraMatrix(tm, kFBModelViewProj);

		float fMatrix[16];
		for (int i = 0; i < 16; ++i)
		{
			fMatrix[i] = static_cast<float>(tm[i]);
		}
		glProgramUniformMatrix4fv(programId, loc, 1, GL_FALSE, fMatrix);
	}
}

bool UserBufferShader::PrepUniforms(const int variationIndex)
{
	RemoveShaderProperties();
	ResetSystemUniformLocations();

	if (!GetShaderPtr())
		return false;

	const GLuint programId = GetShaderPtr()->GetProgramObj();

	GLint numUniforms = 0;
	glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &numUniforms);


	for (GLint i = 0; i < numUniforms; ++i)
	{
		ShaderProperty prop;

		glGetActiveUniform(programId, i, sizeof(prop.uniformName), &prop.length, &prop.size, &prop.type, prop.uniformName);
		prop.location = glGetUniformLocation(programId, prop.uniformName);

		// TODO: skip system properties, but track down the locations
		const int systemUniformId = IsSystemUniform(prop.uniformName);
		if (systemUniformId >= 0)
		{
			mSystemUniformLocations[systemUniformId] = prop.location;
			continue;
		}

		prop.property = mUserObject->GetOrMakeProperty(prop);

		if (prop.property != nullptr)
		{
			mShaderProperties.emplace(prop.uniformName, prop);
		}
	}

	return true;
}

//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
bool UserBufferShader::CollectUIValues(PostPersistentData* pData, const PostEffectContext& effectContext, int maskIndex)
{
	CollectCommonData(pData, nullptr);

	GLSLShaderProgram* shader = GetShaderPtr();
	FBVector4d v;

	BindSystemUniforms(pData, effectContext);

	// setup user uniforms

	GLint userTextureSlot = PostEffectBufferShader::GetUserSamplerId(); //!< start index to bind user textures

	for (auto& prop : mShaderProperties)
	{
		if (prop.second.type == GL_FLOAT)
		{
			const GLint loc = glGetUniformLocation(shader->GetProgramObj(), prop.first.c_str());
			prop.second.location = loc;

			if (strstr(prop.first.c_str(), "_flag") != nullptr)
			{
				bool bvalue = false;
				prop.second.property->GetData(&bvalue, sizeof(bool));	
				prop.second.value[0] = bvalue ? 1.0f : 0.0f;
			}
			else
			{
				prop.second.property->GetData(v, sizeof(double));
				prop.second.value[0] = static_cast<float>(v[0]);
			}
		}
		else if (prop.second.type == GL_FLOAT_VEC2)
		{
			prop.second.property->GetData(v, sizeof(double) * 2);

			const GLint loc = glGetUniformLocation(shader->GetProgramObj(), prop.first.c_str());
			prop.second.location = loc;
			prop.second.value[0] = static_cast<float>(v[0]);
			prop.second.value[1] = static_cast<float>(v[1]);
		}
		else if (prop.second.type == GL_FLOAT_VEC3)
		{
			prop.second.property->GetData(v, sizeof(double) * 3);

			const GLint loc = glGetUniformLocation(shader->GetProgramObj(), prop.first.c_str());
			prop.second.location = loc;
			prop.second.value[0] = static_cast<float>(v[0]);
			prop.second.value[1] = static_cast<float>(v[1]);
			prop.second.value[2] = static_cast<float>(v[2]);
		}
		else if (prop.second.type == GL_FLOAT_VEC4)
		{
			prop.second.property->GetData(v, sizeof(double) * 4);

			const GLint loc = glGetUniformLocation(shader->GetProgramObj(), prop.first.c_str());
			prop.second.location = loc;
			prop.second.value[0] = static_cast<float>(v[0]);
			prop.second.value[1] = static_cast<float>(v[1]);
			prop.second.value[2] = static_cast<float>(v[2]);
			prop.second.value[3] = static_cast<float>(v[3]);
		}
		else if (prop.second.type == GL_SAMPLER_2D)
		{
			prop.second.texture = nullptr;
			prop.second.shaderUserObject = nullptr;

			if (FBPropertyListObject* listObjProp = FBCast<FBPropertyListObject>(prop.second.property))
			{
				if (listObjProp->GetCount() > 0)
				{
					if (FBIS(listObjProp->GetAt(0), FBTexture))
					{
						FBTexture* texture = FBCast<FBTexture>(listObjProp->GetAt(0));
						assert(texture != nullptr);
					
						prop.second.texture = texture;
					}
					else if (FBIS(listObjProp->GetAt(0), EffectShaderUserObject))
					{
						EffectShaderUserObject* shaderObject = FBCast<EffectShaderUserObject>(listObjProp->GetAt(0));
						assert(shaderObject != nullptr);

						prop.second.shaderUserObject = shaderObject;
						shaderObject->GetUserShaderPtr()->CollectUIValues(pData, effectContext, maskIndex);
					}
				}
			}
		}
	}

	return true;
}

void UserBufferShader::UploadUniforms(PostEffectBuffers* buffers, FrameBuffer* dstBuffer, int colorAttachment, const GLuint inputTextureId, int w, int h, bool generateMips)
{
	UploadCommonData();

	GLint userTextureSlot = PostEffectBufferShader::GetUserSamplerId(); //!< start index to bind user textures
	GLSLShaderProgram* shader = GetShaderPtr();

	for (auto& prop : mShaderProperties)
	{
		if (prop.second.type == GL_FLOAT)
		{
			glUniform1f(prop.second.location, prop.second.value[0]);
		}
		else if (prop.second.type == GL_FLOAT_VEC2)
		{
			glUniform2fv(prop.second.location, 1, prop.second.value);
		}
		else if (prop.second.type == GL_FLOAT_VEC3)
		{
			glUniform3fv(prop.second.location, 1, prop.second.value);
		}
		else if (prop.second.type == GL_FLOAT_VEC4)
		{
			glUniform4fv(prop.second.location, 1, prop.second.value);
		}
		else if (FBTexture* texture = prop.second.texture)
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
		else if (prop.second.shaderUserObject)
		{
			UserBufferShader* bufferShader = prop.second.shaderUserObject->GetUserShaderPtr();

			// bind sampler from another rendered buffer shader
			const std::string bufferName = std::string(GetName()) + "_" + std::string(prop.second.shaderUserObject->Name);

			// dst should be buffer textures ?!

			FrameBuffer* buffer = buffers->RequestFramebuffer(bufferName);

			bufferShader->Render(buffers, buffer, 0, inputTextureId, w, h, generateMips);

			const GLuint bufferTextureId = buffer->GetColorObject();
			buffers->ReleaseFramebuffer(bufferName);

			// TODO: bind input buffers
			glActiveTexture(GL_TEXTURE0 + userTextureSlot);
			glBindTexture(GL_TEXTURE_2D, bufferTextureId);
			glActiveTexture(GL_TEXTURE0);

			userTextureSlot += 1;
		}
	}
}
/*
void UserBufferShader::OnPreRender(PostEffectBuffers* buffers, FrameBuffer* dstBuffer, int colorAttachment, const GLuint inputTextureId, int w, int h, bool generateMips)
{
	GLint userTextureSlot = PostEffectBufferShader::GetUserSamplerId(); //!< start index to bind user textures
	GLSLShaderProgram* shader = GetShaderPtr();

	for (auto& prop : mShaderProperties)
	{
		if (prop.second.type == GL_FLOAT)
		{
			glUniform1f(prop.second.location, prop.second.value[0]);
		}
		else if (prop.second.type == GL_FLOAT_VEC2)
		{
			glUniform2fv(prop.second.location, 1, prop.second.value);
		}
		else if (prop.second.type == GL_FLOAT_VEC3)
		{
			glUniform3fv(prop.second.location, 1, prop.second.value);
		}
		else if (prop.second.type == GL_FLOAT_VEC4)
		{
			glUniform4fv(prop.second.location, 1, prop.second.value);
		}
		else if (FBTexture* texture = prop.second.texture)
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
		else if (prop.second.shaderUserObject)
		{
			UserBufferShader* bufferShader = prop.second.shaderUserObject->GetUserShaderPtr();

			// bind sampler from another rendered buffer shader
			const std::string bufferName = std::string(GetName()) + "_" + std::string(prop.second.shaderUserObject->Name);

			// dst should be buffer textures ?!

			FrameBuffer* buffer = buffers->RequestFramebuffer(bufferName);

			bufferShader->Render(buffers, buffer, 0, inputTextureId, w, h, generateMips);

			const GLuint bufferTextureId = buffer->GetColorObject();
			buffers->ReleaseFramebuffer(bufferName);

			// TODO: bind input buffers
			glActiveTexture(GL_TEXTURE0 + userTextureSlot);
			glBindTexture(GL_TEXTURE_2D, bufferTextureId);
			glActiveTexture(GL_TEXTURE0);

			userTextureSlot += 1;
		}
	}
}*/