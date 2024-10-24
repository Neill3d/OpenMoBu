#pragma once

// posteffectdof
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
/// depth of field post processing effect
/// </summary>
struct PostEffectDOF : public PostEffectBase, public CommonEffectUniforms
{
public:

	//! a constructor
	PostEffectDOF();

	//! a destructor
	virtual ~PostEffectDOF();

	int GetNumberOfShaders() const override { return 1; }

	virtual const char *GetName() const override;
	virtual const char *GetVertexFname(const int shaderIndex) const override;
	virtual const char *GetFragmentFname(const int shaderIndex) const override;

	const char* GetEnableMaskPropertyName() const override { return "Depth Of Field Use Masking"; }

	virtual bool PrepUniforms(const int shaderIndex) override;
	virtual bool CollectUIValues(PostPersistentData *pData, PostEffectContext& effectContext) override;

protected:

	// shader locations
	enum { LOCATIONS_COUNT = 27 };
	union
	{
		struct
		{
			// locations
			GLint		focalDistance;
			GLint		focalRange;

			GLint		textureWidth;
			GLint		textureHeight;

			GLint		zNear;
			GLint		zFar;

			GLint		fstop;

			GLint		samples;
			GLint		rings;

			GLint		blurForeground;

			GLint		manualdof; // = false; //manual dof calculation
			GLint		ndofstart; // = 1.0; //near dof blur start
			GLint		ndofdist; // = 2.0; //near dof blur falloff distance
			GLint		fdofstart; // = 1.0; //far dof blur start
			GLint		fdofdist; // = 3.0; //far dof blur falloff distance

			GLint		focusPoint;

			GLint		CoC; // = 0.03;//circle of confusion size in mm (35mm film = 0.03mm)

			GLint		autofocus; // = false; //use autofocus in shader? disable if you use external focalDepth value
			GLint		focus; // = vec2(0.5,0.5); // autofocus point on screen (0.0,0.0 - left lower corner, 1.0,1.0 - upper right)
			

			GLint		threshold; // = 0.5; //highlight threshold;
			GLint		gain; // = 2.0; //highlight gain;

			GLint		bias; // = 0.5; //bokeh edge bias
			GLint		fringe; // = 0.7; //bokeh chromatic aberration/fringing

			GLint		noise; // = true; //use noise instead of pattern for sample dithering
			
			/*
			next part is experimental
			not looking good with small sample and ring count
			looks okay starting from samples = 4, rings = 4
			*/

			GLint		pentagon; // = false; //use pentagon as bokeh shape?
			GLint		feather; // = 0.4; //pentagon shape feather

			GLint		debugBlurValue;
		};

		GLint		mLocations[LOCATIONS_COUNT];
	};

};
