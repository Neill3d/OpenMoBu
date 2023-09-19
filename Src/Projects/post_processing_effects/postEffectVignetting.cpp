
#include "postEffectVignetting.h"

constexpr const char* SHADER_VIGNETTE_NAME{ "Vignetting" };
constexpr const char* SHADER_VIGNETTE_VERTEX{ "\\GLSL\\simple.vsh" };
constexpr const char* SHADER_VIGNETTE_FRAGMENT{ "\\GLSL\\vignetting.fsh" };

////////////////////////////////////////////////////////////////////////////////////
// post vignetting

//! a constructor
PostEffectVignetting::PostEffectVignetting()
    : PostEffectBase()
{
    for (int i = 0; i < LOCATIONS_COUNT; ++i)
        mLocations[i] = -1;
}

//! a destructor
PostEffectVignetting::~PostEffectVignetting()
{

}

const char *PostEffectVignetting::GetName()
{
    return SHADER_VIGNETTE_NAME;
}
const char *PostEffectVignetting::GetVertexFname(const int)
{
    return SHADER_VIGNETTE_VERTEX;
}
const char *PostEffectVignetting::GetFragmentFname(const int)
{
    return SHADER_VIGNETTE_FRAGMENT;
}

bool PostEffectVignetting::PrepUniforms(const int shaderIndex)
{
    bool lSuccess = false;

    GLSLShader* mShader = mShaders[shaderIndex];
    if (nullptr != mShader)
    {
        mShader->Bind();

        GLint loc = mShader->findLocation("sampler0");
        if (loc >= 0)
            glUniform1i(loc, 0);

        mUpperClip = mShader->findLocation("upperClip");
        mLowerClip = mShader->findLocation("lowerClip");

        mLocAmount = mShader->findLocation("amount");
        mLocVignOut = mShader->findLocation("vignout");
        mLocVignIn = mShader->findLocation("vignin");
        mLocVignFade = mShader->findLocation("vignfade");

        mShader->UnBind();

        lSuccess = true;
    }

    return lSuccess;
}

bool PostEffectVignetting::CollectUIValues(UICallback* uiCallback, const SViewInfo& viewInfo)
{
    bool lSuccess = false;

    const double upperClip = pData->UpperClip;
    const double lowerClip = pData->LowerClip;

    double amount = pData->VignAmount;
    double vignout = pData->VignOut;
    double vignin = pData->VignIn;
    double vignfade = pData->VignFade;

    GLSLShader* mShader = GetShaderPtr();

    if (nullptr != mShader)
    {
        mShader->Bind();

        if (mUpperClip >= 0)
            glUniform1f(mUpperClip, 0.01f * (float)upperClip);

        if (mLowerClip >= 0)
            glUniform1f(mLowerClip, 1.0f - 0.01f * (float)lowerClip);

        if (mLocAmount >= 0)
            glUniform1f(mLocAmount, 0.01f * (float)amount);
        if (mLocVignOut >= 0)
            glUniform1f(mLocVignOut, 0.01f * (float)vignout);
        if (mLocVignIn >= 0)
            glUniform1f(mLocVignIn, 0.01f * (float)vignin);
        if (mLocVignFade >= 0)
            glUniform1f(mLocVignFade, (float)vignfade);

        mShader->UnBind();

        lSuccess = true;
    }

    return lSuccess;
}


