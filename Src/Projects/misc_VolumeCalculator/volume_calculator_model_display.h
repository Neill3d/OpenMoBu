#ifndef __VOLUME_CALCULATOR_MODEL_DISPLAY_H__
#define __VOLUME_CALCULATOR_MODEL_DISPLAY_H__

/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: neill3d@gmail.com
// 
/////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>
#include <vector>

enum kCECameraPresets
{
	kCECameraPreset_None,
	kCECameraPreset_V100R2, // 640 x 480
	kCECameraPreset_S250E,  // ration 1, 832 x 832
	kCECameraPreset_Flex13, // 1280 x 1024
	kCECameraPreset_Prime41, // 2048 x 2048
};

enum kCELensPresets
{
	kCELensPreset_None,
	kCELensPreset_38, 
	kCELensPreset_42,
	kCELensPreset_43,
	kCELensPreset_46,
	kCELensPreset_51,
	kCELensPreset_56,
	kCELensPreset_58,
};

const char * FBPropertyBaseEnum<kCECameraPresets>::mStrings[] = {"Choose Resolution Preset...", "Optitrack Flex V100", "Optitrack S250E", "Optitrack Flex 13", "Optirack Prime41", 0};
const char * FBPropertyBaseEnum<kCELensPresets>::mStrings[] = {"Choose Lens Preset...", "38", "42", "43", "46", "51", "56", "58", 0};

//--- Registration define
#define ORMODELCUSTOMDISPLAY__CLASSNAME		ModelVolumeCalculator
#define ORMODELCUSTOMDISPLAY__CLASSSTR		"ModelVolumeCalculator"
#define ORMODELCUSTOMDISPLAY__DESCSTR		"Model for calculating optical active zone"

/**	Custom Model template.
*/
class ModelVolumeCalculator : public FBModel
{
	//--- FiLMBOX Tool declaration.
	FBStorableClassDeclare( ModelVolumeCalculator, FBModel );

public:
	ModelVolumeCalculator(const char *pName = NULL, HIObject pObject=NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

	virtual bool HasCustomDisplay() override { return true; }

	/** Custom display function called when HasCustomDisplay returns true;
	*/
	virtual void CustomModelDisplay( FBCamera* pCamera, FBModelShadingMode pShadingMode, FBModelRenderPass pRenderPass, float pPickingAreaWidth, float pPickingAreaHeight) override;

	/** Custom picking for selection
	*/
	virtual bool CustomModelPicking(	int pNbHits, unsigned int *pSelectBuffer, FBCamera* pCamera, 
										int pMouseX,int pMouseY,
										FBTVector* localRaySrc, FBTVector* localRayDir,
										FBTVector* pWorldRaySrc, FBTVector* pWorldRayDir,
										FBMatrix* pGlobalInverseMatrix,
										FBTVector* pOutPickedPoint) override;

	virtual bool FbxStore(FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat);
	virtual bool FbxRetrieve(FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat);

	/** Override to reuse regular object type's property viewSet.
	*/
	virtual bool UseCustomPropertyViewSet() const override { return true; }
	static void AddPropertyView(const char* pPropertyName, const char* pHierarchy, bool pIsFolder = false);
	static void AddPropertiesToPropertyViewManager();

public:
	// some user properties
	FBPropertyDouble	Length;				// room length
	FBPropertyDouble	Width;				// put this values in meters
	FBPropertyDouble	Height;				// put this values in meters
	

	FBPropertyBool		DisplayRoom;
	FBPropertyBool		DisplayZone;			// Vicon style ab zones

	FBPropertyBool		IsZoneCircle;			// is active zone circle or rectangle

	FBPropertyDouble	ZoneX;			// active zone offset from the center
	FBPropertyDouble	ZoneZ;			// with this width

	FBPropertyDouble	ZoneLength;			// we planned active zone like this
	FBPropertyDouble	ZoneWidth;			// with this width

	FBPropertyDouble	SolveStep;			// step to test next point in volume
	FBPropertyInt		CameraCoverage;		// number of cameras to make point active

	FBPropertyListObject	Cameras;		// list of cameras to solve

	FBPropertyListObject	Marker;		// marker to test visibility from cameras

	FBPropertyAction	Solve;				// compute active volume from given cameras
	FBPropertyBool		DisplayPoints;		// display active points in volume
	FBPropertyInt		Marks;				// number of result points (with occolusion)

	FBPropertyAction	SwitchToCamera;		// make selected camera current, and show it's frustum planes
	FBPropertyAction	About;				// show information about me )

	FBPropertyBaseEnum<kCECameraPresets> Presets;		//!< setup camera from the preset
	FBPropertyBaseEnum<kCELensPresets> LensPresets;		//!< setup camera from the lens preset

	FBPropertyColor		RoomColor;
	FBPropertyColor		ZoneColor;
	FBPropertyColor		PointColor;
	FBPropertyColor		RayColor;

public:

	void	DoVolumeSolve();

private:
	void	PrepareVolumePoints(double l, double w, double h, double step);
	void	DrawVolumePoints(const bool is_pick);
	void	DrawMarkerRays(const bool is_pick);

	void	DrawActiveZone(const bool is_pick);

	void DrawRoom(double l, double w, double h, const bool is_pick);

	std::vector<FBVector3d>		mPoints;	// volume points array
};


#endif /* __VOLUME_CALCULATOR_MODEL_DISPLAY_H__ */
