
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
	: PostEffectBufferShader(uiComponent)
	, mUIComponent(uiComponent)
{
	//mTextureId = CommonEffectUniforms::GetColorSamplerSlot();

	if (FBIS(uiComponent, EffectShaderBilateralBlurUserObject))
	{
		EffectShaderBilateralBlurUserObject* userObject = FBCast<EffectShaderBilateralBlurUserObject>(uiComponent);

		ShaderProperty textureProperty(EffectShaderBilateralBlurUserObject::INPUT_TEXTURE_LABEL, "colorSampler", EPropertyType::TEXTURE, &userObject->InputTexture);
		AddProperty(std::move(textureProperty))
			.SetValue(CommonEffect::ColorSamplerSlot);

		ShaderProperty blurScaleProperty(EffectShaderBilateralBlurUserObject::BLUR_SCALE_LABEL, "scale", EPropertyType::VEC2, &userObject->BlurScale);
		AddProperty(std::move(blurScaleProperty));
	}
}

PostEffectShaderBilateralBlur::~PostEffectShaderBilateralBlur()
{}


//! an effect public name
const char* PostEffectShaderBilateralBlur::GetName() const
{
	return SHADER_NAME;
}
//! get a filename of vertex shader, for this effect. returns a relative filename
const char* PostEffectShaderBilateralBlur::GetVertexFname(const int variationIndex) const
{
	return VERTEX_SHADER_FILE;
}

//! get a filename of a fragment shader, for this effect, returns a relative filename
const char* PostEffectShaderBilateralBlur::GetFragmentFname(const int variationIndex) const
{
	return FRAGMENT_SHADER_FILE;
}

//! prepare uniforms for a given variation of the effect
bool PostEffectShaderBilateralBlur::OnPrepareUniforms(const int variationIndex)
{
	/*
	GLSLShaderProgram* shader = GetShaderPtr();
	if (!shader)
		return false;

	shader->Bind();

	mColorSamplerLoc = shader->findLocation("colorSampler");
	mLocImageBlurScale = shader->findLocation("scale");

	shader->UnBind();

	if (FBIS(mUIComponent, EffectShaderBilateralBlurUserObject))
	{
		EffectShaderBilateralBlurUserObject* userObject = FBCast<EffectShaderBilateralBlurUserObject>(mUIComponent);

		if (ShaderProperty* textureProperty = FindProperty(userObject->InputTexture.GetName()))
		{
			textureProperty->location = mColorSamplerLoc;
		}
		if (ShaderProperty* scaleProperty = FindProperty(userObject->BlurScale.GetName()))
		{
			scaleProperty->location = mLocImageBlurScale;
		}
	}
	*/
	return true;
}

//! grab from UI all needed parameters to update effect state (uniforms) during evaluation
bool PostEffectShaderBilateralBlur::OnCollectUI(const IPostEffectContext* effectContext, int maskIndex)
{
	/*
	if (pData)
	{
		mBlurMaskScale = pData->GetMaskScale(maskIndex);
	}
	*/
	/*
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
	*/
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
	/*
	GLSLShaderProgram* shader = GetShaderPtr();
	if (!shader)
		return false;

	if (mColorSamplerLoc >= 0)
		glUniform1i(mColorSamplerLoc, mTextureId);

	if (BlurScale->location >= 0)
	{
		const float* value = BlurScale->GetFloatData();

		glUniform4f(BlurScale->location,
			static_cast<float>(value[0]) / static_cast<float>(w),
			static_cast<float>(value[1]) / static_cast<float>(h),
			1.0f / static_cast<float>(w),
			1.0f / static_cast<float>(h));
	}

	if (mLocImageBlurScale >= 0)
	{
		glUniform4f(mLocImageBlurScale,
			static_cast<float>(mBlurMaskScale.mValue[0]) / static_cast<float>(w),
			static_cast<float>(mBlurMaskScale.mValue[1]) / static_cast<float>(h),
			1.0f / static_cast<float>(w),
			1.0f / static_cast<float>(h));
	}
	*/
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

	FBPropertyPublish(this, InputTexture, INPUT_TEXTURE_LABEL, nullptr, nullptr);
	FBPropertyPublish(this, BlurScale, BLUR_SCALE_LABEL, nullptr, nullptr);

	BlurScale = FBVector2d(1.0, 1.0);
	ShaderFile = PostEffectShaderBilateralBlur::FRAGMENT_SHADER_FILE;
	ShaderFile.ModifyPropertyFlag(FBPropertyFlag::kFBPropertyFlagReadOnly, true);
	NumberOfPasses.ModifyPropertyFlag(FBPropertyFlag::kFBPropertyFlagReadOnly, true);
	UniqueClassId = 63;

	return true;
}
