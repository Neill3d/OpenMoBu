
/** \file posteffect_shader_userobject.cxx

Sergei <Neill3d> Solokhin 2018-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

// Class declaration
#include "posteffect_shader_userobject.h"
#include "postprocessing_ini.h"
#include <vector>
#include <limits>
#include <ctime>
#include <filesystem>

#include "FileUtils.h"
#include "mobu_logging.h"

#include "postpersistentdata.h"
#include "posteffect_buffers.h"
#include <hashUtils.h>

// custom assets inserting



//--- FiLMBOX Registration & Implementation.
FBClassImplementation(EffectShaderUserObject);
FBUserObjectImplement(EffectShaderUserObject,
                        "User Object used to store a persistance data for one effect shader.",
						"cam_switcher_toggle.png");                                          //Register UserObject class
PostEffectFBElementClassImplementation(EffectShaderUserObject, "User Effect Shader", "cam_switcher_toggle.png");                  //Register to the asset system

////////////////////////////////////////////////////////////////////////////////
//

const char* FBPropertyBaseEnum<EEffectResolution>::mStrings[] = {
	"Original",
	"Downscale 1/2",
	"Downscale 1/4",
	0
};

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
		p->RequestShadersReload();
	}
}

void EffectShaderUserObject::ActionOpenFolder(HIObject pObject, bool value)
{
	EffectShaderUserObject* p = FBCast<EffectShaderUserObject>(pObject);
	if (p && value)
	{
		p->DoOpenFolderWithShader();
	}
}

void EffectShaderUserObject::ActionExportShaderScheme(HIObject pObject, bool value)
{
	EffectShaderUserObject* p = FBCast<EffectShaderUserObject>(pObject);
	if (p && value)
	{
		p->DoExportShaderScheme();
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

	FBPropertyPublish(this, VertexFile, "Vertex File", nullptr, nullptr);
	FBPropertyPublish(this, FragmentFile, "Shader File", nullptr, nullptr);
	FBPropertyPublish(this, UseShaderToyCompatibility, "ShaderToy Compatibility", nullptr, nullptr);
	FBPropertyPublish(this, ReloadShaders, "Reload Shader", nullptr, ActionReloadShaders);
	FBPropertyPublish(this, OpenFolder, "Open Folder", nullptr, ActionOpenFolder);
	FBPropertyPublish(this, ExportShaderScheme, "Export Shader Scheme", nullptr, ActionExportShaderScheme);

	FBPropertyPublish(this, NumberOfPasses, "Number Of Passes", nullptr, nullptr);

	FBPropertyPublish(this, UseMasking, "Use Masking", nullptr, nullptr);
	FBPropertyPublish(this, MaskingChannel, "Masking Channel", nullptr, nullptr);

	Resolution = eEffectResolutionOriginal;

	VertexFile = UserBufferShader::DEFAULT_VERTEX_SHADER_FILE;
	FragmentFile = UserBufferShader::DEFAULT_FRAGMENT_SHADER_FILE;
	UseShaderToyCompatibility = false;
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

bool EffectShaderUserObject::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	if (pStoreWhat == kFbxObjectStore::kCleanup)
	{
		RequestShadersReload();
	}

	return ParentClass::FbxRetrieve(pFbxObject, pStoreWhat);
}

bool EffectShaderUserObject::PlugNotify(FBConnectionAction pAction, FBPlug* pThis, int pIndex, FBPlug* pPlug, FBConnectionType pConnectionType, FBPlug* pNewPlug)
{
	if (FBIS(pPlug, EffectShaderUserObject))
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
		}
	}

	return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
}


bool EffectShaderUserObject::RequestShadersReload()
{
	mReloadShaders = true;
	for (int i=0; i<GetDstCount(); ++i)
	{
		FBPlug* dstPlug = GetDst(i);
		
		if (FBIS(dstPlug, PostPersistentData))
		{
			if (PostPersistentData* persistentData = static_cast<PostPersistentData*>(dstPlug))
			{
				constexpr const bool isExternal{ true };
				constexpr const bool propagateToCustomEffects{ false };

				persistentData->RequestShadersReload(isExternal, propagateToCustomEffects);
			}
		}
		else if (FBIS(dstPlug, EffectShaderUserObject))
		{
			if (EffectShaderUserObject* effectShaderObj = static_cast<EffectShaderUserObject*>(dstPlug))
			{
				effectShaderObj->RequestShadersReload();
			}
		}
	}
	return true;
}

bool EffectShaderUserObject::CalculateShaderFilePaths(std::filesystem::path& vertexShaderPath, std::filesystem::path& fragmentShaderPath)
{
	const char* vertexRelativePath = VertexFile;

	if (!vertexRelativePath || !*vertexRelativePath)
	{
		LOGE("[%s] Vertex File property is empty!\n", LongName.AsString());
		return false;
	}

	const char* fragmentRelativePath = FragmentFile;

	if (!fragmentRelativePath || !*fragmentRelativePath)
	{
		LOGE("[%s] Fragment File property is empty!\n", LongName.AsString());
		return false;
	}

	const auto foundVertexPath = FindEffectLocation(std::filesystem::path(AnsiToWide(vertexRelativePath)));
	const auto foundFragmentPath = FindEffectLocation(std::filesystem::path(AnsiToWide(fragmentRelativePath)));

	if (!foundVertexPath || !foundFragmentPath)
	{
		LOGE("[%s] Failed to find shaders location for %s, %s!\n", LongName.AsString(), vertexRelativePath, fragmentRelativePath);
		return false;
	}

	vertexShaderPath = *foundVertexPath;
	fragmentShaderPath = *foundFragmentPath;

	LOGI("[%s] Vertex shader path: %ls\n", LongName.AsString(), vertexShaderPath.c_str());
	LOGI("[%s] Fragment shader path: %ls\n", LongName.AsString(), fragmentShaderPath.c_str());
	return true;
}

bool EffectShaderUserObject::DoReloadShaders()
{
	std::filesystem::path vertexPath;
	std::filesystem::path fragmentPath;

	if (!CalculateShaderFilePaths(vertexPath, fragmentPath))
		return false;

	constexpr int variationIndex = 0;
	if (!mUserShader->Load(variationIndex, vertexPath, fragmentPath, UseShaderToyCompatibility))
	{
		LOGE("[%s] Failed to load shaders for %ls, %ls!\n",
			LongName.AsString(), vertexPath.c_str(), fragmentPath.c_str());
		return false;
	}
	
	// reload connected input buffers
	for (int i = 0; i < GetSrcCount(); ++i)
	{
		FBPlug* srcPlug = GetSrc(i);
		if (FBIS(srcPlug, EffectShaderUserObject))
		{
			if (EffectShaderUserObject* effectShaderObj = static_cast<EffectShaderUserObject*>(srcPlug))
			{
				effectShaderObj->DoReloadShaders();
			}
		}
	}

	SetReloadShadersState(false);
	return true;
}

std::filesystem::path SanitizeRelative(const std::filesystem::path& rel)
{
	std::string s = rel.string();

	// Remove any leading slashes or backslashes
	while (!s.empty() && (s[0] == '/' || s[0] == '\\'))
		s.erase(s.begin());

	return std::filesystem::path(s);
}

std::filesystem::path ComputeFullShaderPath(
	const char* fragment_shader_rpath,
	const char* fragment_abs_path_only)
{
	namespace fs = std::filesystem;

	fs::path base = fs::path(fragment_abs_path_only);
	fs::path rel = SanitizeRelative(fs::path(fragment_shader_rpath));

	fs::path full = fs::weakly_canonical(base / rel);

	return full;
}

bool OpenExplorerFolder(const std::filesystem::path& path)
{
	if (!std::filesystem::exists(path))
		return false;

	// We open the folder, not the file
	std::filesystem::path folder = path;
	if (std::filesystem::is_regular_file(path))
		folder = path.parent_path();

#if defined(_WIN32)
	std::string winPath = folder.string();
	std::replace(winPath.begin(), winPath.end(), '/', '\\');
	std::string cmd = "explorer \"" + winPath + "\"";
#elif defined(__APPLE__)
	std::string cmd = "open \"" + folder.string() + "\"";
#else  // Linux / Unix
	std::string cmd = "xdg-open \"" + folder.string() + "\"";
#endif

	return system(cmd.c_str()) == 0;
}

bool EffectShaderUserObject::DoOpenFolderWithShader()
{
	const char* fragmentRelativePath = FragmentFile;

	if (!fragmentRelativePath || !*fragmentRelativePath)
	{
		LOGE("[%s] Shader File property is empty!\n", LongName.AsString());
		return false;
	}

	const auto shaderPath = FindEffectLocation(std::filesystem::path(AnsiToWide(fragmentRelativePath)));
	if (!shaderPath)
	{
		LOGE("[%s] Failed to find shader for relative path %s!\n", LongName.AsString(), fragmentRelativePath);
		return false;
	}

	if (!OpenExplorerFolder(*shaderPath))
	{
		LOGE("[%s] Failed to open folder for %ls\n", LongName.AsString(), shaderPath->c_str());
		return false;
	}

	return true;
}

bool EffectShaderUserObject::DoExportShaderScheme()
{
	namespace fs = std::filesystem;

	const char* fragmentRelativePath = FragmentFile;

	if (!fragmentRelativePath || !*fragmentRelativePath)
	{
		LOGE("[%s] Shader File property is empty!\n", LongName.AsString());
		return false;
	}

	const auto shaderPath = FindEffectLocation(fs::path(AnsiToWide(fragmentRelativePath)));

	if (!shaderPath)
	{
		LOGE("[%s] Failed to find shader for relative path %s!\n", LongName.AsString(), fragmentRelativePath);
		return false;
	}

	const char* objectName = GetFullName();

	if (!objectName || !*objectName)
		return false;

	fs::path outputPath = shaderPath->parent_path() / fs::path(AnsiToWide(objectName));

	outputPath += L".json";
	outputPath = outputPath.lexically_normal();

	const ShaderPropertyScheme* scheme = mUserShader ? mUserShader->GetPropertySchemePtr() : nullptr;

	if (!scheme || !scheme->ExportToJSON(outputPath))
	{
		LOGE("[%s] Failed to export shader scheme to %ls\n", LongName.AsString(), outputPath.c_str());
		return false;
	}

	return true;
}

void EffectShaderUserObject::DefaultValues()
{}

FBProperty* EffectShaderUserObject::MakePropertyInt(const ShaderProperty& prop)
{
	constexpr const bool isUser{ false };
	FBProperty* newProp = PropertyCreate(prop.GetName(), FBPropertyType::kFBPT_int, ANIMATIONNODE_TYPE_INTEGER, false, isUser, nullptr);
	PropertyAdd(newProp);
	return newProp;
}

FBProperty* EffectShaderUserObject::MakePropertyFloat(const ShaderProperty& prop)
{
	constexpr const bool isUser{ false };
	FBProperty* newProp = nullptr;

	if (strstr(prop.GetUniformName(), "_flag") != nullptr)
	{
		newProp = PropertyCreate(prop.GetName(), FBPropertyType::kFBPT_bool, ANIMATIONNODE_TYPE_BOOL, true, isUser, nullptr);
	}
	else
	{
		newProp = PropertyCreate(prop.GetName(), FBPropertyType::kFBPT_double, ANIMATIONNODE_TYPE_NUMBER, true, isUser, nullptr);

		if (strstr(prop.GetUniformName(), "_slider") != nullptr)
		{
			newProp->SetMinMax(0.0, 100.0);
		}
	}

	PropertyAdd(newProp);
	return newProp;
}

FBProperty* EffectShaderUserObject::MakePropertyVec2(const ShaderProperty& prop)
{
	constexpr const bool isUser{ false };
	FBProperty* newProp = nullptr;
	
	if (strstr(prop.GetUniformName(), "_wstoss") != nullptr)
	{
		// a property for world position that is going to be converted into screen space position
		newProp = PropertyCreate(prop.GetName(), FBPropertyType::kFBPT_Vector3D, ANIMATIONNODE_TYPE_VECTOR, true, isUser, nullptr);
	}
	else
	{
		newProp = PropertyCreate(prop.GetName(), FBPropertyType::kFBPT_Vector2D, ANIMATIONNODE_TYPE_VECTOR, true, isUser, nullptr);
	}
	
	PropertyAdd(newProp);
	return newProp;
}

FBProperty* EffectShaderUserObject::MakePropertyVec3(const ShaderProperty& prop)
{
	constexpr const bool isUser{ false };
	FBProperty* newProp = nullptr;

	if (strstr(prop.GetUniformName(), "_color") != nullptr)
	{
		newProp = PropertyCreate(prop.GetName(), FBPropertyType::kFBPT_ColorRGB, ANIMATIONNODE_TYPE_COLOR, true, isUser, nullptr);
	}
	else
	{
		newProp = PropertyCreate(prop.GetName(), FBPropertyType::kFBPT_Vector3D, ANIMATIONNODE_TYPE_VECTOR, true, isUser, nullptr);
	}

	PropertyAdd(newProp);
	return newProp;
}

FBProperty* EffectShaderUserObject::MakePropertyVec4(const ShaderProperty& prop)
{
	constexpr const bool isUser{ false };
	FBProperty* newProp = nullptr;

	if (strstr(prop.GetUniformName(), "_color") != nullptr)
	{
		newProp = PropertyCreate(prop.GetName(), FBPropertyType::kFBPT_ColorRGBA, ANIMATIONNODE_TYPE_COLOR_RGBA, true, isUser, nullptr);
	}
	else
	{
		newProp = PropertyCreate(prop.GetName(), FBPropertyType::kFBPT_Vector4D, ANIMATIONNODE_TYPE_VECTOR_4, true, isUser, nullptr);
	}

	PropertyAdd(newProp);
	return newProp;
}

FBProperty* EffectShaderUserObject::MakePropertySampler(const ShaderProperty& prop)
{
	constexpr const bool isUser{ false };
	FBProperty* newProp = PropertyCreate(prop.GetName(), FBPropertyType::kFBPT_object, ANIMATIONNODE_TYPE_OBJECT, false, isUser, nullptr);
	if (FBPropertyListObject* listObjProp = FBCast<FBPropertyListObject>(newProp))
	{
		//listObjProp->SetFilter(FBTexture::GetInternalClassId() | EffectShaderUserObject::GetInternalClassId());
		listObjProp->SetSingleConnect(true);

		PropertyAdd(newProp);
		return newProp;
	}
	return nullptr;
}

FBProperty* EffectShaderUserObject::GetOrMakeProperty(const ShaderProperty& prop)
{
	FBProperty* fbProperty = PropertyList.Find(prop.GetName());
	const FBPropertyType fbPropertyType = ShaderProperty::ShaderPropertyToFBPropertyType(prop);

	// NOTE: check not only user property, but also a property type !
	if (!fbProperty || fbProperty->GetPropertyType() != fbPropertyType)
	{
		// base on type, make a custom property
		fbProperty = nullptr;

		switch (prop.GetType())
		{
		case EPropertyType::INT:
			fbProperty = MakePropertyInt(prop);
			break;
		case EPropertyType::FLOAT:
			fbProperty = MakePropertyFloat(prop);
			break;
		case EPropertyType::VEC2:
			fbProperty = MakePropertyVec2(prop);
			break;
		case EPropertyType::VEC3:
			fbProperty = MakePropertyVec3(prop);
			break;
		case EPropertyType::VEC4:
			fbProperty = MakePropertyVec4(prop);
			break;
		case EPropertyType::TEXTURE:
			fbProperty = MakePropertySampler(prop);
			break;
		default:
			LOGE("[%s] not supported prop type for %s uniform\n", LongName.AsString(), prop.GetName());
		}
	}
	return fbProperty;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UserBufferShader

uint32_t UserBufferShader::SHADER_NAME_HASH = xxhash32(UserBufferShader::SHADER_NAME);

UserBufferShader::UserBufferShader(EffectShaderUserObject* UserObject)
	: PostEffectBufferShader(UserObject)
	, mUserObject(UserObject)
{}

const char* UserBufferShader::GetName() const
{
	return (mUserObject) ? mUserObject->GetFullName() : SHADER_NAME;
}

uint32_t UserBufferShader::GetNameHash() const
{
	return (mUserObject) ? xxhash32(mUserObject->GetFullName()) : SHADER_NAME_HASH;
}

const char* UserBufferShader::GetUseMaskingPropertyName() const 
{
	return (mUserObject) ? mUserObject->UseMasking.GetName() : "Use Masking"; 
}
const char* UserBufferShader::GetMaskingChannelPropertyName() const 
{ 
	return (mUserObject) ? mUserObject->MaskingChannel.GetName() : "Masking Channel"; 
}

const char* UserBufferShader::GetVertexFname(const int variationIndex) const 
{ 
	return (mUserObject) ? mUserObject->VertexFile.AsString() : DEFAULT_VERTEX_SHADER_FILE; 
}
//! get a filename of a fragment shader, for this effect, returns a relative filename
const char* UserBufferShader::GetFragmentFname(const int variationIndex) const 
{ 
	return (mUserObject) ? mUserObject->FragmentFile.AsString() : DEFAULT_FRAGMENT_SHADER_FILE; 
}

/// new feature to have several passes for a specified effect
int UserBufferShader::GetNumberOfPasses() const
{
	return (mUserObject) ? mUserObject->NumberOfPasses : 1;
}
//! initialize a specific path for drawing
bool UserBufferShader::OnRenderPassBegin(const int pass, PostEffectRenderContext& renderContext, PostEffectContextProxy* effectContext)
{
	// TODO: define iPass value for the shader
	
	return true;
}

void UserBufferShader::OnPropertySchemeUpdated(const ShaderPropertyScheme* newScheme, const ShaderPropertyScheme* oldScheme)
{
	if (!newScheme)
		return;

	std::unordered_map<uint32_t, FBProperty*> mPropertiesToRemove;
	mPropertiesToRemove.clear();

	if (oldScheme)
	{
		for (const auto& prop : oldScheme->GetProperties())
		{
			if (prop.IsGeneratedByUniform())
			{
				if (FBProperty* fbProperty = mUserObject->PropertyList.Find(prop.GetName()))
				{
					mPropertiesToRemove.emplace(prop.GetNameHash(), fbProperty);
				}
			}
		}
	}
	
	for (const auto& prop : newScheme->GetProperties())
	{
		if (prop.IsGeneratedByUniform())
		{
			auto iter = mPropertiesToRemove.find(prop.GetNameHash());
			if (iter != mPropertiesToRemove.end())
			{
				mPropertiesToRemove.erase(iter);
			}
			//
			mUserObject->GetOrMakeProperty(prop);
		}
	}

	for (auto& pair : mPropertiesToRemove)
	{
		mUserObject->PropertyRemove(pair.second);
	}
	mPropertiesToRemove.clear();
}

//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
bool UserBufferShader::OnCollectUI(PostEffectContextProxy* effectContext, int maskIndex) const
{
	BindSystemUniforms(effectContext);
	return true;
}

void EffectShaderUserObject::RecalculateWidthAndHeight(int& w, int& h) const
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
	w = std::max(1, w);
	h = std::max(1, h);
}
