#ifndef __DYNAMICLIGHTING_H__
#define __DYNAMICLIGHTING_H__

// SuperDynamicLighting_shader.h
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//--- SDK include
#include <fbsdk/fbsdk.h>
#include "SuperShader.h"
#include "OGL_Utils.h"
#include "ShadowManager.h"

#include <vector>

//--- Registration define

#define DYNAMICLIGHTING__CLASSSTR	"SuperDynamicLighting"
#define DYNAMICLIGHTING__DESCSTR	"Super Dynamic Lighting"


#define DYNAMICLIGHTING__CLASSNAME	SuperDynamicLighting

//! Dynamic lighting shader using CG to support pixel lighting and normal mapping.
class SuperDynamicLighting : public FBShader
{
    // Declaration.
	FBShaderDeclare(SuperDynamicLighting, FBShader);

public:

    // Open Reality constructor and destructor. Do not use C++ constructor/destructor.
    virtual bool FBCreate() override;
    virtual void FBDestroy() override;

    /**
    * \name Real-time shading callbacks for kFBMaterialEffectShader. 
    */
    //@{

    /** For all the shader instances of the same type, to setup the common state for all this type of shaders. 
    *	\param	pOptions	Render options.
    *	\param	pPass		Rendering pass.
    *	\param	pInfo		Shader-model information.
    */
    virtual void ShaderPassTypeBegin    ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass) override;  

    /** For all the shader instances of the same type, to clean the common state for all this type of shaders 
    *	\param	pOptions	Render options.
    *	\param	pPass		Rendering pass.
    */
    virtual void ShaderPassTypeEnd      ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass) override;     

    /** For all the models associated with this shader instance, to setup the common state for this shader instance. 
    *	\param	pOptions	Render options.
    *	\param	pPass		Rendering pass.
    */
    virtual void ShaderPassInstanceBegin( FBRenderOptions* pRenderOptions, FBRenderingPass pPass) override;  

    /** For all the models associated with this shader instance, to clean the common state for this shader instance.
    *	\param	pOptions	Render options.
    *	\param	pPass		Rendering pass.
    */
    virtual void ShaderPassInstanceEnd  ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass) override;  

    /** For all the models mapped with this material and associated with this shader instance, to setup the common material state for this shader instance.
    *	\param	pOptions	Render options.
    *	\param	pPass		Rendering pass.
    *	\param	pInfo		Shader-model information.
    */
    virtual void ShaderPassMaterialBegin( FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo* pInfo)override;  

    /** For all the models mapped with this material and associated with this shader instance, to clean the common material state for this shader instance.
    *	\param	pOptions	Render options.
    *	\param	pPass		Rendering pass.
    *	\param	pInfo		Shader-model information.
    */
    virtual void ShaderPassMaterialEnd  ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo* pInfo) override;  

    /** Callback for Cg/CgFx shader to setup WorldMaitrx/WorldMatrixIT parameters. 
    *	\param	pOptions	Render options.
    *	\param	pPass		Rendering pass.
    *	\param	pInfo		Shader-model information.
    */
    virtual void ShaderPassModelDraw ( FBRenderOptions* pRenderOptions, FBRenderingPass pPass, FBShaderModelInfo* pInfo) override;

    /** Shadow draw pass begin notify for shader with kFBShaderCapacityMaterialEffect and kFBShaderCapacityDrawShow.
    *	\param	pOptions	Render options.
    */
    virtual void ShaderPassDrawShadowBegin( FBRenderOptions* pRenderOptions) override;

    /** Shadow draw pass end notify for shader with kFBShaderCapacityMaterialEffect and kFBShaderCapacityDrawShow.
    *	\param	pOptions	Render options.
    */
    virtual void ShaderPassDrawShadowEnd( FBRenderOptions* pRenderOptions) override;

    //@}

    /** Create a new shader-model information object.
    *	\param	pOptions	        Render options.
    *	\param	pModelRenderInfo 	Internal Model Render Info.
    *	\return	Newly allocated shader-model information object.
    */
    virtual FBShaderModelInfo* NewShaderModelInfo(HKModelRenderInfo pModelRenderInfo, int pSubRegionIndex) override;

    /** Update shader-model information when model, material & texture mapping or shader setting change.
    *	\param	pOptions	Render options.
    *	\param	pInfo		Shader-model information object to be updated.
    */
    virtual void UpdateModelShaderInfo( FBRenderOptions* pOptions, FBShaderModelInfo* pInfo ) override;

    /** Callback for uploading the modelview array when draw instanced.
    *   \param pModelViewMatrixArray    pointer of 4x4 column major modelview matrix array, length is 4*4*pCount;
    *   \param pCount                   number of modelview matrixs in the array.
    */
    virtual void UploadModelViewMatrixArrayForDrawInstanced(const double* pModelViewMatrixArray, int pCount) override;

	virtual bool PlugDataNotify(FBConnectionAction pAction, FBPlug* pThis, void* pData, void* pDataOld, int pDataSize) override;
	virtual bool PlugNotify(FBConnectionAction pAction, FBPlug* pThis, int pIndex, FBPlug* pPlug, FBConnectionType pConnectionType, FBPlug* pNewPlug) override;

	virtual void DetachDisplayContext(FBRenderOptions* pOptions, FBShaderModelInfo* pInfo) override;

    // Accessors and mutators for transparency type property.
    void SetTransparencyType( FBAlphaSource pTransparency );
    FBAlphaSource GetTransparencyType();

	// Will be automatically called when the Transparency property will be changed.
	static void SetTransparencyProperty(HIObject pObject, FBAlphaSource pState);

	static void SetReloadShaders(HIObject pObject, bool val);

public:
    
	FBPropertyAction		ReloadShaders;
	
	FBPropertyBool			UseSceneLights;

	FBPropertyListObject	AffectingLights;		//!< Selected Lights to illuminate the connected models (to avoid maximum lights number limitation in OpenGL)
    FBPropertyAlphaSource   Transparency;
    FBPropertyAnimatableDouble TransparencyFactor;  

    //
    FBPropertyBool				Shadows;			//!< flag to enable rendering to offscreen shadow textures and use them in lighting shader
    FBPropertyInt               ShadowMapSize;
    FBPropertyInt               ShadowPCFKernelSize;
    FBPropertyListObject        ShadowCasters;
    FBPropertyAnimatableDouble  ShadowStrength;
    FBPropertyAnimatableDouble  OffsetScale;
    FBPropertyAnimatableDouble  OffsetBias;

	//
	FBPropertyBool				SwitchAlbedoTosRGB;
	FBPropertyBool				ForceUpdateTextures;

	// Rim lighting
	FBPropertyAnimatableDouble			UseRim;
	FBPropertyAnimatableDouble			RimPower;
	FBPropertyAnimatableColor			RimColor;

	// MatCap
	FBPropertyBool						UseMatCap;
	FBPropertyListObject				MatCapTexture;

public:

	const Graphics::ShaderLightManager *GetShaderLightsPtr() const {
		return mShaderLights.get();
	}

	// add update task to the queue
	void		AskToUpdateLightList();
	// process lighting list if update task exist
	void		EventBeforeRenderNotify();

protected:
    static Graphics::SuperShader*  mpLightShader;
    static int                     mpLightShaderRefCount;

	void DoReloadShaders();

protected:

	bool						mSkipRendering;
	bool								mNeedUpdateLightsList;
	bool								mNeedUpdateTextures;	// we should update textures after change a context

	std::vector<FBLight*>							mLightsPtr;
	std::unique_ptr<Graphics::ShaderLightManager>		mShaderLights;
	
	OGLCullFaceInfo			mCullFaceInfo;
	FBModelCullingMode		mLastCullingMode;

	bool					mHasExclusiveLights;

    Graphics::ShadowManager shadowManager;

};

#endif // End __DYNAMICLIGHTING_H__
