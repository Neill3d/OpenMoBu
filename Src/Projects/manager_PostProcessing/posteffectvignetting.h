#pragma once

// posteffectvignetting
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
/// vignetting post processing effect
/// </summary>
struct PostEffectVignetting : public PostEffectBase, public CommonEffectUniforms
{
public:

	//! a constructor
	PostEffectVignetting();

	//! a destructor
	virtual ~PostEffectVignetting();

	int GetNumberOfShaders() const override { return 1; }

	virtual const char* GetName() override;
	virtual const char* GetVertexFname(const int shaderIndex) override;
	virtual const char* GetFragmentFname(const int shaderIndex) override;

	const char* GetEnableMaskPropertyName() const override { return "Vignetting Use Masking"; }

	virtual bool PrepUniforms(const int shaderIndex) override;
	virtual bool CollectUIValues(PostPersistentData* pData, int w, int h, FBCamera* pCamera) override;

protected:

	// shader locations
	enum { LOCATIONS_COUNT = 4 };
	union
	{
		struct
		{
			GLint		mLocAmount;	//!< amount of an effect applied
			GLint		mLocVignOut;
			GLint		mLocVignIn;
			GLint		mLocVignFade;
		};

		GLint		mLocations[LOCATIONS_COUNT];
	};
};