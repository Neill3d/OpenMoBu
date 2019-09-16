
/**	\file	postprocessing_effect.cxx

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "postprocessing_effectChain.h"
#include "postprocessing_effectSSAO.h"
#include "postprocessing_effectDisplacement.h"
#include "postprocessing_effectMotionBlur.h"
#include "postprocessing_helper.h"

#define SHADER_FISH_EYE_NAME			"Fish Eye"
#define SHADER_FISH_EYE_VERTEX			"\\GLSL\\fishEye.vsh"
#define SHADER_FISH_EYE_FRAGMENT		"\\GLSL\\fishEye.fsh"

#define SHADER_COLOR_NAME				"Color Correction"
#define SHADER_COLOR_VERTEX				"\\GLSL\\simple.vsh"
#define SHADER_COLOR_FRAGMENT			"\\GLSL\\color.fsh"

#define SHADER_VIGNETTE_NAME			"Vignetting"
#define SHADER_VIGNETTE_VERTEX			"\\GLSL\\simple.vsh"
#define SHADER_VIGNETTE_FRAGMENT		"\\GLSL\\vignetting.fsh"

#define SHADER_FILMGRAIN_NAME			"Film Grain"
#define SHADER_FILMGRAIN_VERTEX			"\\GLSL\\simple.vsh"
#define SHADER_FILMGRAIN_FRAGMENT		"\\GLSL\\filmGrain.fsh"

#define SHADER_LENSFLARE_NAME			"Lens Flare"
#define SHADER_LENSFLARE_VERTEX			"\\GLSL\\simple.vsh"
#define SHADER_LENSFLARE_FRAGMENT		"\\GLSL\\lensFlare.fsh"

#define SHADER_DOF_NAME					"Depth Of Field"
#define SHADER_DOF_VERTEX				"\\GLSL\\simple.vsh"
#define SHADER_DOF_FRAGMENT				"\\GLSL\\dof.fsh"

// shared shaders

#define SHADER_DEPTH_LINEARIZE_VERTEX		"\\GLSL\\simple.vsh"
#define SHADER_DEPTH_LINEARIZE_FRAGMENT		"\\GLSL\\depthLinearize.fsh"

#define SHADER_BLUR_VERTEX					"\\GLSL\\simple.vsh"
#define SHADER_BLUR_FRAGMENT				"\\GLSL\\blur.fsh"

#define SHADER_MIX_VERTEX					"\\GLSL\\simple.vsh"
#define SHADER_MIX_FRAGMENT					"\\GLSL\\mix.fsh"

#define SHADER_DOWNSCALE_VERTEX				"\\GLSL\\downscale.vsh"
#define SHADER_DOWNSCALE_FRAGMENT			"\\GLSL\\downscale.fsh"

/////////////////////////////////////////////////////////////////////////
// EffectBase

PostEffectBase::PostEffectBase()
{
	mShader = nullptr;
}

PostEffectBase::~PostEffectBase()
{
	FreeShader();
}

const char *PostEffectBase::GetName()
{
	return "empty";
}
const char *PostEffectBase::GetVertexFname()
{
	return "empty";
}
const char *PostEffectBase::GetFragmentFname()
{
	return "empty";
}

// load and initialize shader from a specified location

void PostEffectBase::FreeShader()
{
	if (nullptr != mShader)
	{
		delete mShader;
		mShader = nullptr;
	}
}

bool PostEffectBase::Load(const char *vname, const char *fname)
{
	FreeShader();

	bool lSuccess = true;
	mShader = new GLSLShader();

	try
	{
		if (nullptr == mShader)
		{
			throw std::exception("failed to allocate memory for the shader object");
		}

		if (false == mShader->LoadShaders(vname, fname))
		{
			throw std::exception("failed to locate and load shader files");
		}

	}
	catch (const std::exception &e)
	{
		FBTrace("Post Effect Chain ERROR: %s\n", e.what());

		delete mShader;
		mShader = nullptr;

		lSuccess = false;
	}

	return lSuccess;
}

bool PostEffectBase::PrepUniforms()
{
	return false;
}

bool PostEffectBase::CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera)
{
	return false;
}

const int PostEffectBase::GetNumberOfPasses() const
{
	return 1;
}
bool PostEffectBase::PrepPass(const int pass)
{
	return true;
}

void PostEffectBase::Bind()
{
	if (nullptr != mShader)
	{
		mShader->Bind();
	}
}
void PostEffectBase::UnBind()
{
	if (nullptr != mShader)
	{
		mShader->UnBind();
	}
}

GLSLShader *PostEffectBase::GetShaderPtr() {
	return mShader;
}


///////////////////////////////////////////////////////////////////////////
// Effect FishEye

//! a constructor
PostEffectFishEye::PostEffectFishEye()
: PostEffectBase()
{
	for (int i = 0; i < LOCATIONS_COUNT; ++i)
		mLocations[i] = -1;
}

//! a destructor
PostEffectFishEye::~PostEffectFishEye()
{

}

const char *PostEffectFishEye::GetName()
{
	return SHADER_FISH_EYE_NAME;
}

const char *PostEffectFishEye::GetVertexFname()
{
	return SHADER_FISH_EYE_VERTEX;
}

const char *PostEffectFishEye::GetFragmentFname()
{
	return SHADER_FISH_EYE_FRAGMENT;
}

bool PostEffectFishEye::PrepUniforms()
{
	bool lSuccess = false;

	if (nullptr != mShader)
	{
		mShader->Bind();

		GLint loc = mShader->findLocation("sampler0");
		if (loc >= 0)
			glUniform1i(loc, 0);

		upperClip = mShader->findLocation("upperClip");
		lowerClip = mShader->findLocation("lowerClip");

		mLocAmount = mShader->findLocation("amount");
		mLocLensRadius = mShader->findLocation("lensradius");
		mLocSignCurvature = mShader->findLocation("signcurvature");

		mShader->UnBind();

		lSuccess = true;
	}

	return lSuccess;
}

bool PostEffectFishEye::CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera)
{
	bool lSuccess = false;

	const double _upperClip = pData->UpperClip;
	const double _lowerClip = pData->LowerClip;

	double amount = pData->FishEyeAmount;
	double lensradius = pData->FishEyeLensRadius;
	double signcurvature = pData->FishEyeSignCurvature;

	if (nullptr != mShader)
	{
		mShader->Bind();

		if (upperClip >= 0)
			glUniform1f(upperClip, 0.01f * (float)_upperClip);

		if (lowerClip >= 0)
			glUniform1f(lowerClip, 1.0f - 0.01f * (float)_lowerClip);

		if (mLocAmount >= 0)
			glUniform1f(mLocAmount, 0.01f * (float)amount);
		if (mLocLensRadius >= 0)
			glUniform1f(mLocLensRadius, (float)lensradius);
		if (mLocSignCurvature >= 0)
			glUniform1f(mLocSignCurvature, (float)signcurvature);

		mShader->UnBind();

		lSuccess = true;
	}
	
	return lSuccess;
}

/////////////////////////////////////////////////////////////////////////////////////
// Effect Color Correction

//! a constructor
PostEffectColor::PostEffectColor()
: PostEffectBase()
{
	for (int i = 0; i < LOCATIONS_COUNT; ++i)
		mLocations[i] = -1;
}

//! a destructor
PostEffectColor::~PostEffectColor()
{

}

const char *PostEffectColor::GetName() 
{
	return SHADER_COLOR_NAME;
}

const char *PostEffectColor::GetVertexFname()
{
	return SHADER_COLOR_VERTEX;
}

const char *PostEffectColor::GetFragmentFname()
{
	return SHADER_COLOR_FRAGMENT;
}

bool PostEffectColor::PrepUniforms()
{
	bool lSuccess = false;

	if (nullptr != mShader)
	{
		mShader->Bind();

		GLint loc = mShader->findLocation("sampler0");
		if (loc >= 0)
			glUniform1i(loc, 0);

		mResolution = mShader->findLocation("gResolution");
		mChromaticAberration = mShader->findLocation("gCA");

		mUpperClip = mShader->findLocation("upperClip");
		mLowerClip = mShader->findLocation("lowerClip");

		mLocCSB = mShader->findLocation("gCSB");
		mLocHue = mShader->findLocation("gHue");

		mShader->UnBind();

		lSuccess = true;
	}

	return lSuccess;
}

bool PostEffectColor::CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera)
{
	bool lSuccess = false;

	const double upperClip = pData->UpperClip;
	const double lowerClip = pData->LowerClip;

	const float chromatic_aberration = (pData->ChromaticAberration) ? 1.0f : 0.0f;
	const FBVector2d ca_dir = pData->ChromaticAberrationDirection;

	double saturation = 1.0 + 0.01 * pData->Saturation;
	double brightness = 1.0 + 0.01 * pData->Brightness;
	double contrast = 1.0 + 0.01 * pData->Contrast;
	double gamma = 0.01 * pData->Gamma;

	const float inverse = (pData->Inverse) ? 1.0f : 0.0f;
	double hue = 0.01 * pData->Hue;
	double hueSat = 0.01 * pData->HueSaturation;
	double lightness = 0.01 * pData->Lightness;

	if (nullptr != mShader)
	{
		mShader->Bind();

		if (mResolution >= 0)
		{
			glUniform2f(mResolution, static_cast<float>(w), static_cast<float>(h));
		}

		if (mChromaticAberration >= 0)
		{
			glUniform4f(mChromaticAberration, static_cast<float>(ca_dir[0]), static_cast<float>(ca_dir[1]), 0.0f, chromatic_aberration);
		}

		if (mUpperClip >= 0)
			glUniform1f(mUpperClip, 0.01f * (float)upperClip);

		if (mLowerClip >= 0)
			glUniform1f(mLowerClip, 1.0f - 0.01f * (float)lowerClip);

		if (mLocCSB >= 0)
			glUniform4f(mLocCSB, (float)contrast, (float)saturation, (float)brightness, (float)gamma);

		if (mLocHue >= 0)
			glUniform4f(mLocHue, (float)hue, (float)hueSat, (float)lightness, inverse);

		mShader->UnBind();

		lSuccess = true;
	}

	return lSuccess;
}

////////////////////////////////////////////////////////////////////////////////////
// post vignetting

//! a constructor
PostEffectVignetting::PostEffectVignetting()
: PostEffectBase()
{
	for (int i = 0; i < LOCATIONS_COUNT; ++i)
		mLocations[i] = -1;
}

//! a destructor
PostEffectVignetting::~PostEffectVignetting()
{

}

const char *PostEffectVignetting::GetName()
{
	return SHADER_VIGNETTE_NAME;
}
const char *PostEffectVignetting::GetVertexFname()
{
	return SHADER_VIGNETTE_VERTEX;
}
const char *PostEffectVignetting::GetFragmentFname()
{
	return SHADER_VIGNETTE_FRAGMENT;
}

bool PostEffectVignetting::PrepUniforms()
{
	bool lSuccess = false;

	if (nullptr != mShader)
	{
		mShader->Bind();

		GLint loc = mShader->findLocation("sampler0");
		if (loc >= 0)
			glUniform1i(loc, 0);

		mUpperClip = mShader->findLocation("upperClip");
		mLowerClip = mShader->findLocation("lowerClip");

		mLocAmount = mShader->findLocation("amount");
		mLocVignOut = mShader->findLocation("vignout");
		mLocVignIn = mShader->findLocation("vignin");
		mLocVignFade = mShader->findLocation("vignfade");

		mShader->UnBind();

		lSuccess = true;
	}

	return lSuccess;
}

bool PostEffectVignetting::CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera)
{
	bool lSuccess = false;

	const double upperClip = pData->UpperClip;
	const double lowerClip = pData->LowerClip;

	double amount = pData->VignAmount;
	double vignout = pData->VignOut;
	double vignin = pData->VignIn;
	double vignfade = pData->VignFade;

	if (nullptr != mShader)
	{
		mShader->Bind();

		if (mUpperClip >= 0)
			glUniform1f(mUpperClip, 0.01f * (float)upperClip);

		if (mLowerClip >= 0)
			glUniform1f(mLowerClip, 1.0f - 0.01f * (float)lowerClip);

		if (mLocAmount >= 0)
			glUniform1f(mLocAmount, 0.01f * (float)amount);
		if (mLocVignOut >= 0)
			glUniform1f(mLocVignOut, 0.01f * (float)vignout);
		if (mLocVignIn >= 0)
			glUniform1f(mLocVignIn, 0.01f * (float)vignin);
		if (mLocVignFade >= 0)
			glUniform1f(mLocVignFade, (float)vignfade);

		mShader->UnBind();

		lSuccess = true;
	}

	return lSuccess;
}


////////////////////////////////////////////////////////////////////////////////////
// post film grain

//! a constructor
PostEffectFilmGrain::PostEffectFilmGrain()
: PostEffectBase()
{
	for (int i = 0; i < LOCATIONS_COUNT; ++i)
		mLocations[i] = -1;
}

//! a destructor
PostEffectFilmGrain::~PostEffectFilmGrain()
{

}

const char *PostEffectFilmGrain::GetName()
{
	return SHADER_FILMGRAIN_NAME;
}
const char *PostEffectFilmGrain::GetVertexFname()
{
	return SHADER_FILMGRAIN_VERTEX;
}
const char *PostEffectFilmGrain::GetFragmentFname()
{
	return SHADER_FILMGRAIN_FRAGMENT;
}

bool PostEffectFilmGrain::PrepUniforms()
{
	bool lSuccess = false;

	if (nullptr != mShader)
	{
		mShader->Bind();

		GLint loc = mShader->findLocation("sampler0");
		if (loc >= 0)
			glUniform1i(loc, 0);

		upperClip = mShader->findLocation("upperClip");
		lowerClip = mShader->findLocation("lowerClip");

		textureWidth = mShader->findLocation("textureWidth");
		textureHeight = mShader->findLocation("textureHeight");

		timer = mShader->findLocation("timer");
		grainamount = mShader->findLocation("grainamount");
		colored = mShader->findLocation("colored");
		coloramount = mShader->findLocation("coloramount");
		grainsize = mShader->findLocation("grainsize");
		lumamount = mShader->findLocation("lumamount");

		mShader->UnBind();

		lSuccess = true;
	}

	return lSuccess;
}

bool PostEffectFilmGrain::CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera)
{
	bool lSuccess = false;

	const double _upperClip = pData->UpperClip;
	const double _lowerClip = pData->LowerClip;

	FBTime systemTime = (pData->FG_UsePlayTime) ? mSystem.LocalTime : mSystem.SystemTime;

	double timerMult = pData->FG_TimeSpeed;
	double _timer = 0.01 * timerMult * systemTime.GetSecondDouble();
	
	double _grainamount = pData->FG_GrainAmount;
	double _colored = (pData->FG_Colored) ? 1.0 : 0.0;
	double _coloramount = pData->FG_ColorAmount;
	double _grainsize = pData->FG_GrainSize;
	double _lumamount = pData->FG_LumAmount;

	if (nullptr != mShader)
	{
		mShader->Bind();

		if (upperClip >= 0)
			glUniform1f(upperClip, 0.01f * (float)_upperClip);

		if (lowerClip >= 0)
			glUniform1f(lowerClip, 1.0f - 0.01f * (float)_lowerClip);

		if (textureWidth >= 0)
			glUniform1f(textureWidth, (float)w);
		if (textureHeight >= 0)
			glUniform1f(textureHeight, (float)h);

		if (timer >= 0)
			glUniform1f(timer, (float)_timer);
		if (grainamount >= 0)
			glUniform1f(grainamount, 0.01f * (float)_grainamount);
		if (colored >= 0)
			glUniform1f(colored, (float)_colored);
		if (coloramount >= 0)
			glUniform1f(coloramount, 0.01f * (float)_coloramount);
		if (grainsize>= 0)
			glUniform1f(grainsize, 0.01f * (float)_grainsize);
		if (lumamount>= 0)
			glUniform1f(lumamount, 0.01f * (float)_lumamount);

		mShader->UnBind();

		lSuccess = true;
	}

	return lSuccess;
}


////////////////////////////////////////////////////////////////////////////////////
// post lens flare

//! a constructor
PostEffectLensFlare::PostEffectLensFlare()
: PostEffectBase()
{
	for (int i = 0; i < LOCATIONS_COUNT; ++i)
		mLocations[i] = -1;

	m_NumberOfPasses = 1;
}

//! a destructor
PostEffectLensFlare::~PostEffectLensFlare()
{

}

const char *PostEffectLensFlare::GetName()
{
	return SHADER_LENSFLARE_NAME;
}
const char *PostEffectLensFlare::GetVertexFname()
{
	return SHADER_LENSFLARE_VERTEX;
}
const char *PostEffectLensFlare::GetFragmentFname()
{
	return SHADER_LENSFLARE_FRAGMENT;
}

bool PostEffectLensFlare::PrepUniforms()
{
	bool lSuccess = false;

	if (nullptr != mShader)
	{
		mShader->Bind();

		GLint loc = mShader->findLocation("sampler0");
		if (loc >= 0)
			glUniform1i(loc, 0);

		upperClip = mShader->findLocation("upperClip");
		lowerClip = mShader->findLocation("lowerClip");

		amount = mShader->findLocation("amount");

		textureWidth = mShader->findLocation("textureWidth");
		textureHeight = mShader->findLocation("textureHeight");

		timer = mShader->findLocation("iTime");

		light_pos = mShader->findLocation("light_pos");

		tint = mShader->findLocation("tint");
		inner = mShader->findLocation("inner");
		outer = mShader->findLocation("outer");

		fadeToBorders = mShader->findLocation("fadeToBorders");
		borderWidth = mShader->findLocation("borderWidth");
		feather = mShader->findLocation("feather");

		mShader->UnBind();

		lSuccess = true;
	}

	return lSuccess;
}

bool PostEffectLensFlare::CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera)
{
	bool lSuccess = false;
	m_NumberOfPasses = 1;

	const double _upperClip = pData->UpperClip;
	const double _lowerClip = pData->LowerClip;

	FBTime systemTime = (pData->FlareUsePlayTime) ? mSystem.LocalTime : mSystem.SystemTime;

	double _amount = pData->FlareAmount;

	FBColor _tint = pData->FlareTint;
	double _inner = pData->FlareInner;
	double _outer = pData->FlareOuter;

	double timerMult = pData->FlareTimeSpeed;
	double _timer = 0.01 * timerMult * systemTime.GetSecondDouble();

	double _pos[3] = { 0.01 * pData->FlarePosX, 0.01 * pData->FlarePosY, 1.0 };

	const float _fadeToBorders = (pData->FlareFadeToBorders) ? 1.0f : 0.0f;
	double _borderWidth = pData->FlareBorderWidth;
	double _feather = pData->FlareBorderFeather;

	m_DepthAttenuation = (pData->FlareDepthAttenuation) ? 1.0f : 0.0f;

	// TODO: track light position in screen space
	if (pData->UseFlareLightObject && pData->FlareLight.GetCount() > 0)
	{
		m_NumberOfPasses = pData->FlareLight.GetCount();
		m_LightPositions.resize(m_NumberOfPasses);
		m_LightColors.resize(m_NumberOfPasses);

		for (int i = 0; i < m_NumberOfPasses; ++i)
		{
			FBLight *pLight = static_cast<FBLight*>(pData->FlareLight.GetAt(i));

			FBVector3d v;
			pLight->GetVector(v);

			FBMatrix mvp;
			pCamera->GetCameraMatrix(mvp, kFBModelViewProj);

			FBVector4d v4;
			FBVectorMatrixMult(v4, mvp, FBVector4d(v[0], v[1], v[2], 1.0));

			v4[0] = w * 0.5 * (v4[0] + 1.0);
			v4[1] = h * 0.5 * (v4[1] + 1.0);
			

			_pos[0] = v4[0] / w;
			_pos[1] = v4[1] / h;
			_pos[2] = v4[2]; // pCamera->FarPlaneDistance;

			m_LightPositions[i].Set(_pos);
			const FBColor color(pLight->DiffuseColor);
			m_LightColors[i].Set(color);
		}
		
		// relative coords to a screen size
		pData->FlarePosX = 100.0 * _pos[0] / w;
		pData->FlarePosY = 100.0 * _pos[2]; // _pos[1] / h;
	}

	if (nullptr != mShader)
	{
		mShader->Bind();

		if (upperClip >= 0)
			glUniform1f(upperClip, 0.01f * (float)_upperClip);

		if (lowerClip >= 0)
			glUniform1f(lowerClip, 1.0f - 0.01f * (float)_lowerClip);

		if (amount >= 0)
			glUniform1f(amount, 0.01f * (float)_amount);

		if (textureWidth >= 0)
			glUniform1f(textureWidth, (float)w);
		if (textureHeight >= 0)
			glUniform1f(textureHeight, (float)h);

		if (timer >= 0)
			glUniform1f(timer, (float)_timer);
		
		if (light_pos >= 0)
		{
			glUniform4f(light_pos, static_cast<float>(_pos[0]), static_cast<float>(_pos[1]), static_cast<float>(_pos[2]), 0.0f);
		}

		if (tint >= 0)
			glUniform4f(tint, (float)_tint[0], (float)_tint[1], (float)_tint[2], 1.0f);

		if (inner >= 0)
			glUniform1f(inner, 0.01f * (float)_inner);
		if (outer >= 0)
			glUniform1f(outer, 0.01f * (float)_outer);

		if (fadeToBorders >= 0)
			glUniform1f(fadeToBorders, _fadeToBorders);
		if (borderWidth >= 0)
			glUniform1f(borderWidth, (float)_borderWidth);
		if (feather >= 0)
			glUniform1f(feather, 0.01f * (float)_feather);

		mShader->UnBind();

		lSuccess = true;
	}

	return lSuccess;
}

const int PostEffectLensFlare::GetNumberOfPasses() const
{
	return m_NumberOfPasses;
}

bool PostEffectLensFlare::PrepPass(const int pass)
{
	// shader must be binded
	if (light_pos >= 0 && pass < static_cast<int>(m_LightPositions.size()))
	{
		const FBVector3d pos(m_LightPositions[pass]);
		glUniform4f(light_pos, static_cast<float>(pos[0]), static_cast<float>(pos[1]), static_cast<float>(pos[2]), m_DepthAttenuation);

		const FBColor _tint(m_LightColors[pass]);
		glUniform4f(tint, (float)_tint[0], (float)_tint[1], (float)_tint[2], 1.0f);
		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////
// post DOF

//! a constructor
PostEffectDOF::PostEffectDOF()
: PostEffectBase()
{
	for (int i = 0; i < LOCATIONS_COUNT; ++i)
		mLocations[i] = -1;
}

//! a destructor
PostEffectDOF::~PostEffectDOF()
{

}

const char *PostEffectDOF::GetName()
{
	return SHADER_DOF_NAME;
}
const char *PostEffectDOF::GetVertexFname()
{
	return SHADER_DOF_VERTEX;
}
const char *PostEffectDOF::GetFragmentFname()
{
	return SHADER_DOF_FRAGMENT;
}

bool PostEffectDOF::PrepUniforms()
{
	bool lSuccess = false;

	if (nullptr != mShader)
	{
		mShader->Bind();

		GLint loc = mShader->findLocation("colorSampler");
		if (loc >= 0)
			glUniform1i(loc, 0);
		loc = mShader->findLocation("depthSampler");
		if (loc >= 0)
			glUniform1i(loc, 1);

		upperClip = mShader->findLocation("upperClip");
		lowerClip = mShader->findLocation("lowerClip");

		focalDistance = mShader->findLocation("focalDistance");
		focalRange = mShader->findLocation("focalRange");

		textureWidth = mShader->findLocation("textureWidth");
		textureHeight = mShader->findLocation("textureHeight");

		zNear = mShader->findLocation("zNear");
		zFar = mShader->findLocation("zFar");

		fstop = mShader->findLocation("fstop");

		samples = mShader->findLocation("samples");
		rings = mShader->findLocation("rings");

		blurForeground = mShader->findLocation("blurForeground");

		manualdof = mShader->findLocation("manualdof");
		ndofstart = mShader->findLocation("ndofstart");
		ndofdist = mShader->findLocation("ndofdist");
		fdofstart = mShader->findLocation("fdofstart");
		fdofdist = mShader->findLocation("fdofdist");

		focusPoint = mShader->findLocation("focusPoint");

		CoC = mShader->findLocation("CoC");

		autofocus = mShader->findLocation("autofocus");
		focus = mShader->findLocation("focus");

		threshold = mShader->findLocation("threshold");
		gain = mShader->findLocation("gain");

		bias = mShader->findLocation("bias");
		fringe = mShader->findLocation("fringe");

		noise = mShader->findLocation("noise");

		pentagon = mShader->findLocation("pentagon");
		feather = mShader->findLocation("feather");

		debugBlurValue = mShader->findLocation("debugBlurValue");

		mShader->UnBind();

		lSuccess = true;
	}

	return lSuccess;
}

bool PostEffectDOF::CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera)
{
	bool lSuccess = false;

	const double _upperClip = pData->UpperClip;
	const double _lowerClip = pData->LowerClip;

	double _znear = pCamera->NearPlaneDistance;
	double _zfar = pCamera->FarPlaneDistance;

	double _focalDistance = pData->FocalDistance;
	double _focalRange = pData->FocalRange;
	double _fstop = pData->FStop;
	int _samples = pData->Samples;
	int _rings = pData->Rings;

	float _useFocusPoint = (pData->UseFocusPoint) ? 1.0f : 0.0f;
	FBVector2d _focusPoint = pData->FocusPoint;

	double _blurForeground = (pData->BlurForeground) ? 1.0 : 0.0;

	double _CoC = pData->CoC;
	double _threshold = pData->Threshold;
	
//	double _gain = pData->Gain;
	double _bias = pData->Bias;
	double _fringe = pData->Fringe;
	double _feather = pData->PentagonFeather;

	double _debugBlurValue = (pData->DebugBlurValue) ? 1.0 : 0.0;

	if (pData->UseCameraDOFProperties)
	{
		_focalDistance = pCamera->FocusSpecificDistance;
		_focalRange = pCamera->FocusAngle;

		FBModel *pInterest = nullptr;

		if (kFBFocusDistanceCameraInterest == pCamera->FocusDistanceSource)
			pInterest = pCamera->Interest;
		else if (kFBFocusDistanceModel == pCamera->FocusDistanceSource)
			pInterest = pCamera->FocusModel;

		if (nullptr != pInterest)
		{
			FBMatrix modelView, modelViewI;

			((FBModel*)pCamera)->GetMatrix(modelView);
			FBMatrixInverse(modelViewI, modelView);

			FBVector3d lPos;
			pInterest->GetVector(lPos);

			FBTVector p(lPos[0], lPos[1], lPos[2], 1.0);
			FBVectorMatrixMult(p, modelViewI, p);
			double dist = p[0];

			// Dont write to property
			// FocalDistance = dist;
			_focalDistance = dist;
		}
	}
	else
	if (pData->AutoFocus && pData->FocusObject.GetCount() > 0)
	{
		FBMatrix modelView, modelViewI;

		((FBModel*)pCamera)->GetMatrix(modelView);
		FBMatrixInverse(modelViewI, modelView);

		FBVector3d lPos;
		FBModel *pModel = (FBModel*)pData->FocusObject.GetAt(0);
		pModel->GetVector(lPos);

		FBTVector p(lPos[0], lPos[1], lPos[2]);
		FBVectorMatrixMult(p, modelViewI, p);
		double dist = p[0];

		// Dont write to property
		// FocalDistance = dist;
		_focalDistance = dist;
	}

	if (nullptr != mShader)
	{
		mShader->Bind();

		if (upperClip >= 0)
			glUniform1f(upperClip, 0.01f * (float)_upperClip);

		if (lowerClip >= 0)
			glUniform1f(lowerClip, 1.0f - 0.01f * (float)_lowerClip);

		if (textureWidth >= 0)
			glUniform1f(textureWidth, (float)w);
		if (textureHeight >= 0)
			glUniform1f(textureHeight, (float)h);

		if (focalDistance >= 0)
			glUniform1f(focalDistance, (float)_focalDistance);
		if (focalRange >= 0)
			glUniform1f(focalRange, (float)_focalRange);
		if (fstop >= 0)
			glUniform1f(fstop, (float)_fstop);

		if (zNear>= 0)
			glUniform1f(zNear, (float)_znear);
		if (zFar >= 0)
			glUniform1f(zFar, (float)_zfar);

		if (samples >= 0)
			glUniform1i(samples, _samples);
		if (rings >= 0)
			glUniform1i(rings, _rings);

		if (blurForeground >= 0)
			glUniform1f(blurForeground, (float)_blurForeground);

		if (CoC >= 0)
			glUniform1f(CoC, 0.01f * (float)_CoC);

		if (blurForeground >= 0)
			glUniform1f(blurForeground, (float)_blurForeground);

		if (threshold >= 0)
			glUniform1f(threshold, 0.01f * (float)_threshold);
		if (bias >= 0)
			glUniform1f(bias, 0.01f * (float)_bias);

		if (fringe>= 0)
			glUniform1f(fringe, 0.01f * (float)_fringe);
		if (feather>= 0)
			glUniform1f(feather, 0.01f * (float)_feather);

		if (debugBlurValue >= 0)
			glUniform1f(debugBlurValue, (float)_debugBlurValue);

		if (focusPoint >= 0)
			glUniform4f(focusPoint, 0.01f * (float)_focusPoint[0], 0.01f * (float)_focusPoint[1], 0.0f, _useFocusPoint);

		mShader->UnBind();

		lSuccess = true;
	}

	return lSuccess;
}


////////////////////////////////////////////////////////////////////////////////////
// post effect buffers

#define GL_COMPRESSED_ETC1_RGB8_OES                      0x8D64 // ETC1 - GL_OES_compressed_ETC1_RGB8_texture

PostEffectBuffers::PostEffectBuffers()
{
	mSrc = 0;
	mDst = 1;

	mWidth = 1;
	mHeight = 1;

	mPreviewSignal = false;
	mPreviewRunning = false;
	mPreviewWidth = 1;
	mPreviewHeight = 1;

	mCompressedPreviewId = 0;
	mCompressionFormat = GL_RGB;
	mCompressionInternal = GL_COMPRESSED_ETC1_RGB8_OES;
	mCompressionType = GL_UNSIGNED_SHORT_5_6_5;
	mCompressOnFlyId = 0;

	mCurPBO = 0;
	mPBOs[0] = mPBOs[1] = 0;
}

PostEffectBuffers::~PostEffectBuffers()
{
	ChangeContext();
}

void PostEffectBuffers::ChangeContext()
{
	FreeBuffers();
	FreeTextures();
}

unsigned int nearestPowerOf2(unsigned int value)
{
	unsigned int v = value; // compute the next highest power of 2 of 32-bit v

	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;

	return v;
}

bool PostEffectBuffers::ReSize(const int w, const int h, bool useScale, double scaleFactor, bool filterMips)
{
	bool lSuccess = true;

	mWidth = w;
	mHeight = h;

	// resize fbos
	const int flags = FrameBuffer::eCreateColorTexture | FrameBuffer::eCreateDepthTexture | FrameBuffer::eDeleteFramebufferOnCleanup;

	if (nullptr == mBufferPost0.get())
	{
		mBufferPost0.reset(new FrameBuffer(1, 1, flags));
		
		//mBufferPost0->SetDepthFormat(GL_DEPTH);
		//mBufferPost0->SetDepthInternalFormat(GL_DEPTH_COMPONENT32F);
		//mBufferPost0->SetDepthType(GL_FLOAT);
		
		mBufferPost0->SetDepthFormat(GL_DEPTH_STENCIL);
		mBufferPost0->SetDepthInternalFormat(GL_DEPTH24_STENCIL8);
		mBufferPost0->SetDepthType(GL_UNSIGNED_INT_24_8);

		mBufferPost0->SetClamp(0, GL_CLAMP_TO_EDGE);
		mBufferPost0->SetFilter(0, (filterMips) ? FrameBuffer::filterMipmap : FrameBuffer::filterLinear);
	}
	if (nullptr == mBufferPost1.get())
	{
		mBufferPost1.reset(new FrameBuffer(1, 1, flags));
		
		//mBufferPost1->SetDepthFormat(GL_DEPTH);
		//mBufferPost1->SetDepthInternalFormat(GL_DEPTH_COMPONENT32F);
		//mBufferPost1->SetDepthType(GL_FLOAT);
		
		mBufferPost1->SetDepthFormat(GL_DEPTH_STENCIL);
		mBufferPost1->SetDepthInternalFormat(GL_DEPTH24_STENCIL8);
		mBufferPost1->SetDepthType(GL_UNSIGNED_INT_24_8);

		mBufferPost1->SetClamp(0, GL_CLAMP_TO_EDGE);
		mBufferPost1->SetFilter(0, (filterMips) ? FrameBuffer::filterMipmap : FrameBuffer::filterLinear);
	}

	if (nullptr == mBufferDepth.get())
	{
		mBufferDepth.reset(new FrameBuffer(1, 1));
		mBufferDepth->SetColorFormat(0, GL_RED);
		mBufferDepth->SetColorInternalFormat(0, GL_R32F);
		mBufferDepth->SetColorType(0, GL_FLOAT);
		mBufferDepth->SetFilter(0, FrameBuffer::filterNearest);
		mBufferDepth->SetClamp(0, GL_CLAMP_TO_EDGE);
	}

	if (nullptr == mBufferBlur.get())
	{
		mBufferBlur.reset(new FrameBuffer(1, 1));
	}

	if (nullptr == mBufferDownscale.get())
	{
		mBufferDownscale.reset(new FrameBuffer(1, 1));
	}

	if (false == mBufferPost0->ReSize(w, h))
		lSuccess = false;
	if (false == mBufferPost1->ReSize(w, h))
		lSuccess = false;
	if (false == mBufferDepth->ReSize(w, h))
		lSuccess = false;
	if (false == mBufferBlur->ReSize(w, h))
		lSuccess = false;

	if (true == useScale)
	{
		double sw = 0.01 * (double)w * scaleFactor;
		double sh = 0.01 * (double)h * scaleFactor;

		// find nearest power of two
		mPreviewWidth = nearestPowerOf2((unsigned int)sw);
		mPreviewHeight = nearestPowerOf2((unsigned int)sh);

		//mPreviewWidth = 256;
		//mPreviewHeight = 128;

		if (true == mBufferDownscale->ReSize(mPreviewWidth, mPreviewHeight))
		{
			AllocPreviewTexture(mPreviewWidth, mPreviewHeight);
		}
	}

	if (true == lSuccess)
	{
		if (mPBOs[0] > 0)
		{
			glDeleteBuffers(2, mPBOs);
			mPBOs[0] = mPBOs[1] = 0;
		}
		mCurPBO = 0;
	}

	return lSuccess;
}

bool PostEffectBuffers::Ok()
{
	if (nullptr == mBufferPost0.get() || nullptr == mBufferPost1.get() || nullptr == mBufferDepth.get() || nullptr == mBufferBlur.get())
	{
		return false;
	}
	if (0 == mBufferPost0->GetFrameBuffer() || 0 == mBufferPost1->GetFrameBuffer() || 0 == mBufferDepth->GetFrameBuffer()
		|| 0 == mBufferBlur->GetFrameBuffer())
	{
		return false;
	}
	
	return true;
}

void PostEffectBuffers::FreeBuffers()
{
	mBufferPost0.reset(nullptr);
	mBufferPost1.reset(nullptr);
	mBufferDepth.reset(nullptr);
	mBufferBlur.reset(nullptr);
	mBufferDownscale.reset(nullptr);
}

const GLuint PostEffectBuffers::PrepAndGetBufferObject()
{
	mSrc = 0;
	mDst = 1;

	return mBufferPost0->GetFrameBuffer();
}

FrameBuffer *PostEffectBuffers::GetSrcBufferPtr()
{
	return (0 == mSrc) ? mBufferPost0.get() : mBufferPost1.get();
}
FrameBuffer *PostEffectBuffers::GetDstBufferPtr()
{
	return (0 == mDst) ? mBufferPost0.get() : mBufferPost1.get();
}
FrameBuffer *PostEffectBuffers::GetBufferDepthPtr()
{
	return mBufferDepth.get();
}
FrameBuffer *PostEffectBuffers::GetBufferBlurPtr()
{
	return mBufferBlur.get();
}

FrameBuffer *PostEffectBuffers::GetBufferDownscalePtr()
{
	return mBufferDownscale.get();
}

void PostEffectBuffers::SwapBuffers()
{
	// swap buffers
	int temp = mDst;
	mDst = mSrc;
	mSrc = temp;
}

// get a result of effect computation
const GLuint PostEffectBuffers::GetFinalColor()
{
	return (0==mSrc) ? mBufferPost0->GetColorObject() : mBufferPost1->GetColorObject();
}

const GLuint PostEffectBuffers::GetFinalFBO()
{
	return (0 == mSrc) ? mBufferPost0->GetFrameBuffer() : mBufferPost1->GetFrameBuffer();
}

const GLuint PostEffectBuffers::GetPreviewColor()
{
	return (mBufferDownscale.get()) ? mBufferDownscale->GetColorObject() : 0;
}

const GLuint PostEffectBuffers::GetPreviewFBO()
{
	return (mBufferDownscale.get()) ? mBufferDownscale->GetFrameBuffer() : 0;
}

const GLuint PostEffectBuffers::GetPreviewCompressedColor()
{
	//return mBufferDownscale->GetColorObject();
	return mCompressOnFlyId;
	return mCompressedPreviewId;
}

void PostEffectBuffers::AllocPreviewTexture(int w, int h)
{
	FreeTextures();

}

void PostEffectBuffers::FreeTextures()
{
	if (mCompressedPreviewId > 0)
	{
		glDeleteTextures(1, &mCompressedPreviewId);
		mCompressedPreviewId = 0;
	}

	if (mCompressOnFlyId > 0)
	{
		glDeleteTextures(1, &mCompressOnFlyId);
		mCompressOnFlyId = 0;
	}

	if (mPBOs[0] > 0)
	{
		glDeleteBuffers(2, mPBOs);
		mPBOs[0] = mPBOs[1] = 0;
		mCurPBO = 0;
	}
}

bool PostEffectBuffers::PreviewOpenGLCompress(EImageCompression	compressionType, GLint &compressionCode)
{
	if (nullptr == mBufferDownscale.get() || 0 == mBufferDownscale->GetColorObject())
		return false;

	if (0 == mBufferDownscale->GetColorObject())
		return false;

	if (mPreviewWidth <= 1 || mPreviewHeight <= 1)
		return false;

	size_t imageSize = mPreviewWidth * mPreviewHeight * 3;
	mUnCompressSize = imageSize;

	//glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
	
	if (0 == mPBOs[0])
	{
		glGenBuffers(2, mPBOs);

		// create 2 pixel buffer objects, you need to delete them when program exits.
		// glBufferDataARB with NULL pointer reserves only memory space.
		for (int i = 0; i<2; ++i)
		{
			glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBOs[i]);
			glBufferData(GL_PIXEL_PACK_BUFFER, imageSize, 0, GL_STREAM_DRAW);
		}
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	}

	//
	mCurPBO = 1 - mCurPBO;

	// read pixels from framebuffer to PBO
	mBufferDownscale->Bind();

	glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBOs[mCurPBO]);
	glReadPixels(0, 0, mPreviewWidth, mPreviewHeight, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	mBufferDownscale->UnBind();
	/*
	const GLuint srcId = mBufferDownscale->GetColorObject();
	glBindTexture(GL_TEXTURE_2D, srcId);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, mUnCompressData.data());
	glBindTexture(GL_TEXTURE_2D, 0);
	*/
	//glMemoryBarrier(GL_PIXEL_BUFFER_BARRIER_BIT);

	// map the PBO to process it's data by CPU
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, mPBOs[1 - mCurPBO]);
	//GLubyte *ptr = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

	//if (nullptr != ptr)
	{
		glHint(GL_TEXTURE_COMPRESSION_HINT, GL_FASTEST);

		if (0 == mCompressOnFlyId)
		{
			GLint internalFormat = GL_COMPRESSED_RGB;

			switch (compressionType)
			{
			case eImageCompressionETC2:
				internalFormat = GL_COMPRESSED_RGB8_ETC2;
				break;
			case eImageCompressionS3TC:
				internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
				break;
			case eImageCompressionASTC:
				internalFormat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR;
				break;
			}
			compressionCode = internalFormat;

			glGenTextures(1, &mCompressOnFlyId);
			glBindTexture(GL_TEXTURE_2D, mCompressOnFlyId);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, mPreviewWidth, mPreviewHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0); // ptr);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, mCompressOnFlyId);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mPreviewWidth, mPreviewHeight, GL_RGB, GL_UNSIGNED_BYTE, 0); // ptr);
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB, mPreviewWidth, mPreviewHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, mUnCompressData.data());

			// get stat
			GLint compressed = 0;
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, &compressed);

			if (compressed == GL_TRUE)
			{
				GLint compressed_size;
				GLint internalFormat;

				glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
				glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressed_size);

				mCompressedSize = compressed_size;
				compressionCode = internalFormat;
			}

			glBindTexture(GL_TEXTURE_2D, 0);
		}

		//glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	}

	// back to conventional pixel operation
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	
	return true;
}
/*
void PostEffectBuffers::MapCompressedData(const float timestamp, Network::CPacketImageHeader &header)
{
	glBindTexture(GL_TEXTURE_2D, mCompressOnFlyId);

	GLint compressed = 0;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, &compressed);

	if (compressed == GL_TRUE)
	{
		GLint compressed_size;
		GLint internalFormat;

		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressed_size);

		mCompressedData.resize(compressed_size + 1); // sizeof(CompressImageHeader));

		//CompressImageHeader *pHeader = (CompressImageHeader*)mCompressedData.data();
		//pHeader->timestamp = timestamp;
		header.aspect = 1.0f * (float)mWidth / (float)mHeight;
		header.width = (unsigned int)mPreviewWidth;
		header.height = (unsigned int)mPreviewHeight;
		header.internalFormat = internalFormat;
		header.dataSize = compressed_size;
		header.dataFormat = GL_UNSIGNED_BYTE;

		unsigned char *ptr = mCompressedData.data();
		glGetCompressedTexImage(GL_TEXTURE_2D, 0, ptr);

		//printf("compressed size - %d\n", compressed_size);
	}
	else
	{
		GLint internalFormat;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);

		GLint size = mPreviewWidth * mPreviewHeight * 3;
		mCompressedData.resize(size + 1); // sizeof(CompressImageHeader));

		header.aspect = 1.0f * (float)mWidth / (float)mHeight;
		header.width = (unsigned int)mPreviewWidth;
		header.height = (unsigned int)mPreviewHeight;
		header.internalFormat = GL_RGB8;
		header.dataSize = size;
		header.dataFormat = GL_UNSIGNED_BYTE;

		unsigned char *ptr = mCompressedData.data();
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, ptr);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

bool PostEffectBuffers::PreviewCompressBegin()
{
	if (false == mPreviewSignal)
		return false;

	if (0 == mCompressedPreviewId)
		return false;
	if (nullptr == mBufferDownscale.get() || 0 == mBufferDownscale->GetColorObject())
		return false;

	if (0 == mBufferDownscale->GetColorObject())
		return false;

	if (mPreviewWidth <= 1 || mPreviewHeight <= 1)
		return false;

	mUnCompressData.resize(mPreviewWidth * mPreviewHeight * 4);
	mCompressedData.resize(mPreviewWidth * mPreviewHeight / 2);

	const GLuint srcId = mBufferDownscale->GetColorObject();
	glBindTexture(GL_TEXTURE_2D, srcId);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, mUnCompressData.data());
	glBindTexture(GL_TEXTURE_2D, 0);

	int pitch = mPreviewWidth * 4; // * bpp
	CompressImageBegin(mCompressedData.data(), mPreviewWidth, mPreviewHeight, mUnCompressData.data(), pitch, 0);

	mPreviewRunning = false; // true;
	mPreviewSignal = false;
	return true;
}

bool PostEffectBuffers::PreviewCompressEnd()
{
	if (0 == mCompressedPreviewId)
		return false;
	if (nullptr == mBufferDownscale.get() || 0 == mBufferDownscale->GetColorObject())
		return false;

	if (false == mPreviewRunning)
		return false;

	CompressImageEnd();
	
	if (mCompressedData.size() > 0)
	{

		if (0 == mCompressedPreviewId)
		{
			glGenTextures(1, &mCompressedPreviewId);

			glBindTexture(GL_TEXTURE_2D, mCompressedPreviewId);
			glCompressedTexImage2D(GL_TEXTURE_2D, 0, mCompressionInternal, mPreviewWidth, mPreviewHeight, 0, mCompressedData.size(), mCompressedData.data());
			//glTexImage2D(GL_TEXTURE_2D, 0, mCompressionFormat, w, h, 0, GL_RGB, mCompressionType, nullptr);
			glBindTexture(GL_TEXTURE_2D, 0);
			CHECK_GL_ERROR();
		}
		
		glBindTexture(GL_TEXTURE_2D, mCompressedPreviewId);
		glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mPreviewWidth, mPreviewHeight, mCompressionInternal, mCompressedData.size(), mCompressedData.data());
		//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mPreviewWidth, mPreviewHeight, GL_RGB, mCompressionType, mCompressedData.data());
		glBindTexture(GL_TEXTURE_2D, 0);
		CHECK_GL_ERROR();
	}

	return true;
}
*/
////////////////////////////////////////////////////////////////////////////////////
// post effect chain

//! a constructor
PostEffectChain::PostEffectChain()
{
	mIsCompressedDataReady = false;
	mNeedReloadShaders = true;
	mLocDepthLinearizeClipInfo = -1;
	mLastCompressTime = 0.0;
}

//! a destructor
PostEffectChain::~PostEffectChain()
{

}

void PostEffectChain::ChangeContext()
{
	FreeShaders();
	mNeedReloadShaders = true;
	mIsCompressedDataReady = false;
	mLastCompressTime = 0.0;
}

bool PostEffectChain::Prep(PostPersistentData *pData, int w, int h, FBCamera *pCamera)
{
	bool lSuccess = true;

	mSettings = pData;
	mLastCamera = pCamera;

	if (false == mSettings.Ok())
		return false;

	if (true == mSettings.Ok())
	{
		if (true == mSettings->IsNeedToReloadShaders())
		{
			mNeedReloadShaders = true;
			mSettings->SetReloadShadersState(false);
		}
	}
	else
	{
		lSuccess = false;
	}

	if (mNeedReloadShaders)
	{
		if (false == LoadShaders())
			lSuccess = false;

		mNeedReloadShaders = false;
	}

	// update UI values

	if (true == mSettings->FishEye && nullptr != mFishEye.get())
		mFishEye->CollectUIValues(mSettings, w,h, pCamera);
	
	if (true == mSettings->ColorCorrection && nullptr != mColor.get())
		mColor->CollectUIValues(mSettings, w,h, pCamera);
	
	if (true == mSettings->Vignetting && nullptr != mVignetting.get())
		mVignetting->CollectUIValues(mSettings, w,h, pCamera);
	
	if (true == mSettings->FilmGrain && nullptr != mFilmGrain.get())
		mFilmGrain->CollectUIValues(mSettings, w,h, pCamera);

	if (true == mSettings->LensFlare && nullptr != mLensFlare.get())
		mLensFlare->CollectUIValues(mSettings, w,h, pCamera);

	if (true == mSettings->SSAO && nullptr != mSSAO.get())
		mSSAO->CollectUIValues(mSettings, w,h, pCamera);

	if (true == mSettings->DepthOfField && nullptr != mDOF.get())
		mDOF->CollectUIValues(mSettings, w, h, pCamera);

	if (true == mSettings->Displacement && nullptr != mDisplacement.get())
		mDisplacement->CollectUIValues(mSettings, w, h, pCamera);

	if (true == mSettings->MotionBlur && nullptr != mMotionBlur.get())
		mMotionBlur->CollectUIValues(mSettings, w, h, pCamera);

	return lSuccess;
}

bool PostEffectChain::BeginFrame(PostEffectBuffers *buffers)
{
	return true;

	/*
	if (false == buffers->Ok())
		return false;

	if (false == mSettings.Ok())
		return false;

	bool lSuccess = false;

	// if we need a preview
	if (mSettings->OutputPreview && buffers->GetPreviewWidth() > 1 && nullptr != buffers->GetBufferDownscalePtr())
	{
		
		// grab texture data

		lSuccess = buffers->PreviewCompressBegin();
	}

	return lSuccess;
	*/
}

bool PostEffectChain::Process(PostEffectBuffers *buffers, double systime)
{
	mIsCompressedDataReady = false;

	if (false == buffers->Ok())
		return false;

	if (false == mSettings.Ok())
		return false;

	// prepare chain count and order

	int count = 0;

	if (mSettings->FishEye)
		count += 1;
	if (mSettings->ColorCorrection)
		count += 1;
	if (mSettings->Vignetting)
		count += 1;
	if (mSettings->FilmGrain)
		count += 1;
	if (mSettings->LensFlare)
		count += 1;
	if (mSettings->SSAO)
		count += 1;
	if (mSettings->DepthOfField)
		count += 1;
	if (mSettings->Displacement)
		count += 1;
	if (mSettings->MotionBlur)
		count += 1;

	if (0 == count && 0 == mSettings->OutputPreview.AsInt())
		return false;

	mChain.resize(count);

	count = 0;

	int blurAndMix = -1;
	int blurAndMix2 = -1;

	// ordering HERE

	if (mSettings->SSAO)
	{
		mChain[count] = mSSAO.get();

		if (mSettings->SSAO_Blur)
		{
			blurAndMix = count;
		}
		count += 1;
	}
	if (mSettings->MotionBlur)
	{
		mChain[count] = mMotionBlur.get();
		count += 1;
	}
	if (mSettings->DepthOfField)
	{
		mChain[count] = mDOF.get();
		count += 1;
	}
	if (mSettings->ColorCorrection)
	{
		mChain[count] = mColor.get();
		if (mSettings->Bloom)
		{
			blurAndMix2 = count;
		}
		count += 1;
	}
	if (mSettings->LensFlare)
	{
		mChain[count] = mLensFlare.get();
		count += 1;
	}
	if (mSettings->Displacement)
	{
		mChain[count] = mDisplacement.get();
		count += 1;
	}
	if (mSettings->FishEye)
	{
		mChain[count] = mFishEye.get();
		count += 1;
	}
	if (mSettings->FilmGrain)
	{
		mChain[count] = mFilmGrain.get();
		count += 1;
	}
	if (mSettings->Vignetting)
	{
		mChain[count] = mVignetting.get();
		count += 1;
	}

	if (mSettings->SSAO)
	{
		const GLuint depthId = buffers->GetSrcBufferPtr()->GetDepthObject();

		// prep data

		float znear = (float)mLastCamera->NearPlaneDistance;
		float zfar = (float)mLastCamera->FarPlaneDistance;
		bool perspective = (mLastCamera->Type == FBCameraType::kFBCameraTypePerspective);

		float clipInfo[4];

		clipInfo[0] = znear * zfar;
		clipInfo[1] = znear - zfar;
		clipInfo[2] = zfar;
		clipInfo[3] = (perspective) ? 1.0f : 0.0f;

		FrameBuffer *pBufferDepth = buffers->GetBufferDepthPtr();

		// render

		pBufferDepth->Bind();

		mShaderDepthLinearize->Bind();

		glBindTexture(GL_TEXTURE_2D, depthId);

		if (mLocDepthLinearizeClipInfo >= 0)
			glUniform4fv(mLocDepthLinearizeClipInfo, 1, clipInfo);

		drawOrthoQuad2d(pBufferDepth->GetWidth(), pBufferDepth->GetHeight());

		mShaderDepthLinearize->UnBind();
		pBufferDepth->UnBind();

		// DONE: bind a depth texture
		const GLuint linearDepthId = pBufferDepth->GetColorObject();

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, linearDepthId);
		glActiveTexture(GL_TEXTURE0);
	}
	
	if (mSettings->DepthOfField)
	{
		const GLuint depthId = buffers->GetSrcBufferPtr()->GetDepthObject();

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthId);
		glActiveTexture(GL_TEXTURE0);
	}

	// compute effect chain with double buffer

	// DONE: when buffer is attached, buffer is used itself !
	bool lSuccess = false;
	const bool generateMips = mSettings->GenerateMipMaps;

	if (count > 0)
	{
		GLuint texid = 0;
		
		const int w = buffers->GetWidth();
		const int h = buffers->GetHeight();

		const float blurSharpness = 0.1f * (float)mSettings->SSAO_BlurSharpness;
		const float invRes[2] = { 1.0f / float(w), 1.0f / float(h) };

		// generate mipmaps for the first target
		texid = buffers->GetSrcBufferPtr()->GetColorObject();

		if (true == generateMips)
		{
			if (texid > 0)
			{
				glBindTexture(GL_TEXTURE_2D, texid);
				glGenerateMipmap(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}
		
		for (int i = 0; i < count; ++i)
		{
			mChain[i]->Bind();

			for (int j = 0; j < mChain[i]->GetNumberOfPasses(); ++j)
			{
				mChain[i]->PrepPass(j);

				texid = buffers->GetSrcBufferPtr()->GetColorObject();
				glBindTexture(GL_TEXTURE_2D, texid);

				if (generateMips)
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				}
				else
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				}

				buffers->GetDstBufferPtr()->Bind();

				drawOrthoQuad2d(w, h);

				buffers->GetDstBufferPtr()->UnBind(generateMips);

				//
				buffers->SwapBuffers();
			}

			mChain[i]->UnBind();

			// if we need more passes, blur and mix for SSAO
			if (i == blurAndMix || i == blurAndMix2)
			{
				if (false == mSettings->OnlyAO || (i == blurAndMix2))
				{
					// Bilateral Blur Pass

					texid = buffers->GetSrcBufferPtr()->GetColorObject();
					glBindTexture(GL_TEXTURE_2D, texid);

					buffers->GetBufferBlurPtr()->Bind();
					mShaderBlur->Bind();

					if (mLocBlurSharpness >= 0)
						glUniform1f(mLocBlurSharpness, blurSharpness);
					if (mLocBlurRes >= 0)
						glUniform2f(mLocBlurRes, invRes[0], invRes[1]);

					const float color_shift = (mSettings->Bloom) ? static_cast<float>(0.01 * mSettings->BloomMinBright) : 0.0f;
					mShaderBlur->setUniformFloat("g_ColorShift", color_shift);
					
					drawOrthoQuad2d(w, h);

					mShaderBlur->UnBind();
					buffers->GetBufferBlurPtr()->UnBind();

					//
					buffers->SwapBuffers();

					// Mix AO and Color Pass
					const GLuint blurId = buffers->GetBufferBlurPtr()->GetColorObject();
					texid = buffers->GetSrcBufferPtr()->GetColorObject();

					glActiveTexture(GL_TEXTURE3);
					glBindTexture(GL_TEXTURE_2D, blurId);
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, texid);

					buffers->GetDstBufferPtr()->Bind();
					mShaderMix->Bind();

					if (mSettings->Bloom)
					{
						mShaderMix->setUniformVector("gBloom", static_cast<float>(0.01 * mSettings->BloomTone), static_cast<float>(0.01 * mSettings->BloomStretch), 0.0f, 1.0f);
					}
					else
					{
						mShaderMix->setUniformVector("gBloom", 0.0f, 0.0f, 0.0f, 0.0f);
					}

					drawOrthoQuad2d(w, h);

					mShaderMix->UnBind();
					buffers->GetDstBufferPtr()->UnBind();

					glActiveTexture(GL_TEXTURE3);
					glBindTexture(GL_TEXTURE_2D, 0);
					glActiveTexture(GL_TEXTURE0);

					//
					buffers->SwapBuffers();
				}
				else
				{
					// Bilateral Blur Pass

					texid = buffers->GetSrcBufferPtr()->GetColorObject();
					glBindTexture(GL_TEXTURE_2D, texid);

					buffers->GetDstBufferPtr()->Bind();
					mShaderBlur->Bind();

					if (mLocBlurSharpness >= 0)
						glUniform1f(mLocBlurSharpness, blurSharpness);
					if (mLocBlurRes >= 0)
						glUniform2f(mLocBlurRes, invRes[0], invRes[1]);

					drawOrthoQuad2d(w, h);

					mShaderBlur->UnBind();
					buffers->GetDstBufferPtr()->UnBind();

					//
					buffers->SwapBuffers();
				}
			}
		}

		lSuccess = true;
	}

	// if we need a preview (rate - compress 25-30 frames per second)
	if (mSettings->OutputPreview)
	{
		int updaterate = mSettings->OutputUpdateRate.AsInt();
		if (updaterate <= 0 || updaterate > 30)
			updaterate = 10;

		double rate = 1.0 / updaterate;

		if (0.0 == mLastCompressTime || (systime - mLastCompressTime) > rate)
		{
			GLuint texid = buffers->GetSrcBufferPtr()->GetColorObject();
			glBindTexture(GL_TEXTURE_2D, texid);

			buffers->GetBufferDownscalePtr()->Bind();
			mShaderDownscale->Bind();

			GLint loc = mShaderDownscale->findLocation("texelSize");
			if (loc >= 0)
				glUniform2f(loc, 1.0f / (float)buffers->GetWidth(), 1.0f / (float)buffers->GetHeight());

			drawOrthoQuad2d(buffers->GetPreviewWidth(), buffers->GetPreviewHeight());

			mShaderDownscale->UnBind();
			buffers->GetBufferDownscalePtr()->UnBind();

			const unsigned int previewW = buffers->GetPreviewWidth();
			const unsigned int previewH = buffers->GetPreviewHeight();
			const double ratio = 1.0 * (double)buffers->GetWidth() / (double)buffers->GetHeight();

			if (mSettings->OutputUseCompression)
			{
				GLint compressionCode = 0;

				if (true == buffers->PreviewOpenGLCompress(mSettings->OutputCompression, compressionCode))
				{
					mSettings->SetPreviewTextureId(buffers->GetPreviewCompressedColor(), ratio, previewW, previewH,
						static_cast<int32_t>(buffers->GetUnCompressedSize()), 
						static_cast<int32_t>(buffers->GetCompressedSize()), 
						compressionCode, systime);

					mIsCompressedDataReady = true;
				}

				
			}
			else
			{
				GLint compressionCode = GL_RGB8;

				mSettings->SetPreviewTextureId(buffers->GetPreviewColor(), ratio, previewW, previewH,
					static_cast<int32_t>(buffers->GetUnCompressedSize()),
					static_cast<int32_t>(buffers->GetCompressedSize()), 
					compressionCode, systime);
					
			}
			
			mLastCompressTime = systime;
		}
	}

	//
	if (mSettings->DepthOfField)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
	}
	if (mSettings->SSAO)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
	}

	return lSuccess;
}


PostEffectBase *PostEffectChain::ShaderFactory(const int type, const char *shadersLocation)
{

	PostEffectBase *newEffect = nullptr;

	switch (type)
	{
	case SHADER_TYPE_FISHEYE:
		newEffect = new PostEffectFishEye();
		break;
	case SHADER_TYPE_COLOR:
		newEffect = new PostEffectColor();
		break;
	case SHADER_TYPE_VIGNETTE:
		newEffect = new PostEffectVignetting();
		break;
	case SHADER_TYPE_FILMGRAIN:
		newEffect = new PostEffectFilmGrain();
		break;
	case SHADER_TYPE_LENSFLARE:
		newEffect = new PostEffectLensFlare();
		break;
	case SHADER_TYPE_SSAO:
		newEffect = new PostEffectSSAO();
		break;
	case SHADER_TYPE_DOF:
		newEffect = new PostEffectDOF();
		break;
	case SHADER_TYPE_DISPLACEMENT:
		newEffect = new PostEffectDisplacement();
		break;
	case SHADER_TYPE_MOTIONBLUR:
		newEffect = new PostEffectMotionBlur();
		break;
	}


	try
	{
		if (nullptr == newEffect)
		{
			throw std::exception("failed to allocate memory for the shader");
		}

		FBString vertex_path(shadersLocation, newEffect->GetVertexFname() );
		FBString fragment_path(shadersLocation, newEffect->GetFragmentFname() );

		
		if (false == newEffect->Load(vertex_path, fragment_path))
		{
			throw std::exception("failed to load and prepare effect");
		}

		// samplers and locations
		newEffect->PrepUniforms();

	}
	catch (const std::exception &e)
	{
		FBTrace("Post Effect Chain ERROR: %s\n", e.what());

		delete newEffect;
		newEffect = nullptr;
	}

	return newEffect;
}

const bool PostEffectChain::CheckShadersPath(const char* path) const
{
	const char* test_shaders[] = {
		SHADER_DEPTH_LINEARIZE_VERTEX,
		SHADER_DEPTH_LINEARIZE_FRAGMENT,

		SHADER_BLUR_VERTEX,
		SHADER_BLUR_FRAGMENT,

		SHADER_MIX_VERTEX,
		SHADER_MIX_FRAGMENT,

		SHADER_DOWNSCALE_VERTEX,
		SHADER_DOWNSCALE_FRAGMENT	
	};

	for (const char* shader_path : test_shaders)
	{
		FBString full_path(path, shader_path);

		if (!IsFileExists(full_path))
		{
			return false;
		}
	}

	return true;
}

bool PostEffectChain::LoadShaders()
{
	FreeShaders();

	FBString shaders_path(mSystem.ApplicationPath);
	shaders_path = shaders_path + "\\plugins";

	bool status = true;

	if (!CheckShadersPath(shaders_path))
	{
		status = false;
		
		const FBStringList& plugin_paths = mSystem.GetPluginPath();

		for (int i = 0; i < plugin_paths.GetCount(); ++i)
		{
			if (CheckShadersPath(plugin_paths[i]))
			{
				shaders_path = plugin_paths[i];
				status = true;
				break;
			}
		}
	}

	if (status == false)
	{
		FBTrace("[PostProcessing] Failed to find shaders location!\n");
		return false;
	}
	
	FBTrace("[PostProcessing] Shaders Location - %s\n", shaders_path);

	mFishEye.reset(ShaderFactory(SHADER_TYPE_FISHEYE, shaders_path));
	mColor.reset(ShaderFactory(SHADER_TYPE_COLOR, shaders_path));
	mVignetting.reset(ShaderFactory(SHADER_TYPE_VIGNETTE, shaders_path));
	mFilmGrain.reset(ShaderFactory(SHADER_TYPE_FILMGRAIN, shaders_path));
	mLensFlare.reset(ShaderFactory(SHADER_TYPE_LENSFLARE, shaders_path));
	mSSAO.reset(ShaderFactory(SHADER_TYPE_SSAO, shaders_path));
	mDOF.reset(ShaderFactory(SHADER_TYPE_DOF, shaders_path));
	mDisplacement.reset(ShaderFactory(SHADER_TYPE_DISPLACEMENT, shaders_path));
	mMotionBlur.reset(ShaderFactory(SHADER_TYPE_MOTIONBLUR, shaders_path));

	// load shared shaders (blur, mix)

	bool lSuccess = true;

	GLSLShader *pNewShader = nullptr;

	try
	{
		//
		// DEPTH LINEARIZE

		pNewShader = new GLSLShader();

		if (nullptr == pNewShader)
		{
			throw std::exception("failed to allocate memory for a depth linearize shader");
		}

		FBString vertex_path(shaders_path, SHADER_DEPTH_LINEARIZE_VERTEX);
		FBString fragment_path(shaders_path, SHADER_DEPTH_LINEARIZE_FRAGMENT);

		if (false == pNewShader->LoadShaders(vertex_path, fragment_path))
		{
			throw std::exception("failed to load and prepare depth linearize shader");
		}

		// samplers and locations
		pNewShader->Bind();

		GLint loc = pNewShader->findLocation("depthSampler");
		if (loc >= 0)
			glUniform1i(loc, 0);
		mLocDepthLinearizeClipInfo = pNewShader->findLocation("gClipInfo");

		pNewShader->UnBind();

		mShaderDepthLinearize.reset(pNewShader);

		//
		// BLUR

		pNewShader = new GLSLShader();

		if (nullptr == pNewShader)
		{
			throw std::exception("failed to allocate memory for a blur shader");
		}

		vertex_path = FBString(shaders_path, SHADER_BLUR_VERTEX);
		fragment_path = FBString(shaders_path, SHADER_BLUR_FRAGMENT);

		if (false == pNewShader->LoadShaders(vertex_path, fragment_path))
		{
			throw std::exception("failed to load and prepare blur shader");
		}

		// samplers and locations
		pNewShader->Bind();

		loc = pNewShader->findLocation("sampler0");
		if (loc >= 0)
			glUniform1i(loc, 0);
		loc = pNewShader->findLocation("linearDepthSampler");
		if (loc >= 0)
			glUniform1i(loc, 2);

		mLocBlurSharpness = pNewShader->findLocation("g_Sharpness");
		mLocBlurRes = pNewShader->findLocation("g_InvResolutionDirection");

		pNewShader->UnBind();

		mShaderBlur.reset(pNewShader);

		//
		// MIX

		pNewShader = new GLSLShader();

		if (nullptr == pNewShader)
		{
			throw std::exception("failed to allocate memory for a mix shader");
		}

		vertex_path = FBString(shaders_path, SHADER_MIX_VERTEX);
		fragment_path = FBString(shaders_path, SHADER_MIX_FRAGMENT);

		if (false == pNewShader->LoadShaders(vertex_path, fragment_path))
		{
			throw std::exception("failed to load and prepare mix shader");
		}

		// samplers and locations
		pNewShader->Bind();

		loc = pNewShader->findLocation("sampler0");
		if (loc >= 0)
			glUniform1i(loc, 0);
		loc = pNewShader->findLocation("sampler1");
		if (loc >= 0)
			glUniform1i(loc, 3);

		pNewShader->UnBind();

		mShaderMix.reset(pNewShader);

		//
		// DOWNSCALE

		pNewShader = new GLSLShader();

		if (nullptr == pNewShader)
		{
			throw std::exception("failed to allocate memory for a downscale shader");
		}

		vertex_path = FBString(shaders_path, SHADER_DOWNSCALE_VERTEX);
		fragment_path = FBString(shaders_path, SHADER_DOWNSCALE_FRAGMENT);

		if (false == pNewShader->LoadShaders(vertex_path, fragment_path))
		{
			throw std::exception("failed to load and prepare downscale shader");
		}

		// samplers and locations
		pNewShader->Bind();

		loc = pNewShader->findLocation("sampler");
		if (loc >= 0)
			glUniform1i(loc, 0);
		
		pNewShader->UnBind();

		mShaderDownscale.reset(pNewShader);
	}
	catch (const std::exception &e)
	{
		FBTrace("Post Effect Chain ERROR: %s\n", e.what());

		if (nullptr != pNewShader)
		{
			delete pNewShader;
			pNewShader = nullptr;
		}

		lSuccess = false;
	}

	return lSuccess;
}

void PostEffectChain::FreeShaders()
{
	mFishEye.reset(nullptr);
	mColor.reset(nullptr);
	mVignetting.reset(nullptr);
	mFilmGrain.reset(nullptr);
	mLensFlare.reset(nullptr);
	mSSAO.reset(nullptr);
	mDOF.reset(nullptr);
	mDisplacement.reset(nullptr);
	mMotionBlur.reset(nullptr);

	mShaderDepthLinearize.reset(nullptr);
	mShaderBlur.reset(nullptr);
	mShaderMix.reset(nullptr);
	mShaderDownscale.reset(nullptr);
}