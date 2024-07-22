#pragma once

// posteffectcolor
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
/// color correction post processing effect
/// </summary>
struct PostEffectColor : public PostEffectBase, public CommonEffectUniforms
{
public:
	//! a constructor
	PostEffectColor();
	//! a destructor
	virtual ~PostEffectColor();

	int GetNumberOfShaders() const override { return 1; }

	virtual const char* GetName() const override;
	virtual const char* GetVertexFname(const int shaderIndex) const override;
	virtual const char* GetFragmentFname(const int shaderIndex) const override;

	const char* GetEnableMaskPropertyName() const override { return "Color Correction Use Masking"; }

	virtual bool PrepUniforms(const int shaderIndex) override;
	virtual bool CollectUIValues(PostPersistentData* pData, int w, int h, FBCamera* pCamera) override;

protected:

	// shader locations
	enum { LOCATIONS_COUNT = 4 };
	union
	{
		struct
		{
			GLint		mResolution;
			GLint		mChromaticAberration;

			GLint		mLocCSB;
			GLint		mLocHue;
		};

		GLint		mLocations[LOCATIONS_COUNT];
	};
};
