#ifndef __ORCUSOTMRENDERER_RENDERER_H__
#define __ORCUSOTMRENDERER_RENDERER_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: colors_renderer.h
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/OpenMoBu
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>
#include <fbsdk/fbrenderer.h>

#include <vector>
#include <map>
#include <memory>

#include "glslShader.h"

#ifndef ORSDK_DLL
	/** \def ORSDK_DLL
	*	Be sure that ORSDK_DLL is defined only once...
	*/
	#define ORSDK_DLL K_DLLIMPORT
#endif

/////////////////////////////////////////////////////////////////////////////
// Unique color id render


#define ORColorsRendererCallback__CLASSNAME		ColorsRendererCallback
#define ORColorsRendererCallback__CLASSSTR		"ColorsRendererCallback"
#define ORColorsRendererCallback__DESCSTR		"Colors Renderer"

class ORSDK_DLL ColorsRendererCallback : public FBRendererCallback
{
    FBRendererCallbackDeclare(ColorsRendererCallback, FBRendererCallback);

public:
	virtual bool FBCreate() override;		//!< Constructor.
	virtual void FBDestroy() override;		//!< Destructor.

    virtual const char* GetCallbackName() const override;
	virtual const char* GetCallbackDesc() const override;

    virtual unsigned int GetCallbackPrefCount() const override;
    virtual const char*  GetCallbackPrefName(unsigned int pIndex) const override;

    virtual void Attach() override;
    virtual void Detach() override;
    virtual void DetachDisplayContext(FBViewingOptions* pViewOption) override;

    virtual void Render(FBRenderOptions* pRenderOptions) override;


    FBPropertyBool      CustomLightingSetting;   //!< <b>Read/Write Property:</b> Demo how to use Custom Lighting Setting. 

    /** <b>Read/Write Property:</b> NoFrustumculling, for demo purpose.
    *   Frustum culling could improve rendering performance, and provide hint to allow 
    *   MotionBuilder core evaluation engine not perform heavy deformation task when the 
    *   model is outside of view port for a while.
    */
    FBPropertyBool      NoFrustumculling;       
    FBPropertyBool      CustomFrstumCulling;    //!< <b>Read/Write Property:</b> Demo how to perform naive frustum culling if true.
    FBPropertyBool      CacheSceneGraph;        //!< <b>Read/Write Property:</b> Demo how to cache/update scene graph for optimized rendering (but relative complex) if true.

    void CacheSceneGraphSet(bool pValue);

protected:

    /** Demo how to utilize the per frame callback at pipeline's critical stage/timing in the application main loop.
    */
    void OnPerFrameSynchronizationCallback      (HISender pSender, HKEvent pEvent);
    void OnPerFrameRenderingPipelineCallback    (HISender pSender, HKEvent pEvent);
    void OnPerFrameEvaluationPipelineCallback   (HISender pSender, HKEvent pEvent);

    /** Demo how to listen the scene change events. 
    *   this could be useful for those performance greedy render implementation, instead of dynamically query 
    *   scene graph for rendering per frame, plugin developer could cache and reuse the scene graph info, 
    *   incremental update or fully rebuild it upon any relevant scene change events occurs. 
    */
    void EventSceneChange			            (HISender pSender, HKEvent pEvent);
    void EventConnNotify                        (HISender pSender, HKEvent pEvent);
    void EventConnStateNotify                   (HISender pSender, HKEvent pEvent);
    void EventConnDataNotify                    (HISender pSender, HKEvent pEvent);

protected:
    unsigned int        mAttachCount;          //!< How many view panes use this renderer callback instance currently.

	std::unique_ptr<GLSLShader>   mColorShader;

	GLint				mLocationMVP;
	GLint				mLocationCubeMin;
	GLint				mLocationCubeSize;

	bool				LoadShader();
	void				FreeShader();

	bool				InitShaderLocations();

	//
	void RenderColorIds(FBRenderOptions *pRenderOptions);

	//
	void RenderNormalizedColors(FBRenderOptions *pRenderOptions);

    /** Demo a simple rendering approach by iterating through each model in the scene at frame */
    void RenderWithSimpleIterating(FBRenderOptions* pRenderOptions);

    /**
    * \name Function / Data Members for rendering with cached scene graph info.
    */
    //@{

    /** Demo a relative complicated approach by caching/updating the needed scene graph info.*/
    void RenderWithCachedInfo(FBRenderOptions* pRenderOptions);
    
    void CacheSceneReset();

    /** Fully re-build the cached scene graph in one shot, instead of incrementally*/
    void CacheSceneFullRestructure();

    typedef std::pair<FBModel*, int> _ORModelRenderItem;
    typedef std::vector<_ORModelRenderItem> _ORModelRenderItemList;
    typedef std::map<FBMaterial*, _ORModelRenderItemList*> _ORModelRenderItemListPerMaterialMap;

    /** A simple cached scene graph data structure which sort render item by material.
    *   Plugin developer could build more optimized & sophisticated cached scene graph data structure. 
    *   e.g, spatial accelerated, sort by material / culling mode, user defined attributes. 
    */
    _ORModelRenderItemListPerMaterialMap mModelRenderItemListPerMaterialMap;

    /** Need to fully re-build the cached rendering info if true. otherwise in incremental way*/
    bool mNeedFullRestructure;

    //@}

};


#endif /* __ORCUSOTMRENDERER_RENDERER_H__ */
