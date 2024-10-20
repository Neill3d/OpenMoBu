#pragma once

// posteffectfisheye
/*
Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "GL/glew.h"
#include "posteffectbase.h"

namespace FBSDKNamespace
{
	// forward
	class FBCamera;
}

/// <summary>
/// fish eye post processing effect
/// </summary>
struct PostEffectFishEye : public PostEffectBase, public CommonEffectUniforms
{
public:

	//! a constructor
	PostEffectFishEye();

	//! a destructor
	virtual ~PostEffectFishEye();

	int GetNumberOfShaders() const override { return 1; }

	virtual const char* GetName() const override;
	virtual const char* GetVertexFname(const int shaderIndex) const override;
	virtual const char* GetFragmentFname(const int shaderIndex) const override;

	const char* GetEnableMaskPropertyName() const override { return "Fish Eye Use Masking"; }

	virtual bool PrepUniforms(const int shaderIndex) override;
	virtual bool CollectUIValues(PostPersistentData* pData, PostEffectContext& effectContext) override;

protected:

	enum { LOCATIONS_COUNT = 3 };
	union
	{
		struct
		{
			GLint		mLocAmount;
			GLint		mLocLensRadius;
			GLint		mLocSignCurvature;
		};

		GLint		mLocations[LOCATIONS_COUNT];
	};
};



