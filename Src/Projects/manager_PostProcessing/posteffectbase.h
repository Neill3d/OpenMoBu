
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
class ScopedEffectBind;

namespace FBSDKNamespace
{
	class FBCamera;
}

//////////////////////////////

enum class BuildInEffect
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
class CommonEffectUniforms
{
public:

	void PrepareUniformLocations(GLSLShaderProgram* shader);

	/// <summary>
	/// collect common data values into a buffer, /see UploadCommonData to upload values into gpu shader
	/// </summary>
	void CollectCommonData(PostPersistentData* data, const char* enableMaskingPropertyName);

	/// <summary>
	/// glsl shader must be binded, upload uniforms from collected common data values
	/// </summary>
	void UploadCommonData();

	/// <summary>
	/// a property name in @sa PostPersistentData
	///  the property will be used to update useMasking glsl uniform value
	/// </summary>
	//virtual const char* GetEnableMaskPropertyName() const = 0;


	static GLint GetColorSamplerSlot() { return 0; }

	static GLint GetDepthSamplerSlot() { return 1; }

	static GLint GetLinearDepthSamplerSlot() { return 2; }

	/// <summary>
	/// glsl sampler slot binded for a mask texture
	/// </summary>
	static GLint GetMaskSamplerSlot() { return 4; }

protected:
	// common functionality of effects needs common uniforms
	GLint lowerClipLoc{ -1 };
	GLint upperClipLoc{ -1 };
	GLint useMaskLoc{ -1 };

	struct CommonShaderData
	{
		float useMasking{ 0.0f };
		float lowerClip{ 0.0f };
		float upperClip{ 0.0f };
		float pad{ 0.0f };
	} mCommonShaderData;
};

struct PostEffectContext
{
	FBCamera* camera{ nullptr };
	int w{ 1 }; //!< viewport width
	int h{ 1 }; //!< viewport height
	int localFrame{ 0 }; //!< playback frame number
	
	double sysTime{ 0.0 }; //!< system time (in seconds)
	double sysTimeDT{ 0.0 };

	double localTime{ 0.0 }; //!< playback time (in seconds)
	double localTimeDT{ 0.0 };
};

/// <summary>
/// one single fragment shader that we do one number of passes to process the input
/// </summary>
class PostEffectBufferShader
{
public:

	PostEffectBufferShader();
	virtual ~PostEffectBufferShader();

	/// number of variations of the same effect, but with a different algorithm (for instance, 3 ways of making a lens flare effect)
	virtual int GetNumberOfVariations() const abstract;

	//! an effect public name
	virtual const char* GetName() const abstract;
	//! get a filename of vertex shader, for this effect. returns a relative filename
	virtual const char* GetVertexFname(const int variationIndex) const abstract;
	//! get a filename of a fragment shader, for this effect, returns a relative filename
	virtual const char* GetFragmentFname(const int variationIndex) const abstract;

	// does shader uses the scene depth sampler (part of a system input)
	virtual bool IsDepthSamplerUsed() const { return false; }
	// does shader uses the scene linear depth sampler (part of a system input)
	virtual bool IsLinearDepthSamplerUsed() const { return false; }

	/// load and initialize shader from a specified location, vname and fname are computed absolute path
	bool Load(const int variationIndex, const char* vname, const char* fname);

	bool Load(const char* shaderLocation);

	//! prepare uniforms for a given variation of the effect
	virtual bool PrepUniforms(const int variationIndex);
	//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
	virtual bool CollectUIValues(PostPersistentData* pData, PostEffectContext& effectContext, int maskIndex);		//!< grab main UI values for the effect

	//! upload collected data values into gpu shader
	virtual void UploadUniforms() {};

	/// new feature to have several passes for a specified effect
	virtual const int GetNumberOfPasses() const;
	
	//! get a pointer to a current shader program
	GLSLShaderProgram* GetShaderPtr();

	/// <summary>
	/// the given buffer shader will process the given inputTextureId and write result into dst frame buffer
	/// </summary>
	void Render(PostEffectBuffers* buffers, FrameBuffer* dstBuffer, const GLuint inputTextureId, int w, int h, bool generateMips);

	void SetDownscaleMode(const bool value);
	bool IsDownscaleMode() const { return isDownscale; }
	int GetVersion() const { return version; }

protected:
	bool isDownscale{ false };
	int version; //!< keep track of resolution modifications, inc version everytime we change resolution
	int mCurrentShader{ 0 };
	std::vector<std::unique_ptr<GLSLShaderProgram>>	mShaders;

	
	void SetCurrentShader(const int index) { mCurrentShader = index; }
	void FreeShaders();

	//! initialize a specific path for drawing
	virtual bool PrepPass(const int pass, int w, int h);

	//! bind effect shader program
	virtual void Bind();
	//! unbind effect shader program
	virtual void UnBind();

	void RenderPass(int passIndex, FrameBuffer* dstBuffer, const GLuint inputTextureId, int w, int h, bool generateMips);
};

/// <summary>
/// a set of effect buffer shaders to process the input and write output to the effects chain
/// </summary>
class PostEffectBase
{
public:

	//! a constructor
	PostEffectBase();
	//! a destructor
	virtual ~PostEffectBase();

	//! an effect public name
	virtual const char* GetName() const abstract; // { return mName.c_str(); }

	bool Load(const char* shaderLocation);

	bool CollectUIValues(PostPersistentData* pData, PostEffectContext& effectContext);

	//! define internal mask channel index or -1 for default, it comes from a user input (UI)
	void SetMaskIndex(const int maskIndex) { mMaskIndex = maskIndex; }
	//! get defined mask channel index
	int GetMaskIndex() const { return mMaskIndex; }

	virtual bool IsDepthSamplerUsed() const;
	virtual bool IsLinearDepthSamplerUsed() const;

	struct EffectContext
	{
		// input in the effects chain for this effect
		GLuint srcTextureId;
		GLuint depthTextureId;

		PostEffectBuffers* buffers;

		// write an effect composition to a given frame buffer
		FrameBuffer* dstFrameBuffer;

		int viewWidth;
		int viewHeight;

		bool generateMips;
	};

	virtual void Process(const EffectContext& context);

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

/// <summary>
/// this is for cases when effect contains of one buffer shader execution and directly output to effects chain buffer
/// </summary>
/// <typeparam name="T"></typeparam>
template<typename T>
class PostEffectSingleShader : public PostEffectBase
{
public:
	PostEffectSingleShader()
		: PostEffectBase()
		, mBufferShader(std::make_unique<T>())
	{}
	virtual ~PostEffectSingleShader()
	{}

	virtual const char* GetName() const override
	{
		return mBufferShader->GetName();
	}

	virtual int GetNumberOfBufferShaders() const override { return 1; }
	virtual PostEffectBufferShader* GetBufferShaderPtr(const int bufferShaderIndex) override { return mBufferShader.get(); }
	virtual const PostEffectBufferShader* GetBufferShaderPtr(const int bufferShaderIndex) const override { return mBufferShader.get(); }

protected:

	std::unique_ptr<T>		mBufferShader;
};


// TODO: effect mix shaders with input - for effect which do processing on input and then a computed output mix it up with input source

/*
class ScopedEffectBind {
public:
	ScopedEffectBind(PostEffectBase* effect) : mEffect(effect) { mEffect->Bind(); }
	~ScopedEffectBind() { mEffect->UnBind(); }
private:
	PostEffectBase* mEffect;
};
*/
