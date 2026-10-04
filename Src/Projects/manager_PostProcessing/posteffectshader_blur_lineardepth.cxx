
/**	\file	posteffectshader_blur_lineardepth.cxx

Sergei <Neill3d> Solokhin 2018-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectshader_blur_lineardepth.h"
#include "shaderproperty_writer.h"
#include "postpersistentdata.h"
#include "mobu_logging.h"
#include <hashUtils.h>


uint32_t EffectShaderBlurLinearDepth::SHADER_NAME_HASH = xxhash32(EffectShaderBlurLinearDepth::SHADER_NAME);

EffectShaderBlurLinearDepth::EffectShaderBlurLinearDepth(FBComponent* uiComponent)
	: PostEffectBufferShader(uiComponent)
{}

void EffectShaderBlurLinearDepth::OnPopulateProperties(ShaderPropertyScheme* scheme)
{
	mColorTexture = scheme->AddProperty(ShaderProperty("color", "sampler0"))
		.SetType(EPropertyType::TEXTURE)
		.SetFlag(PropertyFlag::SKIP, true)
		.SetDefaultValue(CommonEffect::ColorSamplerSlot)
		.GetProxy();

	mLinearDepthTexture = scheme->AddProperty(ShaderProperty("linearDepth", "linearDepthSampler"))
		.SetType(EPropertyType::TEXTURE)
		.SetFlag(PropertyFlag::SKIP, true)
		.SetDefaultValue(CommonEffect::LinearDepthSamplerSlot)
		.GetProxy();

	mBlurSharpness = scheme->AddProperty(ShaderProperty("blurSharpness", "g_Sharpness"))
		.SetType(EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::SKIP, true)
		.GetProxy();

	mColorShift = scheme->AddProperty(ShaderProperty("colorShift", "g_ColorShift"))
		.SetType(EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::SKIP, true)
		.GetProxy();

	mInvRes = scheme->AddProperty(ShaderProperty("invRes", "g_InvResolutionDirection"))
		.SetType(EPropertyType::VEC2)
		.SetFlag(PropertyFlag::SKIP, true)
		.GetProxy();
}

//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
bool EffectShaderBlurLinearDepth::OnCollectUI(PostEffectContextProxy* effectContext, int maskIndex) const
{
	if (const PostPersistentData* data = effectContext->GetPostProcessData())
	{
		const int w = effectContext->GetViewWidth();
		const int h = effectContext->GetViewHeight();

		const float blurSharpness = 0.1f * (float)data->SSAO_BlurSharpness;
		const float invRes0 = 1.0f / static_cast<float>(w);
		const float invRes1 = 1.0f / static_cast<float>(h);

		ShaderPropertyWriter writer(this, effectContext);
		writer(mBlurSharpness, blurSharpness)
			(mColorShift, 0.0f)
			(mInvRes, invRes0, invRes1);
	}

	return true;
}