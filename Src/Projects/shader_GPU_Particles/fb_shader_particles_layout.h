#ifndef __ORSHADER_TEMPLATE_LAYOUT_H__
#define __ORSHADER_TEMPLATE_LAYOUT_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: fb_shader_particles_layout.h
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "fb_shader_particles_shader.h"

enum EGenerationProperties
{
	ePropertyEditEmitterType,
	
	ePropertySep1,

	ePropertyEditPlayMode,
	ePropertyEditUseRange,
	ePropertyEditStartFrame,
	ePropertyEditStopFrame,

	ePropertySep2,

	ePropertyEditResetTime,
	ePropertyEditResetCount,

	ePropertyEditUseRate,
	ePropertyEditRate,
	ePropertyEditUsePreGenerated,

	ePropertyNextCollumn,

	ePropertyEditDirection,
	ePropertyEditDirSpreadHor,
	ePropertyEditDirSpreadVer,
	ePropertyEditUseNormalsAsDir,

	ePropertyEditSpeed,
	ePropertyEditSpeedSpread,
	ePropertyEditInheritEmitterVel,

	ePropertyEditLifeTime,
	ePropertyEditLifeTimeVariation,

	ePropertyCount
};


enum EDynamicProperties
{
	ePropDynamicMass,
	ePropDynamicDamping,

	ePropDynamicUseGravity,
	ePropDynamicGravity,

	ePropDynamicUseFloor,
	ePropDynamicFloorFriction,
	ePropDynamicFloorLevel,

	ePropDynamicNextColumn1,

	ePropDynamicUseForces,
	ePropDynamicForces,

	ePropDynamicUseCollisions,
	ePropDynamicCollisions,

	ePropDynamicSelfCollisions,

	ePropDynamicNextColumn2,

	ePropDynamicUseTurbulence,
	ePropDynamicNoiseFrequency,
	ePropDynamicNoiseSpeed,
	ePropDynamicNoiseAmplitude,

	ePropDynamicCount
};

enum EShadingProperties
{
	ePropertyInheritEmitterColor,

	ePropShadingPointSmooth,
	ePropShadingPointFalloff,
	ePropShadingPrimitiveType,
	ePropShadingInstanceObject,
	ePropShadingTextureObject,
	
	ePropShadingShadeMode,
	ePropShadingAffectingLights,
	ePropShadingTransparency,
	ePropShadingTransparencyFactor,

	ePropShadingColor,
	ePropShadingColorVariation,
	ePropShadingUseColorCurve,
	ePropShadingColorCurve,
	
	ePropShadingSize,
	ePropShadingSizeVariation,

	ePropShadingMinPointScale,
	ePropShadingMaxPointScale,
	ePropShadingPointScaleDistance,

	ePropShadingUseSizeCurve,
	ePropShadingSuzeCurve,

	ePropShadingCount

};

enum ECommonProperties
{
	ePropCommonMaxParticles,
	ePropCommonIterations,
	ePropCommonDeltaTimeLimit,
	ePropCommonAdaptiveSubSteps,
	ePropCommonSubSteps,

	ePropCommonCount
};

/////////////////////////////////////////////////////////////////////////////////
//! Particles Shader layout.

class GPUshader_ParticlesLayout : public FBShaderLayout
{
	//--- FiLMBOX declaration.
	FBShaderLayoutDeclare( GPUshader_ParticlesLayout, FBShaderLayout );

public:
	//--- FiLMBOX Creation/Destruction.
	virtual bool FBCreate();			//!< FiLMBOX constructor.
	virtual void FBDestroy();			//!< FiLMBOX destructor.

private:
	// UI Management
	void	UICreate	();
	void		UICreateProps(FBLayout &layout, const int count, const char *names[], FBEditProperty *props);
	void		UIResizeProps(FBLayout &layout, const int count, const char *names[], FBEditProperty *props);

	void	UIConfigure	();
	void		UIConfigureProps(FBLayout &layout, const int count, const char *names[], FBEditProperty *props);
	
	void	UIReset		();
	void		UIResetGeneration(FBLayout &layout);
	void		UIResetDynamic(FBLayout &layout);
	void		UIResetShading(FBLayout &layout);

	// UI Callbacks
	void	OnButtonTestClick(HISender pSender, HKEvent pEvent);

	void	OnResizeCallback(HISender pSender, HKEvent pEvent);

	void	OnTabChange(HISender pSender, HKEvent pEvent);

protected:
	
	int					mLayoutWidth;
	int					mLayoutHeight;

	FBEditProperty		mButtonActive;
	FBEditProperty		mButtonReset;
	FBEditProperty		mButtonResetAll;
	FBEditProperty		mButtonReload;

	FBTabPanel			mTabs;
	
	//
	// generation

	FBLayout			mLayoutGeneration;
	FBEditProperty		mEditGenerationProps[ePropertyCount];

	//
	// dynamic 

	FBLayout			mLayoutDynamic;
	FBEditProperty		mEditDynamicProps[ePropDynamicCount];
	
	//
	// shading

	FBLayout			mLayoutShading;
	FBEditProperty		mEditShadingProps[ePropShadingCount];

	//
	// common

	FBLayout			mLayoutCommon;
	FBEditProperty		mEditCommonProps[ePropCommonCount];
};

#endif /* __ORSHADER_TEMPLATE_LAYOUT_H__ */
