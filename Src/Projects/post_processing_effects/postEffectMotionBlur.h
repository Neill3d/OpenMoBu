
#pragma once

// postprocessing_effectMotionBlur
/*
Sergei <Neill3d> Solokhin 2018-22

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "postEffectBase.h"
#include <random>

///////////////////////////////////////////////////////////////////////////////////////
// PostEffectMotionBlur

class PostEffectMotionBlur : public PostEffectBase
{
public:

	//! a constructor
	PostEffectMotionBlur();

	//! a destructor
	virtual ~PostEffectMotionBlur();

	int GetNumberOfShaders() const override { return 1; }

	virtual const char *GetName() override;
	virtual const char *GetVertexFname(const int shaderIndex) override;
	virtual const char *GetFragmentFname(const int shaderIndex) override;

	virtual bool PrepUniforms(const int shaderIndex) override;
	virtual bool CollectUIValues(UICallback* uiCallback, const SViewInfo& viewInfo) override;

	virtual void Bind() override;
	virtual void UnBind() override;

protected:

	// shader locations
	enum { LOCATIONS_COUNT = 13 };
	union 
	{
		struct
		{
			// locations

			GLint		zNear;
			GLint		zFar;

			GLint		dt;
			
			GLint		upperClip;
			GLint		lowerClip;

			GLint		clipInfo;

			GLint		projInfo;
			GLint		projOrtho;
			GLint		InvQuarterResolution;
			GLint		InvFullResolution;

			GLint		uInverseModelViewMat;
			GLint		uPrevModelViewProj;
		};

		GLint		arr[LOCATIONS_COUNT];
	} mLoc;

	mat4			    mLastModelViewProj;
	float				mLastProj[16] = { 0.0f };

    double              m_LastTime{ 0.0 };
    int                 m_LastTimeFrame{ 0 };
};