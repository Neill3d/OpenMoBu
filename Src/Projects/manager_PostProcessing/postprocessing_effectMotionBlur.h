
#pragma once

// postprocessing_effectMotionBlur
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "postprocessing_effectChain.h"
#include <random>

///////////////////////////////////////////////////////////////////////////////////////
// PostEffectMotionBlur

struct PostEffectMotionBlur : public PostEffectBase, public CommonEffectUniforms
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
	virtual bool CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera) override;

	virtual void Bind() override;
	virtual void UnBind() override;

protected:

	// shader locations
	enum { LOCATIONS_COUNT = 10 };
	union 
	{
		struct
		{
			// locations
			GLint		zNear;
			GLint		zFar;

			GLint		dt;
			
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

	FBMatrix			mLastModelViewProj;
	float				mLastProj[16] = { 0.0f };

	FBTime				mLastTime;
};