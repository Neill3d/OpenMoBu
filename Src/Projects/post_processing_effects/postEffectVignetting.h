#pragma once

#include "postEffectBase.h"

///////////////////////////////////////////////////////////////////////////////////////
// PostEffectVignetting

class PostEffectVignetting : public PostEffectBase
{
public:

    //! a constructor
    PostEffectVignetting();

    //! a destructor
    virtual ~PostEffectVignetting();

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
            GLint		mUpperClip;
            GLint		mLowerClip;

            GLint		mLocAmount;	// amount of an effect applied
            GLint		mLocVignOut;
            GLint		mLocVignIn;
            GLint		mLocVignFade;
        };

        GLint		mLocations[LOCATIONS_COUNT];
    };
};
