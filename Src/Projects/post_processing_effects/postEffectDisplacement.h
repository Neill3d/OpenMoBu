
#pragma once

// postprocessing_effectDisplacement.h
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "postEffectBase.h"
#include <random>

///////////////////////////////////////////////////////////////////////////////////////
// PostEffectDisplacement

class PostEffectDisplacement : public PostEffectBase
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

	virtual bool PrepUniforms(const int shaderIndex) override;
	virtual bool CollectUIValues(UICallback* uiCallback, const SViewInfo& viewInfo) override;

	virtual void Bind() override;
	virtual void UnBind() override;

protected:

	// shader locations
	enum { LOCATIONS_COUNT = 9 };
	union 
	{
		struct
		{
			// locations

			GLint		upperClip;
			GLint		lowerClip;

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