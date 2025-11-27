
/**	\file	posteffectshader_blur_lineardepth.cxx

Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectshader_blur_lineardepth.h"
#include "shaderpropertywriter.h"
#include "postpersistentdata.h"
#include "mobu_logging.h"
#include <hashUtils.h>


uint32_t EffectShaderBlurLinearDepth::SHADER_NAME_HASH = xxhash32(EffectShaderBlurLinearDepth::SHADER_NAME);

EffectShaderBlurLinearDepth::EffectShaderBlurLinearDepth(FBComponent* uiComponent)
	: PostEffectBufferShader(uiComponent)
{
	mColorTexture = &AddProperty(ShaderProperty("color", "sampler0"))
		.SetType(EPropertyType::TEXTURE)
		.SetFlag(PropertyFlag::ShouldSkip, true)
		.SetDefaultValue(CommonEffect::ColorSamplerSlot);

	mLinearDepthTexture = &AddProperty(ShaderProperty("linearDepth", "linearDepthSampler"))
		.SetType(EPropertyType::TEXTURE)
		.SetFlag(PropertyFlag::ShouldSkip, true)
		.SetDefaultValue(CommonEffect::LinearDepthSamplerSlot);

	mBlurSharpness = &AddProperty(ShaderProperty("blurSharpness", "g_Sharpness"))
		.SetType(EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::ShouldSkip, true);

	mColorShift = &AddProperty(ShaderProperty("colorShift", "g_ColorShift"))
		.SetType(EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::ShouldSkip, true);

	mInvRes = &AddProperty(ShaderProperty("invRes", "g_InvResolutionDirection"))
		.SetType(EPropertyType::VEC2)
		.SetFlag(PropertyFlag::ShouldSkip, true);
}

//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
bool EffectShaderBlurLinearDepth::OnCollectUI(IPostEffectContext* effectContext, int maskIndex)
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

		//mBlurSharpness->SetValue(blurSharpness);
		//mColorShift->SetValue(0.0f);
		//mInvRes->SetValue(invRes0, invRes1);
	}

	return true;
}