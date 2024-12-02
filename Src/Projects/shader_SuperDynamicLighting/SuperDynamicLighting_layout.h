#ifndef __DYNAMICLIGHTING_LAYOUT_H__
#define __DYNAMICLIGHTING_LAYOUT_H__


/**	\file	DynamicLighting_layout.h

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Class declaration
class SuperDynamicLighting;

//! Simple device layout.
class SuperDynamicLightingLayout : public FBShaderLayout
{
    // Declaration.
    FBShaderLayoutDeclare( SuperDynamicLightingLayout, FBShaderLayout );

public:

    // Open Reality constructor and destructor. Do not use C++ constructor/destructor.
    virtual bool FBCreate();
    virtual void FBDestroy();

    // Callback for transparency type changes in UI.
    void EventTransparencyChange(HISender pSender, HKEvent pEvent);

private:

    // UI Management
    void	UICreate	();
    void	UIConfigure	();
    void	UIReset		();

private:

    // The shader.
    SuperDynamicLighting* mShader;

    // UI variables.
    FBLabel mTransLabel;
    FBLabel mAffectLabel;
    FBLabel mCastersLabel;
	FBLabel mMatCapLabel;
    FBLabel mTransFactorLabel;
    FBList  mTransparency;
    FBEditProperty				mTransparencyFactor;
	FBEditProperty				mUseSceneLights;
	FBPropertyConnectionEditor	mAffectingLights;
    FBEditProperty              mCastShadows;
    FBPropertyConnectionEditor  mShadowCasters; // models that cast shadows
	FBEditProperty				mUseRim;
	FBEditProperty				mRimPower;
	FBEditProperty				mRimColor;
	FBEditProperty				mUseMatCap;
	FBPropertyConnectionEditor	mMatCapTexture;

};

#endif /* __DYNAMICLIGHTING_LAYOUT_H__ */
