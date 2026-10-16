#pragma once

/**	\file	posteffect_cameralens.h

Sergei <Neill3d> Solokhin 2026

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include
#include <fbsdk/fbsdk.h>

/**
* camera lens data that is shared between system uniforms and effects (like DOF)
* NOTE: defaults describe a "no camera" context
*/
struct CameraLens
{
	double focusDistance{ 0.0 };	//!< scene units, resolved from the camera focus distance source
	double focusAngle{ 0.0 };		//!< degrees
	double focalLength{ 0.0 };		//!< mm
	double fieldOfView{ 0.0 };		//!< degrees, the camera FieldOfView property (its axis follows the camera aperture mode)
	double focalLengthPx{ 0.0 };	//!< focal length in pixels of the view height, 0 for an orthogonal camera
};

/// view space depth of a model world position, positive in front of the camera
/// @param modelView an opengl camera modelview matrix (column-major)
double ComputeViewDepth(const double* modelView, FBModel* target, FBEvaluateInfo* pEvaluateInfo);

/// resolve the camera lens data, the focus distance comes from the camera focus distance source
///  (camera interest, focus model or a specific distance)
/// @param modelView, projection - camera matrices of the same evaluation
/// @param viewHeight - viewport height in pixels
void ComputeCameraLens(CameraLens& out, FBCamera* camera, const FBMatrix& modelView, const FBMatrix& projection,
	int viewHeight, bool isOrtho, FBEvaluateInfo* pEvaluateInfo);
