#pragma once

// posteffectfilmgrain
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
/// film grain post processign effect
/// </summary>
struct PostEffectFilmGrain : public PostEffectBase, public CommonEffectUniforms
{
public:

	//! a constructor
	PostEffectFilmGrain();

	//! a destructor
	virtual ~PostEffectFilmGrain();

	int GetNumberOfShaders() const override { return 1; }

	virtual const char* GetName() override;
	virtual const char* GetVertexFname(const int shaderIndex) override;
	virtual const char* GetFragmentFname(const int shaderIndex) override;

	const char* GetEnableMaskPropertyName() const override { return "Grain Use Masking"; }

	virtual bool PrepUniforms(const int shaderIndex) override;
	virtual bool CollectUIValues(PostPersistentData* pData, int w, int h, FBCamera* pCamera) override;

protected:

	FBSystem		mSystem;

	// shader locations
	enum { LOCATIONS_COUNT = 8 };
	union
	{
		struct
		{
			// locations
			GLint		textureWidth;
			GLint		textureHeight;

			GLint		timer;

			GLint		grainamount; //!< = 0.05; //grain amount
			GLint		colored; //!< = false; //colored noise?
			GLint		coloramount; // = 0.6;
			GLint		grainsize; // = 1.6; //grain particle size (1.5 - 2.5)
			GLint		lumamount; // = 1.0; //
		};

		GLint		mLocations[LOCATIONS_COUNT];
	};
};
