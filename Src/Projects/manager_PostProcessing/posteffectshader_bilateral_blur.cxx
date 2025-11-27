
/**	\file	posteffectshader_bilateral_blur.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectshader_bilateral_blur.h"
#include "postpersistentdata.h"
#include "mobu_logging.h"
#include <hashUtils.h>

//--- FiLMBOX Registration & Implementation.
FBClassImplementation(EffectShaderBilateralBlurUserObject);
FBUserObjectImplement(EffectShaderBilateralBlurUserObject,
	"Effect shader for a bilateral (gaussian) blur",
	"cam_switcher_toggle.png");                                          //Register UserObject class
PostEffectFBElementClassImplementation(EffectShaderBilateralBlurUserObject, "Blur Shader", "cam_switcher_toggle.png");                  //Register to the asset system


/////////////////////////////////////////////////////////////////////////
// PostEffectShaderBilateralBlur

uint32_t PostEffectShaderBilateralBlur::SHADER_NAME_HASH = xxhash32(PostEffectShaderBilateralBlur::SHADER_NAME);

PostEffectShaderBilateralBlur::PostEffectShaderBilateralBlur(FBComponent* uiComponent)
	: PostEffectBufferShader(uiComponent)
	, mUIComponent(uiComponent)
{
	if (FBIS(uiComponent, EffectShaderBilateralBlurUserObject))
	{
		EffectShaderBilateralBlurUserObject* userObject = FBCast<EffectShaderBilateralBlurUserObject>(uiComponent);

		ShaderProperty textureProperty(EffectShaderBilateralBlurUserObject::INPUT_TEXTURE_LABEL, "colorSampler", EPropertyType::TEXTURE, &userObject->InputTexture);
		ColorTexture = &AddProperty(std::move(textureProperty))
			.SetDefaultValue(CommonEffect::ColorSamplerSlot);

		ShaderProperty blurScaleProperty(EffectShaderBilateralBlurUserObject::BLUR_SCALE_LABEL, "scale", EPropertyType::VEC2, &userObject->BlurScale);
		BlurScale = &AddProperty(std::move(blurScaleProperty));
	}
	else
	{
		ColorTexture = &AddProperty(ShaderProperty("color", "colorSampler"))
			.SetType(EPropertyType::TEXTURE)
			.SetDefaultValue(CommonEffect::ColorSamplerSlot);

		BlurScale = &AddProperty(ShaderProperty("scale", "scale"))
			.SetType(EPropertyType::VEC2)
			.SetFlag(PropertyFlag::ShouldSkip, true);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// EffectShaderBilateralBlurUserObject

EffectShaderBilateralBlurUserObject::EffectShaderBilateralBlurUserObject(const char* pName, HIObject pObject)
	: ParentClass(pName, pObject)
{
	FBClassInit;
}

/************************************************
 *  FiLMBOX Constructor.
 ************************************************/
bool EffectShaderBilateralBlurUserObject::FBCreate()
{
	ParentClass::FBCreate();

	FBPropertyPublish(this, InputTexture, INPUT_TEXTURE_LABEL, nullptr, nullptr);
	FBPropertyPublish(this, BlurScale, BLUR_SCALE_LABEL, nullptr, nullptr);

	BlurScale = FBVector2d(1.0, 1.0);
	ShaderFile = PostEffectShaderBilateralBlur::FRAGMENT_SHADER_FILE;
	ShaderFile.ModifyPropertyFlag(FBPropertyFlag::kFBPropertyFlagReadOnly, true);
	NumberOfPasses.ModifyPropertyFlag(FBPropertyFlag::kFBPropertyFlagReadOnly, true);
	UniqueClassId = 63;

	return true;
}
