#ifndef __POST_PROCESSING_DATA_H__
#define __POST_PROCESSING_DATA_H__


/** \file postpersistentdata.h

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include
#include <fbsdk/fbsdk.h>
#include "postprocessing_helper.h"

//--- Registration define

#define POSTDATA__CLASSSTR	"PostPersistentData"

// forward declaration
class PostPersistentData;
class PostEffectBase;
class PostEffectUserObject;

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

/// <summary>
/// This class will make sure that the data related to the post processing is persistent.
/// </summary>
class PostPersistentData : public FBUserObject 
{
    //--- FiLMBOX declaration.
	FBClassDeclare(PostPersistentData, FBUserObject)
	FBDeclareUserObject(PostPersistentData);

public:
	// common

	static constexpr const char* UPPER_CLIP = "Bottom Clip Percent";
	static constexpr const char* LOWER_CLIP = "Top Clip Percent";

	// Color Correction

	static constexpr const char* COLOR_USE_MASKING = "Color Correction Use Masking";
	static constexpr const char* COLOR_MASKING_CHANNEL = "Color Correction Masking Channel";

	static constexpr const char* CHROMATIC_ABERRATION = "Chromatic Aberration";
	static constexpr const char* CHROMATIC_ABERRATION_DIR = "Chromatic Aberration Direction";
	static constexpr const char* CONTRAST = "Contrast";
	static constexpr const char* BRIGHTNESS = "Brightness";
	static constexpr const char* SATURATION = "Saturation";

	static constexpr const char* COLOR_GAMMA = "Gamma";
	static constexpr const char* COLOR_INVERSE = "Inverse";
	static constexpr const char* BLOOM = "Bloom";
	static constexpr const char* BLOOM_MIN_BRIGHT = "Bloom Min Bright";
	static constexpr const char* BLOOM_TONE = "Bloom Tone";
	static constexpr const char* BLOOM_STRETCH = "Bloom Stretch";

	static constexpr const char* COLOR_HUE = "Hue";
	static constexpr const char* COLOR_HUE_SATURATION = "Hue Saturation";
	static constexpr const char* COLOR_LIGHTNESS = "Lightness";

	// Depth of Field

	static constexpr const char* DOF_USE_MASKING = "Depth Of Field Use Masking";
	static constexpr const char* DOF_MASKING_CHANNEL = "Depth Of Field Masking Channel";

	static constexpr const char* USE_CAMERA_DOF_PROPS = "Use Camera DOF Properties";
	static constexpr const char* RESET_DOF = "Reset DOF";
	static constexpr const char* DOF_DEBUG_BLUR_VALUE = "Debug Blur Value";
	static constexpr const char* DOF_DEBUG_FAR_DIST = "Debug Camera Far Dist";
	static constexpr const char* DOF_FIX_CAM_SETTINGS = "Fix Camera Settings";

	static constexpr const char* DOF_FOCAL_DISTANCE = "Focal Distance";
	static constexpr const char* DOF_FOCAL_RANGE = "Focal Range";
	static constexpr const char* DOF_FSTOP = "F-Stop";

	static constexpr const char* DOF_AUTO_FOCUS = "Auto Focus";
	static constexpr const char* DOF_FOCUS_OBJECT = "Focus Object";
	static constexpr const char* DOF_BLUR_FOREGROUND = "Blur Foreground";

	static constexpr const char* DOF_USE_FOCUS_POINT = "Use Focus Point";
	static constexpr const char* DOF_FOCUS_POINT = "Focus Point";
	static constexpr const char* DOF_SAMPLES = "Samples";
	static constexpr const char* DOF_RINGS = "Ring count";
	static constexpr const char* DOF_COC = "Circle of confusion";
	static constexpr const char* DOF_THRESHOLD = "Highlight Threshold";
	static constexpr const char* DOF_GAIN = "Highlight Gain";
	static constexpr const char* DOF_BIAS = "Bokeh Bias";
	static constexpr const char* DOF_FRINGE = "Bokeh Fringe";
	static constexpr const char* DOF_NOISE = "Noise";
	static constexpr const char* DOF_PENTAGON = "Pentagon";
	static constexpr const char* DOF_PENTAGON_FEATHER = "Pentagon feather";

	// displacement

	static constexpr const char* DISP_USE_MASKING = "Disp Use Masking";
	static constexpr const char* DISP_MASKING_CHANNEL = "Disp Masking Channel";

	static constexpr const char* DISP_USE_QUAKE_EFFECT = "Use Quake Water Effect";
	static constexpr const char* DISP_USE_PLAY_TIME = "Disp Use Play Time";
	static constexpr const char* DISP_SPEED = "Displacement Speed";
	static constexpr const char* DISP_MAGNITUDE_X = "Disp Magnitude X";
	static constexpr const char* DISP_MAGNITUDE_Y = "Disp Magnitude Y";
	static constexpr const char* DISP_SIN_CYCLES_X = "Disp Sin Cycles X";
	static constexpr const char* DISP_SIN_CYCLES_Y = "Disp Sin Cycles Y";

	// film grain

	static constexpr const char* GRAIN_USE_MASKING = "Grain Use Masking";
	static constexpr const char* GRAIN_MASKING_CHANNEL = "Grain Masking Channel";

	static constexpr const char* GRAIN_USE_PLAY_TIME = "Grain Use Play Time";
	static constexpr const char* GRAIN_SPEED = "Grain Time Speed";

	static constexpr const char* GRAIN_AMOUNT = "Grain Amount";
	static constexpr const char* GRAIN_COLORED = "Grain Colored";
	static constexpr const char* GRAIN_COLOR_AMOUNT = "Grain Color Amount";
	static constexpr const char* GRAIN_SIZE = "Grain Size";
	static constexpr const char* GRAIN_LUMAMOUNT = "Grain Lum Amount";

	// Fish Eye

	static constexpr const char* FISHEYE_USE_MASKING = "Fish Eye Use Masking";
	static constexpr const char* FISHEYE_MASKING_CHANNEL = "Fish Eye Masking Channel";

	static constexpr const char* FISHEYE_AMOUNT = "Fish Eye Amount";
	static constexpr const char* FISHEYE_LENS_RADIUS = "Fish Eye Lens Radius";
	static constexpr const char* FISHEYE_SIGN_CURV = "Fish Eye Sign Curvature";
	static constexpr const char* FISHEYE_ORDER = "Fish Eye Order";

	// Lens Flare

	static constexpr const char* FLARE_USE_MASKING = "Flare Use Masking";
	static constexpr const char* FLARE_MASKING_CHANNEL = "Flare Masking Channel";

	static constexpr const char* FLARE_USE_OCCLUSION = "Flare Use Occlusion";
	static constexpr const char* FLARE_OCC_SPEED = "Flare Occlusion Speed";
	static constexpr const char* FLARE_OCC_OBJECTS = "Flare Occlusion Objects";

	static constexpr const char* FLARE_TYPE = "Flare Type";
	static constexpr const char* FLARE_SEED = "Flare Seed";
	static constexpr const char* FLARE_USE_PLAY_TIME = "Flare Use Play Time";
	static constexpr const char* FLARE_TIME_SPEED = "Flare Time Speed";

	static constexpr const char* FLARE_USE_LIGHT_OBJECT = "Use Flare Light Object";
	static constexpr const char* FLARE_LIGHT = "Flare Light";
	static constexpr const char* FLARE_AMOUNT = "Lens Flare Amount";
	static constexpr const char* FLARE_DEPTH_ATT = "Flare Depth Attenuation";
	static constexpr const char* FLARE_POSX = "Lens Flare X";
	static constexpr const char* FLARE_POSY = "Lens Flare Y";

	static constexpr const char* FLARE_TINT = "Lens Flare Tint";
	static constexpr const char* FLARE_INNER = "Lens Flare Inner";
	static constexpr const char* FLARE_OUTER = "Lens Flare Outer";
	static constexpr const char* FLARE_FADE_TO_BORDERS = "Flare Fade To Borders";
	static constexpr const char* FLARE_BORDER_WIDTH = "Flare Border Width";
	static constexpr const char* FLARE_BORDER_FEATHER = "Flare Border Feather";

	// Motion Blur

	static constexpr const char* MOTIONBLUR_USE_MASKING = "Motion Blur Use Masking";
	static constexpr const char* MOTIONBLUR_MASKING_CHANNEL = "Motion Blur Masking Channel";

	static constexpr const char* MOTIONBLUR_AMOUNT = "Motion Blur Amount";

	// SSAO

	static constexpr const char* SSAO_USE_MASKING = "SSAO Use Masking";
	static constexpr const char* SSAO_MASKING_CHANNEL = "SSAO Masking Channel";

	static constexpr const char* SSAO_RADIUS = "SSAO Radius";
	static constexpr const char* SSAO_INTENSITY = "SSAO Intensity";
	static constexpr const char* SSAO_BIAS = "SSAO Bias";
	static constexpr const char* SSAO_ONLY_AO = "Only AO";
	static constexpr const char* SSAO_BLUR = "SSAO Blur";
	static constexpr const char* SSAO_BLUR_SHARPNESS = "SSAO Blur Sharpness";

	// vignetting

	static constexpr const char* VIGN_USE_MASKING = "Vignetting Use Masking";
	static constexpr const char* VIGN_MASKING_CHANNEL = "Vignetting Masking Channel";

	static constexpr const char* VIGN_AMOUNT = "Vignetting Amount";
	static constexpr const char* VIGN_OUT = "Vignetting Outer Border";
	static constexpr const char* VIGN_IN = "Vignetting Inner Border";
	static constexpr const char* VIGN_FADE = "Vignetting Fade";

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

	// Global masking properties
	FBPropertyBool				UseCompositeMasking; //!< a global toggle for the masking effect, every effect has own local masking settings
	FBPropertyBool				EnableMaskingForAllEffects; //!< with this option, we are going to activate masking for every effect
	FBPropertyBaseEnum<EMaskingChannel>	GlobalMaskingChannel;
	FBPropertyBool				DebugDisplyMasking; //!< output masking texture into the viewport for a test purpose

	struct SMaskProperties
	{
		FBPropertyBool				InvertMask;		//!< reverse colors in a rendered masks
		FBPropertyBool				BlurMask;		//!< apply a blur pass on a rendered mask
		FBProperty* BlurMaskScale{ nullptr }; //!< control a blur scaling of mask a
		FBPropertyAnimatableDouble	UseRimForMask;	//!< objects in mask will be rendered with rim effect, like a back-lit. that could emulate a mask falloff
		FBPropertyAnimatableDouble	MaskRimPower;	//!< rim effect power, amplify the emulated mask falloff effect
		FBPropertyBool				UseMixWithMask;
		FBPropertyBaseEnum<EMaskingChannel>	MixWithMask;	//!< multiply a given mask with another mask (could be used for a mask subtraction)

		void SetDefaultValues();
	};

	static const int NUMBER_OF_MASKS{ 4 };
	SMaskProperties		Masks[NUMBER_OF_MASKS];

	FBVector2d GetMaskScale(const int maskIndex) const
	{
		assert(maskIndex < NUMBER_OF_MASKS);

		FBVector3d v(1.0, 1.0, 1.0);
		if (maskIndex >= 0 && maskIndex < NUMBER_OF_MASKS)
		{
			Masks[maskIndex].BlurMaskScale->GetData(v, sizeof(FBVector3d));
		}
		
		return FBVector2d(v[0], v[1]);
	}

	// make effect local for a specified camera (override global effect)
	FBPropertyBool				UseCameraObject;
	FBPropertyListObject		Camera;

	FBPropertyAction			ReloadShaders;
	FBPropertyBool				GenerateMipMaps;
	FBPropertyAction			ResetToDefault;

	FBPropertyListObject		UserEffects; //!< connected custom effects to use in the processing chain

	FBPropertyBool				AutoClipFromHUD;
	FBPropertyDouble			UpperClip;
	FBPropertyDouble			LowerClip;

	// DONE: fish eye props here
	FBPropertyAnimatableBool	FishEye;
	FBPropertyBool				FishEye_UseMasking;
	FBPropertyBaseEnum<EMaskingChannel>	FishEye_MaskingChannel;
	FBPropertyAnimatableDouble	FishEyeAmount;
	FBPropertyAnimatableDouble	FishEyeLensRadius;
	FBPropertyAnimatableDouble	FishEyeSignCurvature;
	FBPropertyInt				FishEyeOrder;	//!< used for ordering effects by UI Tool

	// color correction props here
	FBPropertyAnimatableBool	ColorCorrection;
	
	FBPropertyBool				ColorCorrection_UseMasking;
	FBPropertyBaseEnum<EMaskingChannel>	ColorCorrection_MaskingChannel;

	FBPropertyBool					ChromaticAberration;	// ca
	FBPropertyAnimatableVector2d	ChromaticAberrationDirection;

	FBPropertyAnimatableDouble		Contrast;
	FBPropertyAnimatableDouble		Brightness;
	FBPropertyAnimatableDouble		Saturation;

	FBPropertyAnimatableDouble		Gamma;

	FBPropertyBool					Inverse;	//!< inverse colors

	FBPropertyBool					Bloom;
	FBPropertyAnimatableDouble		BloomMinBright;
	FBPropertyAnimatableDouble		BloomTone;
	FBPropertyAnimatableDouble		BloomStretch;

	FBPropertyAnimatableDouble		Hue;
	FBPropertyAnimatableDouble		HueSaturation;
	FBPropertyAnimatableDouble		Lightness;

	// Vignetting
	FBPropertyAnimatableBool	Vignetting;

	FBPropertyBool				Vign_UseMasking;
	FBPropertyBaseEnum<EMaskingChannel>	Vign_MaskingChannel;

	FBPropertyAnimatableDouble		VignAmount;
	FBPropertyAnimatableDouble		VignOut;
	FBPropertyAnimatableDouble		VignIn;
	FBPropertyAnimatableDouble		VignFade;

	// Film Grain
	FBPropertyAnimatableBool	FilmGrain;

	FBPropertyBool				FilmGrain_UseMasking;
	FBPropertyBaseEnum<EMaskingChannel>	FilmGrain_MaskingChannel;

	FBPropertyBool					FG_UsePlayTime;
	FBPropertyAnimatableDouble		FG_TimeSpeed;

	FBPropertyAnimatableDouble		FG_GrainAmount;
	FBPropertyBool					FG_Colored;	//!< colored noise ?
	FBPropertyAnimatableDouble		FG_ColorAmount;
	FBPropertyAnimatableDouble		FG_GrainSize;	//!< grain particle size (1.5 - 2.5)
	FBPropertyAnimatableDouble		FG_LumAmount;

	// Lens Flare
	FBPropertyAnimatableBool	LensFlare;

	FBPropertyBool				LensFlare_UseMasking;
	FBPropertyBaseEnum<EMaskingChannel>	LensFlare_MaskingChannel;

	FBPropertyBool				LensFlare_UseOcclusion; //!< fade out lens flare in case there is some geometry in front
	FBPropertyAnimatableDouble		FlareOcclusionSpeed; //!< a multiplier to a time we spend to fade in or out the flare effect from geometry occlusion
	FBPropertyListObject			FlareOcclusionObjects; //!< models that are taken part in occlusion culling process

	//Louis
	FBPropertyBaseEnum<EFlareType>	FlareType;
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

	FBPropertyBool				SSAO_UseMasking;
	FBPropertyBaseEnum<EMaskingChannel>	SSAO_MaskingChannel;

	FBPropertyAnimatableDouble		SSAO_Radius;
	
	FBPropertyAnimatableDouble		SSAO_Intensity;
	FBPropertyAnimatableDouble		SSAO_Bias;

	FBPropertyBool					OnlyAO;		//!< display only ambient occlusion or multiply with child color

	FBPropertyBool					SSAO_Blur;
	FBPropertyAnimatableDouble		SSAO_BlurSharpness;

	//
	FBPropertyAnimatableBool	MotionBlur;

	FBPropertyBool				MotionBlur_UseMasking;
	FBPropertyBaseEnum<EMaskingChannel>	MotionBlur_MaskingChannel;

	FBPropertyAnimatableDouble	MotionBlurAmount;

	// 3d DOF
	FBPropertyAnimatableBool	DepthOfField;

	FBPropertyBool				DOF_UseMasking;
	FBPropertyBaseEnum<EMaskingChannel>	DOF_MaskingChannel;

	FBPropertyBool				UseCameraDOFProperties;	//!< take focal distance, focal range and focus object from camera properties !

	FBPropertyAction			ResetDOF;

	FBPropertyBool				DebugBlurValue; //!< output only computed blur gradient
	FBPropertyAction			DebugFarDistance; //!< check if camera far distance is too big for glsl float
	FBPropertyAction			FixCameraSettings; //!< update camera far plane, and turns off real-time camera effects

	FBPropertyAnimatableDouble		FocalDistance;
	FBPropertyAnimatableDouble		FocalRange;
	FBPropertyAnimatableDouble		FStop;

	FBPropertyAnimatableBool		AutoFocus;		//!< autofocus to some point (instead of focal distance)
	FBPropertyListObject			FocusObject;	//!<  get a screen position of a specified tm
	
	FBPropertyAction				FocusObjectCreate;
	FBPropertyAction				FocusObjectSelect;
	
	FBPropertyBool					BlurForeground;	//!< flag to make everything clean in foreground

	FBPropertyBool					ManualFocus;
	FBPropertyAnimatableDouble		ManualNear;
	FBPropertyAnimatableDouble		ManualNearFalloff;
	FBPropertyAnimatableDouble		ManualFar;
	FBPropertyAnimatableDouble		ManualFarFalloff;

	FBPropertyBool					UseFocusPoint;
	FBPropertyAnimatableVector2d	FocusPoint;		//!< relative screen coord to grab a depth distance from

	FBPropertyBool						FastPreview;	//!< mode for using pre-calculated blur instead of clean DOF processing
	FBPropertyBaseEnum<EBlurQuality>	PreviewQuality;
	FBPropertyDouble					PreviewBlurAmount;

	FBPropertyInt					Samples;	//!< samples of the first ring
	FBPropertyInt					Rings;		//!< ring count

	FBPropertyAnimatableDouble		CoC;	//!< circle of confusion size in mm (35 mm film = 0.03mm)

	FBPropertyAnimatableDouble		Threshold;	//!< highlight threshold
	FBPropertyAnimatableDouble		Gain;		//!< highlight gain

	FBPropertyAnimatableDouble		Bias;	//!< bokeh edge bias
	FBPropertyAnimatableDouble		Fringe;	//!< bokeh chromatic aberration/fringing

	FBPropertyBool					Noise;	//!< use noise instead of pattern for sample dithering

	FBPropertyBool					Pentagon;		//!< use pentagon as bokeh shape
	FBPropertyAnimatableDouble		PentagonFeather;	//!< pentagon shape feather
	
	//
	// Displacement effect
	FBPropertyAnimatableBool	Displacement;

	FBPropertyBool				Disp_UseMasking;
	FBPropertyBaseEnum<EMaskingChannel>	Disp_MaskingChannel;

	FBPropertyBool				UseQuakeWaterEffect;

	FBPropertyBool				Disp_UsePlayTime;
	FBPropertyAnimatableDouble	Disp_Speed;
	FBPropertyAnimatableDouble	Disp_MagnitudeX;
	FBPropertyAnimatableDouble	Disp_MagnitudeY;

	FBPropertyAnimatableDouble	Disp_SinCyclesX;
	FBPropertyAnimatableDouble	Disp_SinCyclesY;

	// output viewport information
	FBPropertyInt				ViewerWidth;	//!< read-only, size of a whole viewer
	FBPropertyInt				ViewerHeight;	//!< read-only
	FBPropertyInt				LocalWidth;		//!< read-only, size of a effect processing FBO
	FBPropertyInt				LocalHeight;	//!< read-only
	FBPropertyInt				ColorBits;	//!< read-only, should be 32 by default
	FBPropertyInt				DepthBits;	//!< read-only, should be 24 by default

	// output a compressed downscaled image
	FBPropertyBool				OutputPreview;
	FBPropertyInt				OutputUpdateRate;	//!< how many times per second we are preparing a new preview
	FBPropertyDouble			OutputScaleFactor;	//!< in percentage how small we should make a final video (+power of two)
	FBPropertyDouble			OutputRatio;	//!< original aspect ratio
	FBPropertyInt				OutputWidth;
	FBPropertyInt				OutputHeight;
	FBPropertyListObject		OutputVideo;
	FBPropertyBool				OutputUseCompression; //!< enable / disable compression
	FBPropertyBaseEnum<EImageCompression>	OutputCompression;
	FBPropertyInt				OutputCompressionCode;
	FBPropertyInt				OutputUncompressSize;
	FBPropertyInt				OutputCompressedSize;	//!< stats for a ETC1 compressed size
	FBPropertyDouble			OutputCompressedTime;	//!< in secs

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

	/// <summary>
	/// return true if masking is activated globally or in any effect in particular
	/// </summary>
	bool HasAnyActiveMasking() const;

	bool IsMaskActive(const EMaskingChannel maskId) const;

	/// <summary>
	/// return an index of mask we define globally in settings
	/// </summary>
	int GetGlobalMaskIndex() const;

	int GetNumberOfActiveUserEffects();
	PostEffectBase* GetActiveUserEffect(const int index);
	PostEffectUserObject* GetActiveUserEffectObject(const int index);

protected:

	friend class ToolPostProcessing;

	FBSystem			mSystem;

    FBString			mText;
	bool				mReloadShaders{ false };
	EPostAction			mPostAction{ EPostAction::ePostActionNone };

	double				mResetFarPlane{ 0.0 };

	double				mTempLower{ 0.0 };
	double				mTempUpper{ 0.0 };

	int			mLazyLoadCounter{ 0 };

	void		DefaultValues();
	void		LoadFromConfig(const char *sessionFilter=nullptr);
	void		LoadFarValueFromConfig();

	void ComputePointInFront(FBVector3d &v);
};

#endif /* __POST_PROCESSING_DATA_H__ */
