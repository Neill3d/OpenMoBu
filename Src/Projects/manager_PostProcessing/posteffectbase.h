
#pragma once

// posteffectbase
/*
Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "GL/glew.h"

#include "graphics_framebuffer.h"

#include "glslShaderProgram.h"
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

	void PrepareUniformLocations(GLSLShaderProgram* shader);

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

	/// number of variations of the same effect, but with a different algorithm (for instance, 3 ways of making a lens flare effect)
	virtual int GetNumberOfVariations() const abstract; 

	//! an effect public name
	virtual const char *GetName() const abstract;
	//! get a filename of vertex shader, for this effect. returns a relative filename
	virtual const char *GetVertexFname(const int variationIndex) const abstract;
	//! get a filename of a fragment shader, for this effect, returns a relative filename
	virtual const char *GetFragmentFname(const int variationIndex) const abstract;

	/// load and initialize shader from a specified location, vname and fname are computed absolute path
	bool Load(const int variationIndex, const char *vname, const char *fname);

	//! prepare uniforms for a given variation of the effect
	virtual bool PrepUniforms(const int variationIndex);
	//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
	virtual bool CollectUIValues(PostPersistentData* pData, PostEffectContext& effectContext);		//!< grab main UI values for the effect

	/// new feature to have several passes for a specified effect
	virtual const int GetNumberOfPasses() const;
	//! initialize a specific path for drawing
	virtual bool PrepPass(const int pass);

	//! bind effect shader program
	virtual void Bind();
	//! unbind effect shader program
	virtual void UnBind();

	//! get a pointer to a current shader program
	GLSLShaderProgram *GetShaderPtr();

	//! define internal mask channel index or -1 for default, it comes from a user input (UI)
	void SetMaskIndex(const int maskIndex) { mMaskIndex = maskIndex; }
	//! get defined mask channel index
	int GetMaskIndex() const { return mMaskIndex; }

protected:

	int mCurrentShader{ 0 };
	std::vector<GLSLShaderProgram*>	mShaders;

	int mMaskIndex{ -1 }; //!< which mask channel the effect is use (-1 for a default, globally defined mask channel)

 	void SetCurrentShader(const int index) { mCurrentShader = index; }
	void FreeShaders();

};