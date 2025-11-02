
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
		.SetValue(CommonEffect::ColorSamplerSlot);

	// Core depth of field parameters
	mFocalDistance = &AddProperty(ShaderProperty(PostPersistentData::DOF_FOCAL_DISTANCE,
		"focalDistance",
		IEffectShaderConnections::EPropertyType::FLOAT));

	mFocalRange = &AddProperty(ShaderProperty(PostPersistentData::DOF_FOCAL_RANGE,
		"focalRange",
		IEffectShaderConnections::EPropertyType::FLOAT));

	mFStop = &AddProperty(ShaderProperty(PostPersistentData::DOF_FSTOP,
		"fstop",
		IEffectShaderConnections::EPropertyType::FLOAT));

	mCoC = &AddProperty(ShaderProperty(PostPersistentData::DOF_COC,
		"CoC",
		IEffectShaderConnections::EPropertyType::FLOAT));

	// Rendering parameters
	
	mSamples = &AddProperty(ShaderProperty(PostPersistentData::DOF_SAMPLES,
		"samples",
		IEffectShaderConnections::EPropertyType::INT));

	mRings = &AddProperty(ShaderProperty(PostPersistentData::DOF_RINGS,
		"rings",
		IEffectShaderConnections::EPropertyType::INT));

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
		IEffectShaderConnections::EPropertyType::BOOL));

	mThreshold = &AddProperty(ShaderProperty(PostPersistentData::DOF_THRESHOLD,
		"threshold",
		IEffectShaderConnections::EPropertyType::FLOAT))
		.SetScale(0.01f);

	mGain = &AddProperty(ShaderProperty(PostPersistentData::DOF_GAIN,
		"gain",
		IEffectShaderConnections::EPropertyType::FLOAT));

	mBias = &AddProperty(ShaderProperty(PostPersistentData::DOF_BIAS,
		"bias",
		IEffectShaderConnections::EPropertyType::FLOAT))
		.SetScale(0.01f);

	mFringe = &AddProperty(ShaderProperty(PostPersistentData::DOF_FRINGE,
		"fringe",
		IEffectShaderConnections::EPropertyType::FLOAT))
		.SetScale(0.01f);

	mNoise = &AddProperty(ShaderProperty(PostPersistentData::DOF_NOISE,
		"noise",
		IEffectShaderConnections::EPropertyType::BOOL));

	// Experimental bokeh shape parameters
	mPentagon = &AddProperty(ShaderProperty(PostPersistentData::DOF_PENTAGON,
		"pentagon",
		IEffectShaderConnections::EPropertyType::BOOL));

	mFeather = &AddProperty(ShaderProperty(PostPersistentData::DOF_PENTAGON_FEATHER,
		"feather",
		IEffectShaderConnections::EPropertyType::FLOAT))
		.SetScale(0.01f);

	// Debug utilities
	mDebugBlurValue = &AddProperty(ShaderProperty(PostPersistentData::DOF_DEBUG_BLUR_VALUE,
		"debugBlurValue",
		IEffectShaderConnections::EPropertyType::BOOL));
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

	FBCamera* camera = effectContext->GetCamera();

	double _focalDistance = pData->FocalDistance;
	double _focalRange = pData->FocalRange;
	//double _fstop = pData->FStop;
	//int _samples = pData->Samples;
	//int _rings = pData->Rings;

	float _useFocusPoint = (pData->UseFocusPoint) ? 1.0f : 0.0f;
	FBVector2d _focusPoint = pData->FocusPoint;

	//double _blurForeground = (pData->BlurForeground) ? 1.0 : 0.0;

	//double _CoC = pData->CoC;
	//double _threshold = pData->Threshold;
	
//	double _gain = pData->Gain;
	//double _bias = pData->Bias;
	//double _fringe = pData->Fringe;
	//double _feather = pData->PentagonFeather;

	//double _debugBlurValue = (pData->DebugBlurValue) ? 1.0 : 0.0;

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

	//mFStop->SetValue(static_cast<float>(_fstop));

	mManualDOF->SetValue(false);
	mNDOFStart->SetValue(1.0f);
	mNDOFDist->SetValue(2.0f);
	mFDOFStart->SetValue(1.0f);
	mFDOFDist->SetValue(3.0f);

	//mSamples->SetValue(_samples);
	//mRings->SetValue(_rings);
	
	//mBlurForeground->SetValue(static_cast<float>(_blurForeground));

	//mCoC->SetValue(static_cast<float>(_CoC));

	//mBlurForeground->SetValue(static_cast<float>(_blurForeground));

	//mThreshold->SetValue(static_cast<float>(_threshold));
	//mBias->SetValue(static_cast<float>(_bias));
	//mFringe->SetValue(static_cast<float>(_fringe));
	//mFeather->SetValue(static_cast<float>(_feather));
	//mDebugBlurValue->SetValue(static_cast<float>(_debugBlurValue));
	
	mFocusPoint->SetValue(0.01f * (float)_focusPoint[0], 0.01f * (float)_focusPoint[1], 0.0f, _useFocusPoint);

	return true;
}