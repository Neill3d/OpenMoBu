
/**	\file	posteffectdof.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "posteffectdof.h"
#include "postpersistentdata.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "postprocessing_helper.h"

//! a constructor
EffectShaderDOF::EffectShaderDOF(FBComponent* ownerIn)
	: PostEffectBufferShader(ownerIn)
{
	MakeCommonProperties();

	// Sample slots

	AddProperty(ShaderProperty("color", "colorSampler"))
		.SetType(IEffectShaderConnections::EPropertyType::TEXTURE)
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true)
		.SetValue(CommonEffect::ColorSamplerSlot);

	// Core depth of field parameters
	mFocalDistance = &AddProperty(ShaderProperty(PostPersistentData::DOF_FOCAL_DISTANCE,
		"focalDistance",
		IEffectShaderConnections::EPropertyType::FLOAT))
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true);

	mFocalRange = &AddProperty(ShaderProperty(PostPersistentData::DOF_FOCAL_RANGE,
		"focalRange",
		IEffectShaderConnections::EPropertyType::FLOAT))
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true);

	mFStop = &AddProperty(ShaderProperty(PostPersistentData::DOF_FSTOP,
		"fstop",
		IEffectShaderConnections::EPropertyType::FLOAT))
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true);

	mCoC = &AddProperty(ShaderProperty(PostPersistentData::DOF_COC,
		"CoC",
		IEffectShaderConnections::EPropertyType::FLOAT))
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true);

	// Rendering parameters
	
	mSamples = &AddProperty(ShaderProperty(PostPersistentData::DOF_SAMPLES,
		"samples",
		IEffectShaderConnections::EPropertyType::INT))
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true);

	mRings = &AddProperty(ShaderProperty(PostPersistentData::DOF_RINGS,
		"rings",
		IEffectShaderConnections::EPropertyType::INT))
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true);

	// Focus control
	mAutoFocus = &AddProperty(ShaderProperty(PostPersistentData::DOF_AUTO_FOCUS,
		"autoFocus",
		IEffectShaderConnections::EPropertyType::BOOL))
		.SetRequired(false);

	mFocus = &AddProperty(ShaderProperty(PostPersistentData::DOF_USE_FOCUS_POINT,
		"focus",
		IEffectShaderConnections::EPropertyType::BOOL))
		.SetRequired(false);

	mFocusPoint = &AddProperty(ShaderProperty(PostPersistentData::DOF_FOCUS_POINT,
		"focusPoint",
		IEffectShaderConnections::EPropertyType::VEC4))
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true);

	mManualDOF = &AddProperty(ShaderProperty("manualdof", "manualdof",
		IEffectShaderConnections::EPropertyType::BOOL))
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true); // NOTE: skip of automatic reading value and let it be done manually;

	// Near and far DOF blur parameters
	mNDOFStart = &AddProperty(ShaderProperty("ndofstart", "ndofstart",
		IEffectShaderConnections::EPropertyType::FLOAT))
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true); // NOTE: skip of automatic reading value and let it be done manually;

	mNDOFDist = &AddProperty(ShaderProperty("ndofdist", "ndofdist",
		IEffectShaderConnections::EPropertyType::FLOAT))
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true); // NOTE: skip of automatic reading value and let it be done manually;

	mFDOFStart = &AddProperty(ShaderProperty("fdofstart", "fdofstart",
		IEffectShaderConnections::EPropertyType::FLOAT))
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true); // NOTE: skip of automatic reading value and let it be done manually;

	mFDOFDist = &AddProperty(ShaderProperty("fdofdist", "fdofdist",
		IEffectShaderConnections::EPropertyType::FLOAT));

	// Visual enhancement parameters
	mBlurForeground = &AddProperty(ShaderProperty(PostPersistentData::DOF_BLUR_FOREGROUND,
		"blurForeground",
		IEffectShaderConnections::EPropertyType::BOOL))
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true);

	mThreshold = &AddProperty(ShaderProperty(PostPersistentData::DOF_THRESHOLD,
		"threshold",
		IEffectShaderConnections::EPropertyType::FLOAT))
		.SetScale(0.01f)
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true);

	mGain = &AddProperty(ShaderProperty(PostPersistentData::DOF_GAIN,
		"gain",
		IEffectShaderConnections::EPropertyType::FLOAT))
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true);

	mBias = &AddProperty(ShaderProperty(PostPersistentData::DOF_BIAS,
		"bias",
		IEffectShaderConnections::EPropertyType::FLOAT))
		.SetScale(0.01f)
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true);

	mFringe = &AddProperty(ShaderProperty(PostPersistentData::DOF_FRINGE,
		"fringe",
		IEffectShaderConnections::EPropertyType::FLOAT))
		.SetScale(0.01f)
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true);

	mNoise = &AddProperty(ShaderProperty(PostPersistentData::DOF_NOISE,
		"noise",
		IEffectShaderConnections::EPropertyType::BOOL))
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true);

	// Experimental bokeh shape parameters
	mPentagon = &AddProperty(ShaderProperty(PostPersistentData::DOF_PENTAGON,
		"pentagon",
		IEffectShaderConnections::EPropertyType::BOOL))
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true);

	mFeather = &AddProperty(ShaderProperty(PostPersistentData::DOF_PENTAGON_FEATHER,
		"feather",
		IEffectShaderConnections::EPropertyType::FLOAT))
		.SetScale(0.01f)
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true);

	// Debug utilities
	mDebugBlurValue = &AddProperty(ShaderProperty(PostPersistentData::DOF_DEBUG_BLUR_VALUE,
		"debugBlurValue",
		IEffectShaderConnections::EPropertyType::BOOL))
		.SetFlag(IEffectShaderConnections::PropertyFlag::ShouldSkip, true);
}

const char* EffectShaderDOF::GetUseMaskingPropertyName() const noexcept
{
	return PostPersistentData::DOF_USE_MASKING;
}
const char* EffectShaderDOF::GetMaskingChannelPropertyName() const noexcept
{
	return PostPersistentData::DOF_MASKING_CHANNEL;
}

bool EffectShaderDOF::OnCollectUI(const IPostEffectContext* effectContext, int maskIndex)
{
	PostPersistentData* pData = effectContext->GetPostProcessData();
	if (!pData)
		return false;

	FBCamera* camera = effectContext->GetCamera();

	double _focalDistance = pData->FocalDistance;
	double _focalRange = pData->FocalRange;
	double _fstop = pData->FStop;
	int _samples = pData->Samples;
	int _rings = pData->Rings;

	float _useFocusPoint = (pData->UseFocusPoint) ? 1.0f : 0.0f;
	FBVector2d _focusPoint = pData->FocusPoint;

	const bool _blurForeground = pData->BlurForeground;

	double _CoC = pData->CoC;
	double _threshold = pData->Threshold;
	
	double _gain = pData->Gain;
	double _bias = pData->Bias;
	double _fringe = pData->Fringe;
	double _feather = pData->PentagonFeather;

	const bool _debugBlurValue = pData->DebugBlurValue;

	if (pData->UseCameraDOFProperties)
	{
		_focalDistance = camera->FocusSpecificDistance;
		_focalRange = camera->FocusAngle;

		FBModel *pInterest = nullptr;
		FBCameraFocusDistanceSource cameraFocusDistanceSource;
		camera->FocusDistanceSource.GetData(&cameraFocusDistanceSource, sizeof(FBCameraFocusDistanceSource));
		if (kFBFocusDistanceCameraInterest == cameraFocusDistanceSource)
			pInterest = camera->Interest;
		else if (kFBFocusDistanceModel == cameraFocusDistanceSource)
			pInterest = camera->FocusModel;
		
		if (nullptr != pInterest)
		{
			FBMatrix modelView, modelViewI;

			((FBModel*)camera)->GetMatrix(modelView);
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

		((FBModel*)camera)->GetMatrix(modelView);
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

	mFocalDistance->SetValue(static_cast<float>(_focalDistance));
	mFocalRange->SetValue(static_cast<float>(_focalRange));

	mFStop->SetValue(static_cast<float>(_fstop));

	mManualDOF->SetValue(false);
	mNDOFStart->SetValue(1.0f);
	mNDOFDist->SetValue(2.0f);
	mFDOFStart->SetValue(1.0f);
	mFDOFDist->SetValue(3.0f);

	mSamples->SetValue(_samples);
	mRings->SetValue(_rings);
	
	mCoC->SetValue(static_cast<float>(_CoC));

	mBlurForeground->SetValue(_blurForeground);

	mThreshold->SetValue(static_cast<float>(_threshold));
	mGain->SetValue(static_cast<float>(_gain));
	mBias->SetValue(static_cast<float>(_bias));
	mFringe->SetValue(static_cast<float>(_fringe));
	mFeather->SetValue(static_cast<float>(_feather));
	mDebugBlurValue->SetValue(_debugBlurValue);
	
	mNoise->SetValue(pData->Noise);
	mPentagon->SetValue(pData->Pentagon);

	mFocusPoint->SetValue(0.01f * (float)_focusPoint[0], 0.01f * (float)_focusPoint[1], 0.0f, _useFocusPoint);

	return true;
}