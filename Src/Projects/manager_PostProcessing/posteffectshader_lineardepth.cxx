
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

PostEffectShaderLinearDepth::PostEffectShaderLinearDepth()
	: PostEffectBufferShader()
{}

PostEffectShaderLinearDepth::~PostEffectShaderLinearDepth()
{}


//! an effect public name
const char* PostEffectShaderLinearDepth::GetName() const
{
	return "LinearDepth";
}
//! get a filename of vertex shader, for this effect. returns a relative filename
const char* PostEffectShaderLinearDepth::GetVertexFname(const int variationIndex) const
{
	return "/GLSL/simple.vsh";
}

//! get a filename of a fragment shader, for this effect, returns a relative filename
const char* PostEffectShaderLinearDepth::GetFragmentFname(const int variationIndex) const
{
	return "/GLSL/depthLinearize.fsh";
}

//! prepare uniforms for a given variation of the effect
bool PostEffectShaderLinearDepth::OnPrepareUniforms(const int variationIndex)
{
	GLSLShaderProgram* shader = GetShaderPtr();
	if (!shader)
		return false;

	shader->Bind();

	GLint loc = shader->findLocation("depthSampler");
	if (loc >= 0)
		glUniform1i(loc, CommonEffectUniforms::GetDepthSamplerSlot());
	mLocDepthLinearizeClipInfo = shader->findLocation("gClipInfo");

	shader->UnBind();

	return true;
}

//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
bool PostEffectShaderLinearDepth::OnCollectUI(PostPersistentData* pData, const PostEffectContext& effectContext, int maskIndex)
{
	const float znear = static_cast<float>(effectContext.camera->NearPlaneDistance);
	const float zfar = static_cast<float>(effectContext.camera->FarPlaneDistance);
	FBCameraType cameraType;
	effectContext.camera->Type.GetData(&cameraType, sizeof(FBCameraType));
	const bool perspective = (cameraType == FBCameraType::kFBCameraTypePerspective);

	const float newClipInfo[4]{
		znear * zfar,
		znear - zfar,
		zfar,
		(perspective) ? 1.0f : 0.0f
	};
	memmove(clipInfo, newClipInfo, sizeof(float) * 4);
	return true;
}

/// new feature to have several passes for a specified effect
const int PostEffectShaderLinearDepth::GetNumberOfPasses() const
{
	return 1;
}
//! initialize a specific path for drawing
bool PostEffectShaderLinearDepth::PrepPass(const int pass, int w, int h)
{
	GLSLShaderProgram* shader = GetShaderPtr();
	if (!shader)
		return false;

	// TODO: are we assume that the pass is already binded ?!
	shader->Bind();

	if (mLocDepthLinearizeClipInfo >= 0)
		glUniform4fv(mLocDepthLinearizeClipInfo, 1, clipInfo);

	// TODO: we probably could skip that, as we are going to render using the shader
	shader->UnBind();

	return true;
}
