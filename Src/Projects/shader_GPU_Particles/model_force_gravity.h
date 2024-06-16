#ifndef __MODEL_FORCE_DRAG_H__
#define __MODEL_FORCE_DRAG_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: model_force_drag.h
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
#define FORCEGRAVITY__CLASSNAME		ForceGravity
#define FORCEGRAVITY__CLASSSTR		"ForceGravity"
#define FORCEGRAVITY__DESCSTR	    "Force Gravity"

/**	Force Gravity for a GPU Particles.
*/
class ForceGravity : public FBModelMarker
{
	//--- FiLMBOX class declaration.
	FBStorableClassDeclare( ForceGravity, FBModelMarker );

public:
	ForceGravity(const char *pName, HIObject pObject= NULL);

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

	FBPropertyBool					UserDirection;
	FBPropertyAnimatableVector3d	Direction;

	FBPropertyBool					LocalInfluence;
	FBPropertyAnimatableDouble		InfluenceRadius;

	FBPropertyBool					Turbulence;
	FBPropertyAnimatableDouble		Amplitude;
	FBPropertyAnimatableDouble		Frequency;

	FBVector3d						LastTranslation;

	static void AddPropertiesToPropertyViewManager();

	void FillForceData( GPUParticles::TForce &forcedata );

protected:
    int mPickedSubItem;
};


#endif /* __MODEL_FORCE_WIND_H__ */
