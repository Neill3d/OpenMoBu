#ifndef __MODEL_COLLISION_TERRAIN_H__
#define __MODEL_COLLISION_TERRAIN_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: model_collision_terrain.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "GL\glew.h"

//--- SDK include
#include <fbsdk/fbsdk.h>
#include "glslShaderProgram.h"
#include "Framebuffer.h"
#include "ParticleSystem_types.h"
#include "Shader_ParticleSystem.h"

//--- Registration define
#define COLLISIONTERRAIN__CLASSNAME	CollisionTerrain
#define COLLISIONTERRAIN__CLASSSTR	"CollisionTerrain"
#define COLLISIONTERRAIN__DESCSTR	"Collision Terrain"

enum FBTerrainTextureResolution
{
	kFBTerrainTexture128,
	kFBTerrainTexture256,
	kFBTerrainTexture512,
	kFBTerrainTexture1024,
	kFBTerrainTexture2048,
	kFBTerrainTexture4096
};

const char * FBPropertyBaseEnum<FBTerrainTextureResolution>::mStrings[] = {
	"128",
	"256",
	"512",
	"1024",
	"2048",
	"4096",
	0
};


////////////////////////////////////////////////////////////////////////////////////////////
/**	CollisionTerrain.
*/
class CollisionTerrain : public FBModelMarker
{
	//--- FiLMBOX class declaration.
	FBStorableClassDeclare( CollisionTerrain, FBModelMarker );

public:
	CollisionTerrain(const char *pName, HIObject pObject= NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate() override;		//!< FiLMBOX Creation function.
	virtual void FBDestroy() override;		//!< FiLMBOX Destruction function.

	virtual bool HasCustomDisplay() override { return true; }

	/** Custom display function called when HasCustomDisplay returns true
	*/
	virtual void CustomModelDisplay(FBCamera* pCamera, FBModelShadingMode pShadingMode, 
									FBModelRenderPass pRenderPass, float pPickingAreaWidth, 
									float pPickingAreaHeight) override;

	/** Custom picking for selection, called when HasCustomDisplay returns true;
        *    \param    pNbHits                 Number of hits.
        *    \param    pSelectBuffer           OpenGL hit buffer.
        *    \param    pCamera                 Current camera for rendering.
        *    \param    pMouseX                 Mouse X position.
        *    \param    pMouseY                 Mouse Y position.
        *    \param    pLocalRaySrc            Local picking ray source.
        *    \param    pLocalRayDir            Local picking ray direction.
        *    \param    pWorldRaySrc            World coordinate picking ray source.
        *    \param    pWorldRayDir            World coordinate picking ray direction.
        *    \param    pGlobalInverseMatrix    Inverse transformation matrix of the model.
        *    \param    pOutPickedPoint         Picked position.
        */
    virtual bool CustomModelPicking(    int pNbHits, unsigned int *pSelectBuffer, FBCamera* pCamera, 
        int pMouseX,int pMouseY,
        FBTVector* pLocalRaySrc, FBTVector* pLocalRayDir,
        FBTVector* pWorldRaySrc, FBTVector* pWorldRayDir,
        FBMatrix* pGlobalInverseMatrix,
        FBTVector* pOutPickedPoint)    override;

	virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;

    /** Override to reuse regular marker object type's property viewSet.
    */
    virtual bool UseCustomPropertyViewSet() const override { return true; }

	//
	//virtual	bool PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug = NULL,FBConnectionType pConnectionType=kFBConnectionTypeNone,FBPlug* pNewPlug=NULL ) override;


	//
	void	OnRenderEvent(HISender pSender, HKEvent pEvent);

	FBPropertyBool					Enabled;

	//FBPropertyDouble				Size;
	//FBPropertyColor					Color;

	FBPropertyAnimatableDouble		Friction;

	FBPropertyBool					ManualUpdate;		// do we need to generate dynamic terrain
	FBPropertyAction				Update;			// update displacement map
	// Note! use connected objects instead ?!
	FBPropertyListObject			Objects;		// objects for calculation terrain map

	FBPropertyBaseEnum<FBTerrainTextureResolution>		TextureResolution;

	FBPropertyBool					AutoCalculateZ;
	FBPropertyAnimatableDouble		MinZ;
	FBPropertyAnimatableDouble		MaxZ;

	FBPropertyVector3d				Density;		// result points density according to the world / texture sizes

	FBPropertyBool					Preview;		// draw points based on terrain map
	FBPropertyBool					DebugNormals;	// display points normals

	// return terrain depth texture id
	GLuint GetTextureId() {
		return mBuffer.GetColorObject();
	}

	static void AddPropertiesToPropertyViewManager();

	void DoUpdate();

	void FillCollisionData( GPUParticles::TCollision &coldata );

protected:
    
	FBSystem						mSystem;

	GPUParticles::terrainBlock		mShaderData;

		
	// ! Use particles system shader (terrain technique)
	GPUParticles::ParticleShaderFX* mShader{ nullptr };

	bool							mNeedUpdate{ true };
	FBVector3d						mLastTranslation;
	FBTime							mLastTimelineTime;

	FrameBuffer						mBuffer;	// for generating height map
	GLuint							mTextureId{ 0 };
	GLuint64						mTextureAddress{ 0 };

	HGLRC							mLastContext{ 0 };

	void CreateGeometry();

	void ReSize();
	void CalculateDensity();

	bool RenderToHeightMap();
};


#endif /* __MODEL_COLLISION_SPHERE_H__ */
