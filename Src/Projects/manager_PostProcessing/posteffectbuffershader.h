#pragma once

/**	\file	posteffectbuffershader.h

Sergei <Neill3d> Solokhin 2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

#include "effectshaderconnections.h"

#include "Framebuffer.h"
#include "glslShaderProgram.h"
#include <memory>

// forward
class PostEffectBuffers;

/// <summary>
/// one single fragment shader that we do one number of passes to process the input
/// </summary>
class PostEffectBufferShader : public IEffectShaderConnections
{
public:

	PostEffectBufferShader(FBComponent* ownerComponent);
	virtual ~PostEffectBufferShader();

	FBComponent* GetOwner() const { return mOwner; }

	/// number of variations of the same effect, but with a different algorithm (for instance, 3 ways of making a lens flare effect)
	virtual int GetNumberOfVariations() const abstract;

	//! an effect public name
	virtual const char* GetName() const abstract;
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
	bool Load(const int variationIndex, const char* vname, const char* fname);

	/// <summary>
	/// use \ref GetVertexFname and \ref GetFragmentFname to load a shader variance
	///  the given shaderLocation is used to make an absolute path
	/// </summary>
	bool Load(const char* shaderLocation);

	//! is being called after \ref Load is succeed
	//!  so we could initialized some property or system uniform locations
	bool PrepUniforms(const int variationIndex);

	/// <summary>
	/// is being called right before Render and when shader is binded
	/// </summary>
	void UploadUniforms(PostEffectBuffers* buffers, FrameBuffer* dstBuffer, int colorAttachment, const GLuint inputTextureId, int w, int h, bool generateMips, const IPostEffectContext* effectContext);

	/// repeated call of the shader (define iPass uniform to distinguish)
	virtual const int GetNumberOfPasses() const;

	//! get a pointer to a (current variance) shader program
	GLSLShaderProgram* GetShaderPtr();
	const GLSLShaderProgram* GetShaderPtr() const;

	/// <summary>
	/// the given buffer shader will process the given inputTextureId and write result into dst frame buffer
	/// </summary>
	void Render(PostEffectBuffers* buffers, FrameBuffer* dstBuffer, int colorAttachment, const GLuint inputTextureId, int w, int h, bool generateMips, const IPostEffectContext* effectContext);

	// means that processing will use smaller size of a buffer
	void SetDownscaleMode(const bool value);
	bool IsDownscaleMode() const { return isDownscale; }

	// shader version, increments on every shader reload
	int GetVersion() const { return version; }

	// binded textures for connected buffers starts from 5, then custom user textures will start from 10
	static int GetUserSamplerId() { return 5; }

public:
	//
	// IEffectShaderConnections
	virtual ShaderProperty& AddProperty(const ShaderProperty& property) override;
	virtual ShaderProperty& AddProperty(ShaderProperty&& property) override;

	virtual int GetNumberOfProperties() override;
	virtual ShaderProperty& GetProperty(int index) override;
	virtual ShaderProperty* FindProperty(const std::string& name) override;

	// TODO: search for locations

	void MakeCommonProperties();

	int MakePropertyLocationsFromShaderUniforms();
	int MakeSystemLocationsFromShaderUniforms();

	int PopulatePropertiesFromShaderUniforms();

	// TODO: upload properties values into uniforms

	void AutoUploadUniforms(PostEffectBuffers* buffers, const GLuint inputTextureId, int w, int h, bool generateMips, const IPostEffectContext* effectContext);

	// TODO: auto update values from fb properties

	// TODO: let's move it for buildin shaders only ?!
	//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
	bool CollectUIValues(const IPostEffectContext* effectContext, int maskIndex) override;		//!< grab main UI values for the effect

	bool CollectUIValues(FBComponent* component, const IPostEffectContext* effectContext, int maskIndex);

	bool ReloadPropertyShaders();

protected:

	std::unordered_map<std::string, ShaderProperty>		mProperties;

	//! a callback event to process a property added, so that we could make and associate component's FBProperty with it
	virtual void OnPropertyAdded(ShaderProperty& property)
	{
	}

protected:

	static const char* gSystemUniformNames[static_cast<int>(ShaderSystemUniform::COUNT)];
	GLint mSystemUniformLocations[static_cast<int>(ShaderSystemUniform::COUNT)];

	void	ResetSystemUniformLocations();
	int		IsSystemUniform(const char* uniformName); // -1 if not found, or return an index of a system uniform in the ShaderSystemUniform enum
	void	BindSystemUniforms(const IPostEffectContext* effectContext) const;

protected:

	FBComponent* mOwner{ nullptr }; //!< scene component which used to communicate with a user and a scene

	bool isDownscale{ false };
	int version; //!< keep track of resolution modifications, inc version everytime we change resolution
	int mCurrentShader{ 0 }; //!< current variance of a shader
	std::vector<std::unique_ptr<GLSLShaderProgram>>	mShaders; //!< store a list of all variances

	void SetCurrentShader(const int index) { mCurrentShader = index; }
	void FreeShaders();

	//!< TODO: masking property in the UI, should we move it into input connection ?!
	virtual const char* GetUseMaskingPropertyName() const = 0;
	virtual const char* GetMaskingChannelPropertyName() const = 0;
	//!< if true, once shader is loaded, let's inspect all the uniforms and make properties from them
	virtual bool DoPopulatePropertiesFromUniforms() const = 0;

	virtual bool OnPrepareUniforms(const int variationIndex) { return true; }
	virtual bool OnCollectUI(const IPostEffectContext* effectContext, int maskIndex) { return true; }
	virtual void OnUploadUniforms(PostEffectBuffers* buffers, FrameBuffer* dstBuffer, int colorAttachment, const GLuint inputTextureId, int w, int h, bool generateMips, const IPostEffectContext* effectContext)
	{
	}

	//! bind effect shader program
	virtual void Bind();
	//! unbind effect shader program
	virtual void UnBind();

	//! initialize a specific path for drawing
	virtual bool PrepPass(const int pass, int w, int h);

	void RenderPass(int passIndex, FrameBuffer* dstBuffer, int colorAttachment, const GLuint inputTextureId, int w, int h, bool generateMips);
};
