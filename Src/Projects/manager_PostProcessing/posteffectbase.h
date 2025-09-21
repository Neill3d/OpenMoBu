
#pragma once

// posteffectbase
/*
Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "GL/glew.h"

#include "graphics_framebuffer.h"

#include "Framebuffer.h"
#include "glslShaderProgram.h"

#include "posteffectcontext.h"
#include "posteffectbuffershader.h"

// forward
class PostEffectBuffers;
class PostPersistentData;
class ScopedEffectBind;
class EffectShaderUserObject;

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

	bool Load(const char* shaderLocation);

	virtual bool IsReadyAndActive() const;

	// TODO: should it be a general FBComponent instead of pre-defined PostPersistentData user object ?!
	bool CollectUIValues(const IPostEffectContext* effectContext);

	// TODO: mask index is like a pre-defined input connection
	//! define internal mask channel index or -1 for default, it comes from a user input (UI)
	void SetMaskIndex(const int maskIndex) { mMaskIndex = maskIndex; }
	//! get defined mask channel index
	int GetMaskIndex() const { return mMaskIndex; }

	virtual bool IsDepthSamplerUsed() const;
	virtual bool IsLinearDepthSamplerUsed() const;
	virtual bool IsMaskSamplerUsed() const;
	virtual bool IsWorldNormalSamplerUsed() const;

	struct RenderEffectContext
	{
		PostEffectBuffers* buffers;

		// INPUT: input in the effects chain for this effect
		GLuint srcTextureId;
		GLuint depthTextureId;

		int viewWidth;
		int viewHeight;

		bool generateMips;

		// OUTPUT: write an effect composition to a given frame buffer
		FrameBuffer* dstFrameBuffer;
		int colorAttachment; //!< a way to define a color attachment in the dstFrameBuffer where we should render into
	};

	virtual void Process(const RenderEffectContext& renderContext, const IPostEffectContext* effectContext);

	virtual int GetNumberOfBufferShaders() const abstract;
	virtual PostEffectBufferShader* GetBufferShaderPtr(const int bufferShaderIndex) abstract;
	virtual const PostEffectBufferShader* GetBufferShaderPtr(const int bufferShaderIndex) const abstract;

protected:

	//std::string mName;

	int mMaskIndex{ -1 }; //!< which mask channel the effect is use (-1 for a default, globally defined mask channel)

	std::vector<std::unique_ptr<FrameBuffer>>	mFrameBuffers; // in case buffer shader is used to render it into texture
	std::vector<int> mBufferShaderVersions; // keep last processing buffer shader version, every resolution change is going to inc the version

	// in case of render to texture
	void InitializeFrameBuffers(int w, int h);

	void BindFrameBuffer(int bufferIndex);

	void UnBindFrameBuffer(int bufferIndex, bool generateMips);

	GLuint GetTextureTextureId(int bufferIndex) const;

	FrameBuffer* GetFrameBufferForBufferShader(const int shaderIndex);

	bool DoNeedIntermediateBuffers();

};


