
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

 vec3
 _color - color RGB picker

 vec4
 _color - color RGBA picker

*/

//--- Registration define

#define EFFECTSHADER_USEROBJECT__CLASSSTR	"EffectShaderUserObject"
#define POSTEFFECT_USEROBJECT__CLASSSTR		"PostEffectUserObject"

// forward
class EffectShaderUserObject;
class PostEffectUserObject;

enum class ShaderSystemUniform
{
	INPUT_COLOR_SAMPLER_2D, //!< this is an input image that we read from
	iCHANNEL0, //!< this is an input image, compatible with shadertoy
	INPUT_DEPTH_SAMPLER_2D, //!< this is a scene depth texture sampler in case shader will need it for processing
	LINEAR_DEPTH_SAMPLER_2D, //!< a depth texture converted into linear space (used in SSAO)
	INPUT_MASK_SAMPLER_2D, //!< binded mask for a shader processing

	USE_MASKING, //!< float uniform [0; 1] to define if the mask have to be used
	UPPER_CLIP, //!< this is an upper clip image level. defined in a texture coord space to skip processing
	LOWER_CLIP, //!< this is a lower clip image level. defined in a texture coord space to skip processing

	RESOLUTION, //!< vec2 that contains processing absolute resolution, like 1920x1080
	iRESOLUTION, //!< vec2 absolute resolution, compatible with shadertoy

	iTIME, //!< compatible with shadertoy, float, shader playback time (in seconds)
	iDATE, //!< compatible with shadertoy, vec4, (year, month, day, time in seconds)

	MODELVIEW,	//!< current camera modelview matrix
	PROJ,		//!< current camera projection matrix
	MODELVIEWPROJ,	//!< current camera modelview-projection matrix

	COUNT
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// internal buffer shader that is going to be conneced to an internal post effect 
/// </summary>

class UserBufferShader : public PostEffectBufferShader
{
public:

	UserBufferShader(EffectShaderUserObject* UserObject)
		: PostEffectBufferShader()
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

	virtual bool IsDepthSamplerUsed() const override;
	virtual bool IsLinearDepthSamplerUsed() const override;


protected:
	EffectShaderUserObject* mUserObject;
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

    
	void CopyValues(EffectShaderUserObject* pOther);

public: // PROPERTIES

	FBPropertyInt				UniqueClassId;
	FBPropertyAnimatableBool	Active;
	
	// texture is going to use the source chain size
	// TODO: customize the size, to have a downsample option, upsample option and custom defined size option
	
	FBPropertyListObject		OutputVideo; //!< in case of render to texture, let's expose it in the FBVideoMemory

	FBPropertyString			ShaderFile; //!< fragment shader file to evaluate

	FBPropertyAction			ReloadShaders;
	FBPropertyBool				GenerateMipMaps;
	FBPropertyAction			ResetToDefault;

	FBPropertyBool				UseMasking;
	FBPropertyBaseEnum<EMaskingChannel>	MaskingChannel;
	
public:

	bool DoReloadShaders();

	bool IsNeedToReloadShaders();
	void SetReloadShadersState(bool state);

	UserBufferShader* GetUserShaderPtr() const { return mUserShader.get(); }

protected:

	friend class ToolPostProcessing;
	friend class UserBufferShader;

	FBSystem			mSystem;

    FBString			mText;
	bool				mReloadShaders{ false };
	
	std::unique_ptr<UserBufferShader>		mUserShader;
	//GLSLShaderProgram* mShaderProgram{ nullptr };

	struct ShaderProperty
	{
		GLchar uniformName[256]{ 0 };
		GLsizei length{ 0 };
		GLint size{ 0 };
		GLenum type{ 0 };

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

	bool IsDepthSamplerUsed() const { return mSystemUniformLocations[static_cast<int>(ShaderSystemUniform::INPUT_DEPTH_SAMPLER_2D)] >= 0; }
	bool IsLinearDepthSamplerUsed() const { return mSystemUniformLocations[static_cast<int>(ShaderSystemUniform::LINEAR_DEPTH_SAMPLER_2D)] >= 0; }

	void BindSystemUniforms(PostPersistentData* pData, PostEffectContext& effectContext) const;

	FBProperty* MakePropertyFloat(const ShaderProperty& prop);
	FBProperty* MakePropertyVec2(const ShaderProperty& prop);
	FBProperty* MakePropertyVec3(const ShaderProperty& prop);
	FBProperty* MakePropertyVec4(const ShaderProperty& prop);
	FBProperty* MakePropertySampler(const ShaderProperty& prop);

	static void ActionReloadShaders(HIObject pObject, bool value);

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class UserEffect : public PostEffectBase
{
public:
	UserEffect(PostEffectUserObject* UserObjectIn)
		: PostEffectBase()
		, mUserObject(UserObjectIn)
	{}
	virtual ~UserEffect()
	{}

	virtual int GetNumberOfBufferShaders() const override;
	virtual PostEffectBufferShader* GetBufferShaderPtr(const int bufferShaderIndex) override;
	virtual const PostEffectBufferShader* GetBufferShaderPtr(const int bufferShaderIndex) const override;

private:
	PostEffectUserObject* mUserObject;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// A user object for postfx effect, a set of user effect shaders
///  that is designed to be connected to post processing effect
/// </summary>
class PostEffectUserObject : public FBUserObject
{
	//--- FiLMBOX declaration.
	FBClassDeclare(PostEffectUserObject, FBUserObject)
	FBDeclareUserObject(PostEffectUserObject)

public:
	//! a constructor
	PostEffectUserObject(const char* pName = nullptr, HIObject pObject = nullptr);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate() override;        //!< FiLMBOX Creation function.
	virtual void FBDestroy() override;       //!< FiLMBOX Destruction function.


	void CopyValues(EffectShaderUserObject* pOther);

public: // PROPERTIES

	FBPropertyInt				UniqueClassId;
	FBPropertyAnimatableBool	Active;

	// texture is going to use the source chain size
	// TODO: customize the size, to have a downsample option, upsample option and custom defined size option

	FBPropertyListObject		OutputVideo; //!< in case of render to texture, let's expose it in the FBVideoMemory

	FBPropertyListObject		BufferShaders; //!< a set of post effect buffer shaders

	FBPropertyAction			ReloadShaders;
	FBPropertyBool				GenerateMipMaps;
	FBPropertyAction			ResetToDefault;

	FBPropertyBool				UseMasking;
	FBPropertyBaseEnum<EMaskingChannel>	MaskingChannel;

public:

	void DoReloadShaders();

	bool IsNeedToReloadShaders();
	void SetReloadShadersState(bool state);

	UserEffect* GetUserEffectPtr() const { return mUserEffect.get(); }

protected:

	
	FBSystem			mSystem;

	FBString			mText;
	bool				mReloadShaders{ false };

	std::unique_ptr<UserEffect>		mUserEffect;
	
	bool IsDepthSamplerUsed() const;
	bool IsLinearDepthSamplerUsed() const;

	static void ActionReloadShaders(HIObject pObject, bool value);

};
