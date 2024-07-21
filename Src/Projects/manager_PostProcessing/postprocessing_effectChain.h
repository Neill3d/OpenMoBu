#pragma once

// postprocessing_effectChain
/*
Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#include "GL/glew.h"

#include "graphics_framebuffer.h"
#include "postprocessing_data.h"
#include "posteffectbase.h"

#include "glslShader.h"
#include "Framebuffer.h"

#include <memory>
#include <bitset>

// forward
class PostEffectBuffers;

///////////////////////////////////////////////////////////////////////////////////////
// PostEffectFishEye

struct PostEffectFishEye : public PostEffectBase, public CommonEffectUniforms
{
public:

	//! a constructor
	PostEffectFishEye();

	//! a destructor
	virtual ~PostEffectFishEye();

	int GetNumberOfShaders() const override { return 1; }

	virtual const char *GetName() override;
	virtual const char *GetVertexFname(const int shaderIndex) override;
	virtual const char *GetFragmentFname(const int shaderIndex) override;

	const char* GetEnableMaskPropertyName() const override { return "Fish Eye Use Masking"; }

	virtual bool PrepUniforms(const int shaderIndex) override;
	virtual bool CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera) override;

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

///////////////////////////////////////////////////////////////////////////////////////
// PostEffectColor

struct PostEffectColor : public PostEffectBase, public CommonEffectUniforms
{
public:
	//! a constructor
	PostEffectColor();
	//! a destructor
	virtual ~PostEffectColor();

	int GetNumberOfShaders() const override { return 1; }

	virtual const char *GetName() override;
	virtual const char *GetVertexFname(const int shaderIndex) override;
	virtual const char *GetFragmentFname(const int shaderIndex) override;

	const char* GetEnableMaskPropertyName() const override { return "Color Correction Use Masking"; }

	virtual bool PrepUniforms(const int shaderIndex) override;
	virtual bool CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera) override;

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

///////////////////////////////////////////////////////////////////////////////////////
// PostEffectVignetting

struct PostEffectVignetting : public PostEffectBase, public CommonEffectUniforms
{
public:

	//! a constructor
	PostEffectVignetting();

	//! a destructor
	virtual ~PostEffectVignetting();

	int GetNumberOfShaders() const override { return 1; }

	virtual const char *GetName() override;
	virtual const char *GetVertexFname(const int shaderIndex) override;
	virtual const char *GetFragmentFname(const int shaderIndex) override;

	const char* GetEnableMaskPropertyName() const override { return "Vignetting Use Masking"; }

	virtual bool PrepUniforms(const int shaderIndex) override;
	virtual bool CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera) override;

protected:

	// shader locations
	enum {LOCATIONS_COUNT=4};
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

///////////////////////////////////////////////////////////////////////////////////////
// PostEffectFilmGrain

struct PostEffectFilmGrain : public PostEffectBase, public CommonEffectUniforms
{
public:

	//! a constructor
	PostEffectFilmGrain();

	//! a destructor
	virtual ~PostEffectFilmGrain();

	int GetNumberOfShaders() const override { return 1; }

	virtual const char *GetName() override;
	virtual const char *GetVertexFname(const int shaderIndex) override;
	virtual const char *GetFragmentFname(const int shaderIndex) override;

	const char* GetEnableMaskPropertyName() const override { return "Grain Use Masking"; }

	virtual bool PrepUniforms(const int shaderIndex) override;
	virtual bool CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera) override;

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

///////////////////////////////////////////////////////////////////////////////////////
// PostEffectChain

class PostEffectChain
{

public:
	//! a constructor
	PostEffectChain();
	//! a destructor
	~PostEffectChain();

	void ChangeContext();
	/// w,h - local buffer size for processing, pCamera - current pane camera for processing
	bool Prep(PostPersistentData *pData, int w, int h, FBCamera *pCamera);

	bool BeginFrame(PostEffectBuffers* buffers);
	bool Process(PostEffectBuffers* buffers, double time);

	bool IsCompressedDataReady()
	{
		return mIsCompressedDataReady;
	}

protected:

	FBSystem								mSystem;
	HdlFBPlugTemplate<PostPersistentData>	mSettings;
	FBCamera								*mLastCamera;
	// instances of each effect
	std::unique_ptr<PostEffectBase>		mFishEye;
	std::unique_ptr<PostEffectBase>		mColor;
	std::unique_ptr<PostEffectBase>		mVignetting;
	std::unique_ptr<PostEffectBase>		mFilmGrain;
	std::unique_ptr<PostEffectBase>		mLensFlare;
	std::unique_ptr<PostEffectBase>		mSSAO;
	std::unique_ptr<PostEffectBase>		mDOF;
	std::unique_ptr<PostEffectBase>		mDisplacement;
	std::unique_ptr<PostEffectBase>		mMotionBlur;

	// shared shaders
	
	std::unique_ptr<GLSLShader>			mShaderDepthLinearize;	//!< linearize depth for other filters (DOF, SSAO, Bilateral Blur, etc.)
	std::unique_ptr<GLSLShader>			mShaderBlur;	//!< needed for SSAO
	std::unique_ptr<GLSLShader>			mShaderImageBlur; //!< for masking
	std::unique_ptr<GLSLShader>			mShaderMix;		//!< needed for SSAO
	std::unique_ptr<GLSLShader>			mShaderDownscale;

	// order execution chain
	std::vector<PostEffectBase*>		mChain;

	GLint							mLocDepthLinearizeClipInfo;
	GLint							mLocBlurSharpness;
	GLint							mLocBlurRes;
	GLint							mLocImageBlurScale;

	bool							mNeedReloadShaders;
	bool							mIsCompressedDataReady;
	double							mLastCompressTime;

	PostEffectBase *ShaderFactory(const int type, const char *shadersLocation);

	bool LoadShaders();
	void FreeShaders();

private:
	static bool CheckShadersPath(const char* path);

	/// <summary>
	/// return true if a global masking active and any of effects uses a masking
	/// </summary>
	bool HasMaskUsedByEffect() const;

	/// <summary>
	/// return true if there is any model with FXMaskingShader, so that we could render it into a masking texture
	/// </summary>
	bool HasAnyMaskedObject() const;
};
