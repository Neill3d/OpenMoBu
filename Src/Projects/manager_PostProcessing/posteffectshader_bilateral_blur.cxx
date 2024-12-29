
/**	\file	posteffectshader_bilateral_blur.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectshader_bilateral_blur.h"
#include "postpersistentdata.h"
#include "mobu_logging.h"

/////////////////////////////////////////////////////////////////////////
// PostEffectShaderLinearDepth

PostEffectShaderBilateralBlur::PostEffectShaderBilateralBlur()
	: PostEffectBufferShader()
{}

PostEffectShaderBilateralBlur::~PostEffectShaderBilateralBlur()
{}


//! an effect public name
const char* PostEffectShaderBilateralBlur::GetName() const
{
	return "Guassian Blur";
}
//! get a filename of vertex shader, for this effect. returns a relative filename
const char* PostEffectShaderBilateralBlur::GetVertexFname(const int variationIndex) const
{
	return "/GLSL/simple.vsh";
}

//! get a filename of a fragment shader, for this effect, returns a relative filename
const char* PostEffectShaderBilateralBlur::GetFragmentFname(const int variationIndex) const
{
	return "/GLSL/imageBlur.fsh";
}

//! prepare uniforms for a given variation of the effect
bool PostEffectShaderBilateralBlur::PrepUniforms(const int variationIndex)
{
	GLSLShaderProgram* shader = GetShaderPtr();
	if (!shader)
		return false;

	shader->Bind();

	GLint loc = shader->findLocation("colorSampler");
	if (loc >= 0)
		glUniform1i(loc, CommonEffectUniforms::GetColorSamplerSlot());

	mLocImageBlurScale = shader->findLocation("scale");

	shader->UnBind();

	return true;
}

//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
bool PostEffectShaderBilateralBlur::CollectUIValues(PostPersistentData* pData, PostEffectContext& effectContext, int maskIndex)
{
	if (pData)
	{
		mBlurMaskScale = pData->GetMaskScale(maskIndex);
	}
	return true;
}

/// new feature to have several passes for a specified effect
const int PostEffectShaderBilateralBlur::GetNumberOfPasses() const
{
	return 1;
}
//! initialize a specific path for drawing
bool PostEffectShaderBilateralBlur::PrepPass(const int pass, int w, int h)
{
	GLSLShaderProgram* shader = GetShaderPtr();
	if (!shader)
		return false;

	if (mLocImageBlurScale >= 0)
		glUniform4f(mLocImageBlurScale,
			static_cast<float>(mBlurMaskScale.mValue[0]) / static_cast<float>(w),
			static_cast<float>(mBlurMaskScale.mValue[1]) / static_cast<float>(h),
			1.0f / static_cast<float>(w),
			1.0f / static_cast<float>(h));

	return true;
}
