#ifndef __POST_PROCESSING_DATA_H__
#define __POST_PROCESSING_DATA_H__


/** \file postpersistent_data.h

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include
#include <fbsdk/fbsdk.h>
#include "postEffectUtils.h"

//--- Registration define

#define POSTDATA__CLASSSTR	"PostPersistentData"

// forward declaration
class PostPersistentData;

////////////////////////////////////////////////////////
//PostPersistentData		*GetGlobalSettingsPtr(const char *name);

enum EBlurQuality
{
	eBlurQualityFull,
	eBlurQualityPreview2,
	eBlurQualityPreview4
};

enum EPostAction
{
	ePostActionNone,
	ePostActionFocusCreate,
	ePostActionFocusSelect,
	ePostActionFlareCreate,
	ePostActionFlareSelect,
	ePostActionCount
};

/////////////////////////////////////////////////////////////////////////////////////
//! This class will make sure that the data related to the tool is persistent.

class PostPersistentData : public FBUserObject 
{
    //--- FiLMBOX declaration.
	FBClassDeclare(PostPersistentData, FBUserObject)
	FBDeclareUserObject(PostPersistentData);

public:
	//! a constructor
	PostPersistentData(const char *pName = NULL, HIObject pObject = NULL);

    //--- FiLMBOX Construction/Destruction,
    virtual bool FBCreate() override;        //!< FiLMBOX Creation function.
    virtual void FBDestroy() override;       //!< FiLMBOX Destruction function.

    virtual bool FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat ) override;
    virtual bool FbxRetrieve( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat ) override;

	virtual bool PlugNotify(FBConnectionAction pAction, FBPlug* pThis, int pIndex, FBPlug* pPlug, FBConnectionType pConnectionType, FBPlug* pNewPlug) override;

	static void AddPropertyView(const char* pPropertyName, const char* pHierarchy, bool pIsFolder = false);
	static void AddPropertiesToPropertyViewManager();

	void OnUIIdle(HISender pSender, HKEvent pEvent);

	void ConnectCamera(FBCamera *pCamera);
	void ConnectLight(FBLight *pLight);
	void ConnectFocus(FBModelNull *pNull);

	void CopyValues(PostPersistentData *pOther);

public: // PROPERTIES

	FBPropertyInt				UniqueClassId;
	FBPropertyAnimatableBool	Active;

	FBPropertyBool				DrawHUDLayer;

	// make effect local for a specified camera (override global effect)
	FBPropertyBool				UseCameraObject;
	FBPropertyListObject		Camera;

	FBPropertyAction			ReloadShaders;
	FBPropertyBool				GenerateMipMaps;
	FBPropertyAction			ResetToDefault;

	FBPropertyBool				AutoClipFromHUD;
	FBPropertyDouble			UpperClip;
	FBPropertyDouble			LowerClip;

	// DONE: fish eye props here
	FBPropertyAnimatableBool	FishEye;
	FBPropertyAnimatableDouble	FishEyeAmount;
	FBPropertyAnimatableDouble	FishEyeLensRadius;
	FBPropertyAnimatableDouble	FishEyeSignCurvature;
	FBPropertyInt				FishEyeOrder;	// used for ordering effects by UI Tool

	// color correction props here
	FBPropertyAnimatableBool	ColorCorrection;
	
	FBPropertyBool					ChromaticAberration;	// ca
	FBPropertyAnimatableVector2d	ChromaticAberrationDirection;

	FBPropertyAnimatableDouble		Contrast;
	FBPropertyAnimatableDouble		Brightness;
	FBPropertyAnimatableDouble		Saturation;

	FBPropertyAnimatableDouble		Gamma;

	FBPropertyBool					Inverse;	// inverse colors

	FBPropertyBool					Bloom;
	FBPropertyAnimatableDouble		BloomMinBright;
	FBPropertyAnimatableDouble		BloomTone;
	FBPropertyAnimatableDouble		BloomStretch;

	FBPropertyAnimatableDouble		Hue;
	FBPropertyAnimatableDouble		HueSaturation;
	FBPropertyAnimatableDouble		Lightness;

	// Vignetting
	FBPropertyAnimatableBool	Vignetting;

	FBPropertyAnimatableDouble		VignAmount;
	FBPropertyAnimatableDouble		VignOut;
	FBPropertyAnimatableDouble		VignIn;
	FBPropertyAnimatableDouble		VignFade;

	// Film Grain
	FBPropertyAnimatableBool	FilmGrain;

	FBPropertyBool					FG_UsePlayTime;
	FBPropertyAnimatableDouble		FG_TimeSpeed;

	FBPropertyAnimatableDouble		FG_GrainAmount;
	FBPropertyBool					FG_Colored;	// colored noise ?
	FBPropertyAnimatableDouble		FG_ColorAmount;
	FBPropertyAnimatableDouble		FG_GrainSize;	// grain particle size (1.5 - 2.5)
	FBPropertyAnimatableDouble		FG_LumAmount;

	// Lens Flare
	FBPropertyAnimatableBool	LensFlare;

	//Louis
	FBPropertyBaseEnum<PostProcessingEffects::EFlareType>	FlareType;
	FBPropertyAnimatableDouble	FlareSeed;

	FBPropertyBool					FlareUsePlayTime;
	FBPropertyAnimatableDouble		FlareTimeSpeed;

	FBPropertyBool					UseFlareLightObject;
	FBPropertyListObject			FlareLight;
	FBPropertyAction				FlareLightCreate;
	FBPropertyAction				FlareLightSelect;

	FBPropertyAnimatableDouble		FlareAmount;
	FBPropertyBool					FlareDepthAttenuation;
	FBPropertyAnimatableDouble		FlarePosX;
	FBPropertyAnimatableDouble		FlarePosY;

	FBPropertyAnimatableColor		FlareTint;
	FBPropertyAnimatableDouble		FlareInner;
	FBPropertyAnimatableDouble		FlareOuter;

	FBPropertyBool					FlareFadeToBorders;
	FBPropertyAnimatableDouble		FlareBorderWidth;
	FBPropertyAnimatableDouble		FlareBorderFeather;

	// SSAO
	FBPropertyAnimatableBool	SSAO;

	FBPropertyAnimatableDouble		SSAO_Radius;
	
	FBPropertyAnimatableDouble		SSAO_Intensity;
	FBPropertyAnimatableDouble		SSAO_Bias;

	FBPropertyBool					OnlyAO;		//!< display only ambient occlusion or multiply with child color

	FBPropertyBool					SSAO_Blur;
	FBPropertyAnimatableDouble		SSAO_BlurSharpness;

	//
	FBPropertyAnimatableBool	MotionBlur;

	FBPropertyAnimatableDouble	MotionBlurAmount;

	// 3d DOF
	FBPropertyAnimatableBool	DepthOfField;

	FBPropertyBool				UseCameraDOFProperties;	// take focal distance, focal range and focus object from camera properties !

	FBPropertyAction			ResetDOF;

	FBPropertyBool				DebugBlurValue; // output only computed blur gradient
	FBPropertyAction			DebugFarDistance; // check if camera far distance is too big for glsl float
	FBPropertyAction			FixCameraSettings; // update camera far plane, and turns off real-time camera effects

	FBPropertyAnimatableDouble		FocalDistance;
	FBPropertyAnimatableDouble		FocalRange;
	FBPropertyAnimatableDouble		FStop;

	FBPropertyAnimatableBool		AutoFocus;		// autofocus to some point (instead of focal distance)
	FBPropertyListObject			FocusObject;	//  get a screen position of a specified tm
	
	FBPropertyAction				FocusObjectCreate;
	FBPropertyAction				FocusObjectSelect;
	
	FBPropertyBool					BlurForeground;	// flag to make everything clean in foreground

	FBPropertyBool					ManualFocus;
	FBPropertyAnimatableDouble		ManualNear;
	FBPropertyAnimatableDouble		ManualNearFalloff;
	FBPropertyAnimatableDouble		ManualFar;
	FBPropertyAnimatableDouble		ManualFarFalloff;

	FBPropertyBool					UseFocusPoint;
	FBPropertyAnimatableVector2d	FocusPoint;		// relative screen coord to grab a depth distance from

	FBPropertyBool						FastPreview;	//!< mode for using pre-calculated blur instead of clean DOF processing
	FBPropertyBaseEnum<EBlurQuality>	PreviewQuality;
	FBPropertyDouble					PreviewBlurAmount;

	FBPropertyInt					Samples;	// samples of the first ring
	FBPropertyInt					Rings;		// ring count

	FBPropertyAnimatableDouble		CoC;	// circle of confusion size in mm (35 mm film = 0.03mm)

	FBPropertyAnimatableDouble		Threshold;	// highlight threshold
	FBPropertyAnimatableDouble		Gain;		// highlight gain

	FBPropertyAnimatableDouble		Bias;	// bokeh edge bias
	FBPropertyAnimatableDouble		Fringe;	// bokeh chromatic aberration/fringing

	FBPropertyBool					Noise;	// use noise instead of pattern for sample dithering

	FBPropertyBool					Pentagon;		// use pentagon as bokeh shape
	FBPropertyAnimatableDouble		PentagonFeather;	// pentagon shape feather
	
	//
	// Displacement effect
	FBPropertyAnimatableBool	Displacement;

	FBPropertyBool				UseQuakeWaterEffect;

	FBPropertyBool				Disp_UsePlayTime;
	FBPropertyAnimatableDouble	Disp_Speed;
	FBPropertyAnimatableDouble	Disp_MagnitudeX;
	FBPropertyAnimatableDouble	Disp_MagnitudeY;

	FBPropertyAnimatableDouble	Disp_SinCyclesX;
	FBPropertyAnimatableDouble	Disp_SinCyclesY;

	// output viewport information
	FBPropertyInt				ViewerWidth;	// read-only, size of a whole viewer
	FBPropertyInt				ViewerHeight;	// read-only
	FBPropertyInt				LocalWidth;		// read-only, size of a effect processing FBO
	FBPropertyInt				LocalHeight;	// read-only
	FBPropertyInt				ColorBits;	// read-only, should be 32 by default
	FBPropertyInt				DepthBits;	// read-only, should be 24 by default

	// output a compressed downscaled image
	FBPropertyBool				OutputPreview;
	FBPropertyInt				OutputUpdateRate;	// how many times per second we are preparing a new preview
	FBPropertyDouble			OutputScaleFactor;	// in percentage how small we should make a final video (+power of two)
	FBPropertyDouble			OutputRatio;	// original aspect ratio
	FBPropertyInt				OutputWidth;
	FBPropertyInt				OutputHeight;
	FBPropertyListObject		OutputVideo;
	FBPropertyBool				OutputUseCompression; // enable / disable compression
	FBPropertyBaseEnum<PostProcessingEffects::EImageCompression>	OutputCompression;
	FBPropertyInt				OutputCompressionCode;
	FBPropertyInt				OutputUncompressSize;
	FBPropertyInt				OutputCompressedSize;	// stats for a ETC1 compressed size
	FBPropertyDouble			OutputCompressedTime;	// in secs

	FBPropertyBool				IsSynced;
	FBPropertyVector4d			DeviceAddress;
	FBPropertyInt				DevicePort;

	static void ActionReloadShaders(HIObject pObject, bool value);
	static void ActionResetToDefault(HIObject pObject, bool value);

	static void ActionFlareLightCreate(HIObject pObject, bool value);
	static void ActionFlareLightSelect(HIObject pObject, bool value);

	static void ActionFocusObjectCreate(HIObject object, bool value);
	static void ActionFocusObjectSelect(HIObject object, bool value);

	static void ActionResetDOF(HIObject object, bool value);

	static void SetFastPreview(HIObject object, bool value);

	static void ActionDebugFarDist(HIObject object, bool value);
	static void ActionFixCameraSettings(HIObject object, bool value);

public:

	void DoDebugFarDist();
	void DoFixCameraSettings();

	void DoReloadShaders();

	void DoFocusObjectCreate();
	void DoFocusObjectSelect();

	void DoFlareLightCreate();
	void DoFlareLightSelect();

	void DoResetDOF();

	bool IsNeedToReloadShaders();
	void SetReloadShadersState(bool state);

	void SetPreviewTextureId(unsigned int id, double ratio, 
		unsigned int w, unsigned int h, int uncomporessSize, 
		int compressedSize, int compressionCode, double updateTime);

	void PushClipSettings(double upper, double lower);
	void PopClipSettings();

	bool IsLazyLoadReady() { mLazyLoadCounter = (mLazyLoadCounter >= 0) ? mLazyLoadCounter-1 : -1; return mLazyLoadCounter < 0; }

protected:

	FBSystem			mSystem;

    FBString			mText;
	bool				mReloadShaders;
	EPostAction			mPostAction;

	double				mResetFarPlane;

	double				mTempLower;
	double				mTempUpper;

	int			mLazyLoadCounter;

	void		DefaultValues();
	void		LoadFromConfig(const char *sessionFilter=nullptr);
	void		LoadFarValueFromConfig();

	void ComputePointInFront(FBVector3d &v);
};

#endif /* __POST_PROCESSING_DATA_H__ */
