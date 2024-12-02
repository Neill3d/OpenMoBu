
#pragma once

// FBCameraProxy.h
/*
Sergei <Neill3d> Solokhin 2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "ShadowManager.h"
//--- SDK include
#include <fbsdk/fbsdk.h>

namespace Graphics
{
	
	// mobu implementation for camera proxy
	class FBCameraProxy : public CameraProxy
	{
	public:

		FBCameraProxy(FBCamera* cameraIn)
			: cameraPlug(cameraIn)
		{}

		virtual ~FBCameraProxy()
		{}

	private:

		HdlFBPlugTemplate<FBCamera>	cameraPlug;
	};

};