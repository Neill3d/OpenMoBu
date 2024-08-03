
#pragma once

// posteffectdisplacement.h
/*
Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectbase.h"
#include <random>

// forward
class PostPersistentData;

/// <summary>
/// displacement post processing effect
/// </summary>
struct PostEffectDisplacement : public PostEffectBase, public CommonEffectUniforms
{
public:

	//! a constructor
	PostEffectDisplacement();

	//! a destructor
	virtual ~PostEffectDisplacement();

	int GetNumberOfShaders() const override { return 1; }

	const char *GetName() const override;
	const char *GetVertexFname(const int shaderIndex) const override;
	const char *GetFragmentFname(const int shaderIndex) const override;

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