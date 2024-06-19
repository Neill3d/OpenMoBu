#ifndef __MODEL_FORCE_WIND_H__
#define __MODEL_FORCE_WIND_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: model_force_wind.h
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "GL\glew.h"

//--- SDK include
#include <fbsdk/fbsdk.h>
#include "ParticleSystem_types.h"

//--- Registration define
#define FORCEWIND__CLASSNAME	ForceWind
#define FORCEWIND__CLASSSTR		"ForceWind"
#define FORCEWIND__DESCSTR	    "Force Wind"

/**	Force Wind for a GPU Particles.
*/
class ForceWind : public FBModelMarker
{
	//--- FiLMBOX class declaration.
	FBStorableClassDeclare( ForceWind, FBModelMarker );

public:
	ForceWind(const char *pName, HIObject pObject= NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate() override;		//!< FiLMBOX Creation function.
	virtual void FBDestroy() override;		//!< FiLMBOX Destruction function.

	virtual bool HasCustomDisplay() override { return true; }

	/** Custom display function called when HasCustomDisplay returns true
	*/
	virtual void CustomModelDisplay(FBCamera* pCamera, FBModelShadingMode pShadingMode, 
									FBModelRenderPass pRenderPass, float pPickingAreaWidth, 
									float pPickingAreaHeight) override;

	
	virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;

    /** Override to reuse regular marker object type's property viewSet.
    */
    virtual bool UseCustomPropertyViewSet() const override { return true; }

	FBPropertyBool					Enabled;

	FBPropertyAnimatableDouble		Magnitude;

	FBPropertyBool					UseNoise;
	FBPropertyAnimatableDouble		NoiseFrequency;
	FBPropertyAnimatableDouble		NoiseSpeed;

	FBPropertyBool					LocalInfluence;
	FBPropertyAnimatableDouble		InfluenceRadius;

	FBPropertyBool					Turbulence;
	FBPropertyAnimatableDouble		Amplitude;
	FBPropertyAnimatableDouble		Frequency;

	FBPropertyAnimatableDouble		ConeAngle;
	FBPropertyBool					ShowDebug;

	static void AddPropertiesToPropertyViewManager();

	void SetWindVectors( const FBVector4d wind1, const FBVector4d wind2, const FBVector4d wind3, const FBVector4d wind4 );

	void FillForceData( GPUParticles::TForce &forcedata, const float elapsedTime );

protected:
	int					mPickedSubItem{ -1 };

	FBVector4d			mWind1;
	FBVector4d			mWind2;
	FBVector4d			mWind3;
	FBVector4d			mWind4;
};


#endif /* __MODEL_FORCE_WIND_H__ */
