
#pragma once

// posteffectssao
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectbase.h"
#include <random>

// forward
class PostPersistentData;

/// <summary>
/// screen space ambient occlusion post processing effect
/// </summary>
struct PostEffectSSAO : public PostEffectBase, public CommonEffectUniforms
{
public:

	//! a constructor
	PostEffectSSAO();

	//! a destructor
	virtual ~PostEffectSSAO();

	int GetNumberOfShaders() const override { return 1; }

	virtual const char *GetName() const override;
	virtual const char *GetVertexFname(const int shaderIndex) const override;
	virtual const char *GetFragmentFname(const int shaderIndex) const override;

	const char* GetEnableMaskPropertyName() const override { return "SSAO Use Masking"; }

	virtual bool PrepUniforms(const int shaderIndex) override;
	virtual bool CollectUIValues(PostPersistentData* pData, PostEffectContext& effectContext) override;

	virtual void Bind() override;
	virtual void UnBind() override;

	static GLint GetColorSamplerSlot() { return 0; }
	static GLint GetDepthSamplerSlot() { return 2; }
	static GLint GetRandomSamplerSlot() { return 5; }

protected:

	// shader locations
	enum { LOCATIONS_COUNT = 13 };
	union 
	{
		struct
		{
			// locations

			GLint		clipInfo;

			GLint		projInfo;
			GLint		projOrtho;
			GLint		InvQuarterResolution;
			GLint		InvFullResolution;

			GLint		RadiusToScreen;
			GLint		R2;
			GLint		NegInvR2;
			GLint		NDotVBias;

			GLint		AOMultiplier;
			GLint		PowExponent;

			GLint		OnlyAO;
			GLint		hbaoRandom;
		};

		GLint		arr[LOCATIONS_COUNT];
	} mLoc;

	GLuint	hbao_random;

	float		mRandom[4]{ 0.0f };
	

	std::random_device					rd;
	std::mt19937						e2;		// engine
	std::uniform_real_distribution<>	dist;	// distribution

	bool InitMisc();
	void DeleteTextures();
};