#ifndef __ORSHADER_TEMPLATE_H__
#define __ORSHADER_TEMPLATE_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: fb_shader_particles_shader.h
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>
#include "curveEditor_popup.h"

#include "Timer.h"

#include <map>

#ifndef ORSDK_DLL
	/** \def ORSDK_DLL
	*	Be sure that ORSDK_DLL is defined only once...
	*/
	#define ORSDK_DLL K_DLLIMPORT
#endif

#include "ParticleSystem.h"

enum FBParticlePlayMode
{
	kFBParticleLife,
	kFBParticlePlay
};

const char * FBPropertyBaseEnum<FBParticlePlayMode>::mStrings[] = {
	"Live",
	"Play time",
	0
};

enum FBParticlePrimitive
{
	kFBParticlePoints,
	kFBParticleQuads,
	kFBParticleBillboard,
	kFBParticleStretchedBillboard,
	kFBParticleInstance
};

const char * FBPropertyBaseEnum<FBParticlePrimitive>::mStrings[] = {
	"Points",
	"Quads",
	"Billboard",
	"Stretched Billboard",
	"Instancing",
	0
};

enum FBParticleEmitter
{
	kFBParticleEmitterVertices,
	kFBParticleEmitterVolume,
	kFBParticleEmitterSurface
};

const char * FBPropertyBaseEnum<FBParticleEmitter>::mStrings[] = {
	"Vertices",
	"Volume",
	"Surface",
	0
};

enum FBParticleShadeMode
{
	kFBParticleShadeSimple,
	kFBParticleShadeFlat,
	kFBParticleShadeDynamic
};

const char * FBPropertyBaseEnum<FBParticleShadeMode>::mStrings[] = {
	"No Lighting",
	"Flat",
	"Dynamic",
	0
};

//--- Registration define
#define ORSHADER_TEMPLATE__CLASSNAME	GPUshader_Particles
#define ORSHADER_TEMPLATE__CLASSSTR		"GPUshader_Particles"
#define ORSHADER_TEMPLATE__DESCSTR		"Particles - GPU Shader"

//////////////////////////////////////////////////////////////////////////////////////////////////////
//! GPU Particles shader.

class ORSDK_DLL GPUshader_Particles : public FBShader
{
	//--- FiLMBOX declaration.
	FBShaderDeclare( GPUshader_Particles, FBShader );

public:
    //--- Construct custom shader from an FBMaterial object.
    GPUshader_Particles(FBMaterial *pMaterial );

public:
	virtual bool FBCreate();		//!< FiLMBOX Constructor.
	virtual void FBDestroy();		//!< FiLMBOX Destructor.

	/** Create a new shader-model information object.
	*	\param	pModelRenderInfo 	Internal Model Render Info.
	*   \param  pSubRegionIndex
	*	\return	Newly allocated shader-model information object.
	*/
	virtual FBShaderModelInfo* NewShaderModelInfo(HKModelRenderInfo pModelRenderInfo, int pSubRegionIndex) override;

	/** Update shader-model information when model, material & texture mapping or shader setting change.
	*	\param	pOptions	Render options.
	*	\param	pInfo		Shader-model information object to be updated.
	*/
    virtual void UpdateModelShaderInfo( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo ) override;

		/** Destroy shader-model information object callback.
	*	\param	pOptions	Render options.
	*	\param	pInfo		Shader-model information object to destroy.
	*/
	virtual void DestroyShaderModelInfo( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo ) override;

    virtual bool ShaderNeedBeginRender() override;
    virtual void ShaderBeginRender( FBRenderOptions* pRenderOptions, FBShaderModelInfo* pShaderModelInfo ) override;
	virtual void ShadeModel( FBRenderOptions* pRenderOptions, FBShaderModelInfo* pShaderModelInfo, FBRenderingPass pPass ) override;


	/**	Detach the display context from the shader.
		*	\param	pOptions	Render options.
		*	\param	pInfo		Shader-model information object.
		*/
	virtual void DetachDisplayContext( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo ) override;


public:

	/** PlugDataNotify when overloaded is equivalent of FBSystem.OnConnectionDataNotify but in the context of the derived object only
	  * \param pAction Plug action, the event type.
	  * \param pThis Plug concerned.
	  * \param pData Current data, meaning depends of plug action.
	  * \param pDataOld Previous data, meaning depends of plug action.
	  * \param pDataSize Size of data.
	  * \return unused should return true
	*/
	virtual	bool PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData=NULL,void* pDataOld=NULL,int pDataSize=0) override;

	/** PlugStateNotify when overloaded is equivalent of FBSystem.OnConnectionStateNotify but in the context of the derived object only
	  * \param pAction Plug action, the event type.
	  * \param pThis Plug concerned.
	  * \param pData Current data, meaning depends of plug action.
	  * \param pDataOld Previous data, meaning depends of plug action.
	  * \param pDataSize Size of data.
	  * \return unused should return true except in the case of property rename kFBRename
	*/
	//virtual	bool PlugStateNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData=NULL,void* pDataOld=NULL,int pDataSize=0);

	/** PlugNotify when overloaded is equivalent of FBSystem.OnConnectionNotify but in the context of the derived object only
	  * \param pAction Plug action, the event type.
	  * \param pThis Plug concerned.
	  * \param pIndex Index of the plug.
	  * \param pPlug Other plug concerned.
	  * \param pConnectionType Connection type between the plugs.
	  * \param pNewPlug New plug (for the replace event).
	  * \return should by default return true except in the case of connection requests (kFBRequestConnectSrc or kFBRequestConnectDst)
	*/
	virtual	bool PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug = NULL,FBConnectionType pConnectionType=kFBConnectionTypeNone,FBPlug* pNewPlug=NULL ) override;

	//

	void OnPerFrameRenderingPipelineCallback    (HISender pSender, HKEvent pEvent);
	void OnVideoFrameRendering	(HISender pSender, HKEvent pEvent);


//
// Common particles properties
//
public:

	FBPropertyInt								InternalClassId;

#ifdef _DEBUG
	FBPropertyAction							ReloadShader;
#endif

	FBPropertyAction							About;

	FBPropertyInt								DisplayedCount;		// result of particle work, number of particles in frame
	FBPropertyInt								MaximumParticles;	// this a limit for memory allocation for that system (1024*1024 by default)
	
	FBPropertyTime								ResetTime;			// at this frame particles will be reseted (rewind to beginning) - Start Frame
	FBPropertyAnimatableAction					Reset;				// reset particles (start from the beginning)
	FBPropertyAnimatableAction					ResetAll;
	FBPropertyInt								ResetCount;			// this a startup count of particles!

	FBPropertyDouble							ExtrudeResetPosition;
	FBPropertyBool								GenerationSkipZeroAlpha;
	FBPropertyDouble							GenerateSkipAlphaLimit;

	FBPropertyBool								UseRate;
	FBPropertyInt								ParticleRate;		// particles / second
	FBPropertyBool								UsePreGeneratedParticles;	// pre-cache launchers or generate dynamically in a glsl shader
	FBPropertyBool								GenerateOnMotion;	// emit particle only when geometry has velocity on a vertex
	FBPropertyDouble							GenerateOnMotionFactor;

	// mask out particles when generating from a surface
	FBPropertyBool								UseGenerationMask;
	FBPropertyListObject						GenerationMask;

	// how fast we should reach a reset pose
	FBPropertyAnimatableDouble					ConstraintMagnitude;

	FBPropertyInt								RandomSeed;

	//
	// EMITTER

	// property for live, play time mode
	FBPropertyBaseEnum<FBParticlePlayMode>		PlayMode;		//!< real-time or player slider time

	// emit start, stop
	FBPropertyBool								UseCustomRange;
	FBPropertyTime								EmitStart;			
	FBPropertyTime								EmitStop;			// 0 means no limit (takes player control parameters)

	// life time, and percent of variantion
	FBPropertyAnimatableDouble					LifeTime;
	FBPropertyAnimatableDouble					LifeTimeVariation;

	FBPropertyBaseEnum<FBParticleEmitter>		Emitter;			// emitter shape
	FBPropertyBool								InheritEmitterColors;

	FBPropertyAnimatableVector3d				EmitDirection;	// emit particles in that direction
	FBPropertyAnimatableDouble					EmitDirSpreadHor;	// percent for dir randomize
	FBPropertyAnimatableDouble					EmitDirSpreadVer;	// percent for dir randomize
	FBPropertyBool								UseEmitterNormals;	// use normals for birth direction of a particle
	
	FBPropertyAnimatableDouble					EmitSpeed;
	FBPropertyAnimatableDouble					EmitSpeedSpread;	// randomize start speed
	FBPropertyBool								InheritEmitterSpeed;	// use difference between prev and current pos to calculate directions
	
	// emit rotation

	FBPropertyAnimatableVector3d				InitialOrientation;
	FBPropertyAnimatableVector3d				InitialOrientationSpread;

	// angular velocity for each euler angle
	FBPropertyAnimatableVector3d				AngularVelocity;					
	FBPropertyAnimatableVector3d				AngularVelocitySpread;


	//
	// DYNAMIC parameters

	FBPropertyAnimatableDouble					Mass;
	FBPropertyAnimatableDouble					Damping;
	FBPropertyBool								UseGravity;
	FBPropertyAnimatableVector3d				Gravity;
	// still need property for include / exclude force objects
	
	// Note! use connected forces instead
	FBPropertyBool								UseForces;		// use additional forces (wind, vortex, turbulence, etc.)
	FBPropertyListObject						Forces;

	// additional turbulence behaviour
	FBPropertyBool								UseTurbulence;
	FBPropertyAnimatableDouble					NoiseFrequency;
	FBPropertyAnimatableDouble					NoiseSpeed;
	FBPropertyAnimatableDouble					NoiseAmplitude;
	
	// still need property for include / exclude collision objects
	FBPropertyBool								UseCollisions;	// like sphere or terrain collisions
	// Note! use connected sollisions instead
	FBPropertyListObject						Collisions;		// simple geometry shapes to collide with (sphere, box, plane)

	FBPropertyBool								SelfCollisions;

	FBPropertyBool								UseFloor;
	FBPropertyAnimatableDouble					FloorFriction;
	FBPropertyAnimatableDouble					FloorLevel;		// y value of floor collision and constraint

	// Common evaluation
	FBPropertyInt								Iterations;		// evaluation iterations per second
	FBPropertyDouble							DeltaTimeLimit;	// if current timestep greater, then make several evaluation cycles to unroll timestep
	// with animatable property you can make this substep addaptive to the emitter motion speed
	FBPropertyBool								AdaptiveSubSteps;	// automaticaly calculate substeps depends on the emitter motion speed
	FBPropertyAnimatableInt						SubSteps;		// number of steps during one step (for rapid motion)

	// Common graphic parameters
	FBPropertyBool								PointSmooth;
	FBPropertyBool								PointFalloff;
	FBPropertyBaseEnum<FBParticlePrimitive>		PrimitiveType;		//!< type of a particle shape (point, billboard, geometry instance, etc.)
	FBPropertyListObject						InstanceObject;		//!< draw this object as an instance
	// ?! grab texture from the diffuse channel of an instance object
	FBPropertyListObject						TextureObject;		//!< draw with this texture

	FBPropertyBaseEnum<FBParticleShadeMode>		ShadeMode;
	FBPropertyListObject						AffectingLights;		//!< Selected Lights to illuminate the connected models (to avoid maximum lights number limitation in OpenGL)
    FBPropertyAlphaSource						Transparency;
    FBPropertyAnimatableDouble					TransparencyFactor; 

	FBPropertyAnimatableColorAndAlpha					Color;
	FBPropertyBool										UseColor2;
	FBPropertyAnimatableColorAndAlpha					Color2;
	FBPropertyBool										UseColor3;
	FBPropertyAnimatableColorAndAlpha					Color3;
	FBPropertyAnimatableDouble							ColorVariation;
	FBPropertyBool										UseColorCurve;
	FBPropertyAction									ColorCurve;
	FBPropertyAnimatableColorAndAlpha					ColorCurveHolder;	// animatable node for holding curve paramters
	//ORPopup_ColorEditor									ColorCurveEditor;

	// TODO: add alpha curve (color attenuation\fading)

	FBPropertyAnimatableDouble							Size;
	FBPropertyAnimatableDouble							SizeVariation;	// percent of size variations

	FBPropertyAnimatableDouble							MinPointScale;
	FBPropertyAnimatableDouble							MaxPointScale;
	FBPropertyAnimatableDouble							PointScaleDistance;

	// TODO: rename to size attenuation \ fading

	FBPropertyBool										UseSizeCurve;
	FBPropertyAction									SizeCurve;
	FBPropertyAnimatableDouble							SizeCurveHolder;
	//ORPopup_CurveEditor									SizeCurveEditor;

	//
	// TODO: Caching block (WIP)

	FBPropertyBool			UseCache;
	FBPropertyBool			AutoCache;

	FBPropertyAction		CacheCutLeft;
	FBPropertyAction		CacheCutRight;
	FBPropertyAction		ClearCache;	// remove or just disconnect
	FBPropertyAction		LoadCache; // assign a cache file
	FBPropertyAction		SaveCache;
	FBPropertyAction		CacheTimeRange; // do a cache for a current time

	FBPropertyString		CacheFile;
	FBPropertyInt			CacheFrames; // read-only - number of cached frames
	FBPropertyInt			CacheFPS;		// read-only - cache file number of frames

	FBPropertyInt			FrameInMemory; // pre-load n-frames in memory
	FBPropertyInt			MemoryUsed; // amount of memory used for pre-caching

	static void AddPropertiesToPropertyViewManager();

	// Accessors and mutators for transparency type property.
    void SetTransparencyType( FBAlphaSource pTransparency );
    FBAlphaSource GetTransparencyType();

    // Will be automatically called when the Transparency property will be changed.
    static void SetTransparencyProperty(HIObject pObject, FBAlphaSource pState);
	static void SetEmitterProperty(HIObject pObject, FBParticleEmitter pState);

	//
#ifdef _DEBUG
	static void ReloadShaderAction(HIObject pObject, bool value);
#endif
	static void AboutAction(HIObject pObject, bool value);
	static void ResetAction(HIObject pObject, bool value);
	static void ResetAllAction(HIObject pObject, bool value);
	static void SetColorCurve(HIObject pObject, bool value);
	static void SetSizeCurve(HIObject pObject, bool value);
	static int GetDisplayedCount(HIObject pObject);
	static int GetInternalClassId(HIObject pObject);

	void DoReloadShader();
	void DoReset();
	void DoResetAll();
	void DoColorCurve();
	void DoSizeCurve();

protected:
	FBSystem				mSystem;

	bool					mIsOfflineRenderer;

    FBMaterial				*mMaterial;
    int						mRenderFrameId;

	// make it unique per model
	//std::map<FBModel*, double>	mLastFrameTimeMap;
	//std::map<FBModel*, bool>	mIsResetDoneMap;
	//double					mLastFrameTime;

	bool	firstBeginRender;
	bool	firstShadeModel;

	bool		mLastResetState;
	bool		mLastResetAllState;

	unsigned int		mTotalCycles;		// total simulation cycles
	unsigned int		mDisplayedCount;

	//bool					mIsFirst;
	FBTime					mLastTimelineTime;
	//bool					mIsResetDone;

	int						mLastRenderFrameId;

	bool					mNeedReloadShaders;
	bool					mNeedUpdatePropertyTexture;

	// TODO: use rotation for calculating velocity
	//FBVector3d				mLastEmitterPos; // for calculating velocity (need for option inherit emitter velocity)

	bool						mUseSizeCurve;
	bool						mUseColorCurve;

	ColorPropertyTexture		mColorCurve;		// animate color during lifetime
	DoublePropertyTexture		mSizeCurve;			// animate size during lifetime

	GPUParticles::ParticleSystemConnections		mParticleConnections;
	std::map<FBModel*, GPUParticles::ParticleSystem*>	mParticleMap;

	void SyncForcesPropWithComponents();
	void RemoveForceFromComponents();

	void UpdateEmitterGeometryBufferOnCPU(FBModel *pModel, GPUParticles::ParticleSystem *pParticles);
	void UpdateEmitterGeometryBufferOnGPU(FBModel *pModel, GPUParticles::ParticleSystem *pParticles);

	void	UpdateConnectedCollisionsData();
	void	UpdateConnectedForcesData();

	// ?! update altogether collisions and forces
	void	UpdateConnectedData();
	void	UploadConnectedDataToGPU(GPUParticles::ParticleSystem *pParticles);

	bool	UpdateInstanceData();

	// update connected terrain models (render to depth)
	void	UpdateConnectedTerrain();
	// pass data from UI properties into the particle system
	void	UpdateEvaluationData(FBModel *pEmitterModel, GPUParticles::ParticleSystem *pParticles, const bool enableEmit);

	void SetUpPropertyTextures( FBPropertyAnimatableDouble *sizeProp, FBPropertyAnimatableColorAndAlpha *colorProp )
	{
		mColorCurve.SetUp(colorProp);
		mSizeCurve.SetUp(sizeProp);
	}
	void UpdatePropertyTextures()
	{
		mColorCurve.Generate();
		mSizeCurve.Generate();
	}

	void FreeParticles();

	void LocalShaderBeginRender( FBRenderOptions* pRenderOptions, FBModel *pModel );
	void LocalShadeModel( FBRenderOptions* pRenderOptions, FBModel *pModel, FBRenderingPass pPass );

	void DebugDisplay(FBModel *pModel);

};

#endif /* __ORSHADER_TEMPLATE_H__ */
