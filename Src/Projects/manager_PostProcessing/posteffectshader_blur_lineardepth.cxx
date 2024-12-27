
/**	\file	posteffectshader_blur_lineardepth.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectshader_blur_lineardepth.h"
#include "postpersistentdata.h"
#include "mobu_logging.h"

/////////////////////////////////////////////////////////////////////////
// PostEffectShaderBlurLinearDepth

PostEffectShaderBlurLinearDepth::PostEffectShaderBlurLinearDepth()
	: PostEffectBufferShader()
{}

PostEffectShaderBlurLinearDepth::~PostEffectShaderBlurLinearDepth()
{}


//! an effect public name
const char* PostEffectShaderBlurLinearDepth::GetName() const
{
	return "Blur w/th LinearDepth";
}
//! get a filename of vertex shader, for this effect. returns a relative filename
const char* PostEffectShaderBlurLinearDepth::GetVertexFname(const int variationIndex) const
{
	return "\\GLSL\\simple.vsh";
}

//! get a filename of a fragment shader, for this effect, returns a relative filename
const char* PostEffectShaderBlurLinearDepth::GetFragmentFname(const int variationIndex) const
{
	return "\\GLSL\\depthLinearize.fsh";
}

//! prepare uniforms for a given variation of the effect
bool PostEffectShaderBlurLinearDepth::PrepUniforms(const int variationIndex)
{
	GLSLShaderProgram* shader = GetShaderPtr();
	if (!shader)
		return false;

	shader->Bind();

	GLint loc = shader->findLocation("sampler0");
	if (loc >= 0)
		glUniform1i(loc, CommonEffectUniforms::GetColorSamplerSlot());
	loc = shader->findLocation("linearDepthSampler");
	if (loc >= 0)
		glUniform1i(loc, CommonEffectUniforms::GetLinearDepthSamplerSlot());

	mLocBlurSharpness = shader->findLocation("g_Sharpness");
	mLocBlurRes = shader->findLocation("g_InvResolutionDirection");

	shader->UnBind();

	return true;
}

//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
bool PostEffectShaderBlurLinearDepth::CollectUIValues(PostPersistentData* pData, PostEffectContext& effectContext)
{
	const int w = effectContext.w; // buffers->GetWidth();
	const int h = effectContext.h; // buffers->GetHeight();

	blurSharpness = 0.1f * (float)pData->SSAO_BlurSharpness;
	invRes[0] = 1.0f / static_cast<float>(w);
	invRes[1] = 1.0f / static_cast<float>(h);

	return true;
}

/// new feature to have several passes for a specified effect
const int PostEffectShaderBlurLinearDepth::GetNumberOfPasses() const
{
	return 1;
}
//! initialize a specific path for drawing
bool PostEffectShaderBlurLinearDepth::PrepPass(const int pass)
{
	GLSLShaderProgram* shader = GetShaderPtr();
	if (!shader)
		return false;

	// TODO: are we assume that the pass is already binded ?!
	shader->Bind();

	if (mLocBlurSharpness >= 0)
		glUniform1f(mLocBlurSharpness, blurSharpness);
	if (mLocBlurRes >= 0)
		glUniform2f(mLocBlurRes, invRes[0], invRes[1]);

	// TODO: we probably could skip that, as we are going to render using the shader
	shader->UnBind();

	return true;
}
