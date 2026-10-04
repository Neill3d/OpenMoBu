
/**	\file	posteffectshader_bilateral_blur.cxx

Sergei <Neill3d> Solokhin 2018-2026

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
{}

const char* PostEffectShaderBilateralBlur::GetName() const 
{
	if (FBIS(mUIComponent, EffectShaderBilateralBlurUserObject))
	{
		EffectShaderBilateralBlurUserObject* userObject = FBCast<EffectShaderBilateralBlurUserObject>(mUIComponent);
		return userObject->LongName;
	}
	return SHADER_NAME; 
}
uint32_t PostEffectShaderBilateralBlur::GetNameHash() const 
{
	if (FBIS(mUIComponent, EffectShaderBilateralBlurUserObject))
	{
		EffectShaderBilateralBlurUserObject* userObject = FBCast<EffectShaderBilateralBlurUserObject>(mUIComponent);
		const char* longName = userObject->LongName;
		return xxhash32(longName);
	}
	return SHADER_NAME_HASH; 
}

const char* PostEffectShaderBilateralBlur::GetUseMaskingPropertyName() const
{
	if (FBIS(mUIComponent, EffectShaderBilateralBlurUserObject))
	{
		EffectShaderBilateralBlurUserObject* userObject = FBCast<EffectShaderBilateralBlurUserObject>(mUIComponent);
		return userObject->UseMasking.GetName();
	}
	return nullptr;
}
const char* PostEffectShaderBilateralBlur::GetMaskingChannelPropertyName() const
{
	if (FBIS(mUIComponent, EffectShaderBilateralBlurUserObject))
	{
		EffectShaderBilateralBlurUserObject* userObject = FBCast<EffectShaderBilateralBlurUserObject>(mUIComponent);
		return userObject->MaskingChannel.GetName();
	}
	return nullptr;
}

void PostEffectShaderBilateralBlur::OnPopulateProperties(ShaderPropertyScheme* scheme)
{
	if (FBIS(mUIComponent, EffectShaderBilateralBlurUserObject))
	{
		EffectShaderBilateralBlurUserObject* userObject = FBCast<EffectShaderBilateralBlurUserObject>(mUIComponent);

		ShaderProperty textureProperty(EffectShaderBilateralBlurUserObject::INPUT_TEXTURE_LABEL, "colorSampler", EPropertyType::TEXTURE, &userObject->InputTexture);
		ColorTexture = scheme->AddProperty(std::move(textureProperty))
			.SetDefaultValue(CommonEffect::ColorSamplerSlot)
			.GetProxy();

		ShaderProperty blurScaleProperty(EffectShaderBilateralBlurUserObject::BLUR_SCALE_LABEL, "scale", EPropertyType::VEC2, &userObject->BlurScale);
		BlurScale = scheme->AddProperty(std::move(blurScaleProperty)).GetProxy();
	}
	else
	{
		ColorTexture = scheme->AddProperty(ShaderProperty("color", "colorSampler"))
			.SetType(EPropertyType::TEXTURE)
			.SetDefaultValue(CommonEffect::ColorSamplerSlot)
			.GetProxy();

		BlurScale = scheme->AddProperty(ShaderProperty("scale", "scale"))
			.SetType(EPropertyType::VEC2)
			.SetFlag(PropertyFlag::SKIP, true)
			.GetProxy();
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
	VertexFile = PostEffectShaderBilateralBlur::VERTEX_SHADER_FILE;
	VertexFile.ModifyPropertyFlag(FBPropertyFlag::kFBPropertyFlagReadOnly, true);
	FragmentFile = PostEffectShaderBilateralBlur::FRAGMENT_SHADER_FILE;
	FragmentFile.ModifyPropertyFlag(FBPropertyFlag::kFBPropertyFlagReadOnly, true);
	NumberOfPasses.ModifyPropertyFlag(FBPropertyFlag::kFBPropertyFlagReadOnly, true);
	UniqueClassId = 63;

	return true;
}