
/**	\file	posteffect_cameralens.cpp

Sergei <Neill3d> Solokhin 2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

#include "posteffect_cameralens.h"

#include <algorithm>

double ComputeViewDepth(const double* modelView, FBModel* target, FBEvaluateInfo* pEvaluateInfo)
{
	if (!target)
		return 0.0;

	FBVector3d pos;
	target->GetVector(pos, kModelTranslation, true, pEvaluateInfo);

	// opengl camera looks along -Z, z row of a column-major matrix
	const double* m = modelView;
	if (!m)
		return 0.0;
	const double viewZ = m[2] * pos[0] + m[6] * pos[1] + m[10] * pos[2] + m[14];
	return -viewZ;
}

void ComputeCameraLens(CameraLens& out, FBCamera* camera, const FBMatrix& modelView, const FBMatrix& projection,
	int viewHeight, bool isOrtho, FBEvaluateInfo* pEvaluateInfo)
{
	out = CameraLens();

	if (!camera)
		return;

	camera->FocusSpecificDistance.GetData(&out.focusDistance, sizeof(double), pEvaluateInfo);
	camera->FocusAngle.GetData(&out.focusAngle, sizeof(double), pEvaluateInfo);
	camera->FocalLength.GetData(&out.focalLength, sizeof(double), pEvaluateInfo);
	camera->FieldOfView.GetData(&out.fieldOfView, sizeof(double), pEvaluateInfo);

	FBCameraFocusDistanceSource focusDistanceSource{ kFBFocusDistanceSpecificDistance };
	camera->FocusDistanceSource.GetData(&focusDistanceSource, sizeof(FBCameraFocusDistanceSource), pEvaluateInfo);

	FBModel* focusTarget = nullptr;
	switch (focusDistanceSource)
	{
	case kFBFocusDistanceCameraInterest: focusTarget = camera->Interest;
		break;
	case kFBFocusDistanceModel: focusTarget = camera->FocusModel;
		break;
	default:
		break;
	}

	if (focusTarget)
	{
		out.focusDistance = ComputeViewDepth(modelView, focusTarget, pEvaluateInfo);
	}

	if (!isOrtho)
	{
		const double* p = projection;
		out.focalLengthPx = std::max(0.5 * static_cast<double>(viewHeight) * p[5], 0.0);
	}
}
