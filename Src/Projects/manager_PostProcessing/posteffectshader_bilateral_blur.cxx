
/**	\file	posteffectshader_bilateral_blur.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectshader_bilateral_blur.h"
#include "postpersistentdata.h"
#include "mobu_logging.h"

//--- FiLMBOX Registration & Implementation.
FBClassImplementation(EffectShaderBilateralBlurUserObject);
FBUserObjectImplement(EffectShaderBilateralBlurUserObject,
	"Effect shader for a bilateral (gaussian) blur",
	"cam_switcher_toggle.png");                                          //Register UserObject class
PostEffectFBElementClassImplementation(EffectShaderBilateralBlurUserObject, "Blur Shader", "cam_switcher_toggle.png");                  //Register to the asset system


/////////////////////////////////////////////////////////////////////////
// PostEffectShaderLinearDepth

PostEffectShaderBilateralBlur::PostEffectShaderBilateralBlur(FBComponent* uiComponent)
	: PostEffectBufferShader()
	, mUIComponent(uiComponent)
{
	mTextureId = CommonEffectUniforms::GetColorSamplerSlot();
}

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
	return "/GLSL/simple130.glslv";
}

//! get a filename of a fragment shader, for this effect, returns a relative filename
const char* PostEffectShaderBilateralBlur::GetFragmentFname(const int variationIndex) const
{
	return "/GLSL/imageBlur.glslf";
}

//! prepare uniforms for a given variation of the effect
bool PostEffectShaderBilateralBlur::OnPrepareUniforms(const int variationIndex)
{
	GLSLShaderProgram* shader = GetShaderPtr();
	if (!shader)
		return false;

	shader->Bind();

	mColorSamplerLoc = shader->findLocation("colorSampler");
	mLocImageBlurScale = shader->findLocation("scale");

	shader->UnBind();

	return true;
}

void PostEffectShaderBilateralBlur::SetTextureId(GLint textureId)
{
	mTextureId = textureId;
}

void PostEffectShaderBilateralBlur::SetScale(const FBVector2d& scale)
{
	mBlurMaskScale = scale;
}

//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
bool PostEffectShaderBilateralBlur::OnCollectUI(PostPersistentData* pData, const PostEffectContext& effectContext, int maskIndex)
{
	/*
	if (pData)
	{
		mBlurMaskScale = pData->GetMaskScale(maskIndex);
	}
	*/

	if (mUIComponent)
	{
		if (FBIS(mUIComponent, EffectShaderBilateralBlurUserObject))
		{
			if (EffectShaderBilateralBlurUserObject* userObject = FBCast<EffectShaderBilateralBlurUserObject>(mUIComponent))
			{
				double value[4];
				userObject->BlurScale.GetData(value, sizeof(double)*2);
				mBlurMaskScale[0] = value[0];
				mBlurMaskScale[1] = value[1];
			}
		}
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

	if (mColorSamplerLoc >= 0)
		glUniform1i(mColorSamplerLoc, mTextureId);

	if (mLocImageBlurScale >= 0)
	{
		glUniform4f(mLocImageBlurScale,
			static_cast<float>(mBlurMaskScale.mValue[0]) / static_cast<float>(w),
			static_cast<float>(mBlurMaskScale.mValue[1]) / static_cast<float>(h),
			1.0f / static_cast<float>(w),
			1.0f / static_cast<float>(h));
	}
	
	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// EffectShaderBilateralBlurUserObject

EffectShaderBilateralBlurUserObject::EffectShaderBilateralBlurUserObject(const char* pName, HIObject pObject)
	: ParentClass(pName, pObject)
{
	FBClassInit;
}

/************************************************
 *  FiLMBOX Constructor.
 ************************************************/
bool EffectShaderBilateralBlurUserObject::FBCreate()
{
	ParentClass::FBCreate();

	FBPropertyPublish(this, BlurScale, "Blur Scale", nullptr, nullptr);

	BlurScale = FBVector2d(1.0, 1.0);
	ShaderFile = "/GLSL/imageBlur.glslf";
	ShaderFile.ModifyPropertyFlag(FBPropertyFlag::kFBPropertyFlagReadOnly, true);
	NumberOfPasses.ModifyPropertyFlag(FBPropertyFlag::kFBPropertyFlagReadOnly, true);
	UniqueClassId = 63;

	return true;
}
