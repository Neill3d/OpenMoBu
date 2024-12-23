
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
//--- Registration define

#define POSTEFFECT_USEROBJECT__CLASSSTR	"PostEffectUserObject"

// forward
class PostEffectUserObject;

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

		FBProperty* property; //!< property associated with the given shader uniform
	};

	std::unordered_map<std::string, ShaderProperty> mShaderProperties;

	void		DefaultValues();
	void		LoadFromConfig(const char *sessionFilter=nullptr);
	void		LoadFarValueFromConfig();

	void		RemoveShaderProperties();

	void		CheckUniforms();

	static void ActionReloadShaders(HIObject pObject, bool value);

};
