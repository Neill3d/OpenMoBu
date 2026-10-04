
/**	\file	posteffectshader_downscale.cxx

Sergei <Neill3d> Solokhin 2018-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectshader_downscale.h"
#include "shaderproperty_writer.h"
#include "mobu_logging.h"
#include <hashUtils.h>

uint32_t PostEffectShaderDownscale::SHADER_NAME_HASH = xxhash32(PostEffectShaderDownscale::SHADER_NAME);

PostEffectShaderDownscale::PostEffectShaderDownscale(FBComponent* uiComponent)
	: PostEffectBufferShader(uiComponent)
{}

void PostEffectShaderDownscale::OnPopulateProperties(ShaderPropertyScheme* scheme)
{
	mColorSampler = scheme->AddProperty(ShaderProperty("color", "sampler"))
		.SetType(EPropertyType::TEXTURE)
		.SetDefaultValue(CommonEffect::ColorSamplerSlot)
		.GetProxy();

	mTexelSize = scheme->AddProperty(ShaderProperty("texelSize", "texelSize"))
		.SetType(EPropertyType::VEC2)
		.SetFlag(PropertyFlag::SKIP, true)
		.GetProxy();
}

//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
bool PostEffectShaderDownscale::OnCollectUI(PostEffectContextProxy* effectContext, int maskIndex) const
{
	ShaderPropertyWriter writer(this, effectContext);
	writer(mTexelSize, 1.0f / static_cast<float>(effectContext->GetViewWidth()), 1.0f / static_cast<float>(effectContext->GetViewHeight()));
	return true;
}