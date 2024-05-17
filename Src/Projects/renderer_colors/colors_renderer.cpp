
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: colors_renderer.cpp
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/OpenMoBu
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ORSDK_DLL
/** \def ORSDK_DLL
*	Be sure that ORSDK_DLL is defined only once...
*/
#define ORSDK_DLL K_DLLEXPORT
#endif

// Class declaration
#include "colors_renderer.h"
#include <fbsdk/fbsystem.h>
#include <fbsdk/fbscene.h>
#include <fbsdk/fbrenderer.h>
#include <fbsdk/fbshader.h>
#include <fbsdk/fbsystem.h>

#include "FileUtils.h"

//--- Registration defines
#define ORColorsRendererCallback__CLASS		ORColorsRendererCallback__CLASSNAME
#define ORColorsRendererCallback__DESC		ORColorsRendererCallback__DESCSTR

FBRendererCallbackImplementation(ORColorsRendererCallback__CLASS);

FBRegisterRendererCallback	(
    ORColorsRendererCallback__CLASSSTR,
    ORColorsRendererCallback__CLASS,
    ORColorsRendererCallback__CLASSSTR,
    ORColorsRendererCallback__DESC,
    FB_DEFAULT_SDK_ICON	);

// define new task cycle index
FBProfiler_CreateTaskCycle( ColorIdRenderer, 0.5f, 0.5f, 0.5f );


static void ORCacheSceneGraphSet(HIObject pMbObject, bool pValue)
{
    ColorsRendererCallback* pFbObject = FBCast<ColorsRendererCallback>( pMbObject );
    pFbObject->CacheSceneGraphSet(pValue);
}

bool ColorsRendererCallback::FBCreate()
{
    //
    // Register task cycle index in profiler.
    //
    FBProfiling_SetupTaskCycle( ColorIdRenderer );

    //
    //  Enabled advanced lighting mode, include UI widgets for various advanced lighting setting, includes: 
    //   Light: area light, spot light inner/outer angles, barndoors and etc.,; 
    //   Model: PrimaryVisibility, CastsShadows and ReceiveShadows.
    //
    FBSystem::TheOne().Renderer->AdvancedLightingMode = true;

    //
    // Init properties. 
    //
    FBPropertyPublish(this, CustomLightingSetting,  "CustomLightingSetting",    NULL, NULL);
    FBPropertyPublish(this, NoFrustumculling,       "NoFrustumculling",         NULL, NULL);
    FBPropertyPublish(this, CustomFrstumCulling,    "CustomFrustumCulling",     NULL, NULL);
    FBPropertyPublish(this, CacheSceneGraph,        "CacheSceneGraph",          NULL, ORCacheSceneGraphSet);
    
    SupportIDBufferPicking  = true;
    CustomLightingSetting   = false;
    NoFrustumculling        = false;
    CustomFrstumCulling     = true;
    CacheSceneGraph         = false;

    // Init data members.
    mAttachCount            = 0;

    mNeedFullRestructure    = true;

	mColorShader			= nullptr;

	glewInit();

    return true;
} 

void ColorsRendererCallback::FBDestroy()
{
}

const char* ColorsRendererCallback::GetCallbackName() const
{
    return ORColorsRendererCallback__CLASSSTR;
}

const char* ColorsRendererCallback::GetCallbackDesc() const
{
    // Return description which 
    return ORColorsRendererCallback__DESCSTR;
}

unsigned int ColorsRendererCallback::GetCallbackPrefCount() const
{
    // Set preference count to 3. And this count need to match the following GetCallbackPrefName() function. 
    return 2;
}

const char*  ColorsRendererCallback::GetCallbackPrefName(unsigned int pIndex) const
{
    // Return preference description.
    switch (pIndex)
    {
    case 0: 
        return "Color Id";
    case 1: 
        return "Normalized Color";
    default: 
        return "Wrong Pref Index!!!";
    }
}

bool ColorsRendererCallback::LoadShader()
{
	FreeShader();

	mColorShader.reset(new GLSLShader());

	FBString fragment_filename( "\\GLSL\\renderer_colorRamp.fsh" );
	FBString vertex_filename( "\\GLSL\\renderer_colorRamp.vsh" );

	FBString effectPath, effectFullName;

	if (false == FindEffectLocation( fragment_filename, effectPath, effectFullName ) )
	{
		FreeShader();
		return false;
	}

	// most of shaders share the same simple vertex shader
	if (false == mColorShader->LoadShaders( FBString(effectPath, vertex_filename), FBString(effectPath, fragment_filename) ) )
	{
		FreeShader();
		return false;
	}

	if (false == InitShaderLocations() )
	{
		FreeShader();
		return false;
	}

	return true;
}

bool ColorsRendererCallback::InitShaderLocations()
{
	if (mColorShader == nullptr)
		return false;

	mLocationMVP = mColorShader->findLocation( "MVP" );
	if (mLocationMVP < 0)
		return false;

	mLocationCubeMin = mColorShader->findLocation( "CubeMin" );
	if (mLocationCubeMin < 0)
		return false;

	mLocationCubeSize = mColorShader->findLocation( "CubeSize" );
	if (mLocationCubeSize < 0)
		return false;

	return true;
}

void ColorsRendererCallback::FreeShader()
{
	if (mColorShader)
	{
        mColorShader.reset();
	}
}

void ColorsRendererCallback::Attach()
{
    //
    // This callback occurs when one view pane attach this renderer callback instance.
    //
    FBTrace("ORCustomRendererCallback::Attach()\n");

    //
    // Prepare for first time. 
    //
    if (mAttachCount == 0)
    {
        // Register callback function for critical timings at the pipeline.
        FBEvaluateManager::TheOne().OnSynchronizationEvent   .Add(this, (FBCallback)&ColorsRendererCallback::OnPerFrameSynchronizationCallback);
        FBEvaluateManager::TheOne().OnEvaluationPipelineEvent.Add(this, (FBCallback)&ColorsRendererCallback::OnPerFrameEvaluationPipelineCallback);
        FBEvaluateManager::TheOne().OnRenderingPipelineEvent .Add(this, (FBCallback)&ColorsRendererCallback::OnPerFrameRenderingPipelineCallback);

		if (false == LoadShader() )
			FBMessageBox( "Color Renderer", "Failed to load shaders!", "Ok" );
    }

    // Increase attachment count.
    mAttachCount++;
}

void ColorsRendererCallback::Detach()
{
    //
    // This callback occurs when one view pane detach this renderer callback instance.
    //
    FBTrace("ORCustomRendererCallback::Detach()\n");

    // Decrease attachment count.
    mAttachCount--;

    // 
    // This callback instance is not be used by any view panes anymore. 
    //
    if (mAttachCount == 0)
    {
        // Unregister callback function for critical timings at the pipeline.
        FBEvaluateManager::TheOne().OnSynchronizationEvent   .Remove(this, (FBCallback)&ColorsRendererCallback::OnPerFrameSynchronizationCallback);
        FBEvaluateManager::TheOne().OnEvaluationPipelineEvent.Remove(this, (FBCallback)&ColorsRendererCallback::OnPerFrameEvaluationPipelineCallback);
        FBEvaluateManager::TheOne().OnRenderingPipelineEvent .Remove(this, (FBCallback)&ColorsRendererCallback::OnPerFrameRenderingPipelineCallback);

		FreeShader();
    }
}

void ColorsRendererCallback::DetachDisplayContext(FBViewingOptions* pViewOption)
{
    //
    // This callback occurs when no view pane use this renderer callback instance anymore, 
    // or some internal events request GL context invalidation. 
    // We should release allocated OpenGL resources here.
    //

    FBTrace("ORCustomRendererCallback::DetachDisplayContext()\n");
}

void ColorsRendererCallback::Render(FBRenderOptions* pRenderOptions)
{
    //
    // Start CustomRenderer task cycle profiling, 
    //
    FBProfilerHelper lProfiling( FBProfiling_TaskCycleIndex( ColorIdRenderer ), FBGetDisplayInfo() );

    //
    // Push all GL attributes and pop at the end to avoid the in perfect 
    // implementations affect the rest MoBu rendering functionality.
    //
    glPushAttrib(GL_ALL_ATTRIB_BITS | GL_CLIENT_ALL_ATTRIB_BITS);

    //
    // In this example, we demo a simple forward rendering implementation on the
    // default FBO provided by MoBu. 
    //
    // However plugin developer may want to implement their own deferred renderer
    // by creating multiple additional FBOs, and perform creative rendering algorithms,
    // blit the final result back to Color Buffer of MoBu's default FBO at the end.
    //

	if (pRenderOptions->GetViewerOptions()->RenderCallbackPrefIndex() == 0)
	{
		RenderColorIds(pRenderOptions);
	}
	else if (pRenderOptions->GetViewerOptions()->RenderCallbackPrefIndex() == 1)
	{
		RenderNormalizedColors(pRenderOptions);
	}
	
    //
    // Pop GL states.
    //
    glPopAttrib();
}

/////////////////////////
void ColorsRendererCallback::RenderColorIds(FBRenderOptions *pRenderOptions)
{

	FBRenderer* lRenderer = FBSystem::TheOne().Renderer;
    FBCamera* lCamera = lRenderer->CurrentCamera;
    
    FBMatrix lCameraMVPMatrix;
    lCamera->GetCameraMatrix( lCameraMVPMatrix, kFBModelViewProj );

	if ( FBIS(lCamera, FBCameraSwitcher) )
		lCamera = ( (FBCameraSwitcher*) lCamera )->CurrentCamera;

	if (lCamera == nullptr)
		return;

    glDisable(GL_LIGHTING); 
       

    glMatrixMode(GL_MODELVIEW);

    //
    // Loop though each model, and render accordingly.
    //
    int llDisplayGeometryCount = lRenderer->DisplayableGeometryCount;
    while(llDisplayGeometryCount--)
    {
        FBModel* lModel = lRenderer->GetDisplayableGeometry( llDisplayGeometryCount );
        FBModelVertexData* lModelVertexData = lModel->ModelVertexData;

        // Skips model if it is not visible (affected by its self visibility property, visibility inheritance and parent & owner's visibility status),
        // or if it is not supposed to be drawn (e.g., deformed vertices are not yet ready for drawing, or if model is hidden via Z-Depth Selection tool)
        if( !lModel->IsVisible() || !lModelVertexData->IsDrawable() )  
            continue;

        FBMatrix lModelTransformMatrix;
        lModel->GetMatrix(lModelTransformMatrix, kModelTransformation_Geometry);

        //
        // Early frustum culling is important for improving rendering performance. 
        // and provide hint to allow MotionBuilder core evaluation engine not 
        // perform heavy deformation task when the model is outside of view port 
        // for a few frames.
        //
        if (NoFrustumculling == false)
        {
            
            if (lRenderer->IsModelInsideCameraFrustum(lModel, lCamera) == false)
				continue;
           
        }


        //If IDBuffer rendering requested (for display or picking), set model color to be the Model's Unique Color ID.
       
        FBColor lUniqueColorId = lModel->UniqueColorId;
		glColor3dv(lUniqueColorId);

        glPushMatrix();
        glMultMatrixd(lModelTransformMatrix);

        const int lSubRegionCount = lModelVertexData->GetSubRegionCount();

        //Calling PushZDepthClipOverride() disables the OpenGL custom clip-plane (used for Z-Depth HideFront selection) if this model is selected using
        //Z-Depth HideFront selection tool. This is so that the model is not clipped, i.e., remains visible.
        lModelVertexData->PushZDepthClipOverride();

        lModelVertexData->EnableOGLVertexData();        //Bind Vertex Array or Vertex Buffer Object.

        for(int lSubRegionIdx = 0; lSubRegionIdx < lSubRegionCount; ++lSubRegionIdx)
        {
           
			// ID Buffer rendering, simply draw geometry. 
			lModelVertexData->DrawSubRegion(lSubRegionIdx); // draw all the sub patches inside this sub regions.
           
        }

        lModelVertexData->DisableOGLVertexData();   //Unbind Vertex Array or Vertex Buffer Object.
        lModelVertexData->PopZDepthClipOverride();  //Re-enables Z-Depth HideFront clip-plane if it was previously disabled via PushZDepthClipOverride().
        glPopMatrix();
    }


}

/////////////////////////////
void ColorsRendererCallback::RenderNormalizedColors(FBRenderOptions *pRenderOptions)
{

	if (mColorShader == nullptr)
		return;


	FBRenderer* lRenderer = FBSystem::TheOne().Renderer;
    FBCamera* lCamera = lRenderer->CurrentCamera;
    
	if ( FBIS(lCamera, FBCameraSwitcher) )
		lCamera = ( (FBCameraSwitcher*) lCamera )->CurrentCamera;

	if (lCamera == nullptr)
		return;

    FBMatrix lCameraMVPMatrix;
    lCamera->GetCameraMatrix( lCameraMVPMatrix, kFBModelViewProj );

	//
	float fmatrix[16];

	if (pRenderOptions->IsIDBufferRendering())
	{
		glDisable(GL_LIGHTING); 
		glMatrixMode(GL_MODELVIEW);
	}
	else
	{
		for (int i=0; i<16; ++i)
			fmatrix[i] = (float) lCameraMVPMatrix[i];
		
		mColorShader->Bind();
		mColorShader->setUniformMatrix( mLocationMVP, fmatrix );
		mColorShader->setUniformVector( mLocationCubeMin, -500.0f, -500.0f, -500.0f, 1.0f );
		mColorShader->setUniformVector( mLocationCubeSize, 1000.0f, 1000.0f, 1000.0f, 1.0f );

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		
	}

    //
    // Loop though each model, and render accordingly.
    //
    int llDisplayGeometryCount = lRenderer->DisplayableGeometryCount;
    while(llDisplayGeometryCount--)
    {
        FBModel* lModel = lRenderer->GetDisplayableGeometry( llDisplayGeometryCount );
        FBModelVertexData* lModelVertexData = lModel->ModelVertexData;

        // Skips model if it is not visible (affected by its self visibility property, visibility inheritance and parent & owner's visibility status),
        // or if it is not supposed to be drawn (e.g., deformed vertices are not yet ready for drawing, or if model is hidden via Z-Depth Selection tool)
        if( !lModel->IsVisible() || !lModelVertexData->IsDrawable() )  
            continue;

        FBMatrix lModelTransformMatrix;
        lModel->GetMatrix(lModelTransformMatrix, kModelTransformation_Geometry);

        //
        // Early frustum culling is important for improving rendering performance. 
        // and provide hint to allow MotionBuilder core evaluation engine not 
        // perform heavy deformation task when the model is outside of view port 
        // for a few frames.
        //
        if (NoFrustumculling == false)
        {
            
            if (lRenderer->IsModelInsideCameraFrustum(lModel, lCamera) == false)
				continue;
           
        }


        //If IDBuffer rendering requested (for display or picking), set model color to be the Model's Unique Color ID.
       //If IDBuffer rendering requested (for display or picking), set model color to be the Model's Unique Color ID.
        if (pRenderOptions->IsIDBufferRendering())
        {
            FBColor lUniqueColorId = lModel->UniqueColorId;
            glColor3dv(lUniqueColorId);


			glPushMatrix();
			glMultMatrixd(lModelTransformMatrix);
        }
		else
		{
			
			FBMatrixMult( lModelTransformMatrix, lCameraMVPMatrix, lModelTransformMatrix );

			for (int i=0; i<16; ++i)
				fmatrix[i] = (float) lModelTransformMatrix[i];

			mColorShader->setUniformMatrix( mLocationMVP, fmatrix );
			
			FBVector3d vmin, vmax;

			lModel->GetBoundingBox(vmin, vmax);
			FBVector3d diff(vmax[0]-vmin[0], vmax[1]-vmin[1], vmax[2]-vmin[2]);

			diff[0] = std::max( diff[0], 0.001 );
			diff[1] = std::max( diff[1], 0.001 );
			diff[2] = std::max( diff[2], 0.001 );

			mColorShader->setUniformVector( mLocationCubeMin, (float)vmin[0], (float)vmin[1], (float)vmin[2], 1.0f);
			mColorShader->setUniformVector( mLocationCubeSize, (float)diff[0], (float)diff[2], (float)diff[2], 1.0f);
			
		}

//        const int lSubRegionCount = lModelVertexData->GetSubRegionCount();

        //Calling PushZDepthClipOverride() disables the OpenGL custom clip-plane (used for Z-Depth HideFront selection) if this model is selected using
        //Z-Depth HideFront selection tool. This is so that the model is not clipped, i.e., remains visible.
        //lModelVertexData->PushZDepthClipOverride();

		//lModelVertexData->VertexArrayMappingRequest();
		//lModelVertexData->VertexArrayMappingRelease();


		// TODO: use gpu offset pointer (needed for GPU skinning feature)

        lModelVertexData->EnableOGLVertexData();        //Bind Vertex Array or Vertex Buffer Object.

		if (false == pRenderOptions->IsIDBufferRendering())
		{
			

			unsigned int vboId = lModelVertexData->GetVertexArrayVBOId(kFBGeometryArrayID_Point);

			
			glBindBuffer(GL_ARRAY_BUFFER, vboId);
			glVertexAttribPointer(0, 4, GL_FLOAT, false, 0, 0);
		
			//glBindBuffer(GL_ARRAY_BUFFER, vboId);
			glVertexAttribPointer(1, 4, GL_FLOAT, false, 0, 0);

		}

        lModelVertexData->VertexArrayMappingRequest();

		const int *indexArray = lModelVertexData->GetIndexArray();
		FBVertex *posArray = (FBVertex*) lModelVertexData->GetVertexArray(kFBGeometryArrayID_Point);

		glBegin(GL_TRIANGLES);

		for(int i=0; i<lModelVertexData->GetSubPatchCount(); ++i)
        {
			const int offset = lModelVertexData->GetSubPatchIndexOffset(i);
            const int size = lModelVertexData->GetSubPatchIndexSize(i);

			for (int j=0; j<size; j+=3)
			{

				glVertex3fv( posArray[indexArray[offset+j]] );
				glVertex3fv( posArray[indexArray[offset+j+1]] );
				glVertex3fv( posArray[indexArray[offset+j+2]] );

			}

		}

		glEnd();

        lModelVertexData->VertexArrayMappingRelease();

		/*
        for(int lSubRegionIdx = 0; lSubRegionIdx < lSubRegionCount; ++lSubRegionIdx)
        {
           
			// ID Buffer rendering, simply draw geometry. 
			lModelVertexData->DrawSubRegion(lSubRegionIdx); // draw all the sub patches inside this sub regions.
        }
		*/
		

        lModelVertexData->DisableOGLVertexData();   //Unbind Vertex Array or Vertex Buffer Object.
        //lModelVertexData->PopZDepthClipOverride();  //Re-enables Z-Depth HideFront clip-plane if it was previously disabled via PushZDepthClipOverride().
    
		if (pRenderOptions->IsIDBufferRendering())
		{
			glPopMatrix();
		}
    }

	//
	if (false == pRenderOptions->IsIDBufferRendering())
	{
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		mColorShader->UnBind();
	}
	
}

/** Demo a simple rendering approach by iterating through each model in the scene at frame */
void ColorsRendererCallback::RenderWithSimpleIterating(FBRenderOptions* pRenderOptions)
{
    FBRenderer* lRenderer = FBSystem::TheOne().Renderer;
    FBCamera* lCamera = lRenderer->CurrentCamera;
    
    FBMatrix lCameraMVPMatrix;
    lCamera->GetCameraMatrix( lCameraMVPMatrix, kFBModelViewProj );

    if (pRenderOptions->IsIDBufferRendering() == false)
    {
        if (CustomLightingSetting)
        {
            //
            // Plugin developer could query FBRenderer::DisplayableLightCount property 
            // and FBRenderer::GetDisplayableLight() to get lights in the scene, and 
            // setup them properly according to their need. 
            // However here we only demo no lighting case for simplicity.
            //
            if(pRenderOptions->IsOfflineRendering())
                glColor3d(1.0, 0.0, 0.8); //!< Draw purple in offline rendering
            else
                glColor3d(0.0, 1.0, 0.8); //!< Draw cyan in the viewer

            glDisable(GL_LIGHTING); 
        }
        else
        {
            //
            // Ask MotionBuilder to set up lights in default way.
            //
            lRenderer->OGLSetupSceneLights(*pRenderOptions); 

            glEnable(GL_LIGHTING); 
        }
    }

    glMatrixMode(GL_MODELVIEW);

    //////////////////////////////////////////////////////////////////////////
    /// Adjust rendering pref setting. 
    //////////////////////////////////////////////////////////////////////////
    switch (pRenderOptions->GetViewerOptions()->RenderCallbackPrefIndex())
    {
    case 0: 
        //Point Cloud
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT );
        glPointSize( 3.0 );
        break;
    case 1: 
        // Wire Frame
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE );
        glLineWidth( 2.0 );
        break;
    case 2: 
        // Polygon
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
    }

    //
    // Loop though each model, and render accordingly.
    //
    int llDisplayGeometryCount = lRenderer->DisplayableGeometryCount;
    while(llDisplayGeometryCount--)
    {
        FBModel* lModel = lRenderer->GetDisplayableGeometry( llDisplayGeometryCount );
        FBModelVertexData* lModelVertexData = lModel->ModelVertexData;

        // Skips model if it is not visible (affected by its self visibility property, visibility inheritance and parent & owner's visibility status),
        // or if it is not supposed to be drawn (e.g., deformed vertices are not yet ready for drawing, or if model is hidden via Z-Depth Selection tool)
        if( !lModel->IsVisible() || !lModelVertexData->IsDrawable() )  
            continue;

        FBMatrix lModelTransformMatrix;
        lModel->GetMatrix(lModelTransformMatrix, kModelTransformation_Geometry);

        //
        // Early frustum culling is important for improving rendering performance. 
        // and provide hint to allow MotionBuilder core evaluation engine not 
        // perform heavy deformation task when the model is outside of view port 
        // for a few frames.
        //
        if (NoFrustumculling == false)
        {
            if (CustomFrstumCulling == false)
            {
                //
                // Use MotionBuilder Built-in Camera Frustum Culling mechanism.
                //

                if (lRenderer->IsModelInsideCameraFrustum(lModel, lCamera) == false)
                    continue;
            }
            else
            {
                //
                // Perform Custom Frustum Culling;
                //

                FBVector3d lMin, lMax;;
                lModel->GetBoundingBox( lMin, lMax );  //!< Get Bounding Box. 
                FBVector4d lBBoxCenter((lMin[0] + lMax[0]) * 0.5, (lMin[1] + lMax[1]) * 0.5, (lMin[2] + lMax[2]) * 0.5);
                FBVectorMatrixMult(lBBoxCenter, lModelTransformMatrix, lBBoxCenter);
                FBVectorMatrixMult(lBBoxCenter, lCameraMVPMatrix, lBBoxCenter);

                // This is coarse frustum culling for demo purpose, only determine if the BBox Center is inside viewport or not.
                if (lBBoxCenter[0] < -1 || lBBoxCenter[1] < -1 || lBBoxCenter[0] >1 || lBBoxCenter[1] > 1) 
                    continue;
            }
        }


        //If IDBuffer rendering requested (for display or picking), set model color to be the Model's Unique Color ID.
        if (pRenderOptions->IsIDBufferRendering())
        {
            FBColor lUniqueColorId = lModel->UniqueColorId;
            glColor3dv(lUniqueColorId);
        }

        glPushMatrix();
        glMultMatrixd(lModelTransformMatrix);

        const int lSubRegionCount = lModelVertexData->GetSubRegionCount();

        //Calling PushZDepthClipOverride() disables the OpenGL custom clip-plane (used for Z-Depth HideFront selection) if this model is selected using
        //Z-Depth HideFront selection tool. This is so that the model is not clipped, i.e., remains visible.
        lModelVertexData->PushZDepthClipOverride();

        lModelVertexData->EnableOGLVertexData();        //Bind Vertex Array or Vertex Buffer Object.

        for(int lSubRegionIdx = 0; lSubRegionIdx < lSubRegionCount; ++lSubRegionIdx)
        {
            if (pRenderOptions->IsIDBufferRendering())
            {
                // ID Buffer rendering, simply draw geometry. 
                lModelVertexData->DrawSubRegion(lSubRegionIdx); // draw all the sub patches inside this sub regions.
            }
            else
            {
                //For normal scene rendering. need to setup light, material, texture and etc.,

                if( CustomLightingSetting )
                {
                    lModelVertexData->DrawSubRegion(lSubRegionIdx); // draw all the sub patches inside this sub regions.
                }
                else 
                {
                    if (lModel->Materials.GetCount() > lSubRegionIdx)
                    {
                        FBMaterial* lMaterial = lModel->Materials[lSubRegionIdx];
                        lMaterial->OGLInit(); // Setup OpenGL fixed pipeline's ambient, diffuse, emissive, specular and shininess materials parameters.
                        FBTexture* lDiffuseTex = lMaterial->GetTexture( kFBMaterialTextureDiffuse );
                        if (lDiffuseTex)
                        {
                            lDiffuseTex->OGLInit();	//Bind color texture and it's associated matrix.
                            lModelVertexData->EnableOGLUVSet(lDiffuseTex->Mapping); //Bind the UVSet. 
                            glEnable( GL_TEXTURE_2D );
                        }

                        lModelVertexData->DrawSubRegion(lSubRegionIdx); // draw all the sub patches inside this sub regions.

                        if (lDiffuseTex)
                        {
                            lModelVertexData->DisableOGLUVSet(); //Unbind the UVSet.
                            glDisable(GL_TEXTURE_2D);
                        }
                    }
                    else
                    {
                        lModelVertexData->DrawSubRegion(lSubRegionIdx); // draw all the sub patches inside this sub regions.
                    }
                }
            }
        }

        lModelVertexData->DisableOGLVertexData();   //Unbind Vertex Array or Vertex Buffer Object.
        lModelVertexData->PopZDepthClipOverride();  //Re-enables Z-Depth HideFront clip-plane if it was previously disabled via PushZDepthClipOverride().
        glPopMatrix();
    }
}

/** Demo a relative complicated approach by caching/updating the needed scene graph info.*/
void ColorsRendererCallback::RenderWithCachedInfo(FBRenderOptions* pRenderOptions)
{
    if (mNeedFullRestructure)
    {
        mNeedFullRestructure = false;
        CacheSceneFullRestructure();
    }

    // 
    //  For demo simplicity, we render all the cache scene render items. 
    //    Refer RenderWithSimpleIterating() for more info about the other factors which should be considered 
    //    when implementing full feature renderer to better follow MotionBuilder's existing viewport display / interaction rules. 
    //

    FBRenderer* lRenderer = FBSystem::TheOne().Renderer;
    lRenderer->OGLSetupSceneLights(*pRenderOptions); 
    glEnable(GL_LIGHTING); 

    _ORModelRenderItemListPerMaterialMap::const_iterator lMaterialIter = mModelRenderItemListPerMaterialMap.cbegin();

    glMatrixMode(GL_MODELVIEW);

    while (lMaterialIter != mModelRenderItemListPerMaterialMap.cend())
    {
        FBMaterial* lMaterial = lMaterialIter->first;
        _ORModelRenderItemList* lModelRenderItemList = lMaterialIter->second;
        lMaterialIter++;

        lMaterial->OGLInit();

        _ORModelRenderItemList::const_iterator lRenderItemIter = lModelRenderItemList->cbegin();
        
        while (lRenderItemIter != lModelRenderItemList->cend())
        {
            FBModel* lModel = lRenderItemIter->first;
            int lRenderItemIndex = lRenderItemIter->second;
            lRenderItemIter++;

            FBModelVertexData* lModelVertexData = lModel->ModelVertexData;

            FBMatrix lModelTransformMatrix;
            lModel->GetMatrix(lModelTransformMatrix, kModelTransformation_Geometry);

            glPushMatrix();
            glMultMatrixd(lModelTransformMatrix);

            lModelVertexData->EnableOGLVertexData();            //Bind Vertex Array or Vertex Buffer Object.

            lModelVertexData->DrawSubRegion(lRenderItemIndex);  // draw all the sub patches inside this sub regions.

            lModelVertexData->DisableOGLVertexData();           //Unbind Vertex Array or Vertex Buffer Object.
            glPopMatrix();
        }    
    }
}


void ColorsRendererCallback::CacheSceneGraphSet(bool pValue)
{
    CacheSceneGraph.SetPropertyValue(pValue);
    CacheSceneReset();
}

void ColorsRendererCallback::CacheSceneReset()
{
    if (! mNeedFullRestructure )
    {
        mNeedFullRestructure = true;

        _ORModelRenderItemListPerMaterialMap::const_iterator lIter = mModelRenderItemListPerMaterialMap.cbegin();

        //Release resource.
        while (lIter != mModelRenderItemListPerMaterialMap.cend())
        {
            delete lIter->second;
            lIter++;
        }

        mModelRenderItemListPerMaterialMap.clear();
    }
}

void ColorsRendererCallback::CacheSceneFullRestructure()
{    
    //
    // Fully restructure the cached scene graph recursively. 
    //
    FBRenderer* lRenderer = FBSystem::TheOne().Renderer;

    for (int lIndex = 0, lCount = lRenderer->DisplayableGeometryCount; lIndex < lCount; lIndex++)
    {
        FBModel* lModel = lRenderer->GetDisplayableGeometry(lIndex);

        if ( lModel->HasCustomDisplay() == false ) 
        {
            FBModelVertexData* lModelVertexData = lModel->ModelVertexData;

            // Iterate through each render item. 
            for (int lRenderItemIndex = 0, lRenderItemCount = lModelVertexData->GetSubRegionCount(); lRenderItemIndex < lRenderItemCount; lRenderItemIndex++ )
            {
                FBMaterial* lMaterial = lModelVertexData->GetSubRegionMaterial(lRenderItemIndex);
                
                _ORModelRenderItemList* lModelRenderItemList = mModelRenderItemListPerMaterialMap[lMaterial];
                if (lModelRenderItemList == NULL)
                {
                    lModelRenderItemList = new _ORModelRenderItemList;
                    mModelRenderItemListPerMaterialMap[lMaterial] = lModelRenderItemList;
                }

                lModelRenderItemList->push_back(_ORModelRenderItem::pair(lModel , lRenderItemIndex));
            }
        }
    }
}

void ColorsRendererCallback::EventSceneChange( HISender pSender, HKEvent pEvent )
{
    if (! CacheSceneGraph)
        return;
    
    FBEventSceneChange lEvent( pEvent );

    switch( lEvent.Type )
    {
    case kFBSceneChangeLoadBegin:
    case kFBSceneChangeClearBegin:
    case kFBSceneChangeMergeTransactionBegin:
        {
            // Lots of changes coming, stop incremental scene graph cache update. 
            CacheSceneReset();
        }
        break;
    case kFBSceneChangeTransactionBegin:
        {
            // There are medium volume of changes coming, User could choose either full or incremental rebuild.
            CacheSceneReset();
        }
    default:
        break;
    }
}

void ColorsRendererCallback::EventConnNotify(HISender pSender, HKEvent pEvent)
{
    if (! CacheSceneGraph || mNeedFullRestructure )
        return;

    FBEventConnectionNotify		lEvent(pEvent);
    FBString					lText;
    char						lTmp[32];
    FBPlug*						lPlug;
    static int					lCounter = 0;

    sprintf( lTmp, "%d", lCounter );
    lText += lTmp;
    lText += ": Action=";
    lText += FBGetConnectionActionString(lEvent.Action);
    if (lEvent.SrcPlug)
    {
        if (lEvent.SrcPlug->Is(FBProperty::TypeInfo))
        {
            lPlug = lEvent.SrcPlug->GetOwner();
            lText += ",  Src=";
            lText += ((FBComponent*)lPlug)->Name;
            lText += ".";
            lPlug = lEvent.SrcPlug;
            lText += ((FBProperty*)lPlug)->GetName();
            lText += "(";
            sprintf( lTmp, "%d", (int)lEvent.SrcIndex );
            lText += lTmp;
            lText += ")";
        }
        else if (lEvent.SrcPlug->Is(FBComponent::TypeInfo))
        {
            lPlug = lEvent.SrcPlug;
            lText += ",  Src=";
            lText += ((FBComponent*)lPlug)->Name;
            lText += "(";
            sprintf( lTmp, "%d", (int)lEvent.SrcIndex );
            lText += lTmp;
            lText += ")";
        }
    }

    if (lEvent.DstPlug)
    {
        if (lEvent.DstPlug->Is(FBProperty::TypeInfo))
        {
            lPlug = lEvent.DstPlug->GetOwner();
            lText += ",  Dst=";
            lText += ((FBComponent*)lPlug)->Name;
            lText += ".";
            lPlug = lEvent.DstPlug;
            lText += ((FBProperty*)lPlug)->GetName();
        }
        else if (lEvent.DstPlug->Is(FBComponent::TypeInfo))
        {
            lPlug = lEvent.DstPlug;
            lText += ",  Dst=";
            lText += ((FBComponent*)lPlug)->Name;
        }
    }

    if (lEvent.NewPlug)
    {
        if (lEvent.NewPlug->Is(FBProperty::TypeInfo))
        {
            lPlug = lEvent.NewPlug->GetOwner();
            lText += ",  New=";
            lText += ((FBComponent*)lPlug)->Name;
            lText += ".";
            lPlug = lEvent.NewPlug;
            lText += ((FBProperty*)lPlug)->GetName();
        }
        else if (lEvent.NewPlug->Is(FBComponent::TypeInfo))
        {
            lPlug = lEvent.NewPlug;
            lText += ",  New=";
            lText += ((FBComponent*)lPlug)->Name;
        }
    }

    lText += ",  Type=";
    lText += FBGetConnectionTypeString(lEvent.ConnectionType);
    lText += "\n";

    /*  In windows by passing the argument "-console" when launching MotionBuilder, it is possible to
    *   print formatted messages, as a printf would. On Mac OSX / Linux, the strings are simply sent to stderr.
    *   Upon Python Editor open, those formatted messages will be forwarded to Python console as well.
    *
    *   Global trace detailed level (kFBNORMAL_TRACE by default) could be controlled via 
    *   FBTraceSetLevel() / FBTraceGetLevel() functions, which affect all the trace output targets, 
    *   User could use pythonidelib.SetTraceLevel() / GetTraceLevel() to further adjust python console's 
    *   trace level as well (kFBNO_TRACE by default).
    */
    FBTrace(lText);
}

void ColorsRendererCallback::EventConnStateNotify(HISender pSender, HKEvent pEvent)
{
    if (! CacheSceneGraph || mNeedFullRestructure )
        return;

    FBEventConnectionStateNotify	lEvent(pEvent);
    FBString						lText;
    char							lTmp[32];
    FBPlug*							lPlug;
    static int						lCounter = 0;

    sprintf( lTmp, "%d", lCounter );
    lText += lTmp;
    lText += ": Action=";
    lText += FBGetConnectionActionString(lEvent.Action);

    if (lEvent.Plug)
    {
        if (lEvent.Plug->Is(FBProperty::TypeInfo))
        {
            lPlug = lEvent.Plug->GetOwner();
            lText += ",  Plug=";
            lText += ((FBComponent*)lPlug)->Name;
            lText += ".";
            lPlug = lEvent.Plug;
            lText += ((FBProperty*)lPlug)->GetName();
        }
        else if (lEvent.Plug->Is(FBComponent::TypeInfo))
        {
            lPlug = lEvent.Plug;
            lText += ",  Plug=";
            lText += ((FBComponent*)lPlug)->Name;
        }
    }
    
    lText += ",  Data=";
    sprintf(lTmp, "0x%llX", reinterpret_cast<uintptr_t>(lEvent.GetData()));
    lText += lTmp;
    lText += ",  OldData=";
    sprintf(lTmp, "0x%llX", reinterpret_cast<uintptr_t>(lEvent.GetOldData()));
    lText += lTmp;

    FBTrace(lText);
}

void ColorsRendererCallback::EventConnDataNotify(HISender pSender, HKEvent pEvent)
{
    if (! CacheSceneGraph || mNeedFullRestructure )
        return;

    FBEventConnectionDataNotify	lEvent(pEvent);
    FBString					lText;
    char						lTmp[32];
    FBPlug*						lPlug;
    static int					lCounter = 0;

    sprintf( lTmp, "%d", lCounter );
    lText += lTmp;
    lText += ": Action=";
    lText += FBGetConnectionActionString(lEvent.Action);
    if( lEvent.Plug )
    {
        if (lEvent.Plug->Is(FBProperty::TypeInfo))
        {
            lPlug = lEvent.Plug->GetOwner();
            lText += ",  Plug=";
            lText += ((FBComponent*)lPlug)->Name;
            lText += ".";
            lPlug = lEvent.Plug;
            lText += ((FBProperty*)lPlug)->GetName();
        }
        else if (lEvent.Plug->Is(FBComponent::TypeInfo))
        {
            lPlug = lEvent.Plug;
            lText += ",  Plug=";
            lText += ((FBComponent*)lPlug)->Name;
        }
    }
    lText += ",  Data=";
    sprintf(lTmp, "0x%llX", reinterpret_cast<uintptr_t>(lEvent.GetData()));
    lText += lTmp;
    lText += ",  OldData=";
    sprintf(lTmp, "0x%llX", reinterpret_cast<uintptr_t>(lEvent.GetOldData()));
    lText += lTmp;

    FBTrace(lText);
}

void ColorsRendererCallback::OnPerFrameSynchronizationCallback(HISender pSender, HKEvent pEvent)
{
    FBEventEvalGlobalCallback lFBEvent(pEvent);
    if (lFBEvent.GetTiming() == kFBGlobalEvalCallbackSyn)
    {
        ///
        // This callback occurs when both rendering and evaluation pipeline are stopped,
        // plugin developer could add some lightweight scene modification tasks here
        // and no need to worry complicated thread issues. 
        //
    }
}

void ColorsRendererCallback::OnPerFrameRenderingPipelineCallback(HISender pSender, HKEvent pEvent)
{
    FBEventEvalGlobalCallback lFBEvent(pEvent);
    switch(lFBEvent.GetTiming())
    {
    case kFBGlobalEvalCallbackBeforeRender:
        //
        // This callback occurs immediately after clearing GL back buffer in render pipeline.
        //

        break;
    case kFBGlobalEvalCallbackAfterRender:
        //
        // This callback occurs just before swapping GL back/front buffers. 
        // User could do some special effect, HUD or buffer download (via PBO) here. 
        //

        break;
    default:
        break;
    }
}

void ColorsRendererCallback::OnPerFrameEvaluationPipelineCallback(HISender pSender, HKEvent pEvent)
{
    //
    // Add custom evaluation tasks here will improve the performance if with 
    // parallel pipeline (default) because they occur in the background thread.
    //

    FBEventEvalGlobalCallback lFBEvent(pEvent);
    switch(lFBEvent.GetTiming())
    {
    case kFBGlobalEvalCallbackBeforeDAG:
        //
        // We could add custom tasks here before MoBu perform standard evaluation tasks. 
        //

        break;
    case kFBGlobalEvalCallbackAfterDAG:
        //
        // We could add custom tasks here after MoBu finish standard animation & deformation evaluation tasks. 
        //

        break;
    case kFBGlobalEvalCallbackAfterDeform:
        //
        // We could add custom tasks here after MoBu finish standard deformation evaluation tasks 
        // (if not use GPU deformation). E.g, update the accelerated spatial scene structure for rendering. 
        //

        break;
    default:
        break;
    }    
}
