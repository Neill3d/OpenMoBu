
// posteffectmotionblur.cpp
/*
Sergei <Neill3d> Solokhin 2018-2025

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "posteffectmotionblur.h"
#include "postpersistentdata.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "postprocessing_helper.h"

//! a constructor
EffectShaderMotionBlur::EffectShaderMotionBlur(FBComponent* ownerIn)
	: PostEffectBufferShader(ownerIn)
{
	MakeCommonProperties();

	mDt = &AddProperty(ShaderProperty("dt", "dt", EPropertyType::FLOAT))
		.SetFlag(PropertyFlag::ShouldSkip, true); // NOTE: skip of automatic reading value and let it be done manually
	mClipInfo = &AddProperty(ShaderProperty("clipInfo", "gClipInfo", EPropertyType::VEC4))
		.SetFlag(PropertyFlag::ShouldSkip, true); // NOTE: skip of automatic reading value and let it be done manually
	mProjInfo = &AddProperty(ShaderProperty("projInfo", "projInfo", EPropertyType::VEC4))
		.SetFlag(PropertyFlag::ShouldSkip, true); // NOTE: skip of automatic reading value and let it be done manually
	mProjOrtho = &AddProperty(ShaderProperty("projOrtho", "projOrtho", EPropertyType::INT))
		.SetFlag(PropertyFlag::ShouldSkip, true); // NOTE: skip of automatic reading
	mInvQuarterResolution = &AddProperty(ShaderProperty("InvQuarterResolution", "InvQuarterResolution", EPropertyType::VEC2))
		.SetFlag(PropertyFlag::ShouldSkip, true); // NOTE: skip of automatic reading
	mInvFullResolution = &AddProperty(ShaderProperty("InvFullResolution", "InvFullResolution", EPropertyType::VEC2))
		.SetFlag(PropertyFlag::ShouldSkip, true); // NOTE: skip of automatic reading
}

const char* EffectShaderMotionBlur::GetUseMaskingPropertyName() const noexcept
{
	return PostPersistentData::MOTIONBLUR_USE_MASKING;
}
const char* EffectShaderMotionBlur::GetMaskingChannelPropertyName() const noexcept
{
	return PostPersistentData::MOTIONBLUR_MASKING_CHANNEL;
}

bool EffectShaderMotionBlur::OnCollectUI(const IPostEffectContext* effectContext, int maskIndex)
{
	if (!effectContext->GetCamera() || !effectContext->GetPostProcessData())
		return false;

	const float znear = effectContext->GetCameraNearDistance();
	const float zfar = effectContext->GetCameraFarDistance();

	FBCameraType cameraType;
	effectContext->GetCamera()->Type.GetData(&cameraType, sizeof(FBCameraType));
	const bool isPerspective = (cameraType == FBCameraType::kFBCameraTypePerspective);
	
	// calculate a diagonal fov

	// convert to mm
	const double filmWidth = 25.4 * effectContext->GetCamera()->FilmSizeWidth;
	const double filmHeight = 25.4 * effectContext->GetCamera()->FilmSizeHeight;

	const double diag = sqrt(filmWidth*filmWidth + filmHeight*filmHeight);
	const double focallen = effectContext->GetCamera()->FocalLength;

	const float fov = 2.0 * atan(diag / (focallen * 2.0));

	float clipInfo[4]
	{
		znear * zfar,
		znear - zfar,
		zfar,
		(isPerspective) ? 1.0f : 0.0f
	};

	//

	float P[16];
	const float* dproj = effectContext->GetProjectionMatrixF();

	for (int i = 0; i < 16; ++i)
	{
		P[i] = dproj[i];
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

	const int useOrtho = (false == isPerspective) ? 1 : 0;
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

//	float meters2viewspace = 1.0f;
//	float R = (float)pData->SSAO_Radius * meters2viewspace;
//	float R2 = R * R;
//	float negInvR2 = -1.0f / R2;
//	float RadiusToScreen = R * 0.5f * projScale;

	// resolution
	const int quarterWidth = ((effectContext->GetViewWidth() + 3) / 4);
	const int quarterHeight = ((effectContext->GetViewHeight() + 3) / 4);

	mClipInfo->SetValue(clipInfo[0], clipInfo[1], clipInfo[2], clipInfo[3]);
	mProjInfo->SetValue(projInfo[0], projInfo[1], projInfo[2], projInfo[3]);
	mProjOrtho->SetValue(projOrtho);
	mInvQuarterResolution->SetValue(1.0f / float(quarterWidth), 1.0f / float(quarterHeight));
	mInvFullResolution->SetValue(1.0f / float(effectContext->GetViewWidth()), 1.0f / float(effectContext->GetViewHeight()));

	const int localFrame = effectContext->GetLocalFrame(); 
		
	if (0 == localFrame || (localFrame != mLastLocalFrame))
	{
		mDt->SetValue(static_cast<float>(effectContext->GetLocalTimeDT()));
		mLastLocalFrame = effectContext->GetLocalFrame();
	}

	return true;
}
