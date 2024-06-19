#ifndef __MODEL_FORCE_MOTOR_H__
#define __MODEL_FORCE_MOTOR_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: model_force_motor.h
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
#define FORCEMOTOR__CLASSNAME	ForceMotor
#define FORCEMOTOR__CLASSSTR	"ForceMotor"
#define FORCEMOTOR__DESCSTR	    "Force Motor"

/**	Force Motor for a GPU Particles.
*/
class ForceMotor : public FBModelMarker
{
	//--- FiLMBOX class declaration.
	FBStorableClassDeclare( ForceMotor, FBModelMarker );

public:
	ForceMotor(const char *pName, HIObject pObject= NULL);

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
    virtual bool UseCustomPropertyViewSet() const override { return false; }


	FBPropertyBool					Enabled;

	FBPropertyBool					NewVortexMath;

	FBPropertyAnimatableDouble		Magnitude;
	FBPropertyAnimatableDouble		RotationSpeed;

	FBPropertyBool					LocalInfluence;
	FBPropertyAnimatableDouble		InfluenceRadius;

	FBPropertyBool					Turbulence;
	FBPropertyAnimatableDouble		Amplitude;
	FBPropertyAnimatableDouble		Frequency;

	void FillForceData( GPUParticles::TForce &forcedata );

protected:
	int mPickedSubItem{ -1 };
};


#endif /* __MODEL_FORCE_MOTOR_H__ */
