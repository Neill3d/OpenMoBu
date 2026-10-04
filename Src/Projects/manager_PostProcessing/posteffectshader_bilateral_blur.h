
#pragma once

// posteffectshader_bilateral_blur
/*
Sergei <Neill3d> Solokhin 2018-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffect_shader.h"
#include "posteffect_shader_userobject.h"

// forward
class PostEffectShaderBilateralBlur;

/// <summary>
/// effect with once shader - bilateral blur
/// </summary>
//using PostEffectBilateralBlur = PostEffectSingleShader<PostEffectShaderBilateralBlur>;

/// <summary>
/// one single fragment shader that we do one number of passes to process the input
/// </summary>
class PostEffectShaderBilateralBlur : public PostEffectBufferShader
{
public:

	PostEffectShaderBilateralBlur(FBComponent* uiComponent=nullptr);
	virtual ~PostEffectShaderBilateralBlur() = default;

	/// number of variations of the same effect, but with a different algorithm (for instance, 3 ways of making a lens flare effect)
	virtual int GetNumberOfVariations() const override { return 1; }
	/// new feature to have several passes for a specified effect
	virtual int GetNumberOfPasses() const override { return 1; }

	//! an effect public name
	//! NOTE: public name can be assigned from the connected component
	const char* GetName() const override;
	uint32_t GetNameHash() const override;
	//! get a filename of vertex shader, for this effect. returns a relative filename
	const char* GetVertexFname(const int variationIndex) const override { return VERTEX_SHADER_FILE; }	
	//! get a filename of a fragment shader, for this effect, returns a relative filename
	const char* GetFragmentFname(const int variationIndex) const override { return FRAGMENT_SHADER_FILE; }

public:
	static constexpr const char* SHADER_NAME = "Gaussian Blur";
	static uint32_t SHADER_NAME_HASH;
	static constexpr const char* VERTEX_SHADER_FILE = "/GLSL/simple130.glslv";
	static constexpr const char* FRAGMENT_SHADER_FILE = "/GLSL/imageBlur.glslf";

	ShaderPropertyProxy ColorTexture;
	ShaderPropertyProxy BlurScale;

protected:

	// NOTE: masking property name can be assigned from the connected UI component
	virtual const char* GetUseMaskingPropertyName() const override;
	virtual const char* GetMaskingChannelPropertyName() const override;
	virtual bool DoPopulatePropertiesFromUniforms() const override { return false; }

	virtual void OnPopulateProperties(ShaderPropertyScheme* scheme) override;

private:
	
	FBComponent* mUIComponent{ nullptr };
};



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
	static constexpr const char* INPUT_TEXTURE_LABEL = "Input Texture";
	static constexpr const char* BLUR_SCALE_LABEL = "Blur Scale";
	
public:
	//! a constructor
	EffectShaderBilateralBlurUserObject(const char* pName = nullptr, HIObject pObject = nullptr);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate() override;        //!< FiLMBOX Creation function.
	
public: // PROPERTIES

	FBPropertyListObject	InputTexture;

	// make properties associated with a shader input connections

	FBPropertyVector2d		BlurScale;

protected:

	virtual PostEffectBufferShader* MakeANewClassInstance() override {
		return new PostEffectShaderBilateralBlur(this);
	}

};
