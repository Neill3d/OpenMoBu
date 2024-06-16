#ifndef __MODEL_COLLISION_SPHERE_H__
#define __MODEL_COLLISION_SPHERE_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: model_collision_sphere.h
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
#define ORMARKERCUSTOM__CLASSNAME	CollisionSphere
#define ORMARKERCUSTOM__CLASSSTR	"CollisionSphere"
#define ORMARKERCUSTOM__DESCSTR	    "Collision Sphere"

/**	Custom marker template.
*/
class CollisionSphere : public FBModelMarker
{
	//--- FiLMBOX class declaration.
	FBStorableClassDeclare( CollisionSphere, FBModelMarker );

public:
	CollisionSphere(const char *pName, HIObject pObject= NULL);

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

	FBPropertyAnimatableDouble		Friction;
	FBPropertyAnimatableDouble		Softness;

	FBVector3d						LastTranslation;

public:

	void	FillCollisionData( GPUParticles::TCollision	&coldata );

protected:
    int mPickedSubItem;
};


#endif /* __MODEL_COLLISION_SPHERE_H__ */
