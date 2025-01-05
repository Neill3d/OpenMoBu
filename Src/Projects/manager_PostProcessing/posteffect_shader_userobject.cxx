
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



//--- FiLMBOX Registration & Implementation.
FBClassImplementation(EffectShaderUserObject);
FBUserObjectImplement(EffectShaderUserObject,
                        "User Object used to store a persistance data for one effect shader.",
						"cam_switcher_toggle.png");                                          //Register UserObject class
PostEffectFBElementClassImplementation(EffectShaderUserObject, "Effect Shader", "cam_switcher_toggle.png");                  //Register to the asset system

////////////////////////////////////////////////////////////////////////////////
//



const char* FBPropertyBaseEnum<EEffectResolution>::mStrings[] = {
	"Original",
	"Downscale 1/2",
	"Downscale 1/4",
	0
};

// use uniformName to track down some type casts
FBPropertyType ShaderPropertyToFBPropertyType(const IEffectShaderConnections::ShaderProperty& prop)
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

PostEffectBufferShader* EffectShaderUserObject::MakeANewClassInstance()
{
	return new UserBufferShader(this);
}

/************************************************
 *  FiLMBOX Constructor.
 ************************************************/
bool EffectShaderUserObject::FBCreate()
{
	mUserShader.reset(MakeANewClassInstance());

	// modify system behavoiur
	DisableObjectFlags(kFBFlagClonable);

	FBPropertyPublish(this, UniqueClassId, "UniqueClassId", nullptr, nullptr);
	FBPropertyPublish(this, Active, "Active", nullptr, nullptr);
	
	FBPropertyPublish(this, Resolution, "Resolution", nullptr, nullptr);

	//FBPropertyPublish(this, RenderToTexture, "Render To Texture", nullptr, nullptr);
	FBPropertyPublish(this, OutputVideo, "Output Video", nullptr, nullptr);

	FBPropertyPublish(this, ShaderFile, "Shader File", nullptr, nullptr);
	FBPropertyPublish(this, ReloadShaders, "Reload Shader", nullptr, ActionReloadShaders);

	FBPropertyPublish(this, NumberOfPasses, "Number Of Passes", nullptr, nullptr);

	FBPropertyPublish(this, UseMasking, "Use Masking", nullptr, nullptr);
	FBPropertyPublish(this, MaskingChannel, "Masking Channel", nullptr, nullptr);

	Resolution = eEffectResolutionOriginal;

	ShaderFile = "//GLSL//test.glslf";
	NumberOfPasses = 1;
	NumberOfPasses.SetMinMax(1.0, 12.0, true, true);

	//
	UniqueClassId.ModifyPropertyFlag(kFBPropertyFlagHideProperty, true);
	UniqueClassId.ModifyPropertyFlag(kFBPropertyFlagNotSavable, true);
	UniqueClassId.ModifyPropertyFlag(kFBPropertyFlagReadOnly, true);
	UniqueClassId = 57;
	
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

	constexpr const char* vertex_shader_rpath = "/GLSL/simple130.glslv";

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

		//mUserShader.reset(nullptr);
		return false;
	}
	
	// reload connected input buffers
	if (UniqueClassId == 57)
	{
		if (UserBufferShader* bufferShader = static_cast<UserBufferShader*>(GetUserShaderPtr()))
		{
			for (auto& prop : bufferShader->mProperties)
			{
				if (prop.second.shaderUserObject)
				{
					if (!prop.second.shaderUserObject->DoReloadShaders())
						return false;
				}
			}
		}
	}
	
	return true;
}


void EffectShaderUserObject::DefaultValues()
{}

FBProperty* EffectShaderUserObject::MakePropertyInt(const UserBufferShader::ShaderProperty& prop)
{
	FBProperty* newProp = PropertyCreate(prop.uniformName, FBPropertyType::kFBPT_int, ANIMATIONNODE_TYPE_INTEGER, false, false, nullptr);
	PropertyAdd(newProp);
	return newProp;
}

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
	FBProperty* newProp = nullptr;
	
	if (strstr(prop.uniformName, "_wstoss") != nullptr)
	{
		// a property for world position that is going to be converted into screen space position
		newProp = PropertyCreate(prop.uniformName, FBPropertyType::kFBPT_Vector3D, ANIMATIONNODE_TYPE_VECTOR, true, false, nullptr);
	}
	else
	{
		newProp = PropertyCreate(prop.uniformName, FBPropertyType::kFBPT_Vector2D, ANIMATIONNODE_TYPE_VECTOR, true, false, nullptr);
	}
	
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
		//listObjProp->SetFilter(FBTexture::GetInternalClassId() | EffectShaderUserObject::GetInternalClassId());
		listObjProp->SetSingleConnect(true);

		PropertyAdd(newProp);
		return newProp;
	}
	return nullptr;
}

FBProperty* EffectShaderUserObject::GetOrMakeProperty(const UserBufferShader::ShaderProperty& prop)
{
	FBProperty* fbProperty = PropertyList.Find(prop.uniformName);
	const FBPropertyType fbPropertyType = ShaderPropertyToFBPropertyType(prop);

	// NOTE: check not only user property, but also a property type !
	if (!fbProperty || fbProperty->GetPropertyType() != fbPropertyType)
	{
		// base on type, make a custom property
		fbProperty = nullptr;

		switch (prop.type)
		{
		case IEffectShaderConnections::EPropertyType::INT:
			fbProperty = MakePropertyInt(prop);
			break;
		case IEffectShaderConnections::EPropertyType::FLOAT:
			fbProperty = MakePropertyFloat(prop);
			break;
		case IEffectShaderConnections::EPropertyType::VEC2:
			fbProperty = MakePropertyVec2(prop);
			break;
		case IEffectShaderConnections::EPropertyType::VEC3:
			fbProperty = MakePropertyVec3(prop);
			break;
		case IEffectShaderConnections::EPropertyType::VEC4:
			fbProperty = MakePropertyVec4(prop);
			break;
		case IEffectShaderConnections::EPropertyType::TEXTURE:
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
	for (auto& shaderProperty : mProperties)
	{
		if (shaderProperty.second.fbProperty != nullptr)
		{
			mUserObject->PropertyRemove(shaderProperty.second.fbProperty);
			shaderProperty.second.fbProperty = nullptr;
		}
	}

	mProperties.clear();
}

bool UserBufferShader::OnPrepareUniforms(const int variationIndex)
{
	if (!GetShaderPtr())
		return false;

	PopulatePropertiesFromShaderUniforms();
	return true;
}

void UserBufferShader::OnPropertyAdded(ShaderProperty& prop)
{
	prop.fbProperty = mUserObject->GetOrMakeProperty(prop);
}

//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
bool UserBufferShader::OnCollectUI(const IPostEffectContext* effectContext, int maskIndex)
{
	GLSLShaderProgram* shader = GetShaderPtr();
	if (shader == nullptr)
		return false;

	FBVector4d v;

	BindSystemUniforms(pData, effectContext);

	// setup user uniforms
	const GLuint programId = shader->GetProgramObj();
	GLint userTextureSlot = PostEffectBufferShader::GetUserSamplerId(); //!< start index to bind user textures

	for (auto& prop : mProperties)
	{
		switch (prop.second.type)
		{
		case IEffectShaderConnections::EPropertyType::INT:
		{
			int ivalue = 0;
			prop.second.fbProperty->GetData(&ivalue, sizeof(int));
			prop.second.GetFloatData()[0] = static_cast<float>(ivalue);
		} break;

		case IEffectShaderConnections::EPropertyType::FLOAT:
		{
			if (prop.second.HasFlag(IEffectShaderConnections::PropertyFlag::IsFlag))
			{
				bool bvalue = false;
				prop.second.fbProperty->GetData(&bvalue, sizeof(bool));
				prop.second.GetFloatData()[0] = bvalue ? 1.0f : 0.0f;
			}
			else
			{
				prop.second.fbProperty->GetData(v, sizeof(double));
				prop.second.GetFloatData()[0] = static_cast<float>(v[0]);
			}
		} break;

		case IEffectShaderConnections::EPropertyType::VEC2:
		{
			if (prop.second.HasFlag(IEffectShaderConnections::PropertyFlag::ConvertWorldToScreenSpace))
			{
				// world space to screen space
				prop.second.fbProperty->GetData(v, sizeof(double) * 3);

				FBMatrix mvp(effectContext->GetModelViewProjMatrix());

				FBVector4d v4;
				FBVectorMatrixMult(v4, mvp, FBVector4d(v[0], v[1], v[2], 1.0));

				v4[0] = effectContext->GetViewWidth() * 0.5 * (v4[0] + 1.0);
				v4[1] = effectContext->GetViewHeight() * 0.5 * (v4[1] + 1.0);

				float* value = prop.second.GetFloatData();
				value[0] = static_cast<float>(v4[0]) / static_cast<float>(effectContext->GetViewWidth());
				value[1] = static_cast<float>(v4[1]) / static_cast<float>(effectContext->GetViewHeight());
				//value[2] = static_cast<float>(v4[2]);
			}
			else
			{
				prop.second.fbProperty->GetData(v, sizeof(double) * 2);
				float* value = prop.second.GetFloatData();
				value[0] = static_cast<float>(v[0]);
				value[1] = static_cast<float>(v[1]);
			}
		} break;

		case IEffectShaderConnections::EPropertyType::VEC3:
		{
			prop.second.fbProperty->GetData(v, sizeof(double) * 3);

			float* value = prop.second.GetFloatData();
			value[0] = static_cast<float>(v[0]);
			value[1] = static_cast<float>(v[1]);
			value[2] = static_cast<float>(v[2]);

		} break;

		case IEffectShaderConnections::EPropertyType::VEC4:
		{
			prop.second.fbProperty->GetData(v, sizeof(double) * 4);

			float* value = prop.second.GetFloatData();
			value[0] = static_cast<float>(v[0]);
			value[1] = static_cast<float>(v[1]);
			value[2] = static_cast<float>(v[2]);
			value[3] = static_cast<float>(v[3]);
		} break;

		case IEffectShaderConnections::EPropertyType::TEXTURE:
			prop.second.texture = nullptr;
			prop.second.shaderUserObject = nullptr;

			if (FBPropertyListObject* listObjProp = FBCast<FBPropertyListObject>(prop.second.fbProperty))
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
						shaderObject->GetUserShaderPtr()->CollectUIValues(effectContext, maskIndex);
					}
				}
			}
			break;
		}
	}

	return true;
}

void UserBufferShader::OnUploadUniforms(PostEffectBuffers* buffers, FrameBuffer* dstBuffer, int colorAttachment, const GLuint inputTextureId, int w, int h, bool generateMips)
{
	
}

std::vector<PostEffectBufferShader*> UserBufferShader::GetConnectedShaders()
{
	std::vector<PostEffectBufferShader*> result;
	for (auto& prop : mProperties)
	{
		if (prop.second.shaderUserObject && prop.second.shaderUserObject->GetUserShaderPtr())
		{
			result.push_back(prop.second.shaderUserObject->GetUserShaderPtr());
		}
	}
	return result;
}

void EffectShaderUserObject::RecalculateWidthAndHeight(int& w, int& h)
{
	switch (Resolution)
	{
	case eEffectResolutionDownscale2x:
		w = w / 2;
		h = h / 2;
		break;
	case eEffectResolutionDownscale4x:
		w = w / 4;
		h = h / 4;
		break;
	}
}