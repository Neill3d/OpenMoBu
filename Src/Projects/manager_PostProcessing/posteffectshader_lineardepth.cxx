
/**	\file	posteffectshader_lineardepth.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectshader_lineardepth.h"
#include "mobu_logging.h"

/////////////////////////////////////////////////////////////////////////
// PostEffectShaderLinearDepth

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

//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
bool PostEffectShaderLinearDepth::OnCollectUI(const IPostEffectContext* effectContext, int maskIndex)
{
	FBCamera* camera = effectContext->GetCamera();

	const float znear = static_cast<float>(camera->NearPlaneDistance);
	const float zfar = static_cast<float>(camera->FarPlaneDistance);
	FBCameraType cameraType;
	camera->Type.GetData(&cameraType, sizeof(FBCameraType));
	const bool perspective = (cameraType == FBCameraType::kFBCameraTypePerspective);

	const float newClipInfo[4]{
		znear * zfar,
		znear - zfar,
		zfar,
		(perspective) ? 1.0f : 0.0f
	};
	mClipInfo->SetValue(newClipInfo[0], newClipInfo[1], newClipInfo[2], newClipInfo[3]);
	return true;
}
