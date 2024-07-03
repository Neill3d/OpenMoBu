
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
    FBPropertyPublish(this, CustomLightingSetting,  "CustomLightingSetting",    nullptr, nullptr);
    FBPropertyPublish(this, NoFrustumculling,       "NoFrustumculling", nullptr, nullptr);
    FBPropertyPublish(this, CustomFrstumCulling,    "CustomFrustumCulling", nullptr, nullptr);
    
    SupportIDBufferPicking  = true;
    CustomLightingSetting   = false;
    NoFrustumculling        = false;
    CustomFrstumCulling     = true;
    
    // Init data members.
    mAttachCount            = 0;

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

	const FBString fragment_filename( "\\GLSL\\renderer_colorRamp.fsh" );
	const FBString vertex_filename( "\\GLSL\\renderer_colorRamp.vsh" );

    char effectPath[256]{ 0 };
	if (!FindEffectLocation( fragment_filename, effectPath, 256) )
	{
		FreeShader();
		return false;
	}

	// most of shaders share the same simple vertex shader
	if (!mColorShader->LoadShaders( FBString(effectPath, vertex_filename), FBString(effectPath, fragment_filename) ) )
	{
		FreeShader();
		return false;
	}

	if (!InitShaderLocations() )
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
    FBTrace("ColorsRendererCallback::Attach()\n");

    //
    // Prepare for first time. 
    //
    if (mAttachCount == 0)
    {
		if ( !LoadShader() )
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
    FBTrace("ColorsRendererCallback::Detach()\n");

    // Decrease attachment count.
    mAttachCount--;

    // 
    // This callback instance is not be used by any view panes anymore. 
    //
    if (mAttachCount == 0)
    {
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

    FBTrace("ColorsRendererCallback::DetachDisplayContext()\n");
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

#if(PRODUCT_VERSION >= 2023)
    const int currentPane = lRenderer->GetSelectedPaneIndex();
    FBCamera* lCamera = lRenderer->GetCameraInPane(currentPane);
#else
    FBCamera* lCamera = lRenderer->CurrentCamera;
#endif

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

    const int currentPaneIndex = lRenderer->GetSelectedPaneIndex();
    if (currentPaneIndex < 0)
        return;

    FBCamera* lCamera = lRenderer->GetCameraInPane(currentPaneIndex);
    
	if ( FBIS(lCamera, FBCameraSwitcher) )
		lCamera = ( (FBCameraSwitcher*) lCamera )->CurrentCamera;

	if (lCamera == nullptr)
		return;

    FBMatrix lCameraMVPMatrix;
    lCamera->GetCameraMatrix( lCameraMVPMatrix, kFBModelViewProj );

	//
    float fmatrix[16]{ 0.0f };

	if (pRenderOptions->IsIDBufferRendering())
	{
		glDisable(GL_LIGHTING); 
		glMatrixMode(GL_MODELVIEW);
	}
	else
	{
		for (int i=0; i<16; ++i)
			fmatrix[i] = static_cast<float>(lCameraMVPMatrix[i]);
		
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
				fmatrix[i] = static_cast<float>(lModelTransformMatrix[i]);

			mColorShader->setUniformMatrix( mLocationMVP, fmatrix );
			
			FBVector3d vmin, vmax;

			lModel->GetBoundingBox(vmin, vmax);
			FBVector3d diff(vmax[0]-vmin[0], vmax[1]-vmin[1], vmax[2]-vmin[2]);

			diff[0] = std::max( diff[0], 0.001 );
			diff[1] = std::max( diff[1], 0.001 );
			diff[2] = std::max( diff[2], 0.001 );

			mColorShader->setUniformVector( mLocationCubeMin, static_cast<float>(vmin[0]), static_cast<float>(vmin[1]), static_cast<float>(vmin[2]), 1.0f);
			mColorShader->setUniformVector( mLocationCubeSize, static_cast<float>(diff[0]), static_cast<float>(diff[2]), static_cast<float>(diff[2]), 1.0f);
			
		}


        //Calling PushZDepthClipOverride() disables the OpenGL custom clip-plane (used for Z-Depth HideFront selection) if this model is selected using
        //Z-Depth HideFront selection tool. This is so that the model is not clipped, i.e., remains visible.
        //lModelVertexData->PushZDepthClipOverride();

		// TODO: use gpu offset pointer (needed for GPU skinning feature)

        lModelVertexData->EnableOGLVertexData();        //Bind Vertex Array or Vertex Buffer Object.

		if (!pRenderOptions->IsIDBufferRendering())
		{
			unsigned int vboId = lModelVertexData->GetVertexArrayVBOId(kFBGeometryArrayID_Point);

			glBindBuffer(GL_ARRAY_BUFFER, vboId);
			glVertexAttribPointer(0, 4, GL_FLOAT, false, 0, 0);
		
			//glBindBuffer(GL_ARRAY_BUFFER, vboId);
			glVertexAttribPointer(1, 4, GL_FLOAT, false, 0, 0);

		}

        const unsigned int elementBuffer = lModelVertexData->GetIndexArrayVBOId();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);

        bool useDrawingFallback = false;

        for (int i = 0; i < lModelVertexData->GetSubPatchCount(); ++i)
        {
            bool isOptimized = false;
            const FBGeometryPrimitiveType primitiveType = lModelVertexData->GetSubPatchPrimitiveType(i, &isOptimized);

            if (primitiveType != FBGeometryPrimitiveType::kFBGeometry_TRIANGLES
                && primitiveType != FBGeometryPrimitiveType::kFBGeometry_TRIANGLE_FAN
                && primitiveType != FBGeometryPrimitiveType::kFBGeometry_TRIANGLE_STRIP)
            {
                useDrawingFallback = true;
                break;
            }
        }

        int* indexArray = nullptr;
        FBVertex* posArray = nullptr;

        if (useDrawingFallback)
        {
            lModelVertexData->VertexArrayMappingRequest();

            indexArray = lModelVertexData->GetIndexArray();
            posArray = static_cast<FBVertex*>(lModelVertexData->GetVertexArray(kFBGeometryArrayID_Point));
        }

        // render
		for(int i=0; i<lModelVertexData->GetSubPatchCount(); ++i)
        {
            bool isOptimized = false;
            const FBGeometryPrimitiveType primitiveType = lModelVertexData->GetSubPatchPrimitiveType(i, &isOptimized);

            const int offset = lModelVertexData->GetSubPatchIndexOffset(i);
            const int size = lModelVertexData->GetSubPatchIndexSize(i);

            switch (primitiveType)
            {
            case FBGeometryPrimitiveType::kFBGeometry_TRIANGLES:
                glDrawRangeElements(GL_TRIANGLES, offset, offset + size, size, GL_UNSIGNED_INT, nullptr);
                break;
            case FBGeometryPrimitiveType::kFBGeometry_TRIANGLE_FAN:
                glDrawRangeElements(GL_TRIANGLE_FAN, offset, offset + size, size, GL_UNSIGNED_INT, nullptr);
                break;
            case FBGeometryPrimitiveType::kFBGeometry_TRIANGLE_STRIP:
                glDrawRangeElements(GL_TRIANGLE_STRIP, offset, offset + size, size, GL_UNSIGNED_INT, nullptr);
                break;
            case FBGeometryPrimitiveType::kFBGeometry_QUADS:
            {
                // fallback
                glBegin(GL_QUADS);

                for (int j = 0; j < size; j += 4)
                {
                    glVertex3fv(posArray[indexArray[offset + j]]);
                    glVertex3fv(posArray[indexArray[offset + j + 1]]);
                    glVertex3fv(posArray[indexArray[offset + j + 2]]);
                    glVertex3fv(posArray[indexArray[offset + j + 3]]);
                }

                glEnd();
            } break;
            default:
                FBTrace("Not supported primitive type %d for color renderer\n", static_cast<int>(primitiveType));
            }
                
		}

        if (useDrawingFallback)
        {
            lModelVertexData->VertexArrayMappingRelease();
        }

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
