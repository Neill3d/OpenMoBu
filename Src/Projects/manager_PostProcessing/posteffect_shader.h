#pragma once

/**	\file	posteffect_shader.h

Sergei <Neill3d> Solokhin 2025-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

#include "posteffect_rendercontext.h"
#include <memory>
#include <unordered_map>

// forward
class FrameBuffer;
class PostEffectBuffers;
class GLSLShaderProgram;
class ShaderPropertyStorage;
class PostEffectBufferShader;

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

// evaluate the property scheme, read property values and store them into effect shader property value storage
class EffectShaderPropertyProcessor
{
public:

	//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
	// in case of evaluation, the method must be thread-safe
	// the property values should be written into the ShaderPropertyStorage (effectContext)
	bool CollectUIValues(FBComponent* component, PostEffectContextProxy* effectContext, const PostEffectBufferShader* effectShader, int maskIndex);

private:

};

/// <summary>
/// effect with one or more gpu shaders (number of variations, mostly 1)
/// to process the effects chain input image with a defined number of passes
/// </summary>
class PostEffectBufferShader
{
public:

	PostEffectBufferShader(FBComponent* ownerComponent);
	virtual ~PostEffectBufferShader();

	FBComponent* GetOwner() const { return mOwner; }

	bool IsActive() const { return isActive; }
	void SetActive(const bool value) { isActive = value; }

	/// number of variations of the same effect, but with a different algorithm (for instance, 3 ways of making a lens flare effect)
	virtual int GetNumberOfVariations() const abstract;

	/// repeated call of the shader (define iPass uniform to distinguish)
	virtual int GetNumberOfPasses() const { return 1; }

	//! an effect public name
	virtual const char* GetName() const abstract;
	virtual uint32_t GetNameHash() const abstract;
	//! get a filename of vertex shader, for this effect. returns a relative filename
	virtual const char* GetVertexFname(const int variationIndex) const abstract;
	//! get a filename of a fragment shader, for this effect, returns a relative filename
	virtual const char* GetFragmentFname(const int variationIndex) const abstract;

	// does shader uses the scene depth sampler (part of a system input)
	virtual bool IsDepthSamplerUsed() const;
	// does shader uses the scene linear depth sampler (part of a system input)
	virtual bool IsLinearDepthSamplerUsed() const;

	virtual bool IsMaskSamplerUsed() const;
	virtual bool IsWorldNormalSamplerUsed() const;

	/// load and initialize shader from a specified location, vname and fname are computed absolute path
	bool Load(const int variationIndex, const char* vname, const char* fname, bool useShaderToyCompatibility=false);

	/// <summary>
	/// use \ref GetVertexFname and \ref GetFragmentFname to load a shader variance
	///  the shaderLocation is calculated from system paths, scene current path
	/// </summary>
	bool Load(const char* shaderLocation, bool useShaderToyCompatibility=false);

	bool Load();

	//! get a pointer to a (current variance) shader program
	GLSLShaderProgram* GetShaderPtr();
	const GLSLShaderProgram* GetShaderPtr() const;

	/// <summary>
	/// the given buffer shader will process the given inputTextureId and write result into dst frame buffer
	/// @param dstBuffer defines which buffer render into
	/// @param colorAttachment defines which color attachment of the dstBuffer to render into
	/// @param inputTextureId defines the input texture to process
	/// </summary>
	void Render(PostEffectRenderContext& renderContext, PostEffectContextProxy* effectContext);

	// number of shaders in properties that are used as sources for this effect
	int GetNumberOfSourceShaders(const PostEffectContextProxy* effectContext) const;
	bool HasAnySourceShaders(const PostEffectContextProxy* effectContext) const;

	bool HasAnySourceTextures(const PostEffectContextProxy* effectContext) const;

	// get a source effect by index
	using SourceShadersMapConst = std::vector<const ShaderPropertyValue*>;
	const SourceShadersMapConst GetSourceShadersConst(PostEffectContextProxy* effectContext) const;

	using SourceShadersMap = std::vector<ShaderPropertyValue*>;
	SourceShadersMap GetSourceShaders(PostEffectContextProxy* effectContext) const;

	using SourceTexturesMap = std::vector<ShaderPropertyValue*>;
	SourceTexturesMap GetSourceTextures(PostEffectContextProxy* effectContext) const;

	// means that processing will use smaller size of a buffer
	void SetDownscaleMode(const bool value);
	bool IsDownscaleMode() const { return isDownscale; }

	// shader version, increments on every shader reload
	int GetVersion() const { return version; }

	// TODO: mask index is like a pre-defined input connection
	//! define internal mask channel index or -1 for default, it comes from a user input (UI)
	void SetMaskIndex(const int maskIndex) { mMaskIndex = maskIndex; }
	//! get defined mask channel index
	int GetMaskIndex() const { return mMaskIndex; }

protected:

	bool CheckShadersPath(const char* path) const;

protected:
	friend class EffectShaderPropertyProcessor;
	friend class PostEffectRenderContext;

	
	std::unique_ptr<const ShaderPropertyScheme> mRenderPropertyScheme; // the property scheme to use in render thread

	ShaderPropertyProxy UseMaskingProperty;
	
	// register common properties, like use masking, masking channel, top/bottom clipping, etc.
	void MakeCommonProperties(ShaderPropertyScheme* scheme);

	// register properties in the scheme, properties that could be not directly shader uniforms
	virtual void OnPopulateProperties(ShaderPropertyScheme* scheme) {}

	// user object can make new fb properties according to populated property scheme
	virtual void OnPropertySchemeUpdated(const ShaderPropertyScheme* newScheme, const ShaderPropertyScheme* oldScheme) {}

public:
	
	const ShaderPropertyScheme* GetPropertySchemePtr() const { return mRenderPropertyScheme.get(); }

	virtual int GetNumberOfProperties() const;
	virtual const ShaderProperty& GetProperty(int index) const;
	virtual const ShaderProperty* FindProperty(const std::string_view name) const;
	const ShaderProperty* FindPropertyByUniformName(const std::string_view name) const;

	//! is being called after \ref Load is succeed
	//!  so we could initialized some property or system uniform locations
	bool InitializeUniforms(ShaderPropertyScheme* scheme, int variationIndex);

	// apply default values to a shader uniforms
	void UploadDefaultValues(ShaderPropertyScheme* scheme);

	/**
	* When one of the uniforms is a texture which is connected to a result of another effect,
	* then in this procedure we are going to trigger the render of that effect to have the texture ready
	* @param skipTextureProperties can be useful for multipass uniform update, when textures are already bound
	*/
	void AutoUploadUniforms(const PostEffectRenderContext& renderContext,
		const PostEffectContextProxy* effectContext, bool skipTextureProperties);

protected:

	void	BindSystemUniforms(const PostEffectContextProxy* effectContext) const;
	void	TryUseGlobalMasking(const PostEffectContextProxy* effectContext) const;

	inline GLint GetSystemUniformLoc(ShaderSystemUniform u) const noexcept {
		return mRenderPropertyScheme->GetSystemUniformLoc(u);
	}

protected:

	// variances of post effect

	int mCurrentVariation{ 0 }; //!< current variance of a shader program
	bool bIsNeedToUpdatePropertyScheme{ false };
	bool isActive{ true };
	// variations of shader program for the given effect
	std::vector<std::unique_ptr<GLSLShaderProgram>>	mVariations;

	void SetCurrentVariation(const int index);
	inline int GetCurrentVariation() const noexcept { return mCurrentVariation; }
	void FreeShaders();

	friend class ShaderPropertyWriter;

protected:

	FBComponent* mOwner{ nullptr }; //!< scene component which used to communicate with a user and a scene

	bool isDownscale{ false };
	int version{ 0 }; //!< keep track of resolution modifications, inc version everytime we change resolution
	
	int mMaskIndex{ -1 }; //!< which mask channel the effect is use (-1 for a default, globally defined mask channel)

	//!< TODO: masking property in the UI, should we move it into input connection ?!
	virtual const char* GetUseMaskingPropertyName() const = 0;
	virtual const char* GetMaskingChannelPropertyName() const = 0;
	//!< if true, once shader is loaded, let's inspect all the uniforms and make properties from them
	virtual bool DoPopulatePropertiesFromUniforms() const = 0;

	
	// should be thread safe, write into effect context
	virtual bool OnCollectUI(PostEffectContextProxy* effectContext, int maskIndex) const { return true; }
	virtual void OnUniformsUploaded(int passIndex) {}

	//! bind effect shader program
	virtual bool Bind();
	//! unbind effect shader program
	virtual void UnBind();

	// we going to render all input connected effect shaders and prepare input connected textures
	void PreRender(PostEffectRenderContext& renderContextParent, PostEffectContextProxy* effectContext) const;

	// call at the beginning of the Render method
	virtual void OnRenderBegin(PostEffectRenderContext& renderContextParent, PostEffectContextProxy* effectContext) {}

	//! derived classes could have own preparation steps before each pass
	virtual bool OnRenderPassBegin(int passIndex, PostEffectRenderContext& renderContext, PostEffectContextProxy* effectContext) { return true; }
	
	virtual void RenderPass(int passIndex, PostEffectRenderContext& renderContext, PostEffectContextProxy* effectContext);

	// call at the end of the Render method
	virtual void OnRenderEnd(PostEffectRenderContext& renderContextParent, PostEffectContextProxy* effectContext) {}
};
