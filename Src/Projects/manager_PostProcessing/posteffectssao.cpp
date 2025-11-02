
// posteffectssao.cpp
/*
Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectssao.h"
#include "postpersistentdata.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "postprocessing_helper.h"

#define AO_RANDOMTEX_SIZE 4

static const int  NUM_MRT = 8;
static const int  HBAO_RANDOM_SIZE = AO_RANDOMTEX_SIZE;
static const int  HBAO_RANDOM_ELEMENTS = HBAO_RANDOM_SIZE*HBAO_RANDOM_SIZE;
static const int  MAX_SAMPLES = 8;



static const int        grid = 32;
static const float      globalscale = 16.0f;

//! a constructor
EffectShaderSSAO::EffectShaderSSAO(FBComponent* ownerIn)
	: PostEffectBufferShader(ownerIn)
	, e2(rd())
	, dist(0, 1.0)
{

	MakeCommonProperties();

	AddProperty(ShaderProperty("color", "colorSampler"))
		.SetType(EPropertyType::TEXTURE)
		.SetValue(CommonEffect::ColorSamplerSlot);

	AddProperty(ShaderProperty("random", "texRandom"))
		.SetType(EPropertyType::TEXTURE)
		.SetValue(CommonEffect::UserSamplerSlot);

	// NOTE: skip of automatic reading value and let it be done manually
	//mClipInfo = &AddProperty(ShaderProperty("clipInfo", "gClipInfo", nullptr))
	//	.SetType(EPropertyType::VEC4)
	//	.SetFlag(PropertyFlag::ShouldSkip, true); 
	mProjInfo = &AddProperty(ShaderProperty("projInfo", "projInfo", nullptr))
		.SetType(EPropertyType::VEC4)
		.SetFlag(PropertyFlag::ShouldSkip, true);
	mProjOrtho = &AddProperty(ShaderProperty("projOrtho", "projOrtho", nullptr))
		.SetType(EPropertyType::INT)
		.SetFlag(PropertyFlag::ShouldSkip, true);

	//mInvQuarterResolution = &AddProperty(ShaderProperty("InvQuarterResolution", "InvQuarterResolution", nullptr))
	//	.SetType(EPropertyType::VEC2)
	//	.SetFlag(PropertyFlag::ShouldSkip, true);
	mInvFullResolution = &AddProperty(ShaderProperty("InvFullResolution", "InvFullResolution", nullptr))
		.SetType(EPropertyType::VEC2)
		.SetFlag(PropertyFlag::ShouldSkip, true);

	mRadiusToScreen = &AddProperty(ShaderProperty("RadiusToScreen", "RadiusToScreen", nullptr))
		.SetType(EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::ShouldSkip, true);
	//mR2 = &AddProperty(ShaderProperty("R2", "R2", nullptr))
	//	.SetType(EPropertyType::FLOAT)
	//	.SetFlag(PropertyFlag::ShouldSkip, true);

	mNegInvR2 = &AddProperty(ShaderProperty("NegInvR2", "NegInvR2", nullptr))
		.SetType(EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::ShouldSkip, true);
	mNDotVBias = &AddProperty(ShaderProperty("NDotVBias", "NDotVBias", nullptr))
		.SetType(EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::ShouldSkip, true);

	mAOMultiplier = &AddProperty(ShaderProperty("AOMultiplier", "AOMultiplier", nullptr))
		.SetType(EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::ShouldSkip, true);

	mPowExponent = &AddProperty(ShaderProperty("PowExponent", "PowExponent", nullptr))
		.SetType(EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::ShouldSkip, true);

	mOnlyAO = &AddProperty(ShaderProperty("OnlyAO", "OnlyAO", nullptr))
		.SetType(EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::ShouldSkip, true);
	//mHbaoRandom = &AddProperty(ShaderProperty("g_Jitter", "g_Jitter", nullptr))
	//	.SetType(EPropertyType::VEC4)
	//	.SetFlag(PropertyFlag::ShouldSkip, true);

	// lazy initialize of random texture on first render
	hbaoRandomTexId = 0;
}

//! a destructor
EffectShaderSSAO::~EffectShaderSSAO()
{
	DeleteTextures();
}

const char* EffectShaderSSAO::GetUseMaskingPropertyName() const noexcept
{
	return PostPersistentData::SSAO_USE_MASKING;
}
const char* EffectShaderSSAO::GetMaskingChannelPropertyName() const noexcept
{
	return PostPersistentData::SSAO_MASKING_CHANNEL;
}

void EffectShaderSSAO::DeleteTextures()
{
	if (hbaoRandomTexId > 0)
	{
		glDeleteTextures(1, &hbaoRandomTexId);
		hbaoRandomTexId = 0;
	}
}

bool EffectShaderSSAO::OnCollectUI(const IPostEffectContext* effectContext, int maskIndex)
{
	FBCamera* camera = effectContext->GetCamera();
	const PostPersistentData* pData = effectContext->GetPostProcessData();

	//const float znear = (float) camera->NearPlaneDistance;
	//const float zfar = (float) camera->FarPlaneDistance;
	FBCameraType cameraType;
	camera->Type.GetData(&cameraType, sizeof(FBCameraType));
	const bool perspective = (cameraType == FBCameraType::kFBCameraTypePerspective);
	
	// calculate a diagonal fov

	// convert to mm
	const double filmWidth = 25.4 * camera->FilmSizeWidth;
	const double filmHeight = 25.4 * camera->FilmSizeHeight;

	const double diag = sqrt(filmWidth*filmWidth + filmHeight*filmHeight);
	const double focallen = camera->FocalLength;

	const float fov = 2.0 * atan(diag / (focallen * 2.0));
	/*
	const float clipInfo[4]
	{
		znear * zfar,
		znear - zfar,
		zfar,
		(perspective) ? 1.0f : 0.0f
	};
	*/
	const float onlyAO = (pData->OnlyAO || pData->SSAO_Blur) ? 1.0f : 0.0f;

	FBMatrix dproj, dinvProj;
	camera->GetCameraMatrix(dproj, kFBProjection);
	camera->GetCameraMatrix(dinvProj, kFBProjInverse);

	float P[16];
	for (int i = 0; i < 16; ++i)
	{
		P[i] = static_cast<float>(dproj[i]);
	}

	const float projInfoPerspective[] = {
		2.0f / (P[4 * 0 + 0]),       // (x) * (R - L)/N
		2.0f / (P[4 * 1 + 1]),       // (y) * (T - B)/N
		-(1.0f - P[4 * 2 + 0]) / P[4 * 0 + 0], // L/N
		-(1.0f + P[4 * 2 + 1]) / P[4 * 1 + 1], // B/N
	};

	const float projInfoOrtho[] = {
		2.0f / (P[4 * 0 + 0]),      // ((x) * R - L)
		2.0f / (P[4 * 1 + 1]),      // ((y) * T - B)
		-(1.0f + P[4 * 3 + 0]) / P[4 * 0 + 0], // L
		-(1.0f - P[4 * 3 + 1]) / P[4 * 1 + 1], // B
	};

	int useOrtho = (false == perspective) ? 1 : 0;
	int projOrtho = useOrtho;
	const float *projInfo = useOrtho ? projInfoOrtho : projInfoPerspective;

	float projScale;
	if (useOrtho){
		projScale = float(effectContext->GetViewHeight()) / (projInfoOrtho[1]);
	}
	else {
		projScale = float(effectContext->GetViewHeight()) / (tanf(fov * 0.5f) * 2.0f);
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

	const float aoMult = 1.0f / (1.0f - bias);

	// resolution
	//const int quarterWidth = ((effectContext->GetViewWidth() + 3) / 4);
	//const int quarterHeight = ((effectContext->GetViewHeight() + 3) / 4);

	//mClipInfo->SetValue(clipInfo[0], clipInfo[1], clipInfo[2], clipInfo[3]);
	mOnlyAO->SetValue(onlyAO);
	mProjInfo->SetValue(projInfo[0], projInfo[1], projInfo[2], projInfo[3]);
	mProjOrtho->SetValue(projOrtho);
	mRadiusToScreen->SetValue(RadiusToScreen);
	//mR2->SetValue(R2);
	mNegInvR2->SetValue(negInvR2);
	mPowExponent->SetValue(intensity);
	mNDotVBias->SetValue(bias);
	mAOMultiplier->SetValue(aoMult);
	//mInvQuarterResolution->SetValue(1.0f / float(quarterWidth), 1.0f / float(quarterHeight));
	mInvFullResolution->SetValue(1.0f / float(effectContext->GetViewWidth()), 1.0f / float(effectContext->GetViewHeight()));
	//mHbaoRandom->SetValue(mRandom[0], mRandom[1], mRandom[2], mRandom[3]);

	return true;
}

void EffectShaderSSAO::Bind()
{
	if (hbaoRandomTexId == 0)
	{
		InitMisc();
	}

	// bind a random texture
	glActiveTexture(GL_TEXTURE0 + CommonEffect::UserSamplerSlot);
	glBindTexture(GL_TEXTURE_2D, hbaoRandomTexId);
	glActiveTexture(GL_TEXTURE0);

	PostEffectBufferShader::Bind();
}

void EffectShaderSSAO::UnBind()
{
	// bind a random texture
	glActiveTexture(GL_TEXTURE0 + CommonEffect::UserSamplerSlot);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);

	PostEffectBufferShader::UnBind();
}

bool EffectShaderSSAO::InitMisc()
{
	float	hbaoRandom[HBAO_RANDOM_SIZE][HBAO_RANDOM_SIZE][4];

	float numDir = 8; // keep in sync to glsl

	float Rand1 = (float) dist(e2);
	float Rand2 = (float) dist(e2);
	
	// Use random rotation angles in [0,2PI/NUM_DIRECTIONS)
	float Angle = 2.f * M_PI * Rand1 / numDir;
	mRandom[0] = cosf(Angle);
	mRandom[1] = sinf(Angle);
	mRandom[2] = Rand2;
	mRandom[3] = 0;

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
	glGenTextures(1, &hbaoRandomTexId);

	glBindTexture(GL_TEXTURE_2D, hbaoRandomTexId);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, HBAO_RANDOM_SIZE, HBAO_RANDOM_SIZE, 0, GL_RGBA, GL_FLOAT, hbaoRandom);

	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}