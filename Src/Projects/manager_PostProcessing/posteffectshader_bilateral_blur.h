
#pragma once

// posteffectshader_bilateral_blur
/*
Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "GL/glew.h"

#include "graphics_framebuffer.h"

#include "glslShaderProgram.h"
#include "Framebuffer.h"

#include "posteffectbase.h"
#include "posteffect_shader_userobject.h"

#include <memory>
#include <bitset>

/// <summary>
/// one single fragment shader that we do one number of passes to process the input
/// </summary>
class PostEffectShaderBilateralBlur : public PostEffectBufferShader
{
public:

	PostEffectShaderBilateralBlur(FBComponent* uiComponent=nullptr);
	virtual ~PostEffectShaderBilateralBlur();

	/// number of variations of the same effect, but with a different algorithm (for instance, 3 ways of making a lens flare effect)
	virtual int GetNumberOfVariations() const override { return 1; }

	//! an effect public name
	virtual const char* GetName() const override;
	//! get a filename of vertex shader, for this effect. returns a relative filename
	virtual const char* GetVertexFname(const int variationIndex) const override;
	//! get a filename of a fragment shader, for this effect, returns a relative filename
	virtual const char* GetFragmentFname(const int variationIndex) const override;

	/// new feature to have several passes for a specified effect
	virtual const int GetNumberOfPasses() const override;
	//! initialize a specific path for drawing
	virtual bool PrepPass(const int pass, int w, int h) override;

	

	// could be input layer (slot 0), depth texture or user textures from connected texture of effect shader
	void SetTextureId(GLint textureId);
	void SetScale(const FBVector2d& scale);

protected:
	//! is called once the shader is loaded
	virtual bool OnPrepareUniforms(const int variationIndex) override;
	//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
	virtual bool OnCollectUI(const IPostEffectContext* effectContext, int maskIndex) override;		//!< grab main UI values for the effect

private:
	GLint		mColorSamplerLoc{ -1 };
	GLint		mLocImageBlurScale{ -1 };

	FBVector2d	mBlurMaskScale;
	GLint		mTextureId; //!< a binded texture, that we are going to blur

	FBComponent* mUIComponent{ nullptr };
};

/// <summary>
/// effect with once shader - bilateral blur
/// </summary>
typedef PostEffectSingleShader<PostEffectShaderBilateralBlur> PostEffectBilateralBlur;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// A user object for one shader user object
///  that is designed to be connected to post processing effect
/// </summary>
class EffectShaderBilateralBlurUserObject : public EffectShaderUserObject
{
	//--- FiLMBOX declaration.
	FBClassDeclare(EffectShaderBilateralBlurUserObject, EffectShaderUserObject)
		FBDeclareUserObject(EffectShaderBilateralBlurUserObject)

public:
	//! a constructor
	EffectShaderBilateralBlurUserObject(const char* pName = nullptr, HIObject pObject = nullptr);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate() override;        //!< FiLMBOX Creation function.
	
public: // PROPERTIES

	FBPropertyListObject	InputTexture;

	// make properties associated with a shader input connections

	FBPropertyVector2d		BlurScale;

	// TODO: could be connected to buildin texture ids, scene texture or another rendered effect shader
	//FBPropertyEnum<EShaderInputSampler2D> InputTexture;

protected:

	virtual PostEffectBufferShader* MakeANewClassInstance() override {
		return new PostEffectShaderBilateralBlur(this);
	}

};
