
// postprocessing_effectSSAO.cpp
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "postprocessing_effectSSAO.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "postprocessing_helper.h"

#define SHADER_SSAO_NAME				"SSAO"
#define SHADER_SSAO_VERTEX				"\\GLSL\\simple.vsh"
#define SHADER_SSAO_FRAGMENT			"\\GLSL\\ssao.fsh"

#define AO_RANDOMTEX_SIZE 4

static const int  NUM_MRT = 8;
static const int  HBAO_RANDOM_SIZE = AO_RANDOMTEX_SIZE;
static const int  HBAO_RANDOM_ELEMENTS = HBAO_RANDOM_SIZE*HBAO_RANDOM_SIZE;
static const int  MAX_SAMPLES = 8;

float	hbaoRandom[HBAO_RANDOM_SIZE][HBAO_RANDOM_SIZE][4];

static const int        grid = 32;
static const float      globalscale = 16.0f;

////////////////////////////////////////////////////////////////////////////////////
// post SSAO

//! a constructor
PostEffectSSAO::PostEffectSSAO()
	: PostEffectBase()
	, e2(rd())
	, dist(0, 1.0)
{
	for (int i = 0; i < LOCATIONS_COUNT; ++i)
		mLoc.arr[i] = -1;

	// 
	hbao_random = 0;
	//InitMisc();

}

//! a destructor
PostEffectSSAO::~PostEffectSSAO()
{
	DeleteTextures();
}

void PostEffectSSAO::DeleteTextures()
{
	if (hbao_random > 0)
	{
		glDeleteTextures(1, &hbao_random);
		hbao_random = 0;
	}
}

const char *PostEffectSSAO::GetName()
{
	return SHADER_SSAO_NAME;
}
const char *PostEffectSSAO::GetVertexFname(const int)
{
	return SHADER_SSAO_VERTEX;
}
const char *PostEffectSSAO::GetFragmentFname(const int)
{
	return SHADER_SSAO_FRAGMENT;
}

bool PostEffectSSAO::PrepUniforms(const int shaderIndex)
{
	bool lSuccess = false;

	GLSLShader* mShader = mShaders[shaderIndex];
	if (nullptr != mShader)
	{
		mShader->Bind();

		GLint loc = mShader->findLocation("colorSampler");
		if (loc >= 0)
			glUniform1i(loc, 0);
		loc = mShader->findLocation("depthSampler");
		if (loc >= 0)
			glUniform1i(loc, 2);
		loc = mShader->findLocation("texRandom");
		if (loc >= 0)
			glUniform1i(loc, 4);

		mLoc.upperClip = mShader->findLocation("upperClip");
		mLoc.lowerClip = mShader->findLocation("lowerClip");

		mLoc.clipInfo = mShader->findLocation("gClipInfo");

		mLoc.projInfo = mShader->findLocation("projInfo");
		mLoc.projOrtho = mShader->findLocation("projOrtho");
		mLoc.InvQuarterResolution= mShader->findLocation("InvQuarterResolution");
		mLoc.InvFullResolution= mShader->findLocation("InvFullResolution");

		mLoc.RadiusToScreen= mShader->findLocation("RadiusToScreen");
		mLoc.R2 = mShader->findLocation("R2");
		mLoc.NegInvR2 = mShader->findLocation("NegInvR2");
		mLoc.NDotVBias = mShader->findLocation("NDotVBias");

		mLoc.AOMultiplier = mShader->findLocation("AOMultiplier");
		mLoc.PowExponent = mShader->findLocation("PowExponent");

		mLoc.OnlyAO = mShader->findLocation("OnlyAO");
		mLoc.hbaoRandom = mShader->findLocation("g_Jitter");

		mShader->UnBind();

		//
		InitMisc();

		lSuccess = true;
	}

	return lSuccess;
}

bool PostEffectSSAO::CollectUIValues(PostPersistentData *pData, int w, int h, FBCamera *pCamera)
{
	bool lSuccess = false;

	const double upperClip = pData->UpperClip;
	const double lowerClip = pData->LowerClip;

	float znear = (float) pCamera->NearPlaneDistance;
	float zfar = (float) pCamera->FarPlaneDistance;
	FBCameraType cameraType;
	pCamera->Type.GetData(&cameraType, sizeof(FBCameraType));
	bool perspective = (cameraType == FBCameraType::kFBCameraTypePerspective);
	//float fov = (float) pCamera->FieldOfView;

	// calculate a diagonal fov

	// convert to mm
	double filmWidth = 25.4 * pCamera->FilmSizeWidth;
	double filmHeight = 25.4 * pCamera->FilmSizeHeight;

	double diag = sqrt(filmWidth*filmWidth + filmHeight*filmHeight);
	double focallen = pCamera->FocalLength;

	float fov = 2.0 * atan(diag / (focallen * 2.0));

	float clipInfo[4];

	clipInfo[0] = znear * zfar;
	clipInfo[1] = znear - zfar;
	clipInfo[2] = zfar;
	clipInfo[3] = (perspective) ? 1.0f : 0.0f;

	float onlyAO = (pData->OnlyAO) ? 1.0f : 0.0f;

	if (pData->SSAO_Blur)
		onlyAO = 1.0f;

	FBMatrix dproj, dinvProj;
	pCamera->GetCameraMatrix(dproj, kFBProjection);
	pCamera->GetCameraMatrix(dinvProj, kFBProjInverse);

	float P[16];
	for (int i = 0; i < 16; ++i)
	{
		P[i] = (float)dproj[i];
	}

	float projInfoPerspective[] = {
		2.0f / (P[4 * 0 + 0]),       // (x) * (R - L)/N
		2.0f / (P[4 * 1 + 1]),       // (y) * (T - B)/N
		-(1.0f - P[4 * 2 + 0]) / P[4 * 0 + 0], // L/N
		-(1.0f + P[4 * 2 + 1]) / P[4 * 1 + 1], // B/N
	};

	float projInfoOrtho[] = {
		2.0f / (P[4 * 0 + 0]),      // ((x) * R - L)
		2.0f / (P[4 * 1 + 1]),      // ((y) * T - B)
		-(1.0f + P[4 * 3 + 0]) / P[4 * 0 + 0], // L
		-(1.0f - P[4 * 3 + 1]) / P[4 * 1 + 1], // B
	};

	int useOrtho = (false == perspective) ? 1 : 0;
	int projOrtho = useOrtho;
	float *projInfo = useOrtho ? projInfoOrtho : projInfoPerspective;

	float projScale;
	if (useOrtho){
		projScale = float(h) / (projInfoOrtho[1]);
	}
	else {
		projScale = float(h) / (tanf(fov * 0.5f) * 2.0f);
	}

	// radius

	float meters2viewspace = 1.0f;
	float R = (float)pData->SSAO_Radius * meters2viewspace;
	float R2 = R * R;
	float negInvR2 = -1.0f / R2;
	float RadiusToScreen = R * 0.5f * projScale;

	// ao
	float intensity = 0.01f * (float) pData->SSAO_Intensity;
	if (intensity < 0.0f)
		intensity = 0.0f;

	float bias = 0.01f * (float)pData->SSAO_Bias;
	if (bias < 0.0f)
		bias = 0.0f;
	else if (bias > 1.0f)
		bias = 1.0f;

	float aoMult = 1.0f / (1.0f - bias);

	// resolution
	int quarterWidth = ((w + 3) / 4);
	int quarterHeight = ((h + 3) / 4);

	GLSLShader* mShader = GetShaderPtr();
	if (nullptr != mShader)
	{
		mShader->Bind();

		if (mLoc.upperClip >= 0)
			glUniform1f(mLoc.upperClip, 0.01f * (float)upperClip);

		if (mLoc.lowerClip >= 0)
			glUniform1f(mLoc.lowerClip, 1.0f - 0.01f * (float)lowerClip);

		if (mLoc.clipInfo >= 0)
			glUniform4fv(mLoc.clipInfo, 1, clipInfo);

		if (mLoc.OnlyAO >= 0)
			glUniform1f(mLoc.OnlyAO, (float)onlyAO);

		// proj
		if (mLoc.projInfo >= 0)
			glUniform4fv(mLoc.projInfo, 1, projInfo);
		if (mLoc.projOrtho >= 0)
			glUniform1i(mLoc.projOrtho, projOrtho);

		// pass radius
		if (mLoc.RadiusToScreen >= 0)
			glUniform1f(mLoc.RadiusToScreen, (float)RadiusToScreen);
		if (mLoc.R2 >= 0)
			glUniform1f(mLoc.R2, (float)R2);
		if (mLoc.NegInvR2 >= 0)
			glUniform1f(mLoc.NegInvR2, (float)negInvR2);

		// ao
		if (mLoc.PowExponent >= 0)
			glUniform1f(mLoc.PowExponent, intensity);
		if (mLoc.NDotVBias >= 0)
			glUniform1f(mLoc.NDotVBias, bias);
		if (mLoc.AOMultiplier >= 0)
			glUniform1f(mLoc.AOMultiplier, aoMult);

		// resolution
		if (mLoc.InvQuarterResolution >= 0)
			glUniform2f(mLoc.InvQuarterResolution, 1.0f / float(quarterWidth), 1.0f / float(quarterHeight));
		if (mLoc.InvFullResolution >= 0)
			glUniform2f(mLoc.InvFullResolution, 1.0f / float(w), 1.0f / float(h));

		if (mLoc.hbaoRandom >= 0)
			glUniform4fv(mLoc.hbaoRandom, 1, mRandom);

		mShader->UnBind();

		lSuccess = true;
	}

	return lSuccess;
}

void PostEffectSSAO::Bind()
{
	// bind a random texture
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, hbao_random);
	glActiveTexture(GL_TEXTURE0);

	PostEffectBase::Bind();
}

void PostEffectSSAO::UnBind()
{
	// bind a random texture
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);

	PostEffectBase::UnBind();
}

bool PostEffectSSAO::InitMisc()
{
	float numDir = 8; // keep in sync to glsl

	float Rand1 = (float) dist(e2);
	float Rand2 = (float) dist(e2);
	
	// Use random rotation angles in [0,2PI/NUM_DIRECTIONS)
	float Angle = 2.f * M_PI * Rand1 / numDir;
	mRandom[0] = cosf(Angle);
	mRandom[1] = sinf(Angle);
	mRandom[2] = Rand2;
	mRandom[3] = 0;

	//signed short hbaoRandomShort[HBAO_RANDOM_ELEMENTS*MAX_SAMPLES * 4];

	for (int i = 0; i < HBAO_RANDOM_SIZE; i++)
	{
		for (int j = 0; j < HBAO_RANDOM_SIZE; ++j)
		{
			Rand1 = (float)dist(e2);
			Rand2 = (float)dist(e2);

			// Use random rotation angles in [0,2PI/NUM_DIRECTIONS)
			Angle = 2.f * M_PI * Rand1 / numDir;
			hbaoRandom[i][j][0] = cosf(Angle);
			hbaoRandom[i][j][1] = sinf(Angle);
			hbaoRandom[i][j][2] = Rand2;
			hbaoRandom[i][j][3] = 0;
		}
	
	}
	
	DeleteTextures();
	glGenTextures(1, &hbao_random);

	//glPixelStorei(GL_PACK_ALIGNMENT, 4);
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	glBindTexture(GL_TEXTURE_2D, hbao_random);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	//glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, HBAO_RANDOM_SIZE, HBAO_RANDOM_SIZE);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, HBAO_RANDOM_SIZE, HBAO_RANDOM_SIZE, GL_RGBA, GL_FLOAT, hbaoRandom);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, HBAO_RANDOM_SIZE, HBAO_RANDOM_SIZE, 0, GL_RGBA, GL_FLOAT, hbaoRandom);

	glBindTexture(GL_TEXTURE_2D, 0);

	//glPixelStorei(GL_PACK_ALIGNMENT, 1);
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/*
	glBindTexture(GL_TEXTURE_2D, hbao_random);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16_SNORM, HBAO_RANDOM_SIZE, HBAO_RANDOM_SIZE);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, HBAO_RANDOM_SIZE, HBAO_RANDOM_SIZE, GL_RGBA, GL_SHORT, hbaoRandomShort);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	*/
	return true;
}