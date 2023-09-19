#pragma once

#include "postEffectBase.h"


class PostEffectColor : public PostEffectBase
{
public:
    //! a constructor
    PostEffectColor();
    //! a destructor
    virtual ~PostEffectColor();

    int GetNumberOfShaders() const override { return 1; }

    virtual const char *GetName() override;
    virtual const char *GetVertexFname(const int shaderIndex) override;
    virtual const char *GetFragmentFname(const int shaderIndex) override;

    virtual bool PrepUniforms(const int shaderIndex) override;
    virtual bool CollectUIValues(UICallback* uiCallback, const SViewInfo& viewInfo) override;

protected:

    // shader locations
    enum { LOCATIONS_COUNT = 6 };
    union
    {
        struct
        {
            GLint		mResolution;
            GLint		mChromaticAberration;

            GLint		mUpperClip;
            GLint		mLowerClip;

            GLint		mLocCSB;
            GLint		mLocHue;
        };

        GLint		mLocations[LOCATIONS_COUNT];
    };
};