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

#include "glslShaderProgram.h"

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
    
protected:
    unsigned int        mAttachCount;          //!< How many view panes use this renderer callback instance currently.

	std::unique_ptr<GLSLShaderProgram>   mColorShader;

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

};


#endif /* __ORCUSOTMRENDERER_RENDERER_H__ */
