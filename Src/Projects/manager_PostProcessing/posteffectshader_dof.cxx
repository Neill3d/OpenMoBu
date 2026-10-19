
/**	\file	posteffectshader_dof.cxx

Sergei <Neill3d> Solokhin 2018-2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectshader_dof.h"
#include "postpersistentdata.h"
#include "shaderproperty_writer.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#include <cmath>

#include "postprocessing_helper.h"
#include <hashUtils.h>

uint32_t EffectShaderDOF::SHADER_NAME_HASH = xxhash32(EffectShaderDOF::SHADER_NAME);

EffectShaderDOF::EffectShaderDOF(FBComponent* ownerIn)
	: PostEffectBufferShader(ownerIn)
	, e2(rd())
	, dist(0, 1.0)
{}

EffectShaderDOF::~EffectShaderDOF()
{
	DeleteTexture();
}

const char* EffectShaderDOF::GetUseMaskingPropertyName() const noexcept
{
	return PostPersistentData::DOF_USE_MASKING;
}
const char* EffectShaderDOF::GetMaskingChannelPropertyName() const noexcept
{
	return PostPersistentData::DOF_MASKING_CHANNEL;
}

void EffectShaderDOF::OnPopulateProperties(ShaderPropertyScheme* scheme)
{
	// Sample slots

	scheme->AddProperty("color", "colorSampler")
		.SetType(EPropertyType::TEXTURE)
		.SetFlag(PropertyFlag::SKIP)
		.SetDefaultValue(CommonEffect::ColorSamplerSlot);
	/*
	scheme->AddProperty("random", "randomSampler")
		.SetType(EPropertyType::TEXTURE)
		.SetFlag(PropertyFlag::SKIP)
		.SetDefaultValue(CommonEffect::UserSamplerSlot);
	*/

	// Core depth of field parameters
	mFocalDistance = scheme->AddProperty(PostPersistentData::DOF_FOCAL_DISTANCE, "focalDistance", EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	mFocalRange = scheme->AddProperty(PostPersistentData::DOF_FOCAL_RANGE, "focalRange", EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	mFStop = scheme->AddProperty(PostPersistentData::DOF_FSTOP, "fstop", EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	mCoC = scheme->AddProperty(PostPersistentData::DOF_COC, "CoC", EPropertyType::FLOAT)
		.SetScale(0.01f)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();
	
	mBlurRadius = scheme->AddProperty(PostPersistentData::DOF_BLUR_RADIUS, "blurRadius", EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	mLegacyFocusAngle = scheme->AddProperty(PostPersistentData::DOF_LEGACY_CAMERA_FOCUS_ANGLE, "legacyFocusAngle", EPropertyType::BOOL)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	// the lens model of the camera DOF and a scale of its blur width, computed from the camera focus angle
	//  (no property of the data), the scale is 0 for a zero focus angle, so a flag is needed
	mUseLensBlur = scheme->AddProperty("Use Lens Blur", "useLensBlur", EPropertyType::BOOL)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	mLensBlurScale = scheme->AddProperty("Lens Blur Scale", "lensBlurScale", EPropertyType::FLOAT)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	// Rendering parameters

	mSamples = scheme->AddProperty(PostPersistentData::DOF_SAMPLES, "samples", EPropertyType::INT)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	mRings = scheme->AddProperty(PostPersistentData::DOF_RINGS, "rings", EPropertyType::INT)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	// Focus control
	mFocusPoint = scheme->AddProperty(PostPersistentData::DOF_FOCUS_POINT, "focusPoint", EPropertyType::VEC4)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	// Visual enhancement parameters
	mBlurForeground = scheme->AddProperty(PostPersistentData::DOF_BLUR_FOREGROUND, "blurForeground", EPropertyType::BOOL)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	mThreshold = scheme->AddProperty(PostPersistentData::DOF_THRESHOLD, "threshold", EPropertyType::FLOAT)
		.SetScale(0.01f)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	mGain = scheme->AddProperty(PostPersistentData::DOF_GAIN, "gain", EPropertyType::FLOAT)
		.SetScale(0.01f)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	mBias = scheme->AddProperty(PostPersistentData::DOF_BIAS, "bias", EPropertyType::FLOAT)
		.SetScale(0.01f)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	mFringe = scheme->AddProperty(PostPersistentData::DOF_FRINGE, "fringe", EPropertyType::FLOAT)
		.SetScale(0.01f)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	mNoise = scheme->AddProperty(PostPersistentData::DOF_NOISE, "useNoise", EPropertyType::BOOL)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	// Experimental bokeh shape parameters
	mPentagon = scheme->AddProperty(PostPersistentData::DOF_PENTAGON, "pentagon", EPropertyType::BOOL)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	mFeather = scheme->AddProperty(PostPersistentData::DOF_PENTAGON_FEATHER, "feather", EPropertyType::FLOAT)
		.SetScale(0.01f)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	// Debug utilities
	mDebugBlurValue = scheme->AddProperty(PostPersistentData::DOF_DEBUG_BLUR_VALUE, "debugBlurValue", EPropertyType::BOOL)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();

	mDebugShowFocus = scheme->AddProperty(PostPersistentData::DOF_DEBUG_SHOW_FOCUS, "debugShowFocus", EPropertyType::BOOL)
		.SetFlag(PropertyFlag::SKIP)
		.GetProxy();
}

bool EffectShaderDOF::OnCollectUI(PostEffectContextProxy* effectContext, int maskIndex) const
{
	PostPersistentData* pData = effectContext->GetPostProcessData();
	if (!pData)
		return false;

	FBCamera* camera = effectContext->GetCamera();
	if (!camera)
		return false;

	double _focalDistance = pData->FocalDistance;
	double _focalRange = pData->FocalRange;
	double _fstop = pData->FStop;

	int _samples = pData->Samples;
	int _rings = pData->Rings;

	float _useFocusPoint = (pData->UseFocusPoint) ? 1.0f : 0.0f;
	FBVector2d _focusPoint = pData->FocusPoint;

	const bool _blurForeground = pData->BlurForeground;

	double _CoC = pData->CoC;
	double _blurRadius = pData->BlurRadius;
	double _threshold = pData->Threshold;
	
	double _gain = pData->Gain;
	double _bias = pData->Bias;
	double _fringe = pData->Fringe;
	double _feather = pData->PentagonFeather;

	const bool _debugBlurValue = pData->DebugBlurValue;
	const bool _debugShowFocus = pData->DebugShowFocus;

	// original blur model is applied only for the camera focus angle
	bool _useLegacyFocusAngle = false;
	// the lens model of the camera DOF (otherwise a model with a band is used) and a scale of its blur width,
	//  zero focus angle gives a zero scale, it is a sharp image
	bool _useLensBlur = false;
	float _lensBlurScale = 0.0f;

	if (pData->UseCameraDOFProperties)
	{
		// focus distance and lens values are resolved once per frame in the context cache (shared with system uniforms)
		const CameraLens& lens = effectContext->GetCameraLens();
		_focalDistance = lens.focusDistance;

		const double focusAngle = lens.focusAngle;
		if (pData->LegacyCameraFocusAngle)
		{
			// legacy: focus angle is assigned to the focal range as is, without any calculations,
			//  the shader uses it as a focal length of the original thin lens model
			//  and the original blur spread is about 1 pixel per ring step
			_focalRange = focusAngle;
			_blurRadius = 1.0;
			_useLegacyFocusAngle = true;
		}
		else
		{
			// lens model of the MotionBuilder camera DOF:
			//   width = A * focusAngle * focalLengthInPixels * |depth - focus| / (depth * focus)
			// the shader turns the width into a blur amount, the Blur Radius property is a maximum reach of a ring
			// a focal length in pixels comes from the projection matrix of the camera (an orthogonal one has no blur)
			constexpr double kApertureSizePerDegree = 0.854; // scene units per degree of the focus angle

			_useLensBlur = true;
			_lensBlurScale = static_cast<float>(std::max(focusAngle, 0.0) * kApertureSizePerDegree * lens.focalLengthPx);
		}
	}
	else if (pData->AutoFocus && pData->FocusObject.GetCount() > 0 && FBIS(pData->FocusObject.GetAt(0), FBModel))
	{
		// Dont write to property
		// FocalDistance = dist;
		// NOTE: the class filter is a UI restriction, the type is checked above
		FBModel* pModel = static_cast<FBModel*>(pData->FocusObject.GetAt(0));
		_focalDistance = ComputeViewDepth(effectContext->GetModelViewMatrix(), pModel, effectContext->GetEvaluateInfo());
	}

	ShaderPropertyWriter writer(this, effectContext);

	writer(mFocalDistance, static_cast<float>(_focalDistance))
		(mFocalRange, static_cast<float>(_focalRange))
		(mFStop, static_cast<float>(_fstop))
		(mSamples, _samples)
		(mRings, _rings)
		(mCoC, static_cast<float>(_CoC))
		(mBlurRadius, static_cast<float>(_blurRadius))
		(mLegacyFocusAngle, _useLegacyFocusAngle)
		(mUseLensBlur, _useLensBlur)
		(mLensBlurScale, _lensBlurScale)
		(mBlurForeground, _blurForeground)
		(mThreshold, static_cast<float>(_threshold))
		(mGain, static_cast<float>(_gain))
		(mBias, static_cast<float>(_bias))
		(mFringe, static_cast<float>(_fringe))
		(mFeather, static_cast<float>(_feather))
		(mDebugBlurValue, _debugBlurValue)
		(mDebugShowFocus, _debugShowFocus)
		(mNoise, pData->Noise)
		(mPentagon, pData->Pentagon)
		(mFocusPoint, 0.01f * (float)_focusPoint[0], 0.01f * (float)_focusPoint[1], 0.0f, _useFocusPoint);
	

	return true;
}

bool EffectShaderDOF::Bind()
{
	/*
	if (randomTexId == 0)
	{
		InitTexture();
	}

	// bind a random texture
	glActiveTexture(GL_TEXTURE0 + CommonEffect::UserSamplerSlot);
	glBindTexture(GL_TEXTURE_2D, randomTexId);
	glActiveTexture(GL_TEXTURE0);
	*/
	return PostEffectBufferShader::Bind();
}

void EffectShaderDOF::UnBind()
{
	/*
	// bind a random texture
	glActiveTexture(GL_TEXTURE0 + CommonEffect::UserSamplerSlot);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	*/
	PostEffectBufferShader::UnBind();
}

void EffectShaderDOF::InitTexture()
{
	constexpr int DOF_NOISE_SIZE = 8;
	float dofNoise[DOF_NOISE_SIZE][DOF_NOISE_SIZE][2];

	for (int y = 0; y < DOF_NOISE_SIZE; ++y)
	{
		for (int x = 0; x < DOF_NOISE_SIZE; ++x)
		{
			float angle = 2.0f * float(M_PI) * float(dist(e2));
			float radius = std::sqrt(float(dist(e2))); // uniform disk

			dofNoise[y][x][0] = std::cos(angle) * radius;
			dofNoise[y][x][1] = std::sin(angle) * radius;
		}
	}

	DeleteTexture();
	glGenTextures(1, &randomTexId);

	glBindTexture(GL_TEXTURE_2D, randomTexId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, DOF_NOISE_SIZE, DOF_NOISE_SIZE, 0, GL_RG, GL_FLOAT, dofNoise);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void EffectShaderDOF::DeleteTexture()
{
	if (randomTexId > 0)
	{
		glDeleteTextures(1, &randomTexId);
		randomTexId = 0;
	}
}