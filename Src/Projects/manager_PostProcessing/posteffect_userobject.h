
#pragma once

/** \file posteffect_userobject.h

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include
#include <fbsdk/fbsdk.h>
#include "postprocessing_helper.h"

#include "posteffectbase.h"
#include "glslShaderProgram.h"
#include <string>
#include <unordered_map>

/*
 system postfix for uniforms

 uniform float with
 _slider - double value with a range [0; 100]
 _flag - bool checkbox casted to float [0; 1]

 vec3
 _color - color RGB picker

 vec4
 _color - color RGBA picker

*/

//--- Registration define

#define POSTEFFECT_USEROBJECT__CLASSSTR	"PostEffectUserObject"

// forward
class PostEffectUserObject;


enum class ShaderSystemUniform
{
	INPUT_COLOR_SAMPLER_2D, //!< this is an input image that we read from
	INPUT_DEPTH_SAMPLER_2D, //!< this is a scene depth texture sampler in case shader will need it for processing
	INPUT_MASK_SAMPLER_2D, //!< binded mask for a shader processing

	USE_MASKING, //!< float uniform [0; 1] to define if the mask have to be used
	UPPER_CLIP, //!< this is an upper clip image level. defined in a texture coord space to skip processing
	LOWER_CLIP, //!< this is a lower clip image level. defined in a texture coord space to skip processing

	RESOLUTION, //!< vec2 that contains processing absolute resolution, like 1920x1080
	COUNT
};



class PostUserEffect : public PostEffectBase
{
public:

	PostUserEffect(PostEffectUserObject* UserObject)
		: PostEffectBase()
		, mUserObject(UserObject)
	{}

	/// number of variations of the same effect, but with a different algorithm (for instance, 3 ways of making a lens flare effect)
	virtual int GetNumberOfVariations() const override
	{
		return 1;
	}

	//! an effect public name
	virtual const char* GetName() const override { return "User Effect"; }
	//! get a filename of vertex shader, for this effect. returns a relative filename
	virtual const char* GetVertexFname(const int variationIndex) const override { return "simple.vsh"; }
	//! get a filename of a fragment shader, for this effect, returns a relative filename
	virtual const char* GetFragmentFname(const int variationIndex) const override { return "test.glslf"; }

	//! prepare uniforms for a given variation of the effect
	virtual bool PrepUniforms(const int variationIndex) override
	{
		return true;
	}
	//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
	virtual bool CollectUIValues(PostPersistentData* pData, PostEffectContext& effectContext) override;

	/// new feature to have several passes for a specified effect
	virtual const int GetNumberOfPasses() const override
	{
		return 1;
	}
	//! initialize a specific path for drawing
	virtual bool PrepPass(const int pass) override
	{
		return true;
	}

protected:
	PostEffectUserObject* mUserObject;
};

/// <summary>
/// This class will make sure that the data related to the post effect is persistent.
/// </summary>
class PostEffectUserObject : public FBUserObject 
{
    //--- FiLMBOX declaration.
	FBClassDeclare(PostEffectUserObject, FBUserObject)
	FBDeclareUserObject(PostEffectUserObject);

public:
	//! a constructor
	PostEffectUserObject(const char *pName = nullptr, HIObject pObject = nullptr);

    //--- FiLMBOX Construction/Destruction,
    virtual bool FBCreate() override;        //!< FiLMBOX Creation function.
    virtual void FBDestroy() override;       //!< FiLMBOX Destruction function.

    
	void CopyValues(PostEffectUserObject* pOther);

public: // PROPERTIES

	FBPropertyInt				UniqueClassId;
	FBPropertyAnimatableBool	Active;

	FBPropertyString			ShaderFile; //!< fragment shader file to evaluate

	FBPropertyAction			ReloadShaders;
	FBPropertyBool				GenerateMipMaps;
	FBPropertyAction			ResetToDefault;

	FBPropertyBool				UseMasking;
	FBPropertyBaseEnum<EMaskingChannel>	MaskingChannel;
	
public:

	void DoReloadShaders();

	bool IsNeedToReloadShaders();
	void SetReloadShadersState(bool state);

	PostUserEffect* GetUserEffectPtr() const { return mUserEffect; }

protected:

	friend class ToolPostProcessing;
	friend class PostUserEffect;

	FBSystem			mSystem;

    FBString			mText;
	bool				mReloadShaders{ false };
	
	PostUserEffect*		mUserEffect;
	//GLSLShaderProgram* mShaderProgram{ nullptr };

	struct ShaderProperty
	{
		GLchar uniformName[256];
		GLsizei length;
		GLint size;
		GLenum type;

		GLint location{ -1 };
		FBProperty* property{ nullptr }; //!< property associated with the given shader uniform
	};

	std::unordered_map<std::string, ShaderProperty> mShaderProperties;

	static const char* gSystemUniformNames[static_cast<int>(ShaderSystemUniform::COUNT)];
	GLint mSystemUniformLocations[static_cast<int>(ShaderSystemUniform::COUNT)];


	void		DefaultValues();
	void		LoadFromConfig(const char *sessionFilter=nullptr);
	void		LoadFarValueFromConfig();

	void		RemoveShaderProperties();

	void ResetSystemUniformLocations();
	void		CheckUniforms();

	int IsSystemUniform(const char* uniformName); // -1 if not found, or return an index of a system uniform in the ShaderSystemUniform enum

	FBProperty* MakePropertyFloat(const ShaderProperty& prop);
	FBProperty* MakePropertyVec2(const ShaderProperty& prop);
	FBProperty* MakePropertyVec3(const ShaderProperty& prop);
	FBProperty* MakePropertyVec4(const ShaderProperty& prop);
	FBProperty* MakePropertySampler(const ShaderProperty& prop);

	static void ActionReloadShaders(HIObject pObject, bool value);

};
