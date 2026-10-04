
#pragma once

/** \file posteffect_shader_userobject.h

Sergei <Neill3d> Solokhin 2018-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include
#include <fbsdk/fbsdk.h>
#include "postprocessing_helper.h"
#include "posteffect_shader.h"

#include "glslShaderProgram.h"
#include <string>
#include <unordered_map>
#include <unordered_set>

/*
* Render to texture or render to effects chain
*  In case of render to texture, the processing in stored in the post effect internal texture object
*   that object could be used as input for another effect
*  for example, the effect of screen space god rays require to render scene into a lighting texture (kind of downsampled bloom pass)
* 
* 
 system postfix for uniforms

 uniform float with
 _slider - double value with a range [0; 100]
 _flag - bool checkbox casted to float [0; 1]

 vec2
 _wstoss - convert vec3 property in world space into vec2 uniform in screen space

 vec3
 _color - color RGB picker

 vec4
 _color - color RGBA picker

*/

//--- Registration define

#define EFFECTSHADER_USEROBJECT__CLASSSTR	"EffectShaderUserObject"

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

// forward
class EffectShaderUserObject;



enum EEffectResolution
{
	eEffectResolutionOriginal,
	eEffectResolutionDownscale2x,
	eEffectResolutionDownscale4x
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// internal buffer shader that is going to be conneced to an internal post effect 
/// </summary>

class UserBufferShader : public PostEffectBufferShader
{
public:

	UserBufferShader(EffectShaderUserObject* UserObject);
	virtual ~UserBufferShader() = default;

	/// number of variations of the same effect, but with a different algorithm (for instance, 3 ways of making a lens flare effect)
	virtual int GetNumberOfVariations() const override { return 1; }

	//! an effect public name
	virtual const char* GetName() const override;
	uint32_t GetNameHash() const override;
	//! get a filename of vertex shader, for this effect. returns a relative filename
	virtual const char* GetVertexFname(const int variationIndex) const override;
	//! get a filename of a fragment shader, for this effect, returns a relative filename
	virtual const char* GetFragmentFname(const int variationIndex) const override;

	/// new feature to have several passes for a specified effect
	virtual int GetNumberOfPasses() const override;

	//! initialize a specific path for drawing
	virtual bool OnRenderPassBegin(const int pass, PostEffectRenderContext& renderContext, PostEffectContextProxy* effectContext) override;

protected:
	friend class EffectShaderUserObject;
	
	static constexpr const char* SHADER_NAME = "User Effect";
	static uint32_t SHADER_NAME_HASH;

	static constexpr const char* DEFAULT_VERTEX_SHADER_FILE = "/GLSL/simple130.glslv";
	static constexpr const char* DEFAULT_FRAGMENT_SHADER_FILE = "/GLSL/test.glslf";

	//!< scene object, data container and interaction with the end user
	EffectShaderUserObject* mUserObject{ nullptr };
	
	virtual const char* GetUseMaskingPropertyName() const override;
	virtual const char* GetMaskingChannelPropertyName() const override;
	//!< if true, once shader is loaded, let's inspect all the uniforms and make properties from them
	virtual bool DoPopulatePropertiesFromUniforms() const override { return true; }

	//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
	virtual bool OnCollectUI(PostEffectContextProxy* effectContext, int maskIndex) const override;

	//! a callback event to process a property added, so that we could make and associate component's FBProperty with it
	virtual void OnPropertySchemeUpdated(const ShaderPropertyScheme* newScheme, const ShaderPropertyScheme* oldScheme) override;

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// A user object for one shader user object
///  that is designed to be connected to post processing effect
/// </summary>
class EffectShaderUserObject : public FBUserObject 
{
    //--- FiLMBOX declaration.
	FBClassDeclare(EffectShaderUserObject, FBUserObject)
	FBDeclareUserObject(EffectShaderUserObject)

public:
	//! a constructor
	EffectShaderUserObject(const char *pName = nullptr, HIObject pObject = nullptr);

    //--- FiLMBOX Construction/Destruction,
    virtual bool FBCreate() override;        //!< FiLMBOX Creation function.
    virtual void FBDestroy() override;       //!< FiLMBOX Destruction function.
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;

	virtual bool PlugNotify(FBConnectionAction pAction, FBPlug* pThis, int pIndex, FBPlug* pPlug, FBConnectionType pConnectionType, FBPlug* pNewPlug) override;

public: // PROPERTIES

	FBPropertyInt				UniqueClassId;
	FBPropertyAnimatableBool	Active;
	
	// texture is going to use the source chain size
	// TODO: customize the size, to have a downsample option, upsample option and custom defined size option
	
	FBPropertyBaseEnum<EEffectResolution> Resolution;

	FBPropertyListObject		OutputVideo; //!< in case of render to texture, let's expose it in the FBVideoMemory

	FBPropertyString			VertexFile;   //!< vertex shader file to evaluate
	FBPropertyString			FragmentFile; //!< fragment shader file to evaluate

	FBPropertyAction			ReloadShaders;
	FBPropertyAction			OpenFolder; // open a folder where the shader file is located (if found)
	FBPropertyAction			ExportShaderScheme;
	FBPropertyBool				GenerateMipMaps;
	FBPropertyAction			ResetToDefault;

	FBPropertyBool				UseMasking;
	FBPropertyBaseEnum<EMaskingChannel>	MaskingChannel;
	
	FBPropertyInt				NumberOfPasses; //!< define in how many passes the shader should be executed (global variable iPass)

	// compile fragment shader with a ShaderToy compatibility
	FBPropertyBool				UseShaderToyCompatibility;

public:

	bool RequestShadersReload();
	bool DoReloadShaders();
	bool DoOpenFolderWithShader();
	bool DoExportShaderScheme();

	// calculate absolute paths for vertex and fragment shaders
	// return false in case a given effect file is not found under the expected location
	bool CalculateShaderFilePaths(std::filesystem::path& vertexShaderPath, std::filesystem::path& fragmentShaderPath);

	bool IsNeedToReloadShaders() const { return mReloadShaders; }
	void SetReloadShadersState(bool state) { mReloadShaders = state; }

	PostEffectBufferShader* GetUserShaderPtr() const { return mUserShader.get(); }

	// recalculate width and height based on shader resolution option
	void RecalculateWidthAndHeight(int& w, int& h) const;

protected:

	friend class ToolPostProcessing;
	friend class UserBufferShader;

    FBString			mText;
	bool				mReloadShaders{ false };
	
	std::unique_ptr<PostEffectBufferShader>		mUserShader;
	
	virtual PostEffectBufferShader* MakeANewClassInstance();

	void	DefaultValues();
	void	LoadFromConfig(const char *sessionFilter=nullptr);
	void	LoadFarValueFromConfig();

	FBProperty* MakePropertyInt(const ShaderProperty& prop);
	FBProperty* MakePropertyFloat(const ShaderProperty& prop);
	FBProperty* MakePropertyVec2(const ShaderProperty& prop);
	FBProperty* MakePropertyVec3(const ShaderProperty& prop);
	FBProperty* MakePropertyVec4(const ShaderProperty& prop);
	FBProperty* MakePropertySampler(const ShaderProperty& prop);

	FBProperty* GetOrMakeProperty(const ShaderProperty& prop);

	static void ActionReloadShaders(HIObject pObject, bool value);
	static void ActionOpenFolder(HIObject pObject, bool value);
	static void ActionExportShaderScheme(HIObject pObject, bool value);
};
