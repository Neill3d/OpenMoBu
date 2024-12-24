
/** \file posteffect_userobject.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

// Class declaration
#include "posteffect_userobject.h"
#include "postprocessing_ini.h"
#include <vector>
#include <limits>

#include "FileUtils.h"
#include "mobu_logging.h"

#include "postpersistentdata.h"

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
FBClassImplementation(PostEffectUserObject);
FBUserObjectImplement(PostEffectUserObject,
                        "Use Object used to store a persistance data for the post effect.",
						"cam_switcher_toggle.png");                                          //Register UserObject class
PostEffectFBElementClassImplementation(PostEffectUserObject, "Post Effect", "cam_switcher_toggle.png");                  //Register to the asset system

////////////////////////////////////////////////////////////////////////////////
//

const char* PostEffectUserObject::gSystemUniformNames[static_cast<int>(ShaderSystemUniform::COUNT)] =
{
	"inputSampler", //!< this is an input image that we read from
	"depthSampler", //!< this is a scene depth texture sampler in case shader will need it for processing
	"maskSampler", //!< binded mask for a shader processing

	"useMasking", //!< float uniform [0; 1] to define if the mask have to be used
	"upperClip", //!< this is an upper clip image level. defined in a texture coord space to skip processing
	"lowerClip", //!< this is a lower clip image level. defined in a texture coord space to skip processing

	"gResolution", //!< vec2 that contains processing absolute resolution, like 1920x1080
};

/************************************************
 *  Constructor.
 ************************************************/
PostEffectUserObject::PostEffectUserObject(const char* pName, HIObject pObject)
	: FBUserObject(pName, pObject)
	, mText("")
{
    FBClassInit;

	mReloadShaders = false;
}

void PostEffectUserObject::ActionReloadShaders(HIObject pObject, bool value)
{
	PostEffectUserObject* p = FBCast<PostEffectUserObject>(pObject);
	if (p && value)
	{
		p->DoReloadShaders();
	}
}

/************************************************
 *  FiLMBOX Constructor.
 ************************************************/
bool PostEffectUserObject::FBCreate()
{
	// modify system behavoiur
	DisableObjectFlags(kFBFlagClonable);

	FBPropertyPublish(this, UniqueClassId, "UniqueClassId", nullptr, nullptr);
	FBPropertyPublish(this, Active, "Active", nullptr, nullptr);
	
	FBPropertyPublish(this, ShaderFile, "Shader File", nullptr, nullptr);
	FBPropertyPublish(this, ReloadShaders, "Reload Shader", nullptr, ActionReloadShaders);

	
	FBPropertyPublish(this, UseMasking, "Use Masking", nullptr, nullptr);
	FBPropertyPublish(this, MaskingChannel, "Masking Channel", nullptr, nullptr);

	ShaderFile = "//GLSL//test.glslf";

	//
	UniqueClassId.ModifyPropertyFlag(kFBPropertyFlagHideProperty, true);
	UniqueClassId.ModifyPropertyFlag(kFBPropertyFlagNotSavable, true);
	UniqueClassId.ModifyPropertyFlag(kFBPropertyFlagReadOnly, true);
	UniqueClassId = 57;

	// DONE: READ default values from config file !
	DefaultValues();

	mUserEffect = nullptr;

    return true;
}

/************************************************
 *  FiLMBOX Destructor.
 ************************************************/
void PostEffectUserObject::FBDestroy()
{
	if (mUserEffect)
	{
		delete mUserEffect;
		mUserEffect = nullptr;
	}
}

void PostEffectUserObject::RemoveShaderProperties()
{
	for (auto& shaderProperty : mShaderProperties)
	{
		if (shaderProperty.second.property != nullptr)
		{
			PropertyRemove(shaderProperty.second.property);
			shaderProperty.second.property = nullptr;
		}
	}

	mShaderProperties.clear();
}

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

FBProperty* PostEffectUserObject::MakePropertyFloat(const ShaderProperty& prop)
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

FBProperty* PostEffectUserObject::MakePropertyVec2(const ShaderProperty& prop)
{
	FBProperty* newProp = PropertyCreate(prop.uniformName, FBPropertyType::kFBPT_Vector2D, ANIMATIONNODE_TYPE_VECTOR, true, false, nullptr);
	PropertyAdd(newProp);
	return newProp;
}

FBProperty* PostEffectUserObject::MakePropertyVec3(const ShaderProperty& prop)
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

FBProperty* PostEffectUserObject::MakePropertyVec4(const ShaderProperty& prop)
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

FBProperty* PostEffectUserObject::MakePropertySampler(const ShaderProperty& prop)
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

void PostEffectUserObject::ResetSystemUniformLocations()
{
	for (int i = 0; i < static_cast<int>(ShaderSystemUniform::COUNT); ++i)
	{
		mSystemUniformLocations[i] = -1;
	}
}

int PostEffectUserObject::IsSystemUniform(const char* uniformName)
{
	for (int i = 0; i < static_cast<int>(ShaderSystemUniform::COUNT); ++i)
	{
		if (strncmp(uniformName, gSystemUniformNames[i], 256) == 0)
			return i;
	}
	return -1;
}

void PostEffectUserObject::CheckUniforms()
{
	RemoveShaderProperties();
	ResetSystemUniformLocations();

	if (!mUserEffect || !mUserEffect->GetShaderPtr())
		return;

	const GLuint programId = mUserEffect->GetShaderPtr()->GetProgramObj();

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
		
		const FBPropertyType fbPropertyType = UniformTypeToFBPropertyType(prop.type, prop.uniformName);

		FBProperty* fbProperty = PropertyList.Find(prop.uniformName);
		// NOTE: check not only user property, but also a property type !
		if (fbProperty && fbProperty->IsUserProperty() && fbProperty->GetPropertyType() == fbPropertyType)
		{
			prop.property = fbProperty;
		}
		else
		{
			// base on type, make a custom property
			prop.property = nullptr;

			switch (prop.type)
			{
			case GL_FLOAT:
				prop.property = MakePropertyFloat(prop);
				break;
			case GL_FLOAT_VEC2:
				prop.property = MakePropertyVec2(prop);
				break;
			case GL_FLOAT_VEC3:
				prop.property = MakePropertyVec3(prop);
				break;
			case GL_FLOAT_VEC4:
				prop.property = MakePropertyVec4(prop);
				break;
			case GL_SAMPLER_2D:
				prop.property = MakePropertySampler(prop);
				break;
			default:
				LOGE("[PostEffectUserObject] not supported prop type for %s uniform\n", prop.uniformName);
			}
		}

		if (prop.property != nullptr)
		{
			mShaderProperties.emplace(prop.uniformName, prop);
		}
	}
}

void PostEffectUserObject::DoReloadShaders()
{
	// load a fragment shader from a given path and try to validate the shader and the program

	const char* fragment_shader_rpath = ShaderFile;
	if (!fragment_shader_rpath || strlen(fragment_shader_rpath) < 2)
	{
		LOGE("[PostEffectUserObject] Fragment shader relative path is not defined!\n");
		return;
	}

	if (mUserEffect)
	{
		delete mUserEffect;
		mUserEffect = nullptr;
	}

	mUserEffect = new PostUserEffect(this);

	constexpr const char* vertex_shader_rpath = "\\GLSL\\simple.vsh";

	char vertex_abs_path_only[MAX_PATH];
	char fragment_abs_path_only[MAX_PATH];
	if (!FindEffectLocation(vertex_shader_rpath, vertex_abs_path_only, MAX_PATH)
		|| !FindEffectLocation(fragment_shader_rpath, fragment_abs_path_only, MAX_PATH))
	{
		LOGE("[PostEffectUserObject] Failed to find shaders location!\n");
		return;
	}

	LOGI("[PostEffectUserObject] Vertex shader Location - %s\n", vertex_abs_path_only);
	LOGI("[PostEffectUserObject] Fragment shader Location - %s\n", fragment_abs_path_only);

	FBString vertex_path(vertex_abs_path_only, vertex_shader_rpath);
	FBString fragment_path(fragment_abs_path_only, fragment_shader_rpath);

	if (!mUserEffect->Load(0, vertex_path, fragment_path))
	{
		LOGE("[PostEffectUserObject] Failed to load shaders!\n");
		delete mUserEffect;
		mUserEffect = nullptr;
		return;
	}

	//mShaderProgram->Bind();

	CheckUniforms();

	// TODO: setup default uniforms !
	// TODO: extract uniforms to prepare a UI layout

	//mShaderProgram->UnBind();

}


void PostEffectUserObject::DefaultValues()
{

}


//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
bool PostUserEffect::CollectUIValues(PostPersistentData* pData, PostEffectContext& effectContext)
{
	GLSLShaderProgram* shader = GetShaderPtr();

	FBVector4d v;

	shader->Bind();

	// TODO: setup system uniforms
	// TODO:
	//
	// 
	const GLint* sysLocations = mUserObject->mSystemUniformLocations;

	if (sysLocations[static_cast<int>(ShaderSystemUniform::INPUT_COLOR_SAMPLER_2D)] >= 0)
	{
		glUniform1i(sysLocations[static_cast<int>(ShaderSystemUniform::INPUT_COLOR_SAMPLER_2D)], 0);
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::INPUT_DEPTH_SAMPLER_2D)] >= 0)
	{
		glUniform1i(sysLocations[static_cast<int>(ShaderSystemUniform::INPUT_DEPTH_SAMPLER_2D)], 2);
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::INPUT_MASK_SAMPLER_2D)] >= 0)
	{
		const GLint maskSlot = CommonEffectUniforms::GetMaskSamplerSlot();
		glUniform1i(sysLocations[static_cast<int>(ShaderSystemUniform::INPUT_MASK_SAMPLER_2D)], maskSlot);
	}
	if (sysLocations[static_cast<int>(ShaderSystemUniform::USE_MASKING)] >= 0)
	{
		const GLint loc = sysLocations[static_cast<int>(ShaderSystemUniform::USE_MASKING)];
		glUniform1f(loc, (mUserObject->UseMasking) ? 1.0f : 0.0f);
	}

	if (sysLocations[static_cast<int>(ShaderSystemUniform::UPPER_CLIP)] >= 0)
	{
		const GLint loc = sysLocations[static_cast<int>(ShaderSystemUniform::UPPER_CLIP)];
		const double value = pData->UpperClip;
		glUniform1f(loc, 0.01f * static_cast<float>(value));
	}

	if (sysLocations[static_cast<int>(ShaderSystemUniform::LOWER_CLIP)] >= 0)
	{
		const GLint loc = sysLocations[static_cast<int>(ShaderSystemUniform::LOWER_CLIP)];
		const double value = pData->LowerClip;
		glUniform1f(loc, 0.01f * static_cast<float>(value));
	}
	
	if (sysLocations[static_cast<int>(ShaderSystemUniform::RESOLUTION)] >= 0)
	{
		const GLint loc = sysLocations[static_cast<int>(ShaderSystemUniform::RESOLUTION)];
		glUniform2f(loc, static_cast<float>(effectContext.w), static_cast<float>(effectContext.h));
	}

	// setup user uniforms

	GLint userTextureSlot = 4; //!< start index to bind user textures

	for (auto& prop : mUserObject->mShaderProperties)
	{
		if (prop.second.type == GL_FLOAT)
		{
			prop.second.property->GetData(v, sizeof(double));

			const GLint loc = glGetUniformLocation(shader->GetProgramObj(), prop.first.c_str());
			glUniform1f(loc, static_cast<float>(v[0]));
		}
		else if (prop.second.type == GL_FLOAT_VEC2)
		{
			prop.second.property->GetData(v, sizeof(double) * 2);

			const GLint loc = glGetUniformLocation(shader->GetProgramObj(), prop.first.c_str());
			const float fValues[2] = { static_cast<float>(v[0]), static_cast<float>(v[1]) };
			glUniform2fv(loc, 1, fValues);
		}
		else if (prop.second.type == GL_FLOAT_VEC3)
		{
			prop.second.property->GetData(v, sizeof(double) * 3);

			const GLint loc = glGetUniformLocation(shader->GetProgramObj(), prop.first.c_str());
			const float fValues[3] = { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]) };
			glUniform3fv(loc, 1, fValues);
		}
		else if (prop.second.type == GL_FLOAT_VEC4)
		{
			prop.second.property->GetData(v, sizeof(double) * 4);

			const GLint loc = glGetUniformLocation(shader->GetProgramObj(), prop.first.c_str());
			const float fValues[4] = { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), static_cast<float>(v[3]) };
			glUniform4fv(loc, 1, fValues);
		}
		else if (prop.second.type == GL_SAMPLER_2D)
		{
			if (FBPropertyListObject* listObjProp = FBCast<FBPropertyListObject>(prop.second.property))
			{
				FBTexture* texture = (listObjProp->GetCount() > 0 && FBIS(listObjProp->GetAt(0), FBTexture)) ? FBCast<FBTexture>(listObjProp->GetAt(0)) : nullptr;

				if (texture)
				{
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
					}
				}
			}
		}
	}

	shader->UnBind();

	return true;
}