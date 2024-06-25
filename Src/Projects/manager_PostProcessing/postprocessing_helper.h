
#pragma once

// postprocessing_helper.h
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//--- SDK include
#include <fbsdk/fbsdk.h>
#include "nv_math.h"

//
enum EImageCompression
{
	eImageCompressionDefault,
	eImageCompressionS3TC,
	eImageCompressionETC2,	// for gles 3.0 compatible,
	eImageCompressionASTC
};

//Louis
//Modifiable Flares
enum EFlareType
{
	flare1,
	flare2,
	flare3,
};

/////////////////////
/*
struct CompressImageHeader
{
	float			timestamp;
	float			aspect;			// original image aspect
	int				compressedSize;
	int				internalFormat;	// compressed format
	unsigned short	width;
	unsigned short	height;
};
*/
size_t RgEtc1_CompressSingleImage(unsigned char *stream, int imagewidth, int imageheight,
	unsigned char *imagedata, int pitch, int quality=0);

size_t CompressImageBegin(unsigned char *stream, int imagewidth, int imageheight,
	unsigned char *imagedata, int pitch, int quality);
size_t CompressImageEnd();

///////////////////

void ComputeCameraOrthoPoints(const float renderWidth, const float renderHeight, FBModel *pCamera, double farPlane, double nearPlane, NVMath::vec3 *points);
void ComputeCameraFrustumPoints(const float renderWidth, const float renderHeight, FBModel *pCamera, double farPlane, double nearPlane, double FieldOfView, NVMath::vec3 *points);

void RenderMaskedObjects();