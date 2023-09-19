#pragma once

#include "postEffectBase.h"

///////////////////////////////////////////////////////////////////////////////////////
// PostEffectFilmGrain

class PostEffectFilmGrain : public PostEffectBase
{
public:

    //! a constructor
    PostEffectFilmGrain();

    //! a destructor
    virtual ~PostEffectFilmGrain();

    int GetNumberOfShaders() const override { return 1; }

    virtual const char *GetName() override;
    virtual const char *GetVertexFname(const int shaderIndex) override;
    virtual const char *GetFragmentFname(const int shaderIndex) override;

    virtual bool PrepUniforms(const int shaderIndex) override;
    virtual bool CollectUIValues(UICallback* uiCallback, const SViewInfo& viewInfo) override;

protected:

    // shader locations
    enum { LOCATIONS_COUNT = 10 };
    union
    {
        struct
        {
            GLint		upperClip;
            GLint		lowerClip;

            // locations
            GLint		textureWidth;
            GLint		textureHeight;

            GLint		timer;

            GLint		grainamount; // = 0.05; //grain amount
            GLint		colored; // = false; //colored noise?
            GLint		coloramount; // = 0.6;
            GLint		grainsize; // = 1.6; //grain particle size (1.5 - 2.5)
            GLint		lumamount; // = 1.0; //
        };

        GLint		mLocations[LOCATIONS_COUNT];
    };
};
