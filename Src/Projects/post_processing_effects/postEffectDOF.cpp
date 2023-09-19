
#include "postEffectDOF.h"

constexpr const char* SHADER_DOF_NAME{ "Depth Of Field" };
constexpr const char* SHADER_DOF_VERTEX{ "\\GLSL\\simple.vsh" };
constexpr const char* SHADER_DOF_FRAGMENT{ "\\GLSL\\dof.fsh" };

////////////////////////////////////////////////////////////////////////////////////
// post DOF

//! a constructor
PostEffectDOF::PostEffectDOF()
    : PostEffectBase()
{
    for (int i = 0; i < LOCATIONS_COUNT; ++i)
        mLocations[i] = -1;
}

//! a destructor
PostEffectDOF::~PostEffectDOF()
{

}

const char *PostEffectDOF::GetName()
{
    return SHADER_DOF_NAME;
}
const char *PostEffectDOF::GetVertexFname(const int)
{
    return SHADER_DOF_VERTEX;
}
const char *PostEffectDOF::GetFragmentFname(const int)
{
    return SHADER_DOF_FRAGMENT;
}

bool PostEffectDOF::PrepUniforms(const int shaderIndex)
{
    bool lSuccess = false;

    GLSLShader* mShader = mShaders[shaderIndex];
    if (nullptr != mShader)
    {
        mShader->Bind();

        GLint loc = mShader->findLocation("colorSampler");
        if (loc >= 0)
            glUniform1i(loc, 0);
        loc = mShader->findLocation("depthSampler");
        if (loc >= 0)
            glUniform1i(loc, 1);

        upperClip = mShader->findLocation("upperClip");
        lowerClip = mShader->findLocation("lowerClip");

        focalDistance = mShader->findLocation("focalDistance");
        focalRange = mShader->findLocation("focalRange");

        textureWidth = mShader->findLocation("textureWidth");
        textureHeight = mShader->findLocation("textureHeight");

        zNear = mShader->findLocation("zNear");
        zFar = mShader->findLocation("zFar");

        fstop = mShader->findLocation("fstop");

        samples = mShader->findLocation("samples");
        rings = mShader->findLocation("rings");

        blurForeground = mShader->findLocation("blurForeground");

        manualdof = mShader->findLocation("manualdof");
        ndofstart = mShader->findLocation("ndofstart");
        ndofdist = mShader->findLocation("ndofdist");
        fdofstart = mShader->findLocation("fdofstart");
        fdofdist = mShader->findLocation("fdofdist");

        focusPoint = mShader->findLocation("focusPoint");

        CoC = mShader->findLocation("CoC");

        autofocus = mShader->findLocation("autofocus");
        focus = mShader->findLocation("focus");

        threshold = mShader->findLocation("threshold");
        gain = mShader->findLocation("gain");

        bias = mShader->findLocation("bias");
        fringe = mShader->findLocation("fringe");

        noise = mShader->findLocation("noise");

        pentagon = mShader->findLocation("pentagon");
        feather = mShader->findLocation("feather");

        debugBlurValue = mShader->findLocation("debugBlurValue");

        mShader->UnBind();

        lSuccess = true;
    }

    return lSuccess;
}

bool PostEffectDOF::CollectUIValues(UICallback* uiCallback, const SViewInfo& viewInfo)
{
    bool lSuccess = false;

    const double _upperClip = pData->UpperClip;
    const double _lowerClip = pData->LowerClip;

    double _znear = pCamera->NearPlaneDistance;
    double _zfar = pCamera->FarPlaneDistance;

    double _focalDistance = pData->FocalDistance;
    double _focalRange = pData->FocalRange;
    double _fstop = pData->FStop;
    int _samples = pData->Samples;
    int _rings = pData->Rings;

    float _useFocusPoint = (pData->UseFocusPoint) ? 1.0f : 0.0f;
    FBVector2d _focusPoint = pData->FocusPoint;

    double _blurForeground = (pData->BlurForeground) ? 1.0 : 0.0;

    double _CoC = pData->CoC;
    double _threshold = pData->Threshold;

    //	double _gain = pData->Gain;
    double _bias = pData->Bias;
    double _fringe = pData->Fringe;
    double _feather = pData->PentagonFeather;

    double _debugBlurValue = (pData->DebugBlurValue) ? 1.0 : 0.0;

    if (pData->UseCameraDOFProperties)
    {
        _focalDistance = pCamera->FocusSpecificDistance;
        _focalRange = pCamera->FocusAngle;

        FBModel *pInterest = nullptr;

        if (kFBFocusDistanceCameraInterest == pCamera->FocusDistanceSource)
            pInterest = pCamera->Interest;
        else if (kFBFocusDistanceModel == pCamera->FocusDistanceSource)
            pInterest = pCamera->FocusModel;

        if (nullptr != pInterest)
        {
            FBMatrix modelView, modelViewI;

            ((FBModel*)pCamera)->GetMatrix(modelView);
            FBMatrixInverse(modelViewI, modelView);

            FBVector3d lPos;
            pInterest->GetVector(lPos);

            FBTVector p(lPos[0], lPos[1], lPos[2], 1.0);
            FBVectorMatrixMult(p, modelViewI, p);
            double dist = p[0];

            // Dont write to property
            // FocalDistance = dist;
            _focalDistance = dist;
        }
    }
    else
        if (pData->AutoFocus && pData->FocusObject.GetCount() > 0)
        {
            FBMatrix modelView, modelViewI;

            ((FBModel*)pCamera)->GetMatrix(modelView);
            FBMatrixInverse(modelViewI, modelView);

            FBVector3d lPos;
            FBModel *pModel = (FBModel*)pData->FocusObject.GetAt(0);
            pModel->GetVector(lPos);

            FBTVector p(lPos[0], lPos[1], lPos[2]);
            FBVectorMatrixMult(p, modelViewI, p);
            double dist = p[0];

            // Dont write to property
            // FocalDistance = dist;
            _focalDistance = dist;
        }

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

        if (focalDistance >= 0)
            glUniform1f(focalDistance, (float)_focalDistance);
        if (focalRange >= 0)
            glUniform1f(focalRange, (float)_focalRange);
        if (fstop >= 0)
            glUniform1f(fstop, (float)_fstop);

        if (zNear >= 0)
            glUniform1f(zNear, (float)_znear);
        if (zFar >= 0)
            glUniform1f(zFar, (float)_zfar);

        if (samples >= 0)
            glUniform1i(samples, _samples);
        if (rings >= 0)
            glUniform1i(rings, _rings);

        if (blurForeground >= 0)
            glUniform1f(blurForeground, (float)_blurForeground);

        if (CoC >= 0)
            glUniform1f(CoC, 0.01f * (float)_CoC);

        if (blurForeground >= 0)
            glUniform1f(blurForeground, (float)_blurForeground);

        if (threshold >= 0)
            glUniform1f(threshold, 0.01f * (float)_threshold);
        if (bias >= 0)
            glUniform1f(bias, 0.01f * (float)_bias);

        if (fringe >= 0)
            glUniform1f(fringe, 0.01f * (float)_fringe);
        if (feather >= 0)
            glUniform1f(feather, 0.01f * (float)_feather);

        if (debugBlurValue >= 0)
            glUniform1f(debugBlurValue, (float)_debugBlurValue);

        if (focusPoint >= 0)
            glUniform4f(focusPoint, 0.01f * (float)_focusPoint[0], 0.01f * (float)_focusPoint[1], 0.0f, _useFocusPoint);

        mShader->UnBind();

        lSuccess = true;
    }

    return lSuccess;
}
