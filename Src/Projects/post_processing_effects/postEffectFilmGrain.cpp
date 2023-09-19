
#include "postEffectFilmGrain.h"

constexpr const char* SHADER_FILMGRAIN_NAME{ "Film Grain" };
constexpr const char* SHADER_FILMGRAIN_VERTEX{ "\\GLSL\\simple.vsh" };
constexpr const char* SHADER_FILMGRAIN_FRAGMENT{ "\\GLSL\\filmGrain.fsh" };


////////////////////////////////////////////////////////////////////////////////////
// post film grain

//! a constructor
PostEffectFilmGrain::PostEffectFilmGrain()
    : PostEffectBase()
{
    for (int i = 0; i < LOCATIONS_COUNT; ++i)
        mLocations[i] = -1;
}

//! a destructor
PostEffectFilmGrain::~PostEffectFilmGrain()
{

}

const char *PostEffectFilmGrain::GetName()
{
    return SHADER_FILMGRAIN_NAME;
}
const char *PostEffectFilmGrain::GetVertexFname(const int)
{
    return SHADER_FILMGRAIN_VERTEX;
}
const char *PostEffectFilmGrain::GetFragmentFname(const int)
{
    return SHADER_FILMGRAIN_FRAGMENT;
}

bool PostEffectFilmGrain::PrepUniforms(const int shaderIndex)
{
    bool lSuccess = false;

    GLSLShader* mShader = mShaders[shaderIndex];
    if (nullptr != mShader)
    {
        mShader->Bind();

        GLint loc = mShader->findLocation("sampler0");
        if (loc >= 0)
            glUniform1i(loc, 0);

        upperClip = mShader->findLocation("upperClip");
        lowerClip = mShader->findLocation("lowerClip");

        textureWidth = mShader->findLocation("textureWidth");
        textureHeight = mShader->findLocation("textureHeight");

        timer = mShader->findLocation("timer");
        grainamount = mShader->findLocation("grainamount");
        colored = mShader->findLocation("colored");
        coloramount = mShader->findLocation("coloramount");
        grainsize = mShader->findLocation("grainsize");
        lumamount = mShader->findLocation("lumamount");

        mShader->UnBind();

        lSuccess = true;
    }

    return lSuccess;
}

bool PostEffectFilmGrain::CollectUIValues(UICallback* uiCallback, const SViewInfo& viewInfo)
{
    bool lSuccess = false;

    const double _upperClip = pData->UpperClip;
    const double _lowerClip = pData->LowerClip;

    const double systemTime = (pData->FG_UsePlayTime) ? viewInfo.localTime : viewInfo.systemTime;

    double timerMult = pData->FG_TimeSpeed;
    double _timer = 0.01 * timerMult * systemTime;

    double _grainamount = pData->FG_GrainAmount;
    double _colored = (pData->FG_Colored) ? 1.0 : 0.0;
    double _coloramount = pData->FG_ColorAmount;
    double _grainsize = pData->FG_GrainSize;
    double _lumamount = pData->FG_LumAmount;

    GLSLShader* mShader = GetShaderPtr();
    if (nullptr != mShader)
    {
        mShader->Bind();

        if (upperClip >= 0)
            glUniform1f(upperClip, 0.01f * (float)_upperClip);

        if (lowerClip >= 0)
            glUniform1f(lowerClip, 1.0f - 0.01f * (float)_lowerClip);

        if (textureWidth >= 0)
            glUniform1f(textureWidth, static_cast<float>(viewInfo.w));
        if (textureHeight >= 0)
            glUniform1f(textureHeight, static_cast<float>(viewInfo.h));

        if (timer >= 0)
            glUniform1f(timer, (float)_timer);
        if (grainamount >= 0)
            glUniform1f(grainamount, 0.01f * (float)_grainamount);
        if (colored >= 0)
            glUniform1f(colored, (float)_colored);
        if (coloramount >= 0)
            glUniform1f(coloramount, 0.01f * (float)_coloramount);
        if (grainsize >= 0)
            glUniform1f(grainsize, 0.01f * (float)_grainsize);
        if (lumamount >= 0)
            glUniform1f(lumamount, 0.01f * (float)_lumamount);

        mShader->UnBind();

        lSuccess = true;
    }

    return lSuccess;
}

