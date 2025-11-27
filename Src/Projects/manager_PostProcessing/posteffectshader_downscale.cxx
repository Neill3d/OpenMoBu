
/**	\file	posteffectshader_downscale.cxx

Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectshader_downscale.h"
#include "shaderpropertywriter.h"
#include "mobu_logging.h"
#include <hashUtils.h>

uint32_t PostEffectShaderDownscale::SHADER_NAME_HASH = xxhash32(PostEffectShaderDownscale::SHADER_NAME);

PostEffectShaderDownscale::PostEffectShaderDownscale(FBComponent* uiComponent)
	: PostEffectBufferShader(uiComponent)
{
	mColorSampler = &AddProperty(ShaderProperty("color", "sampler"))
		.SetType(EPropertyType::TEXTURE)
		.SetDefaultValue(CommonEffect::ColorSamplerSlot);

	mTexelSize = &AddProperty(ShaderProperty("texelSize", "texelSize"))
		.SetType(EPropertyType::VEC2)
		.SetFlag(PropertyFlag::ShouldSkip, true);
}

//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
bool PostEffectShaderDownscale::OnCollectUI(IPostEffectContext* effectContext, int maskIndex)
{
	ShaderPropertyWriter writer(this, effectContext);
	writer(mTexelSize, 1.0f / static_cast<float>(effectContext->GetViewWidth()), 1.0f / static_cast<float>(effectContext->GetViewHeight()));
	//mTexelSize->SetValue(1.0f / static_cast<float>(effectContext->GetViewWidth()), 1.0f / static_cast<float>(effectContext->GetViewHeight()));
	return true;
}