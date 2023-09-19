
#pragma once

// postprocessing_helper.h
/*
Sergei <Neill3d> Solokhin 2018-2022

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "nv_math.h"

namespace PostProcessingEffects
{
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

	// TODO: int w, int h, FBCamera *pCamera
	struct SViewInfo
	{
		int w;
		int h;
		vec3 cameraPos;
		mat4 modelview;
		mat4 modelviewInv;
		mat4 proj;

		float renderWidth;
		float renderHeight;
		double farPlane; // pCamera->FarPlaneDistance;
		double nearPlane; //  pCamera->NearPlaneDistance;
		double fieldOfView;

		bool isPerspective; // (pCamera->Type == FBCameraType::kFBCameraTypePerspective)

		int localTimeFrame{ 0 }; // FBSystem::TheOne().LocalTime // localTime.GetFrame();
		double localTime{ 0.0 }; // GetSecondDouble()

		double systemTime{ 0.0 }; // systemTime.GetSecondDouble()

		const float GetRatio() const { return renderWidth / renderHeight; }
	};


	enum class EPostProcessEffectType
	{
		FISHEYE,
		COLOR,
		VIGNETTE,
		FILMGRAIN,
		LENSFLARE,
		SSAO,
		DOF,
		DISPLACEMENT,
		MOTIONBLUR,
		COUNT
	};

	struct SUICommonParams
	{
		bool	isOk;

		bool	useFishEye;
		bool	useColorCorrection;
		bool	useVignetting;
		bool	useFilmGrain;
		bool	useLensFlare;
		bool	useSSAO;
		bool	useDepthOfField;
		bool	useDisplacement;
		bool	useMotionBlur;

		bool	doGenerateMipMaps{ false };
		float   blurSharpness;
		float	bloomMinBright;
		float   bloomTone;
		float	bloomStretch;

		bool	doSSAOBlur;
		bool	doBloom;
		bool	doOnlyAO{ false };

		bool	doOutputUseCompression{ false };
		EImageCompression outputCompression;
		bool	doOutputPreview{ false }; // ->OutputPreview.AsInt()
		int		outputPreviewRate{ 25 };
	};

	class UICallback
	{
	public:

		SUICommonParams	commonParams;

		// create a new property for a specific type, category and name and return an id of it
		virtual int RegisterUIElement(int type, const char* name) abstract;
		// during preparation step, do a query of an UI element value
		virtual const double GetUIElementValue(const int elementID) abstract;

		virtual bool Ok() const abstract;
		virtual bool IsNeedToReloadShaders() const { return m_doReloadShaders; }
		virtual void SetReloadShadersState(bool value) { m_doReloadShaders = value; }



	protected:
		bool	m_doReloadShaders{ false };
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
	size_t RgEtc1_CompressSingleImage(unsigned char* stream, int imagewidth, int imageheight,
		unsigned char* imagedata, int pitch, int quality = 0);

	size_t CompressImageBegin(unsigned char* stream, int imagewidth, int imageheight,
		unsigned char* imagedata, int pitch, int quality);

	size_t CompressImageEnd();

	///////////////////

	void ComputeCameraOrthoPoints(const SViewInfo& viewInfo, vec3* points);
	void ComputeCameraFrustumPoints(const SViewInfo& viewInfo, vec3* points);

}

