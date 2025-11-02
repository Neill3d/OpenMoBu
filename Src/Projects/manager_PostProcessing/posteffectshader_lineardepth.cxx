
/**	\file	posteffectshader_lineardepth.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectshader_lineardepth.h"
#include "mobu_logging.h"


PostEffectShaderLinearDepth::PostEffectShaderLinearDepth(FBComponent* uiComponent)
	: PostEffectBufferShader(uiComponent)
{
	mDepthTexture = &AddProperty(ShaderProperty("depth", "depthSampler"))
		.SetType(EPropertyType::TEXTURE)
		.SetValue(CommonEffect::DepthSamplerSlot);

	mClipInfo = &AddProperty(ShaderProperty("clipInfo", "gClipInfo"))
		.SetType(EPropertyType::VEC4)
		.SetFlag(PropertyFlag::ShouldSkip, true);
}

bool PostEffectShaderLinearDepth::OnCollectUI(const IPostEffectContext* effectContext, int maskIndex)
{
	const float znear = effectContext->GetCameraNearDistance();
	const float zfar = effectContext->GetCameraFarDistance();	
	const float perspective = (!effectContext->IsCameraOrthogonal()) ? 1.0f : 0.0f;

	const float newClipInfo[4]
	{
		znear * zfar,
		znear - zfar,
		zfar,
		perspective
	};

	mClipInfo->SetValue(newClipInfo[0], newClipInfo[1], newClipInfo[2], newClipInfo[3]);
	return true;
}
