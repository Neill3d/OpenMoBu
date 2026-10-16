
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
#include "texture_usereffect.h"

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

	// a new effect has to be loaded once it gets connected to a post processing data
	mReloadShaders = true;
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
	FBPropertyPublish(this, Status, "Status", GetStatusText, nullptr);

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

	Status.ModifyPropertyFlag(kFBPropertyFlagReadOnly, true);
	Status.ModifyPropertyFlag(kFBPropertyFlagNotSavable, true);
	
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
		else if (FBIS(dstPlug, TextureUserEffect))
		{
			if (TextureUserEffect* texture = static_cast<TextureUserEffect*>(dstPlug))
			{
				texture->OnUserEffectChanged();
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
	// The request is consumed even if the loading fails. A broken shader must not be retried every frame
	//  and must not block the rest of the post processing. The effect is going to be skipped in the chain
	//  and the Status property reports the error, a new attempt happens on the next reload request.
	// NOTE: the flag is cleared first, so a request that comes while we are loading is not lost.
	SetReloadShadersState(false);

	std::filesystem::path vertexPath;
	std::filesystem::path fragmentPath;

	if (!CalculateShaderFilePaths(vertexPath, fragmentPath))
	{
		mUserShader->SetActive(false);
		SetLoadState(EEffectLoadState::FileNotFound);
		return false;
	}

	constexpr int variationIndex = 0;
	if (!mUserShader->Load(variationIndex, vertexPath, fragmentPath, UseShaderToyCompatibility))
	{
		LOGE("[%s] Failed to load shaders for %ls, %ls!\n",
			LongName.AsString(), vertexPath.c_str(), fragmentPath.c_str());
		// NOTE: Load has already deactivated the shader
		SetLoadState(EEffectLoadState::LoadFailed);
		return false;
	}

	// reload connected input buffers, each of them reports its own state
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

	// warnings are going to be counted once the property scheme is rebuilt (see UserBufferShader::OnPropertySchemeAssociated)
	SetWarningCount(0);
	SetLoadState(EEffectLoadState::Loaded);
	return true;
}

bool EffectShaderUserObject::IsConnectedToPostProcessing(int depth)
{
	// protection from a cyclic connections
	constexpr int MAX_DEPTH{ 16 };
	if (depth > MAX_DEPTH)
		return false;

	for (int i = 0; i < GetDstCount(); ++i)
	{
		FBPlug* dstPlug = GetDst(i);

		if (FBIS(dstPlug, PostPersistentData) || FBIS(dstPlug, TextureUserEffect))
		{
			return true;
		}
		if (FBIS(dstPlug, EffectShaderUserObject))
		{
			if (EffectShaderUserObject* effectShaderObj = static_cast<EffectShaderUserObject*>(dstPlug))
			{
				if (effectShaderObj->IsConnectedToPostProcessing(depth + 1))
					return true;
			}
		}
	}
	return false;
}

std::string EffectShaderUserObject::ComposeStatusText()
{
	// without a connection the effect is never going to be loaded
	if (!IsConnectedToPostProcessing())
		return "Not connected";

	const EEffectLoadState state = GetLoadState();

	// waiting for the render thread to pick up a request
	if (IsNeedToReloadShaders())
		return (state == EEffectLoadState::NotLoaded) ? "Not loaded" : "Reload pending";

	switch (state)
	{
	case EEffectLoadState::Loaded:
	{
		const int warnings = GetWarningCount();
		if (warnings <= 0)
			return "Ok";
		return (warnings == 1) ? "Ok (1 warning)" : "Ok (" + std::to_string(warnings) + " warnings)";
	}
	case EEffectLoadState::FileNotFound:
		return "Error: shader file not found";
	case EEffectLoadState::LoadFailed:
		return "Error: failed to load shader";
	default:
		break;
	}
	return "Not loaded";
}

const char* EffectShaderUserObject::GetStatusText(HIObject pObject)
{
	EffectShaderUserObject* p = FBCast<EffectShaderUserObject>(pObject);
	if (!p)
		return "";

	p->mStatusText = p->ComposeStatusText();
	return p->mStatusText.c_str();
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

// NOTE: the type of a made property has to be in sync with ShaderProperty::ShaderPropertyToFBPropertyType,
//  otherwise GetOrMakeProperty would see a type mismatch on every shader reload and recreate the property.
//  That is why the choice is based on property flags and not on a uniform name substring.

FBProperty* EffectShaderUserObject::MakePropertyInt(const ShaderProperty& prop)
{
	constexpr const bool isUser{ false };
	FBProperty* newProp = PropertyCreate(prop.GetName(), FBPropertyType::kFBPT_int, ANIMATIONNODE_TYPE_INTEGER, false, isUser, nullptr);
	if (!newProp)
		return nullptr;

	PropertyAdd(newProp);
	return newProp;
}

FBProperty* EffectShaderUserObject::MakePropertyFloat(const ShaderProperty& prop)
{
	constexpr const bool isUser{ false };
	FBProperty* newProp = nullptr;

	if (prop.HasFlag(PropertyFlag::IsFlag))
	{
		newProp = PropertyCreate(prop.GetName(), FBPropertyType::kFBPT_bool, ANIMATIONNODE_TYPE_BOOL, true, isUser, nullptr);
	}
	else
	{
		newProp = PropertyCreate(prop.GetName(), FBPropertyType::kFBPT_double, ANIMATIONNODE_TYPE_NUMBER, true, isUser, nullptr);

		if (newProp && prop.HasFlag(PropertyFlag::IsClamped100))
		{
			newProp->SetMinMax(0.0, 100.0);
		}
	}

	if (!newProp)
		return nullptr;

	PropertyAdd(newProp);
	return newProp;
}

FBProperty* EffectShaderUserObject::MakePropertyVec2(const ShaderProperty& prop)
{
	constexpr const bool isUser{ false };
	FBProperty* newProp = nullptr;

	if (prop.HasFlag(PropertyFlag::ConvertWorldToScreenSpace))
	{
		// a property for world position that is going to be converted into screen space position
		newProp = PropertyCreate(prop.GetName(), FBPropertyType::kFBPT_Vector3D, ANIMATIONNODE_TYPE_VECTOR, true, isUser, nullptr);
	}
	else
	{
		newProp = PropertyCreate(prop.GetName(), FBPropertyType::kFBPT_Vector2D, ANIMATIONNODE_TYPE_VECTOR, true, isUser, nullptr);
	}

	if (!newProp)
		return nullptr;

	PropertyAdd(newProp);
	return newProp;
}

FBProperty* EffectShaderUserObject::MakePropertyVec3(const ShaderProperty& prop)
{
	constexpr const bool isUser{ false };
	FBProperty* newProp = nullptr;

	if (prop.HasFlag(PropertyFlag::IsColor))
	{
		newProp = PropertyCreate(prop.GetName(), FBPropertyType::kFBPT_ColorRGB, ANIMATIONNODE_TYPE_COLOR, true, isUser, nullptr);
	}
	else
	{
		newProp = PropertyCreate(prop.GetName(), FBPropertyType::kFBPT_Vector3D, ANIMATIONNODE_TYPE_VECTOR, true, isUser, nullptr);
	}

	if (!newProp)
		return nullptr;

	PropertyAdd(newProp);
	return newProp;
}

FBProperty* EffectShaderUserObject::MakePropertyVec4(const ShaderProperty& prop)
{
	constexpr const bool isUser{ false };
	FBProperty* newProp = nullptr;

	if (prop.HasFlag(PropertyFlag::IsColor))
	{
		newProp = PropertyCreate(prop.GetName(), FBPropertyType::kFBPT_ColorRGBA, ANIMATIONNODE_TYPE_COLOR_RGBA, true, isUser, nullptr);
	}
	else
	{
		newProp = PropertyCreate(prop.GetName(), FBPropertyType::kFBPT_Vector4D, ANIMATIONNODE_TYPE_VECTOR_4, true, isUser, nullptr);
	}

	if (!newProp)
		return nullptr;

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
	if (fbProperty && fbProperty->GetPropertyType() == fbPropertyType)
	{
		return fbProperty;
	}

	if (fbProperty)
	{
		// the uniform has changed its type (shader was modified), a property can't be re-typed,
		//  remove the old one first, otherwise two properties with the same name are going to live in the property list
		//  and the property scheme could get associated with the outdated one
		LOGI("[%s] property %s has changed its type, re-creating it\n", LongName.AsString(), prop.GetName());
		PropertyRemove(fbProperty);
		fbProperty = nullptr;
	}

	// base on type, make a custom property
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

	if (!fbProperty)
	{
		LOGE("[%s] failed to make a property for %s uniform\n", LongName.AsString(), prop.GetName());
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
	if (!newScheme || !mUserObject)
		return;

	// properties generated from the previous shader uniforms, whatever is left after
	//  the new scheme is processed doesn't exist in the new shader anymore
	std::unordered_map<uint32_t, FBProperty*> mPropertiesToRemove;

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

void UserBufferShader::OnPropertySchemeAssociated(const ShaderPropertyScheme* newScheme)
{
	if (mUserObject && newScheme)
	{
		mUserObject->SetWarningCount(newScheme->GetNumberOfWarnings());
	}
}

//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
bool UserBufferShader::OnCollectUI(PostEffectContextProxy* effectContext, int maskIndex) const
{
	// NOTE: system uniforms are uploaded in the render thread, see PostEffectBufferShader::Render
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
