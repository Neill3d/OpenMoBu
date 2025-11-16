
/** \file postpersistent_tool.cxx

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

// Class declaration
#include "postpersistentdata.h"
#include "postprocessing_ini.h"
#include <vector>
#include <limits>

#include "posteffect_userobject.h"

// custom assets inserting

/** Element Class implementation. (Asset system)
*	This should be placed in the source code file for a class.
*/
#define PostFBElementClassImplementation(ClassName,AssetName,IconFileName)\
	HIObject RegisterElement##ClassName##Create(HIObject /*pOwner*/, const char* pName, void* /*pData*/){\
	ClassName* Class = new ClassName(pName); \
	Class->mAllocated = true; \
if (Class->FBCreate()){\
	__FBRemoveModelFromScene(Class->GetHIObject()); /* Hack in MoBu2013, we shouldn't add object to the scene/entity automatically*/\
	return Class->GetHIObject(); \
}\
else {\
	delete Class; \
	return NULL;\
}\
	}\
		FBLibraryModule(ClassName##Element){\
		FBRegisterObject(ClassName##R2, "Browsing/Templates/Shading Elements", AssetName, "", RegisterElement##ClassName##Create, true, IconFileName); \
		}

//--- FiLMBOX Registration & Implementation.
FBClassImplementation(PostPersistentData);
FBUserObjectImplement(PostPersistentData,
                        "Use Object used to store a persistance data for the post processing tool.",
						"cam_switcher_toggle.png");                                          //Register UserObject class
PostFBElementClassImplementation(PostPersistentData, "Post Processing", "cam_switcher_toggle.png");                  //Register to the asset system

////////////////////////////////////////////////////////////////////////////////
//

const char * FBPropertyBaseEnum<EBlurQuality>::mStrings[] = {
	"Full",
	"Preview2",
	"Preview4",
	0 };

const char * FBPropertyBaseEnum<EImageCompression>::mStrings[] = {
	"Default",
	"S3TC",
	"ETC2",
	"ASTC",
	0 };

//Louis
//Modifiable Flares
const char * FBPropertyBaseEnum<EFlareType>::mStrings[] = {
	"Basic",
	"Bubbles",
	"Anamorphic",
	0
};

const char* FBPropertyBaseEnum<EMaskingChannel>::mStrings[] = {
	"Mask A",
	"Mask B",
	"Mask C",
	"Mask D",
	0
};

/////////////////////////////////////////////////////////////////////////////////////////////
//
/*
PostPersistentData		*GetGlobalSettingsPtr(const char *name)
{
	FBScene *pScene = FBSystem::TheOne().Scene;

	if (true == FBMergeTransactionIsOn())
		return nullptr;

	const int numberOfUserObjects = pScene->UserObjects.GetCount();
	int count = 0;

	PostPersistentData *ptr = nullptr;

	for (int i = 0; i < numberOfUserObjects; ++i)
	{
		if (FBIS(pScene->UserObjects[i], PostPersistentData))
		{
			ptr = (PostPersistentData*)pScene->UserObjects[i];
			count += 1;
		}
	}
	

	// return (PostPersistentData*)pScene->UserObjects[i];

	// otherwise create a new one

	if (nullptr == ptr)
	{
		ptr = (PostPersistentData *)FBCreateObject("FbxStorable/User",
			"PostPersistentData",
			(name != nullptr) ? name : "PostPersistentData");
	}

	return ptr;
}
*/

/************************************************
 *  Constructor.
 ************************************************/
PostPersistentData::PostPersistentData(const char* pName, HIObject pObject) 
	: FBUserObject(pName, pObject)
	, mText("")
{
    FBClassInit;

	mReloadShaders = false;
	mLazyLoadCounter = 0;
}

void PostPersistentData::ActionReloadShaders(HIObject pObject, bool value)
{
	PostPersistentData *p = FBCast<PostPersistentData>(pObject);
	if (p && value)
	{
		p->DoReloadShaders();
	}
}

void PostPersistentData::ActionResetToDefault(HIObject pObject, bool value)
{
	PostPersistentData *p = FBCast<PostPersistentData>(pObject);
	if (p && value)
	{
		p->DefaultValues();
	}
}

void PostPersistentData::ActionFlareLightCreate(HIObject pObject, bool value)
{
	PostPersistentData *p = FBCast<PostPersistentData>(pObject);
	if (p && value)
	{
		p->DoFlareLightCreate();
	}
}

void PostPersistentData::ActionFlareLightSelect(HIObject pObject, bool value)
{
	PostPersistentData *p = FBCast<PostPersistentData>(pObject);
	if (p && value)
	{
		p->DoFlareLightSelect();
	}
}

void PostPersistentData::ActionFocusObjectCreate(HIObject object, bool value)
{
	PostPersistentData *pBase = FBCast<PostPersistentData>(object);
	if (pBase && value)
	{
		pBase->DoFocusObjectCreate();
	}
}

void PostPersistentData::ActionFocusObjectSelect(HIObject object, bool value)
{
	PostPersistentData *pBase = FBCast<PostPersistentData>(object);
	if (pBase && value)
	{
		pBase->DoFocusObjectSelect();
	}
}

void PostPersistentData::ActionResetDOF(HIObject object, bool value)
{
	PostPersistentData *pBase = FBCast<PostPersistentData>(object);
	if (pBase && value)
	{
		pBase->DoResetDOF();
	}
}

void PostPersistentData::SetFastPreview(HIObject object, bool value)
{
	PostPersistentData *pBase = FBCast<PostPersistentData>(object);
	if (pBase)
	{
		//pBase->ChangeGlobalSettings();
		pBase->FastPreview.SetPropertyValue(value);
	}
}

void PostPersistentData::ActionDebugFarDist(HIObject object, bool value)
{
	PostPersistentData *pBase = FBCast<PostPersistentData>(object);
	if (pBase)
	{
		pBase->DoDebugFarDist();
	}
}

void PostPersistentData::ActionFixCameraSettings(HIObject object, bool value)
{
	PostPersistentData *pBase = FBCast<PostPersistentData>(object);
	if (pBase)
	{
		pBase->DoFixCameraSettings();
	}
}

void PostPersistentData::AddPropertyView(const char* pPropertyName, const char* pHierarchy, bool pIsFolder)
{
	FBPropertyViewManager::TheOne().AddPropertyView(POSTDATA__CLASSSTR, pPropertyName, pHierarchy);
}

void PostPersistentData::AddPropertiesToPropertyViewManager()
{
	AddPropertyView("Active", "");
	AddPropertyView("Use Camera Object", "");
	AddPropertyView("Camera", "");
	AddPropertyView("Active", "");
	AddPropertyView("Reload Shaders", "");
	AddPropertyView("Reset To Default", "");
	
	AddPropertyView("User Effects", "");

	AddPropertyView("SSAO", "");
	AddPropertyView("Motion Blur", "");
	AddPropertyView("Depth Of Field", "");
	AddPropertyView("Color Correction", "");
	AddPropertyView("Lens Flare", "");
	AddPropertyView("Displacement", "");
	AddPropertyView("Fish Eye", "");
	AddPropertyView("Film Grain", "");
	AddPropertyView("Vignetting", "");
	
	//
	AddPropertyView("Common Setup", "", true);
	AddPropertyView("Draw HUD Layer", "Common Setup");
	AddPropertyView("Generate MipMaps", "Common Setup");

	AddPropertyView("Auto Clip From HUD", "Common Setup");
	AddPropertyView("Top Clip Percent", "Common Setup");
	AddPropertyView("Bottom Clip Percent", "Common Setup");

	AddPropertyView("Use Masking", "Common Setup");
	
	AddPropertyView("Invert Mask A", "Common Setup.Mask A");
	AddPropertyView("Blur Mask A", "Common Setup.Mask A");
	AddPropertyView("Blur Mask A Scale", "Common Setup.Mask A");
	AddPropertyView("Use Rim For Mask A", "Common Setup.Mask A");
	AddPropertyView("Mask A Rim Power", "Common Setup.Mask A");
	AddPropertyView("Use Mask A To Mix", "Common Setup.Mask A");
	AddPropertyView("Mask A Mix With", "Common Setup.Mask A");

	AddPropertyView("Invert Mask B", "Common Setup.Mask B");
	AddPropertyView("Blur Mask B", "Common Setup.Mask B");
	AddPropertyView("Blur Mask B Scale", "Common Setup.Mask B");
	AddPropertyView("Use Rim For Mask B", "Common Setup.Mask B");
	AddPropertyView("Mask B Rim Power", "Common Setup.Mask B");
	AddPropertyView("Use Mask B To Mix", "Common Setup.Mask B");
	AddPropertyView("Mask B Mix With", "Common Setup.Mask B");

	AddPropertyView("Invert Mask C", "Common Setup.Mask C");
	AddPropertyView("Blur Mask C", "Common Setup.Mask C");
	AddPropertyView("Blur Mask C Scale", "Common Setup.Mask C");
	AddPropertyView("Use Rim For Mask C", "Common Setup.Mask C");
	AddPropertyView("Mask C Rim Power", "Common Setup.Mask C");
	AddPropertyView("Use Mask C To Mix", "Common Setup.Mask C");
	AddPropertyView("Mask C Mix With", "Common Setup.Mask C");

	AddPropertyView("Invert Mask D", "Common Setup.Mask D");
	AddPropertyView("Blur Mask D", "Common Setup.Mask D");
	AddPropertyView("Blur Mask D Scale", "Common Setup.Mask D");
	AddPropertyView("Use Rim For Mask D", "Common Setup.Mask D");
	AddPropertyView("Mask D Rim Power", "Common Setup.Mask D");
	AddPropertyView("Use Mask D To Mix", "Common Setup.Mask D");
	AddPropertyView("Mask D Mix With", "Common Setup.Mask D");

	AddPropertyView("Enable Masking For All Effects", "Common Setup");
	AddPropertyView("Global Masking Channel", "Common Setup");
	AddPropertyView("Debug Display Masking", "Common Setup");

	
	
	//
	AddPropertyView("SSAO Setup", "", true);

	AddPropertyView("SSAO Use Masking", "SSAO Setup");
	AddPropertyView("SSAO Masking Channel", "SSAO Setup");

	AddPropertyView("SSAO Radius", "SSAO Setup");
	AddPropertyView("SSAO Intensity", "SSAO Setup");
	AddPropertyView("SSAO Bias", "SSAO Setup");

	AddPropertyView("Only AO", "SSAO Setup");

	AddPropertyView("SSAO Blur", "SSAO Setup");
	AddPropertyView("SSAO Blur Sharpness", "SSAO Setup");

	//
	AddPropertyView("Motion Blur Setup", "", true);
	AddPropertyView("Motion Blur Use Masking", "Motion Blur Setup");
	AddPropertyView("Motion Blur Masking Channel", "Motion Blur Setup");
	AddPropertyView("Motion Blur Amount", "Motion Blur Setup");

	//
	AddPropertyView("Depth Of Field Setup", "", true);

	AddPropertyView("Depth Of Field Use Masking", "Depth Of Field Setup");
	AddPropertyView("Depth Of Field Masking Channel", "Depth Of Field Setup");

	AddPropertyView("Use Camera DOF Properties", "Depth Of Field Setup");
	AddPropertyView("Reset DOF", "Depth Of Field Setup");
	AddPropertyView("Debug Blur Value", "Depth Of Field Setup");
	AddPropertyView("Debug Camera Far Dist", "Depth Of Field Setup");
	AddPropertyView("Fix Camera Settings", "Depth Of Field Setup");

	AddPropertyView("Focal Distance", "Depth Of Field Setup");
	AddPropertyView("Focal Range", "Depth Of Field Setup");
	AddPropertyView("F-Stop", "Depth Of Field Setup");

	AddPropertyView("Auto Focus", "Depth Of Field Setup");
	AddPropertyView("Focus Object", "Depth Of Field Setup");

	AddPropertyView("Create A Focus Object", "Depth Of Field Setup.Focus Object");
	AddPropertyView("Select A Focus Object", "Depth Of Field Setup.Focus Object");

	AddPropertyView("Fast Preview", "Depth Of Field Setup");
	AddPropertyView("Preview Quality", "Depth Of Field Setup");
	AddPropertyView("Preview Blur Amount", "Depth Of Field Setup");

	AddPropertyView("Blur Foreground", "Depth Of Field Setup");

	AddPropertyView("Samples", "Depth Of Field Setup");
	AddPropertyView("Ring count", "Depth Of Field Setup");

	AddPropertyView("Circle of confusion", "Depth Of Field Setup");

	AddPropertyView("Highlight Threshold", "Depth Of Field Setup");
	AddPropertyView("Highlight Gain", "Depth Of Field Setup");

	AddPropertyView("Bokeh Bias", "Depth Of Field Setup");
	AddPropertyView("Bokeh Fringe", "Depth Of Field Setup");

	AddPropertyView("Noise", "Depth Of Field Setup");

	AddPropertyView("Pentagon", "Depth Of Field Setup");
	AddPropertyView("Pentagon feather", "Depth Of Field Setup");

	AddPropertyView("Use Focus Point", "Depth Of Field Setup");
	AddPropertyView("Focus Point", "Depth Of Field Setup");


	//
	AddPropertyView("Color Correction Setup", "", true);

	AddPropertyView("Color Correction Use Masking", "Color Correction Setup");
	AddPropertyView("Color Correction Masking Channel", "Color Correction Setup");

	AddPropertyView("Chromatic Aberration", "Color Correction Setup");
	AddPropertyView("Chromatic Aberration Direction", "Color Correction Setup");

	AddPropertyView("Contrast", "Color Correction Setup");
	AddPropertyView("Brightness", "Color Correction Setup");
	AddPropertyView("Saturation", "Color Correction Setup");

	AddPropertyView("Gamma", "Color Correction Setup");
	AddPropertyView("Inverse", "Color Correction Setup");

	AddPropertyView("Bloom", "Color Correction Setup");
	AddPropertyView("Bloom Min Bright", "Color Correction Setup");
	AddPropertyView("Bloom Tone", "Color Correction Setup");
	AddPropertyView("Bloom Stretch", "Color Correction Setup");

	AddPropertyView("Hue", "Color Correction Setup");
	AddPropertyView("Hue Saturation", "Color Correction Setup");
	AddPropertyView("Lightness", "Color Correction Setup");

	//
	AddPropertyView("Lens Flare Setup", "", true);

	AddPropertyView("Flare Use Masking", "Lens Flare Setup");
	AddPropertyView("Flare Masking Channel", "Lens Flare Setup");
	AddPropertyView("Flare Use Occlusion", "Lens Flare Setup");
	AddPropertyView("Flare Occlusion Objects", "Lens Flare Setup");
	AddPropertyView("Flare Occlusion Speed", "Lens Flare Setup");

	//Louis 
	AddPropertyView("Flare Type", "Lens Flare Setup");
	AddPropertyView("Flare Seed", "Lens Flare Setup");
	AddPropertyView("Flare Use Play Time", "Lens Flare Setup");
	AddPropertyView("Flare Time Speed", "Lens Flare Setup");

	AddPropertyView("Use Flare Light Object", "Lens Flare Setup");
	AddPropertyView("Flare Light", "Lens Flare Setup");

	AddPropertyView("Create A Flare Light", "Lens Flare Setup.Flare Light");
	AddPropertyView("Select A Flare Light", "Lens Flare Setup.Flare Light");

	AddPropertyView("Lens Flare Amount", "Lens Flare Setup");
	AddPropertyView("Flare Depth Attenuation", "Lens Flare Setup");
	AddPropertyView("Lens Flare X", "Lens Flare Setup");
	AddPropertyView("Lens Flare Y", "Lens Flare Setup");

	AddPropertyView("Lens Flare Tint", "Lens Flare Setup");
	AddPropertyView("Lens Flare Inner", "Lens Flare Setup");
	AddPropertyView("Lens Flare Outer", "Lens Flare Setup");

	AddPropertyView("Flare Fade To Borders", "Lens Flare Setup");
	AddPropertyView("Flare Border Width", "Lens Flare Setup");
	AddPropertyView("Flare Border Feather", "Lens Flare Setup");

	//
	AddPropertyView("Displacement Setup", "", true);

	AddPropertyView("Disp Use Masking", "Displacement Setup");
	AddPropertyView("Disp Masking Channel", "Displacement Setup");

	AddPropertyView("Use Quake Water Effect", "Displacement Setup");
	
	AddPropertyView("Disp Use Play Time", "Displacement Setup");
	AddPropertyView("Displacement Speed", "Displacement Setup");
	AddPropertyView("Disp Magnitude X", "Displacement Setup");
	AddPropertyView("Disp Magnitude Y", "Displacement Setup");

	AddPropertyView("Disp Sin Cycles X", "Displacement Setup");
	AddPropertyView("Disp Sin Cycles Y", "Displacement Setup");

	//
	AddPropertyView("Fish Eye Setup", "", true);
	AddPropertyView("Fish Eye Use Masking", "Fish Eye Setup");
	AddPropertyView("Fish Eye Masking Channel", "Fish Eye Setup");
	AddPropertyView("Fish Eye Order", "Fish Eye Setup");
	AddPropertyView("Fish Eye Amount", "Fish Eye Setup");
	AddPropertyView("Fish Eye Lens Radius", "Fish Eye Setup");
	AddPropertyView("Fish Eye Sign Curvature", "Fish Eye Setup");

	//
	AddPropertyView("Film Grain Setup", "", true);
	AddPropertyView("Grain Use Masking", "Film Grain Setup");
	AddPropertyView("Grain Masking Channel", "Film Grain Setup");
	AddPropertyView("Grain Use Play Time", "Film Grain Setup");
	AddPropertyView("Grain Time Speed", "Film Grain Setup");
	AddPropertyView("Grain Amount", "Film Grain Setup");
	AddPropertyView("Grain Colored", "Film Grain Setup");
	AddPropertyView("Grain Color Amount", "Film Grain Setup");
	AddPropertyView("Grain Size", "Film Grain Setup");
	AddPropertyView("Grain Lum Amount", "Film Grain Setup");

	//
	AddPropertyView("Vignetting Setup", "", true);
	AddPropertyView("Vignetting Use Masking", "Vignetting Setup");
	AddPropertyView("Vignetting Masking Channel", "Vignetting Setup");
	AddPropertyView("Vignetting Amount", "Vignetting Setup");
	AddPropertyView("Vignetting Outer Border", "Vignetting Setup");
	AddPropertyView("Vignetting Inner Border", "Vignetting Setup");
	AddPropertyView("Vignetting Fade", "Vignetting Setup");
	
	//
	AddPropertyView("Preview Output Setup", "", true);
	AddPropertyView("Output Preview", "Preview Output Setup");
	AddPropertyView("Output Update Rate", "Preview Output Setup");
	AddPropertyView("Output Use Compression", "Preview Output Setup");
	AddPropertyView("Output Compression", "Preview Output Setup");
	AddPropertyView("Output Compression Code", "Preview Output Setup");

	AddPropertyView("Output Scale Factor", "Preview Output Setup");
	AddPropertyView("Output Video", "Preview Output Setup");
	
	AddPropertyView("Output Aspect", "Preview Output Setup");
	AddPropertyView("Output Width", "Preview Output Setup");
	AddPropertyView("Output Height", "Preview Output Setup");

	AddPropertyView("Output UnCompress Size", "Preview Output Setup");
	AddPropertyView("Output Compressed Size", "Preview Output Setup");

	AddPropertyView("Output Compressed Time", "Preview Output Setup");

	AddPropertyView("Is Synced", "Preview Output Setup");
	AddPropertyView("Device Address", "Preview Output Setup");
	AddPropertyView("Device Port", "Preview Output Setup");
}

/************************************************
 *  FiLMBOX Constructor.
 ************************************************/
bool PostPersistentData::FBCreate()
{
	// modify system behavoiur
	DisableObjectFlags(kFBFlagClonable);

	FBPropertyPublish(this, UniqueClassId, "UniqueClassId", nullptr, nullptr);
	FBPropertyPublish(this, Active, "Active", nullptr, nullptr);
	FBPropertyPublish(this, DrawHUDLayer, "Draw HUD Layer", nullptr, nullptr);

	FBPropertyPublish(this, UseCameraObject, "Use Camera Object", nullptr, nullptr);
	FBPropertyPublish(this, Camera, "Camera", nullptr, nullptr);

	FBPropertyPublish(this, ReloadShaders, "Reload Shaders", nullptr, ActionReloadShaders);
	FBPropertyPublish(this, GenerateMipMaps, "Generate MipMaps", nullptr, nullptr);
	FBPropertyPublish(this, ResetToDefault, "Reset To Default", nullptr, ActionResetToDefault);

	FBPropertyPublish(this, UserEffects, "User Effects", nullptr, nullptr);

	FBPropertyPublish(this, AutoClipFromHUD, "Auto Clip From HUD", nullptr, nullptr);
	FBPropertyPublish(this, UpperClip, UPPER_CLIP, nullptr, nullptr);
	FBPropertyPublish(this, LowerClip, LOWER_CLIP, nullptr, nullptr);

	// global masking properties
	FBPropertyPublish(this, UseCompositeMasking, "Use Masking", nullptr, nullptr);
	FBPropertyPublish(this, EnableMaskingForAllEffects, "Enable Masking For All Effects", nullptr, nullptr);
	FBPropertyPublish(this, GlobalMaskingChannel, "Global Masking Channel", nullptr, nullptr);
	FBPropertyPublish(this, DebugDisplyMasking, "Debug Display Masking", nullptr, nullptr);

	const char MASK_INDEX_NAMES[NUMBER_OF_MASKS] = { 'A', 'B', 'C', 'D' };
	char buffer[64]{ 0 };

	for (int i = 0; i < NUMBER_OF_MASKS; ++i)
	{
		snprintf(buffer, sizeof(char) * 64, "Invert Mask %c", MASK_INDEX_NAMES[i]);
		FBPropertyPublish(this, Masks[i].InvertMask, buffer, nullptr, nullptr);

		snprintf(buffer, sizeof(char) * 64, "Blur Mask %c", MASK_INDEX_NAMES[i]);
		FBPropertyPublish(this, Masks[i].BlurMask, buffer, nullptr, nullptr);

		snprintf(buffer, sizeof(char) * 64, "Blur Mask %c Scale", MASK_INDEX_NAMES[i]);
		Masks[i].BlurMaskScale = PropertyCreate(buffer, FBPropertyType::kFBPT_Vector2D, "Vector2", true, false, nullptr);

		snprintf(buffer, sizeof(char) * 64, "Use Rim For Mask %c", MASK_INDEX_NAMES[i]);
		FBPropertyPublish(this, Masks[i].UseRimForMask, buffer, nullptr, nullptr);

		snprintf(buffer, sizeof(char) * 64, "Mask %c Rim Power", MASK_INDEX_NAMES[i]);
		FBPropertyPublish(this, Masks[i].MaskRimPower, buffer, nullptr, nullptr);

		snprintf(buffer, sizeof(char) * 64, "Use Mask %c To Mix", MASK_INDEX_NAMES[i]);
		FBPropertyPublish(this, Masks[i].UseMixWithMask, buffer, nullptr, nullptr);

		snprintf(buffer, sizeof(char) * 64, "Mask %c Mix With", MASK_INDEX_NAMES[i]);
		FBPropertyPublish(this, Masks[i].MixWithMask, buffer, nullptr, nullptr);
	}

	// SSAO

	FBPropertyPublish(this, SSAO, "SSAO", nullptr, nullptr);

	FBPropertyPublish(this, SSAO_UseMasking, SSAO_USE_MASKING, nullptr, nullptr);
	FBPropertyPublish(this, SSAO_MaskingChannel, SSAO_MASKING_CHANNEL, nullptr, nullptr);

	FBPropertyPublish(this, SSAO_Radius, SSAO_RADIUS, nullptr, nullptr);
	FBPropertyPublish(this, SSAO_Intensity, SSAO_INTENSITY, nullptr, nullptr);
	FBPropertyPublish(this, SSAO_Bias, SSAO_BIAS, nullptr, nullptr);
	FBPropertyPublish(this, OnlyAO, SSAO_ONLY_AO, nullptr, nullptr);
	FBPropertyPublish(this, SSAO_Blur, SSAO_BLUR, nullptr, nullptr);
	FBPropertyPublish(this, SSAO_BlurSharpness, SSAO_BLUR_SHARPNESS, nullptr, nullptr);

	// Motion Blur
	
	FBPropertyPublish(this, MotionBlur, "Motion Blur", nullptr, nullptr);
	FBPropertyPublish(this, MotionBlur_UseMasking, MOTIONBLUR_USE_MASKING, nullptr, nullptr);
	FBPropertyPublish(this, MotionBlur_MaskingChannel, MOTIONBLUR_MASKING_CHANNEL, nullptr, nullptr);
	FBPropertyPublish(this, MotionBlurAmount, MOTIONBLUR_AMOUNT, nullptr, nullptr);
	
	// Depth Of Field

	FBPropertyPublish(this, DepthOfField, "Depth Of Field", nullptr, nullptr);

	FBPropertyPublish(this, DOF_UseMasking, DOF_USE_MASKING, nullptr, nullptr);
	FBPropertyPublish(this, DOF_MaskingChannel, DOF_MASKING_CHANNEL, nullptr, nullptr);

	FBPropertyPublish(this, UseCameraDOFProperties, USE_CAMERA_DOF_PROPS, nullptr, nullptr);
	FBPropertyPublish(this, ResetDOF, RESET_DOF, nullptr, ActionResetDOF);
	FBPropertyPublish(this, DebugBlurValue, DOF_DEBUG_BLUR_VALUE, nullptr, nullptr);
	FBPropertyPublish(this, DebugFarDistance, DOF_DEBUG_FAR_DIST, nullptr, ActionDebugFarDist);
	FBPropertyPublish(this, FixCameraSettings, DOF_FIX_CAM_SETTINGS, nullptr, ActionFixCameraSettings);

	FBPropertyPublish(this, FocalDistance, DOF_FOCAL_DISTANCE, nullptr, nullptr);
	FBPropertyPublish(this, FocalRange, DOF_FOCAL_RANGE, nullptr, nullptr);
	FBPropertyPublish(this, FStop, DOF_FSTOP, nullptr, nullptr);

	FBPropertyPublish(this, AutoFocus, DOF_AUTO_FOCUS, nullptr, nullptr);
	FBPropertyPublish(this, FocusObject, DOF_FOCUS_OBJECT, nullptr, nullptr);
	FBPropertyPublish(this, FocusObjectCreate, "Create A Focus Object", nullptr, ActionFocusObjectCreate);
	FBPropertyPublish(this, FocusObjectSelect, "Select A Focus Object", nullptr, ActionFocusObjectSelect);

	FBPropertyPublish(this, FastPreview, "Fast Preview", nullptr, SetFastPreview);
	FBPropertyPublish(this, PreviewQuality, "Preview Quality", nullptr, nullptr);
	FBPropertyPublish(this, PreviewBlurAmount, "Preview Blur Amount", nullptr, nullptr);

	FBPropertyPublish(this, BlurForeground, DOF_BLUR_FOREGROUND, nullptr, nullptr);
	/*
	FBPropertyPublish(this, ManualFocus, "Manual mode", nullptr, nullptr);
	FBPropertyPublish(this, ManualNear, "Manual Near Distance", nullptr, nullptr);
	FBPropertyPublish(this, ManualNearFalloff, "Manual Near Falloff", nullptr, nullptr);
	FBPropertyPublish(this, ManualFar, "Manual Far Distance", nullptr, nullptr);
	FBPropertyPublish(this, ManualFarFalloff, "Manual Far Falloff", nullptr, nullptr);
	*/

	FBPropertyPublish(this, UseFocusPoint, DOF_USE_FOCUS_POINT, nullptr, nullptr);
	FBPropertyPublish(this, FocusPoint, DOF_FOCUS_POINT, nullptr, nullptr);

	FBPropertyPublish(this, Samples, DOF_SAMPLES, nullptr, nullptr);
	FBPropertyPublish(this, Rings, DOF_RINGS, nullptr, nullptr);

	FBPropertyPublish(this, CoC, DOF_COC, nullptr, nullptr);

	FBPropertyPublish(this, Threshold, DOF_THRESHOLD, nullptr, nullptr);
	FBPropertyPublish(this, Gain, DOF_GAIN, nullptr, nullptr);

	FBPropertyPublish(this, Bias, DOF_BIAS, nullptr, nullptr);
	FBPropertyPublish(this, Fringe, DOF_FRINGE, nullptr, nullptr);

	FBPropertyPublish(this, Noise, DOF_NOISE, nullptr, nullptr);

	FBPropertyPublish(this, Pentagon, DOF_PENTAGON, nullptr, nullptr);
	FBPropertyPublish(this, PentagonFeather, DOF_PENTAGON_FEATHER, nullptr, nullptr);

	// Color Correction

	FBPropertyPublish(this, ColorCorrection, "Color Correction", nullptr, nullptr);

	FBPropertyPublish(this, ColorCorrection_UseMasking, COLOR_USE_MASKING, nullptr, nullptr);
	FBPropertyPublish(this, ColorCorrection_MaskingChannel, COLOR_MASKING_CHANNEL, nullptr, nullptr);

	FBPropertyPublish(this, ChromaticAberration, CHROMATIC_ABERRATION, nullptr, nullptr);
	FBPropertyPublish(this, ChromaticAberrationDirection, CHROMATIC_ABERRATION_DIR, nullptr, nullptr);

	FBPropertyPublish(this, Contrast, CONTRAST, nullptr, nullptr);
	FBPropertyPublish(this, Brightness, BRIGHTNESS, nullptr, nullptr);
	FBPropertyPublish(this, Saturation, SATURATION, nullptr, nullptr);

	FBPropertyPublish(this, Gamma, COLOR_GAMMA, nullptr, nullptr);
	FBPropertyPublish(this, Inverse, COLOR_INVERSE, nullptr, nullptr);

	FBPropertyPublish(this, Bloom, BLOOM, nullptr, nullptr);
	FBPropertyPublish(this, BloomMinBright, BLOOM_MIN_BRIGHT, nullptr, nullptr);
	FBPropertyPublish(this, BloomTone, BLOOM_TONE, nullptr, nullptr);
	FBPropertyPublish(this, BloomStretch, BLOOM_STRETCH, nullptr, nullptr);
	
	FBPropertyPublish(this, Hue, COLOR_HUE, nullptr, nullptr);
	FBPropertyPublish(this, HueSaturation, COLOR_HUE_SATURATION, nullptr, nullptr);
	FBPropertyPublish(this, Lightness, COLOR_LIGHTNESS, nullptr, nullptr);

	// Lens Flare

	FBPropertyPublish(this, LensFlare, "Lens Flare", nullptr, nullptr);
	FBPropertyPublish(this, LensFlare_UseMasking, FLARE_USE_MASKING, nullptr, nullptr);
	FBPropertyPublish(this, LensFlare_MaskingChannel, FLARE_MASKING_CHANNEL, nullptr, nullptr);
	FBPropertyPublish(this, LensFlare_UseOcclusion, FLARE_USE_OCCLUSION, nullptr, nullptr);
	FBPropertyPublish(this, FlareOcclusionSpeed, FLARE_OCC_SPEED, nullptr, nullptr);
	FBPropertyPublish(this, FlareOcclusionObjects, FLARE_OCC_OBJECTS, nullptr, nullptr);

	//Louis 
	FBPropertyPublish(this, FlareType, FLARE_TYPE, nullptr, nullptr);
	FBPropertyPublish(this, FlareSeed, FLARE_SEED, nullptr, nullptr);
	FBPropertyPublish(this, FlareUsePlayTime, FLARE_USE_PLAY_TIME, nullptr, nullptr);
	FBPropertyPublish(this, FlareTimeSpeed, FLARE_TIME_SPEED, nullptr, nullptr);

	FBPropertyPublish(this, UseFlareLightObject, FLARE_USE_LIGHT_OBJECT, nullptr, nullptr);
	FBPropertyPublish(this, FlareLight, FLARE_LIGHT, nullptr, nullptr);

	FBPropertyPublish(this, FlareLightCreate, "Create A Flare Light", nullptr, ActionFlareLightCreate);
	FBPropertyPublish(this, FlareLightSelect, "Select A Flare Light", nullptr, ActionFlareLightSelect);

	FBPropertyPublish(this, FlareAmount, FLARE_AMOUNT, nullptr, nullptr);
	FBPropertyPublish(this, FlareDepthAttenuation, FLARE_DEPTH_ATT, nullptr, nullptr);
	FBPropertyPublish(this, FlarePosX, FLARE_POSX, nullptr, nullptr);
	FBPropertyPublish(this, FlarePosY, FLARE_POSY, nullptr, nullptr);

	FBPropertyPublish(this, FlareTint, FLARE_TINT, nullptr, nullptr);
	FBPropertyPublish(this, FlareInner, FLARE_INNER, nullptr, nullptr);
	FBPropertyPublish(this, FlareOuter, FLARE_OUTER, nullptr, nullptr);

	FBPropertyPublish(this, FlareFadeToBorders, FLARE_FADE_TO_BORDERS, nullptr, nullptr);
	FBPropertyPublish(this, FlareBorderWidth, FLARE_BORDER_WIDTH, nullptr, nullptr);
	FBPropertyPublish(this, FlareBorderFeather, FLARE_BORDER_FEATHER, nullptr, nullptr);

	// Displacement

	FBPropertyPublish(this, Displacement, "Displacement", nullptr, nullptr);
	FBPropertyPublish(this, Disp_UseMasking, DISP_USE_MASKING, nullptr, nullptr);
	FBPropertyPublish(this, Disp_MaskingChannel, DISP_MASKING_CHANNEL, nullptr, nullptr);

	FBPropertyPublish(this, UseQuakeWaterEffect, DISP_USE_QUAKE_EFFECT, nullptr, nullptr);

	FBPropertyPublish(this, Disp_UsePlayTime, DISP_USE_PLAY_TIME, nullptr, nullptr);
	FBPropertyPublish(this, Disp_Speed, DISP_SPEED, nullptr, nullptr);
	FBPropertyPublish(this, Disp_MagnitudeX, DISP_MAGNITUDE_X, nullptr, nullptr);
	FBPropertyPublish(this, Disp_MagnitudeY, DISP_MAGNITUDE_Y, nullptr, nullptr);

	FBPropertyPublish(this, Disp_SinCyclesX, DISP_SIN_CYCLES_X, nullptr, nullptr);
	FBPropertyPublish(this, Disp_SinCyclesY, DISP_SIN_CYCLES_Y, nullptr, nullptr);

	// Fish Eye

	FBPropertyPublish(this, FishEye, "Fish Eye", nullptr, nullptr);
	FBPropertyPublish(this, FishEye_UseMasking, FISHEYE_USE_MASKING, nullptr, nullptr);
	FBPropertyPublish(this, FishEye_MaskingChannel, FISHEYE_MASKING_CHANNEL, nullptr, nullptr);

	FBPropertyPublish(this, FishEyeAmount, FISHEYE_AMOUNT, nullptr, nullptr);
	FBPropertyPublish(this, FishEyeLensRadius, FISHEYE_LENS_RADIUS, nullptr, nullptr);
	FBPropertyPublish(this, FishEyeSignCurvature, FISHEYE_SIGN_CURV, nullptr, nullptr);
	FBPropertyPublish(this, FishEyeOrder, FISHEYE_ORDER, nullptr, nullptr);

	// Film Grain

	
	FBPropertyPublish(this, FilmGrain, "Film Grain", nullptr, nullptr);
	FBPropertyPublish(this, FilmGrain_UseMasking, GRAIN_USE_MASKING, nullptr, nullptr);
	FBPropertyPublish(this, FilmGrain_MaskingChannel, GRAIN_MASKING_CHANNEL, nullptr, nullptr);

	FBPropertyPublish(this, FG_UsePlayTime, GRAIN_USE_PLAY_TIME, nullptr, nullptr);
	FBPropertyPublish(this, FG_TimeSpeed, GRAIN_SPEED, nullptr, nullptr);

	FBPropertyPublish(this, FG_GrainAmount, GRAIN_AMOUNT, nullptr, nullptr);
	FBPropertyPublish(this, FG_Colored, GRAIN_COLORED, nullptr, nullptr);
	FBPropertyPublish(this, FG_ColorAmount, GRAIN_COLOR_AMOUNT, nullptr, nullptr);
	FBPropertyPublish(this, FG_GrainSize, GRAIN_SIZE, nullptr, nullptr);
	FBPropertyPublish(this, FG_LumAmount, GRAIN_LUMAMOUNT, nullptr, nullptr);

	// Vignetting 

	FBPropertyPublish(this, Vignetting, "Vignetting", nullptr, nullptr);
	FBPropertyPublish(this, Vign_UseMasking, VIGN_USE_MASKING, nullptr, nullptr);
	FBPropertyPublish(this, Vign_MaskingChannel, VIGN_MASKING_CHANNEL, nullptr, nullptr);

	FBPropertyPublish(this, VignAmount, VIGN_AMOUNT, nullptr, nullptr);
	FBPropertyPublish(this, VignOut, VIGN_OUT, nullptr, nullptr);
	FBPropertyPublish(this, VignIn, VIGN_IN, nullptr, nullptr);
	FBPropertyPublish(this, VignFade, VIGN_FADE, nullptr, nullptr);

	// preview output
	FBPropertyPublish(this, OutputPreview, "Output Preview", nullptr, nullptr);
	FBPropertyPublish(this, OutputUpdateRate, "Output Update Rate", nullptr, nullptr);
	FBPropertyPublish(this, OutputScaleFactor, "Output Scale Factor", nullptr, nullptr);
	FBPropertyPublish(this, OutputRatio, "Output Aspect", nullptr, nullptr);
	FBPropertyPublish(this, OutputWidth, "Output Width", nullptr, nullptr);
	FBPropertyPublish(this, OutputHeight, "Output Height", nullptr, nullptr);
	FBPropertyPublish(this, OutputVideo, "Output Video", nullptr, nullptr);
	FBPropertyPublish(this, OutputUseCompression, "Output Use Compression", nullptr, nullptr);
	FBPropertyPublish(this, OutputCompression, "Output Compression", nullptr, nullptr);
	FBPropertyPublish(this, OutputCompressionCode, "Output Compression Code", nullptr, nullptr);
	FBPropertyPublish(this, OutputUncompressSize, "Output UnCompress Size", nullptr, nullptr);
	FBPropertyPublish(this, OutputCompressedSize, "Output Compressed Size", nullptr, nullptr);
	FBPropertyPublish(this, OutputCompressedTime, "Output Compressed Time", nullptr, nullptr);

	FBPropertyPublish(this, IsSynced, "Is Synced", nullptr, nullptr);
	FBPropertyPublish(this, DeviceAddress, "Device Address", nullptr, nullptr);
	FBPropertyPublish(this, DevicePort, "Device Port", nullptr, nullptr);

	//
	OutputPreview = false;
	OutputUpdateRate.SetMinMax(1.0, 60.0);
	OutputScaleFactor.SetMinMax(0.0, 100.0, true, true);
	OutputScaleFactor = 15.0;
	OutputVideo.SetSingleConnect(true);
	OutputVideo.SetFilter(FBVideo::GetInternalClassId());
	OutputRatio = 1.0;
	OutputRatio.ModifyPropertyFlag(kFBPropertyFlagReadOnly, true);
	OutputWidth = 1;
	OutputWidth.ModifyPropertyFlag(kFBPropertyFlagReadOnly, true);
	OutputHeight = 1;
	OutputHeight.ModifyPropertyFlag(kFBPropertyFlagReadOnly, true);
	
	OutputUseCompression = true;
	const EImageCompression defaultCompression = eImageCompressionDefault;
	OutputCompression.SetData((void*)& defaultCompression);

	OutputUncompressSize = 0;
	OutputUncompressSize.ModifyPropertyFlag(kFBPropertyFlagReadOnly, true);
	OutputCompressedSize = 0;
	OutputCompressedSize.ModifyPropertyFlag(kFBPropertyFlagReadOnly, true);
	OutputCompressionCode = 0;
	OutputCompressionCode.ModifyPropertyFlag(kFBPropertyFlagReadOnly, true);
	OutputCompressedTime.ModifyPropertyFlag(kFBPropertyFlagReadOnly, true);

	//
	UniqueClassId.ModifyPropertyFlag(kFBPropertyFlagHideProperty, true);
	UniqueClassId.ModifyPropertyFlag(kFBPropertyFlagNotSavable, true);
	UniqueClassId.ModifyPropertyFlag(kFBPropertyFlagReadOnly, true);
	UniqueClassId = 55;

	UpperClip.SetMinMax(0.0, 100.0);
	LowerClip.SetMinMax(0.0, 100.0);

	//
	Camera.SetFilter(FBCamera::GetInternalClassId());
	Camera.SetSingleConnect(false);

	//
	FishEyeAmount.SetMinMax(0.0, 100.0);
	FishEyeLensRadius.SetMinMax(0.0, 100.0);
	FishEyeSignCurvature.SetMinMax(0.0, 100.0);

	//
	Contrast.SetMinMax(-100.0, 100.0, true, true);
	Saturation.SetMinMax(-100.0, 100.0, true, true);
	Brightness.SetMinMax(-100.0, 100.0, true, true);

	Gamma.SetMinMax(-200.0, 300.0, true, true);

	BloomMinBright.SetMinMax(0.0, 100.0);
	BloomTone.SetMinMax(0.0, 100.0);
	BloomStretch.SetMinMax(0.0, 100.0);

	Hue.SetMinMax(-100.0, 100.0, true, true);
	HueSaturation.SetMinMax(-100.0, 100.0, true, true);
	Lightness.SetMinMax(-100.0, 100.0, true, true);

	//
	VignAmount.SetMinMax(-100.0, 100.0);
	VignOut.SetMinMax(-100.0, 100.0);
	VignIn.SetMinMax(-100.0, 100.0);
	VignFade.SetMinMax(-100.0, 100.0);

	//
	FG_GrainSize.SetMinMax(15.0, 25.0);

	//
	FlareSeed.SetMinMax(0.0, 100.0);
	FlareAmount.SetMinMax(0.0, 100.0);
	FlarePosX.SetMinMax(0.0, 100.0);
	FlarePosY.SetMinMax(0.0, 100.0);

	FlareInner.SetMinMax(0.0, 100.0);
	FlareOuter.SetMinMax(0.0, 100.0);

	FlareLight.SetFilter(FBLight::GetInternalClassId());
	FlareLight.SetSingleConnect(false);

	//
	FocusObject.SetSingleConnect(true);
	FocusObject.SetFilter(FBModelNull::GetInternalClassId());

	Samples.SetMinMax(1, 12, true, true);
	Rings.SetMinMax(1, 32, true, true);
	PentagonFeather.SetMinMax(0.0, 1.0, true, true);

	// SSAO
	SSAO_Intensity.SetMinMax(0.0, 100.0);
	SSAO_Bias.SetMinMax(0.0, 100.0);
	SSAO_BlurSharpness.SetMinMax(0.0, 100.0);

	// MotionBlur
	MotionBlurAmount.SetMinMax(0.0, 100.0);

	// Masks
	for (int i = 0; i < NUMBER_OF_MASKS; ++i)
	{
		Masks[i].UseRimForMask.SetMinMax(0.0, 100.0);
		Masks[i].MaskRimPower.SetMinMax(0.0, 200.0);
	}

	// DONE: READ default values from config file !
	DefaultValues();

    return true;
}

/************************************************
 *  FiLMBOX Destructor.
 ************************************************/
void PostPersistentData::FBDestroy()
{
}

void PostPersistentData::SMaskProperties::SetDefaultValues()
{
	InvertMask = false;
	BlurMask = false;
	FBVector2d v(1.0, 1.0);
	BlurMaskScale->SetData(&v);
	UseRimForMask = 0.0;
	MaskRimPower = 100.0;
	UseMixWithMask = false;
	MixWithMask = EMaskingChannel::eMaskA;
}

void PostPersistentData::DefaultValues()
{
	Active = false;
	DrawHUDLayer = false;
	GenerateMipMaps = false;
	UseCameraObject = true;

	// global masking properties
	UseCompositeMasking = true;
	EnableMaskingForAllEffects = false;
	DebugDisplyMasking = false;

	// masks
	for (int i = 0; i < NUMBER_OF_MASKS; ++i)
	{
		Masks[i].SetDefaultValues();
	}
	
	AutoClipFromHUD = true;
	UpperClip = 0.0;
	LowerClip = 0.0;

	FishEye = false;
	FishEye_UseMasking = false;
	FishEyeAmount = 100.0;
	FishEyeLensRadius = 3.0;
	FishEyeSignCurvature = 3.8;
	FishEyeOrder = 0;
	
	ColorCorrection = false;
	ColorCorrection_UseMasking = false;
	ChromaticAberration = false;
	ChromaticAberrationDirection = FBVector2d(1.0, 1.0);
	Contrast = 0.0;
	Saturation = 0.0;
	Brightness = 0.0;
	Gamma = 100.0;
	Inverse = false;
	Bloom = false;
	BloomMinBright = 50.0;
	BloomTone = 100.0;
	BloomStretch = 100.0;
	Hue = 0.0;
	HueSaturation = 0.0;
	Lightness = 0.0;

	//
	Vignetting = false;
	Vign_UseMasking = false;
	VignAmount = 100.0;
	VignOut = 130.0; // 1.3
	VignIn = 0.0;
	VignFade = 22.0;

	//
	FilmGrain = false;
	FilmGrain_UseMasking = false;
	FG_UsePlayTime = false;
	FG_TimeSpeed = 100.0;

	FG_GrainAmount = 5.0;
	FG_Colored = false;
	FG_ColorAmount = 60.0;
	FG_GrainSize = 16.0;
	FG_LumAmount = 100.0;

	//
	LensFlare = false;
	LensFlare_UseMasking = false;
	LensFlare_UseOcclusion = false;
	FlareOcclusionSpeed = 10.0;
	FlareOcclusionObjects.SetFilter(FBModel::GetInternalClassId());
	FlareOcclusionObjects.SetSingleConnect(false);

	//Louis
	EFlareType defaultFlareType = flare1;
	FlareType.SetData(&defaultFlareType);
	FlareSeed = 30.0;

	FlareUsePlayTime = false;
	FlareTimeSpeed = 100.0;

	UseFlareLightObject = true;

	FlareAmount = 100.0;
	FlareDepthAttenuation = true;
	FlarePosX = 50.0;
	FlarePosY = 50.0;

	FlareTint = FBColor(1.0, 1.0, 1.0);
	FlareInner = 10.0;
	FlareOuter = 50.0;

	FlareFadeToBorders = true;
	FlareBorderWidth = 50.0;
	FlareBorderFeather = 100.0;

	//
	SSAO = false;
	SSAO_UseMasking = false;

	SSAO_Intensity = 150.0; // * 0.01
	SSAO_Radius = 2.0;
	SSAO_Bias = 10.0; // * 0.01

	OnlyAO = false;

	SSAO_Blur = false;
	SSAO_BlurSharpness = 40.0;

	//
	MotionBlur = false;
	MotionBlurAmount = 20.0;

	//
	mResetFarPlane = 99999.90;
	LoadFarValueFromConfig();

	DepthOfField = false;
	DOF_UseMasking = false;

	UseCameraDOFProperties = false;
	DebugBlurValue = false;

	FocalDistance = 40.0;
	FocalRange = 4.0;
	FStop = 0.5;

	BlurForeground = true;
	/*
	ManualFocus = false;
	ManualNear = 1.0;
	ManualNearFalloff = 2.0;
	ManualFar = 1.0;
	ManualFarFalloff = 3.0;
	*/
	AutoFocus = false;

	UseFocusPoint = false;
	FocusPoint = FBVector2d(50.0, 50.0);

	FocusObject.SetSingleConnect(true);
	FocusObject.SetFilter(FBModel::GetInternalClassId());

	FastPreview = true;
	EBlurQuality defaultBlurQuality = eBlurQualityPreview2;
	PreviewQuality.SetData(&defaultBlurQuality);
	PreviewBlurAmount = 100.0;

	Samples = 3;
	Rings = 12;

	CoC = 3.0;
	Threshold = 50.0;
	Gain = 200.0;
	Bias = 50.0;
	Fringe = 70.0;
	Noise = true;

	Pentagon = false;
	PentagonFeather = 40.0;

	//
	Displacement = false;
	Disp_UseMasking = false;
	UseQuakeWaterEffect = false;
	
	Disp_UsePlayTime = false;
	Disp_Speed = 300.0;
	Disp_MagnitudeX = 200.0;
	Disp_MagnitudeY = 200.0;

	Disp_SinCyclesX = 6.28;
	Disp_SinCyclesY = 6.28;

	LoadFromConfig();
}

void PostPersistentData::LoadFromConfig(const char *sessionFilter)
{
	FBConfigFile	lConfig("@PostProcessing.txt");

	char buffer[256];
	FBString sessionName("Common");
	bool insert = false;

	for (int i = 0, count = PropertyList.GetCount(); i < count; ++i)
	{
		FBProperty *pProp = PropertyList[i];

		if (pProp == &Active)
			insert = true;

		if (pProp == &FishEye)
			sessionName = "Fish Eye";
		else if (pProp == &ColorCorrection)
			sessionName = "Color Correction";
		else if (pProp == &Vignetting)
			sessionName = "Vignetting";
		else if (pProp == &FilmGrain)
			sessionName = "Film Grain";
		else if (pProp == &LensFlare)
			sessionName = "Lens Flare";
		else if (pProp == &SSAO)
			sessionName = "SSAO";
		else if (pProp == &DepthOfField)
			sessionName = "Depth Of Field";
		else if (pProp == &Displacement)
			sessionName = "Displacement";
		else if (pProp == &MotionBlur)
			sessionName = "Motion Blur";

		if (nullptr != sessionFilter && 0 != strcmp(sessionName, sessionFilter))
			continue;

		if (true == insert)
		{
			FBPropertyType type = pProp->GetPropertyType();
			const char *propName = pProp->GetName();

			switch (type)
			{
			case kFBPT_int:
			case kFBPT_bool:
			{
				int defValue = pProp->AsInt();
				int value = ConfigHelper::getOrSetInt(lConfig, buffer, sessionName, propName, "", defValue);
				pProp->SetInt(value);
			} break;

			case kFBPT_double:
			{
				double defValue;
				pProp->GetData(&defValue, sizeof(double));
				double value = ConfigHelper::getOrSetDouble(lConfig, buffer, sessionName, propName, "", defValue);
				pProp->SetData(&value);
			} break;
			case kFBPT_enum:
			{
				FBString defValue = pProp->AsString();
				ConfigHelper::getOrSetString(defValue, lConfig, sessionName, propName, "");
				pProp->SetString(defValue);
			} break;
			}
		}
	}

}

void PostPersistentData::LoadFarValueFromConfig()
{
	FBConfigFile	lConfig("@PostProcessing.txt");

	char buffer[256];
	FBString sessionName("Depth Of Field");
	//bool insert = false;
	
	double defValue = mResetFarPlane;
	double value = ConfigHelper::getOrSetDouble(lConfig, buffer, sessionName, "Far Plane Reset Value", "", defValue);
	mResetFarPlane = value;	
}

bool PostPersistentData::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
    if( pStoreWhat == kAttributes )
    {
        //Store default text
        //pFbxObject->FieldWriteC( "Text", mText );
    }

    return false;
}

bool PostPersistentData::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	constexpr int LAZY_COUNTER_VALUE{ 500 };

    if( pStoreWhat == kAttributes )
    {
        //Retrieve default text
        //mText = pFbxObject->FieldReadC("Text");
		mLazyLoadCounter = LAZY_COUNTER_VALUE;
    }

    return false;
}



void PostPersistentData::DoReloadShaders()
{
	mReloadShaders = true;
}

void PostPersistentData::DoDebugFarDist()
{
	FBSystem& mSystem = FBSystem::TheOne();

#if(PRODUCT_VERSION >= 2024)
	const unsigned selectedPaneIndex = mSystem.Renderer->GetSelectedPaneIndex();
	FBCamera* pCamera = mSystem.Renderer->GetCameraInPane(selectedPaneIndex);
#else
	FBCamera* pCamera = mSystem.Renderer->CurrentCamera;
#endif
	if (nullptr == pCamera)
		return;
	if (FBIS(pCamera, FBCameraSwitcher))
		pCamera = ((FBCameraSwitcher*)pCamera)->CurrentCamera;

	double dist = pCamera->FarPlaneDistance;
	double maxfloat = mResetFarPlane; // std::numeric_limits<float>::max();

	if (dist > maxfloat || dist > FLT_MAX)
		FBMessageBox("Post Processing", "Warning: Current Camera Far Distance Is Too Big For The Effect\n", "Ok");
	else
		FBMessageBox("Post Processing", "Current Camera Far Distance Is Fine For The Effect\n", "Ok");
}

void PostPersistentData::DoFixCameraSettings()
{
	FBSystem& mSystem = FBSystem::TheOne();
	FBRenderer *pRenderer = mSystem.Renderer;

	int lOption = FBMessageBox("Post Processing", "Do you want to fix settings for a current pane camera or for DOF connected?", "Current", "All Conn", "Cancel");

	const double farPlaneValue = mResetFarPlane;
	auto fn_fixsettings = [&farPlaneValue](FBCamera *pCamera) {
		
		pCamera->FarPlaneDistance = farPlaneValue;
		pCamera->UseRealTimeMotionBlur = false;
		pCamera->UseAccumulationBuffer = false;
		pCamera->UseAntiAliasing = false;
		pCamera->UseDepthOfField = false;
		pCamera->InteractiveMode = false;
	};

	if (1 == lOption)
	{
		int currPane = pRenderer->GetSelectedPaneIndex();

		if (false == pRenderer->IsCameraSwitcherInPane(currPane) && currPane != pRenderer->GetSchematicViewPaneIndex())
		{
			FBCamera *pCamera = pRenderer->GetCameraInPane(currPane);

			if (false == pCamera->SystemCamera)
			{
				fn_fixsettings(pCamera);
				FBMessageBox("Post Processing", FBString("Updated a far plane value and turned off real-time effects for ", pCamera->LongName), "Ok");
			}
			else
			{
				FBMessageBox("Post Processing", FBString("Skipping a system camera ", pCamera->LongName), "Ok");
			}
		}
	}
	else if (2 == lOption)
	{
		int fixCount = 0;

		if (Camera.GetCount() > 0)
		{
			// for only specified

			for (int i = 0, count = Camera.GetCount(); i < count; ++i)
			{
				FBCamera *pCamera = (FBCamera*)Camera.GetAt(i);
				if (false == pCamera->SystemCamera)
				{
					fn_fixsettings(pCamera);
					fixCount += 1;
				}
			}
		}
		else
		{
			FBScene *pScene = mSystem.Scene;
			// for every non-system camera in the scene
			for (int i = 0, count = pScene->Cameras.GetCount(); i < count; ++i)
			{
				FBCamera *pCamera = pScene->Cameras[i];
				if (false == pCamera->SystemCamera)
				{
					fn_fixsettings(pCamera);
					fixCount += 1;
				}
			}
		}

		char buf[64];
		sprintf_s(buf, sizeof(char)* 64, "Updated %d camera(s)", fixCount);

		FBMessageBox("Post Processing", buf, "Ok");
	}
}

bool PostPersistentData::IsNeedToReloadShaders()
{
	return mReloadShaders;
}

void PostPersistentData::SetReloadShadersState(bool state)
{
	mReloadShaders = state;
}

bool PostPersistentData::PlugNotify(FBConnectionAction pAction, FBPlug* pThis, int pIndex, FBPlug* pPlug, FBConnectionType pConnectionType, FBPlug* pNewPlug)
{
	if (pThis == &Camera)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
		}
	}
	else if (pThis == &UserEffects)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
		}
	}
	else if (pThis == &FlareLight)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
		}
	}
	else if (pThis == &FocusObject)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
		}
	}

	return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
}

void PostPersistentData::CopyValues(PostPersistentData *pOther)
{
	FBVector4d v;
	int ivalue;
	double dvalue;
	
	bool startcopy = false;

	for (int i = 0, count = PropertyList.GetCount(); i < count; ++i)
	{
		FBProperty *pDstProp = PropertyList[i];
		FBProperty *pSrcProp = pOther->PropertyList[i];

		if (&Active == pDstProp)
			startcopy = true;

		if (true == startcopy)
		{
			FBPropertyType	propType = pDstProp->GetPropertyType();

			switch (propType)
			{
			case kFBPT_int:
			case kFBPT_bool:
				ivalue = pSrcProp->AsInt();
				pDstProp->SetInt(ivalue);
				break;
			case kFBPT_double:
				pSrcProp->GetData(&dvalue, sizeof(double));
				pDstProp->SetData(&dvalue);
				break;
			case kFBPT_ColorRGB:
			case kFBPT_Vector3D:
				pSrcProp->GetData(&v, sizeof(FBVector3d));
				pDstProp->SetData(&v);
				break;
			case kFBPT_Vector2D:
				pSrcProp->GetData(&v, sizeof(FBVector2d));
				pDstProp->SetData(&v);
				break;
			case kFBPT_ColorRGBA:
			case kFBPT_Vector4D:
				pSrcProp->GetData(&v, sizeof(FBVector4d));
				pDstProp->SetData(&v);
				break;
			}
		}
	}

	Camera.Clear();
	for (int i = 0, count = pOther->Camera.GetCount(); i < count; ++i)
		Camera.Add(pOther->Camera.GetAt(i));

	FlareLight.Clear();
	for (int i = 0, count = pOther->FlareLight.GetCount(); i < count; ++i)
		FlareLight.Add(pOther->FlareLight.GetAt(i));

	FocusObject.Clear();
	for (int i = 0, count = pOther->FocusObject.GetCount(); i < count; ++i)
		FocusObject.Add(pOther->FocusObject.GetAt(i));
}

void PostPersistentData::ConnectCamera(FBCamera *pCamera)
{
	bool isExist = false;

	for (int i = 0, count = Camera.GetCount(); i < count; ++i)
	{
		if (pCamera == Camera.GetAt(i))
		{
			isExist = true;
			break;
		}
	}

	if (false == isExist)
	{
		FBConnect(pCamera, &Camera);
	}
}

void PostPersistentData::ConnectLight(FBLight *pLight)
{
	bool isExist = false;

	for (int i = 0, count = FlareLight.GetCount(); i < count; ++i)
	{
		if (pLight == FlareLight.GetAt(i))
		{
			isExist = true;
			break;
		}
	}

	if (false == isExist)
	{
		FBConnect(pLight, &FlareLight);
	}
}

void PostPersistentData::ConnectFocus(FBModelNull *pNull)
{
	bool isExist = false;

	for (int i = 0, count = FocusObject.GetCount(); i < count; ++i)
	{
		if (pNull == FocusObject.GetAt(i))
		{
			isExist = true;
			break;
		}
	}

	if (false == isExist)
	{
		FBConnect(pNull, &FocusObject);
	}
}

void PostPersistentData::DoFocusObjectCreate()
{
	mPostAction = ePostActionFocusCreate;
	FBSystem& mSystem = FBSystem::TheOne();
	mSystem.OnUIIdle.Add(this, (FBCallback)&PostPersistentData::OnUIIdle);
}

void PostPersistentData::DoFocusObjectSelect()
{
	mPostAction = ePostActionFocusSelect;
	FBSystem& mSystem = FBSystem::TheOne();
	mSystem.OnUIIdle.Add(this, (FBCallback)&PostPersistentData::OnUIIdle);
}

void PostPersistentData::DoFlareLightCreate()
{
	mPostAction = ePostActionFlareCreate;
	FBSystem& mSystem = FBSystem::TheOne();
	mSystem.OnUIIdle.Add(this, (FBCallback)&PostPersistentData::OnUIIdle);
}

void PostPersistentData::DoFlareLightSelect()
{
	mPostAction = ePostActionFlareSelect;
	FBSystem& mSystem = FBSystem::TheOne();
	mSystem.OnUIIdle.Add(this, (FBCallback)&PostPersistentData::OnUIIdle);
}

void PostPersistentData::OnUIIdle(HISender pSender, HKEvent pEvent)
{
	FBSystem& mSystem = FBSystem::TheOne();
	mSystem.OnUIIdle.Remove(this, (FBCallback)&PostPersistentData::OnUIIdle);

	switch (mPostAction)
	{
	case ePostActionFocusCreate:
	{
		bool createANull = true;

		if (FocusObject.GetCount() > 0)
		{
			int userChooise = FBMessageBox("Post Processing", "Focus Object is already assigned.\n What do you want to do with existing?", "Delete", "Disconnect", "Cancel");
			if (1 == userChooise)
			{
				FBModel *pModel = (FBModel*)FocusObject.GetAt(0);
				FocusObject.RemoveAll();
				pModel->FBDelete();
				pModel = nullptr;
			}
			else if (2 == userChooise)
			{
				FocusObject.RemoveAll();
			}
			else
			{
				createANull = false;
			}
		}

		if (true == createANull)
		{
			FBModelNull *pNewNull = new FBModelNull("Depth Of Field Focus");
			pNewNull->Show = true;
			FocusObject.Add(pNewNull);

			FBVector3d v;
			ComputePointInFront(v);
			pNewNull->SetVector(v);
		}
	}
		break;
	case ePostActionFocusSelect:
		if (FocusObject.GetCount() > 0)
		{
			FocusObject.GetAt(0)->Selected = true;
		}
		break;
	case ePostActionFlareCreate:
	{
		FBLight *pNewLight = new FBLight("Lens Flare Light");
		pNewLight->Show = true;
		FlareLight.Add(pNewLight);

		FBVector3d v;
		ComputePointInFront(v);
		pNewLight->SetVector(v);
	}
		break;
	case ePostActionFlareSelect:
		for (int i=0; i<FlareLight.GetCount(); ++i)
		{
			FlareLight.GetAt(i)->Selected = true;
		}
		break;
	}
}

void PostPersistentData::ComputePointInFront(FBVector3d &v)
{
	FBSystem& mSystem = FBSystem::TheOne();
#if(PRODUCT_VERSION >= 2024)
	const unsigned selectedPaneIndex = mSystem.Renderer->GetSelectedPaneIndex();
	FBCamera* pCamera = mSystem.Renderer->GetCameraInPane(selectedPaneIndex);
#else
	FBCamera* pCamera = mSystem.Renderer->CurrentCamera;
#endif
	if (nullptr == pCamera)
		return;
	if (FBIS(pCamera, FBCameraSwitcher))
		pCamera = FBCast<FBCameraSwitcher>(pCamera)->CurrentCamera;

	if (nullptr == pCamera)
		return;

	FBMatrix tm;
	((FBModel*)pCamera)->GetMatrix(tm);

	FBVector4d v4;
	FBVectorMatrixMult(v4, tm, FBVector4d(50.0, 0.0, 0.0, 1.0));
	
	for (int i = 0; i < 3; ++i)
		v[i] = v4[i];
}

void PostPersistentData::DoResetDOF()
{
	// DONE:
	UseCameraDOFProperties = false;
	DebugBlurValue = false;

	FocalDistance = 40.0;
	FocalRange = 4.0;
	FStop = 0.5;

	BlurForeground = true;
	/*
	ManualFocus = false;
	ManualNear = 1.0;
	ManualNearFalloff = 2.0;
	ManualFar = 1.0;
	ManualFarFalloff = 3.0;
	*/
	AutoFocus = false;

	UseFocusPoint = false;
	FocusPoint = FBVector2d(50.0, 50.0);

	FastPreview = true;
	EBlurQuality defaultBlurQuality = eBlurQualityPreview2;
	PreviewQuality.SetData(&defaultBlurQuality);
	PreviewBlurAmount = 100.0;

	Samples = 3;
	Rings = 12;

	CoC = 3.0;
	Threshold = 50.0;
	Gain = 200.0;
	Bias = 50.0;
	Fringe = 70.0;
	Noise = true;

	Pentagon = false;
	PentagonFeather = 40.0;

	LoadFromConfig("Depth Of Field");
}


void PostPersistentData::SetPreviewTextureId(unsigned int id, double ratio,
	unsigned int w, unsigned int h, int uncomporessSize, int compressedSize, int compressionCode, double updatetime)
{

	FBVideoMemory *pVideo = (OutputVideo.GetCount() > 0) ? (FBVideoMemory*)OutputVideo.GetAt(0) : nullptr;
	if (nullptr == pVideo)
	{
		FBString name(this->Name, "_Preview");
		pVideo = new FBVideoMemory(name);
		OutputVideo.Add(pVideo);

		FBFolder *pFolderVideos = nullptr;

		FBScene *pScene = FBSystem::TheOne().Scene;
		const int numberOfFolders = pScene->Folders.GetCount();
		for (int i = 0; i<numberOfFolders; ++i)
		{
			if (0 == strcmp(pScene->Folders[i]->Name, "Dynamic Videos"))
				pFolderVideos = pScene->Folders[i];

			if (pFolderVideos)
				break;
		}

		//
		if (nullptr == pFolderVideos)
			pFolderVideos = new FBFolder("Dynamic Videos", pVideo);
		else
			pFolderVideos->Items.Add(pVideo);
	}
	
	//
	pVideo->TextureOGLId = id;
	pVideo->SetObjectImageSize(w, h);

	OutputRatio = ratio;
	OutputWidth = w;
	OutputHeight = h;
	OutputUncompressSize = uncomporessSize;
	OutputCompressedSize = compressedSize;
	OutputCompressionCode = compressionCode;
	OutputCompressedTime = updatetime;
}

void PostPersistentData::PushClipSettings(double upper, double lower)
{
	mTempLower = LowerClip;
	mTempUpper = UpperClip;

	if (lower >= 0) 
		UpperClip = lower;
	
	if (upper >= 0) 
		LowerClip = upper;
}

void PostPersistentData::PopClipSettings()
{
	UpperClip = mTempUpper;
	LowerClip = mTempLower;
}

bool PostPersistentData::HasAnyActiveMasking() const
{
	if (!UseCompositeMasking)
		return false;

	return (EnableMaskingForAllEffects || FishEye_UseMasking || ColorCorrection_UseMasking
		|| Vign_UseMasking || FilmGrain_UseMasking || LensFlare_UseMasking
		|| SSAO_UseMasking || DOF_UseMasking || Disp_UseMasking);
}

bool PostPersistentData::IsMaskActive(const EMaskingChannel maskId) const
{
	if (!UseCompositeMasking)
		return false;

	const bool effectMaskFlags[] = {
		EnableMaskingForAllEffects && GlobalMaskingChannel == maskId,
		FishEye_UseMasking && FishEye_MaskingChannel == maskId,
		ColorCorrection_UseMasking && ColorCorrection_MaskingChannel == maskId,
		Vign_UseMasking && Vign_MaskingChannel == maskId,
		FilmGrain_UseMasking && FilmGrain_MaskingChannel == maskId,
		LensFlare_UseMasking && LensFlare_MaskingChannel == maskId,
		SSAO_UseMasking && SSAO_MaskingChannel == maskId,
		DOF_UseMasking && DOF_MaskingChannel == maskId,
		Disp_UseMasking && Disp_MaskingChannel == maskId
	};

	for (const bool flag : effectMaskFlags)
	{
		if (flag) return true;
	}
	return false;
}

int PostPersistentData::GetGlobalMaskIndex() const
{
	const EMaskingChannel maskingChannel = GlobalMaskingChannel;
	return static_cast<int>(maskingChannel);
}

int PostPersistentData::GetNumberOfActiveUserEffects()
{
	int count = 0;
	for (int i = 0; i < UserEffects.GetCount(); ++i)
	{
		if (FBIS(UserEffects[i], PostEffectUserObject))
		{
			PostEffectUserObject* UserObject = FBCast<PostEffectUserObject>(UserEffects[i]);
			if (UserObject && UserObject->Active && UserObject->GetUserEffectPtr())
			{
				count += 1;
			}
		}
	}
	return count;
}

PostEffectBase* PostPersistentData::GetActiveUserEffect(const int index)
{
	int count = 0;
	for (int i = 0; i < UserEffects.GetCount(); ++i)
	{
		if (FBIS(UserEffects[i], PostEffectUserObject))
		{
			PostEffectUserObject* UserObject = FBCast<PostEffectUserObject>(UserEffects[i]);
			if (UserObject && UserObject->Active)
			{
				if (count == index)
				{
					return UserObject->GetUserEffectPtr();
				}
				count += 1;
			}
		}
	}
	return nullptr;
}

PostEffectUserObject* PostPersistentData::GetActiveUserEffectObject(const int index)
{
	int count = 0;
	for (int i = 0; i < UserEffects.GetCount(); ++i)
	{
		if (FBIS(UserEffects[i], PostEffectUserObject))
		{
			PostEffectUserObject* UserObject = FBCast<PostEffectUserObject>(UserEffects[i]);
			if (UserObject && UserObject->Active)
			{
				if (count == index)
				{
					return UserObject;
				}
				count += 1;
			}
		}
	}
	return nullptr;
}