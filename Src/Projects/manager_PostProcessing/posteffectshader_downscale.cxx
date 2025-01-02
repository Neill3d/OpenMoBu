
/**	\file	posteffectshader_downscale.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectshader_downscale.h"
#include "mobu_logging.h"

/////////////////////////////////////////////////////////////////////////
// PostEffectShaderDownscale

PostEffectShaderDownscale::PostEffectShaderDownscale()
	: PostEffectBufferShader()
{}

PostEffectShaderDownscale::~PostEffectShaderDownscale()
{}


//! an effect public name
const char* PostEffectShaderDownscale::GetName() const
{
	return "Downscale";
}
//! get a filename of vertex shader, for this effect. returns a relative filename
const char* PostEffectShaderDownscale::GetVertexFname(const int variationIndex) const
{
	return "/GLSL/downscale.vsh";
}

//! get a filename of a fragment shader, for this effect, returns a relative filename
const char* PostEffectShaderDownscale::GetFragmentFname(const int variationIndex) const
{
	return "/GLSL/downscale.fsh";
}

//! prepare uniforms for a given variation of the effect
bool PostEffectShaderDownscale::OnPrepareUniforms(const int variationIndex)
{
	return true;
}

//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
bool PostEffectShaderDownscale::OnCollectUI(PostPersistentData* pData, const PostEffectContext& effectContext, int maskIndex)
{
	return true;
}

/// new feature to have several passes for a specified effect
const int PostEffectShaderDownscale::GetNumberOfPasses() const
{
	return 1;
}
//! initialize a specific path for drawing
bool PostEffectShaderDownscale::PrepPass(const int pass, int w, int h)
{
	GLSLShaderProgram* shader = GetShaderPtr();
	if (!shader)
		return false;

	// TODO: are we assume that the pass is already binded ?!
	shader->Bind();

	GLint loc = shader->findLocation("texelSize");
	if (loc >= 0)
		glUniform2f(loc, 1.0f / static_cast<float>(w), 1.0f / static_cast<float>(h));

	// TODO: we probably could skip that, as we are going to render using the shader
	shader->UnBind();

	return true;
}
