
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

#define POSTEFFECT_USEROBJECT__CLASSSTR		"PostEffectUserObject"

// forward
class PostEffectUserObject;
class EffectShaderUserObject;

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

	virtual const char* GetName() const override;

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

	// final shader -  have to be last shader that will use a result of previous buffer shaders, mix them up and write into dst buffer
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
