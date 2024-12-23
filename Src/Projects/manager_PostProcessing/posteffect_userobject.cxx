
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

void PostEffectUserObject::CheckUniforms()
{
	RemoveShaderProperties();

	if (!mUserEffect || !mUserEffect->GetShaderPtr())
		return;

	const GLuint programId = mUserEffect->GetShaderPtr()->GetProgramObj();

	GLint numUniforms = 0;
	glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &numUniforms);


	for (GLint i = 0; i < numUniforms; ++i)
	{
		ShaderProperty prop;
		prop.property = nullptr;

		glGetActiveUniform(programId, i, sizeof(prop.uniformName), &prop.length, &prop.size, &prop.type, prop.uniformName);
		
		// base on type, make a custom property
		if (prop.type == GL_FLOAT)
		{
			FBProperty* fbProperty = PropertyList.Find(prop.uniformName);
			// NOTE: check not only user property, but also a property type !
			if (fbProperty && fbProperty->IsUserProperty() && fbProperty->GetPropertyType() == FBPropertyType::kFBPT_double )
			{
				prop.property = fbProperty;
			}
			else
			{
				prop.property = PropertyCreate(prop.uniformName, FBPropertyType::kFBPT_double, ANIMATIONNODE_TYPE_NUMBER, true, false, nullptr);
				PropertyAdd(prop.property);

				if (strstr(prop.uniformName, "_slider") != nullptr)
				{
					prop.property->SetMinMax(0.0, 100.0);
				}
			}
		}

		mShaderProperties.emplace(prop.uniformName, prop);
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

	shader->Bind();

	for (auto& prop : mUserObject->mShaderProperties)
	{
		if (prop.second.type == GL_FLOAT)
		{
			double value = 0.0;
			prop.second.property->GetData(&value, sizeof(double));

			const GLint loc = glGetUniformLocation(shader->GetProgramObj(), prop.first.c_str());
			glUniform1f(loc, static_cast<float>(value));
		}
	}

	shader->UnBind();

	return true;
}