
#pragma once

// posteffectbase
/*
Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectcontext.h"
#include "posteffectbuffershader.h"
#include "posteffect_rendercontext.h"

// forward
class FrameBuffer;
class PostEffectBuffers;
class PostPersistentData;
class ScopedEffectBind;
class EffectShaderUserObject;
class ShaderPropertyStorage;

//////////////////////////////

enum class BuildInEffect : uint8_t
{
	FISHEYE,
	COLOR,
	VIGNETTE,
	FILMGRAIN,
	LENSFLARE,
	SSAO,
	DOF,
	DISPLACEMENT,
	MOTIONBLUR,
	COUNT
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
namespace CommonEffect {
	inline constexpr GLint ColorSamplerSlot = 0;
	inline constexpr GLint DepthSamplerSlot = 1;
	inline constexpr GLint LinearDepthSamplerSlot = 2;
	inline constexpr GLint WorldNormalSamplerSlot = 3;
	/// <summary>
	/// glsl sampler slot binded for a mask texture
	/// </summary>
	inline constexpr GLint MaskSamplerSlot = 4;

	inline constexpr GLint UserSamplerSlot = 5; // for custom user texture input
};


/// <summary>
/// a set of effect buffer shaders to process the input and write output to the effects chain
///  chain processing is sequential, except scenarios
///  - one buffer shader request outputs from several other buffer shaders
///		- when buffer shader request one output from another buffer shader, that could be aligned in a sequntial order
///  - when buffer shader request a downscale, that could be grouped with connected buffer shaders with downscale and processed as a separate sequence and then mix into main chain
/// 
/// that means - effects chain have to be effects tree instead
/// </summary>
class PostEffectBase
{
public:

	//! a constructor
	PostEffectBase();
	//! a destructor
	virtual ~PostEffectBase();

	virtual bool IsActive() const abstract;

	//! an effect public name
	virtual const char* GetName() const abstract; // { return mName.c_str(); }

	virtual bool Load(const char* shaderLocation);

	virtual bool IsReadyAndActive() const;

	// TODO: should it be a general FBComponent instead of pre-defined PostPersistentData user object ?!
	// @param effectContext to get access to fx chain, shader property storage
	//  if we want custom rules to read from UI, we should write into shader property storage
	virtual bool CollectUIValues(IPostEffectContext* effectContext);

	// TODO: mask index is like a pre-defined input connection
	//! define internal mask channel index or -1 for default, it comes from a user input (UI)
	void SetMaskIndex(const int maskIndex) { mMaskIndex = maskIndex; }
	//! get defined mask channel index
	int GetMaskIndex() const { return mMaskIndex; }

	virtual bool IsDepthSamplerUsed() const;
	virtual bool IsLinearDepthSamplerUsed() const;
	virtual bool IsMaskSamplerUsed() const;
	virtual bool IsWorldNormalSamplerUsed() const;

	// render context can be modified to provide next free user texture slot
	// effect context can be modified by updating shader property texture slot values
	virtual void Process(PostEffectRenderContext& renderContext, IPostEffectContext* effectContext);

	bool HasAnySourceShaders(const IPostEffectContext* effectContext) const;
	bool HasAnySourceTextures(const IPostEffectContext* effectContext) const;

	// render context can be modified to provide next free user texture slot
	// effect context is for read/write, we modify user texture slots based on connected source shaders/textures
	bool PreProcessSourceShaders(PostEffectRenderContext& renderContext, IPostEffectContext* effectContext) const;
	bool PreProcessSourceTextures(PostEffectRenderContext& renderContext, IPostEffectContext* effectContext) const;

	virtual int GetNumberOfBufferShaders() const abstract;
	virtual PostEffectBufferShader* GetBufferShaderPtr(const int bufferShaderIndex) abstract;
	virtual const PostEffectBufferShader* GetBufferShaderPtr(const int bufferShaderIndex) const abstract;

protected:

	int mMaskIndex{ -1 }; //!< which mask channel the effect is use (-1 for a default, globally defined mask channel)
};


