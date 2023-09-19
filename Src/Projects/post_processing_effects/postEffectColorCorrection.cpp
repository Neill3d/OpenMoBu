
#include "postEffectColorCorrection.h"

#define SHADER_COLOR_NAME				"Color Correction"
#define SHADER_COLOR_VERTEX				"\\GLSL\\simple.vsh"
#define SHADER_COLOR_FRAGMENT			"\\GLSL\\color.fsh"

/////////////////////////////////////////////////////////////////////////////////////
// Effect Color Correction

//! a constructor
PostEffectColor::PostEffectColor()
    : PostEffectBase()
{
    for (int i = 0; i < LOCATIONS_COUNT; ++i)
        mLocations[i] = -1;
}

//! a destructor
PostEffectColor::~PostEffectColor()
{

}

const char *PostEffectColor::GetName()
{
    return SHADER_COLOR_NAME;
}

const char *PostEffectColor::GetVertexFname(const int)
{
    return SHADER_COLOR_VERTEX;
}

const char *PostEffectColor::GetFragmentFname(const int)
{
    return SHADER_COLOR_FRAGMENT;
}

bool PostEffectColor::PrepUniforms(const int shaderIndex)
{
    bool lSuccess = false;

    GLSLShader* shader = mShaders[shaderIndex];
    if (shader)
    {
        shader->Bind();

        GLint loc = shader->findLocation("sampler0");
        if (loc >= 0)
            glUniform1i(loc, 0);

        mResolution = shader->findLocation("gResolution");
        mChromaticAberration = shader->findLocation("gCA");

        mUpperClip = shader->findLocation("upperClip");
        mLowerClip = shader->findLocation("lowerClip");

        mLocCSB = shader->findLocation("gCSB");
        mLocHue = shader->findLocation("gHue");

        shader->UnBind();

        lSuccess = true;
    }

    return lSuccess;
}

bool PostEffectColor::CollectUIValues(UICallback* uiCallback, const SViewInfo& viewInfo)
{
    bool lSuccess = false;

    const double upperClip = pData->UpperClip;
    const double lowerClip = pData->LowerClip;

    const float chromatic_aberration = (pData->ChromaticAberration) ? 1.0f : 0.0f;
    const FBVector2d ca_dir = pData->ChromaticAberrationDirection;

    double saturation = 1.0 + 0.01 * pData->Saturation;
    double brightness = 1.0 + 0.01 * pData->Brightness;
    double contrast = 1.0 + 0.01 * pData->Contrast;
    double gamma = 0.01 * pData->Gamma;

    const float inverse = (pData->Inverse) ? 1.0f : 0.0f;
    double hue = 0.01 * pData->Hue;
    double hueSat = 0.01 * pData->HueSaturation;
    double lightness = 0.01 * pData->Lightness;

    GLSLShader* mShader = GetShaderPtr();
    if (mShader)
    {
        mShader->Bind();

        if (mResolution >= 0)
        {
            glUniform2f(mResolution, static_cast<float>(viewInfo.w), static_cast<float>(viewInfo.h));
        }

        if (mChromaticAberration >= 0)
        {
            glUniform4f(mChromaticAberration, static_cast<float>(ca_dir[0]), static_cast<float>(ca_dir[1]), 0.0f, chromatic_aberration);
        }

        if (mUpperClip >= 0)
            glUniform1f(mUpperClip, 0.01f * (float)upperClip);

        if (mLowerClip >= 0)
            glUniform1f(mLowerClip, 1.0f - 0.01f * (float)lowerClip);

        if (mLocCSB >= 0)
            glUniform4f(mLocCSB, (float)contrast, (float)saturation, (float)brightness, (float)gamma);

        if (mLocHue >= 0)
            glUniform4f(mLocHue, (float)hue, (float)hueSat, (float)lightness, inverse);

        mShader->UnBind();

        lSuccess = true;
    }

    return lSuccess;
}
