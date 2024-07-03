
#pragma once

// postprocessing_effectDisplacement.h
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "postprocessing_effectChain.h"
#include <random>

///////////////////////////////////////////////////////////////////////////////////////
// PostEffectDisplacement

struct PostEffectDisplacement : public PostEffectBase, public CommonEffectUniforms
{
public:

	//! a constructor
	PostEffectDisplacement();

	//! a destructor
	virtual ~PostEffectDisplacement();

	int GetNumberOfShaders() const override { return 1; }

	const char *GetName() override;
	const char *GetVertexFname(const int shaderIndex) override;
	const char *GetFragmentFname(const int shaderIndex) override;

	const char* GetEnableMaskPropertyName() const override { return "Disp Use Masking"; }

	virtual bool PrepUniforms(const int shaderIndex) override;
	virtual bool CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera) override;

	virtual void Bind() override;
	virtual void UnBind() override;

protected:

	FBSystem		mSystem;

	// shader locations
	enum { LOCATIONS_COUNT = 7 };
	union 
	{
		struct
		{
			// locations

			GLint		iTime;
			GLint		iSpeed;

			GLint		useQuakeEffect;

			GLint		xDistMag;
			GLint		yDistMag;

			GLint		xSineCycles;
			GLint		ySineCycles;
		};

		GLint		arr[LOCATIONS_COUNT];
	} mLoc;

};