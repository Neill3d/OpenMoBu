
#pragma once

// posteffectbase
/*
Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "GL/glew.h"

#include "graphics_framebuffer.h"

#include "glslShader.h"
#include "Framebuffer.h"

#include <memory>
#include <bitset>

// forward
class PostEffectBuffers;
class PostPersistentData;

namespace FBSDKNamespace
{
	class FBCamera;
}

//////////////////////////////

enum
{
	SHADER_TYPE_FISHEYE,
	SHADER_TYPE_COLOR,
	SHADER_TYPE_VIGNETTE,
	SHADER_TYPE_FILMGRAIN,
	SHADER_TYPE_LENSFLARE,
	SHADER_TYPE_SSAO,
	SHADER_TYPE_DOF,
	SHADER_TYPE_DISPLACEMENT,
	SHADER_TYPE_MOTIONBLUR,
	SHADER_TYPE_COUNT
};

enum class CompositionMask : uint32_t
{
	NO_MASK = 0,
	MASK_A = 0 >> 1,
	// TODO:
};

/// <summary>
/// uniforms needed for a common effect functionality, masking, clipping, etc.
/// </summary>
struct CommonEffectUniforms
{
public:

	void PrepareUniformLocations(GLSLShader* shader);

	/// <summary>
	/// must be called inside the binded glsl shader
	/// </summary>
	void UpdateUniforms(PostPersistentData* data);

	/// <summary>
	/// a property name in @sa PostPersistentData
	///  the property will be used to update useMasking glsl uniform value
	/// </summary>
	virtual const char* GetEnableMaskPropertyName() const = 0;

	/// <summary>
	/// glsl sampler slot binded for a mask texture
	/// </summary>
	static GLint GetMaskSamplerSlot() { return 4; }

protected:
	// common functionality of effects needs common uniforms
	GLint lowerClipLoc{ -1 };
	GLint upperClipLoc{ -1 };
	GLint useMaskLoc{ -1 };
};

struct PostEffectContext
{
	FBCamera* camera{ nullptr };
	int w{ 1 };
	int h{ 1 };
	int localFrame{ 0 };
	
	double sysTime{ 0.0 };
	double sysTimeDT{ 0.0 };

	double localTime{ 0.0 };
	double localTimeDT{ 0.0 };
};

/// <summary>
/// base class for every effect in a post processing chain
/// </summary>
struct PostEffectBase
{
public:

	//! a constructor
	PostEffectBase();
	//! a destructor
	virtual ~PostEffectBase();

	virtual int GetNumberOfShaders() const abstract; 

	virtual const char *GetName() const abstract;
	virtual const char *GetVertexFname(const int shaderIndex) const abstract;
	virtual const char *GetFragmentFname(const int shaderIndex) const abstract;

	/// load and initialize shader from a specified location
	bool Load(const int shaderIndex, const char *vname, const char *fname);

	virtual bool PrepUniforms(const int shaderIndex);
	virtual bool CollectUIValues(PostPersistentData* pData, PostEffectContext& effectContext);		//!< grab main UI values for the effect

	/// new feature to have several passes for a specified effect
	virtual const int GetNumberOfPasses() const;
	virtual bool PrepPass(const int pass);

	virtual void Bind();
	virtual void UnBind();

	GLSLShader *GetShaderPtr();

	void SetMaskIndex(const int maskIndex) { mMaskIndex = maskIndex; }
	int GetMaskIndex() const { return mMaskIndex; }

protected:

	int mCurrentShader{ 0 };
	std::vector<GLSLShader*>	mShaders;

	int mMaskIndex{ -1 }; //!< which mask channel the effect is use (-1 for a default, globally defined mask channel)

 	void SetCurrentShader(const int index) { mCurrentShader = index; }
	void FreeShaders();

};