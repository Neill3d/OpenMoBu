#ifndef __POST_PROCESSING_EFFECT_H__
#define __POST_PROCESSING_EFFECT_H__

// postprocessing_effectChain
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#include "GL/glew.h"

#include "graphics_framebuffer.h"
#include "postprocessing_data.h"

#include "glslShader.h"
#include "Framebuffer.h"

#include <memory>
#include <bitset>

//////////////////////////////

enum
{
	SHADER_TYPE_FISHEYE,
	SHADER_TYPE_COLOR,
	SHADER_TYPE_VIGNETTE,
	SHADER_TYPE_FILMGRAIN,
	SHADER_TYPE_LENSFLARE,
	SHADER_TYPE_SSAO,
	SHADER_TYPE_DOF,
	SHADER_TYPE_DISPLACEMENT,
	SHADER_TYPE_MOTIONBLUR,
	SHADER_TYPE_COUNT
};

enum class CompositionMask : uint32_t
{
	NO_MASK = 0,
	MASK_A = 0 >> 1,
	// TODO:
};

/// <summary>
/// uniforms needed for a common effect functionality, masking, clipping, etc.
/// </summary>
struct CommonEffectUniforms
{
public:

	void PrepareUniformLocations(GLSLShader* shader);

	/// <summary>
	/// must be called inside the binded glsl shader
	/// </summary>
	void UpdateUniforms(PostPersistentData* data);

	/// <summary>
	/// a property name in @sa PostPersistentData
	///  the property will be used to update useMasking glsl uniform value
	/// </summary>
	virtual const char* GetEnableMaskPropertyName() const = 0;

	/// <summary>
	/// glsl sampler slot binded for a mask texture
	/// </summary>
	static GLint GetMaskSamplerSlot() { return 4; }

protected:
	// common functionality of effects needs common uniforms
	GLint lowerClipLoc{ -1 };
	GLint upperClipLoc{ -1 };
	GLint useMaskLoc{ -1 };
};

/// <summary>
/// base class for every effect in a post processing chain
/// </summary>
struct PostEffectBase
{
public:

	//! a constructor
	PostEffectBase();
	//! a destructor
	virtual ~PostEffectBase();

	virtual int GetNumberOfShaders() const abstract; 

	virtual const char *GetName() abstract;
	virtual const char *GetVertexFname(const int shaderIndex) abstract;
	virtual const char *GetFragmentFname(const int shaderIndex) abstract;

	/// load and initialize shader from a specified location
	bool Load(const int shaderIndex, const char *vname, const char *fname);

	virtual bool PrepUniforms(const int shaderIndex);
	virtual bool CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera);		//!< grab main UI values for the effect

	/// new feature to have several passes for a specified effect
	virtual const int GetNumberOfPasses() const;
	virtual bool PrepPass(const int pass);

	virtual void Bind();
	virtual void UnBind();

	GLSLShader *GetShaderPtr();

protected:

	int mCurrentShader{ 0 };
	std::vector<GLSLShader*>	mShaders;

	std::bitset<4>	mCompositionMask;

	struct BaseLocations
	{
		GLint	mUseCompositionMask;
	};

 	void SetCurrentShader(const int index) { mCurrentShader = index; }
	void FreeShaders();

};

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
// PostEffectDOF

struct PostEffectDOF : public PostEffectBase, public CommonEffectUniforms
{
public:

	//! a constructor
	PostEffectDOF();

	//! a destructor
	virtual ~PostEffectDOF();

	int GetNumberOfShaders() const override { return 1; }

	virtual const char *GetName() override;
	virtual const char *GetVertexFname(const int shaderIndex) override;
	virtual const char *GetFragmentFname(const int shaderIndex) override;

	const char* GetEnableMaskPropertyName() const override { return "Depth Of Field Use Masking"; }

	virtual bool PrepUniforms(const int shaderIndex) override;
	virtual bool CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera) override;

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

///////////////////////////
// double buffer for effect chain

class PostEffectBuffers
{
public:

	//! a constructor
	PostEffectBuffers();
	//! a destructor
	~PostEffectBuffers();

	void ChangeContext();

	bool ReSize(const int w, const int h, bool useScale, double scaleFactor, bool filterMips=false);

	bool Ok();

	const GLuint PrepAndGetBufferObject();

	FrameBuffer *GetSrcBufferPtr();
	FrameBuffer *GetDstBufferPtr();

	FrameBuffer *GetBufferDepthPtr();
	FrameBuffer *GetBufferBlurPtr();
	FrameBuffer* GetBufferMaskPtr();

	FrameBuffer *GetBufferDownscalePtr();

	void SwapBuffers();

	const int GetWidth() const {
		return mWidth;
	}
	const int GetHeight() const {
		return mHeight;
	}
	const unsigned int GetPreviewWidth() const {
		return mPreviewWidth;
	}
	const unsigned int GetPreviewHeight() const {
		return mPreviewHeight;
	}
	// get a result of effect computation
	const GLuint GetFinalColor();
	const GLuint GetFinalFBO();

	const GLuint GetPreviewColor();
	const GLuint GetPreviewFBO();

	void		PreviewSignal() {
		mPreviewSignal = true;
	}

	//bool		PreviewCompressBegin();
	//bool		PreviewCompressEnd();
	//void		PrepPreviewCompressed();

	bool		PreviewOpenGLCompress(EImageCompression	compressionType, GLint &compressionCode);

	const GLuint GetPreviewCompressedColor();

	//void MapCompressedData(const float timestamp, Network::CPacketImageHeader &header);

	const size_t GetCompressedSize() const {
		return mCompressedSize;
	}
	const size_t GetUnCompressedSize() const {
		return mUnCompressSize;
	}
protected:

	// DONE: double local buffer
	std::unique_ptr<FrameBuffer>			mBufferPost0;
	std::unique_ptr<FrameBuffer>			mBufferPost1;

	std::unique_ptr<FrameBuffer>			mBufferDepth;		//!< buffer to store a linearize depth
	std::unique_ptr<FrameBuffer>			mBufferBlur;

	std::unique_ptr<FrameBuffer>			mBufferDownscale;	//!< output for a preview

	std::unique_ptr<FrameBuffer>			mBufferMasking;		//!< render models into mask texture

	// last local buffers resize
	int								mWidth;
	int								mHeight;
	GLuint							mOutputColorObject;

	bool							mPreviewSignal;
	bool							mPreviewRunning;

	// downscaled size
	unsigned int					mPreviewWidth;
	unsigned int					mPreviewHeight;

	int			mSrc;
	int			mDst;

	// compressed ETC1 output texture
	GLenum								mCompressionInternal;
	GLenum								mCompressionFormat;
	GLenum								mCompressionType;
	GLuint								mCompressedPreviewId;

	GLuint								mCompressOnFlyId;

	int									mCurPBO;
	GLuint								mPBOs[2];

	// temp
	int									mCurUnPack;
	GLuint								mUnPackPBOs[2];

	size_t								mUnCompressSize;
	size_t								mCompressedSize;

	//CompressImageHeader				mCompressHeader;

	void		FreeBuffers();
	void AllocPreviewTexture(int w, int h);
	void		FreeTextures();
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

	bool BeginFrame(PostEffectBuffers *buffers);
	bool Process(PostEffectBuffers *buffers, double time);

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
	std::unique_ptr<GLSLShader>			mShaderMix;		//!< needed for SSAO
	std::unique_ptr<GLSLShader>			mShaderDownscale;

	// order execution chain
	std::vector<PostEffectBase*>		mChain;

	GLint							mLocDepthLinearizeClipInfo;
	GLint							mLocBlurSharpness;
	GLint							mLocBlurRes;


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


#endif /* __POST_PROCESSING_EFFECT_H__ */
