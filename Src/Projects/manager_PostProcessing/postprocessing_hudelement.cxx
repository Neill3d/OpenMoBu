

/**	\file	orhudelement_template_hudelement.cxx

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- Class declaration
#include "postprocessing_hudelement.h"

#include <fbsdk/fbsdk-opengl.h>
#include <math.h>

FBClassImplementation( ORHUDElementCustom );					                                //!< Register class
FBStorableCustomHUDElementImplementation( ORHUDElementCustom, ORHUDELEMENTCUSTOM__DESCSTR );	//!< Register to the store/retrieve system

/************************************************
 *	Constructor.
 ************************************************/
ORHUDElementCustom::ORHUDElementCustom( const char* pName, HIObject pObject ) : FBHUDElement( pName, pObject )
{
	FBClassInit;
}

bool ORHUDElementCustom::FBCreate()
{
    FBPropertyInitTextureConnectable(this, BackgroundColor, "BackgroundColor" );
    BackgroundColor = FBColorAndAlpha(0.3, 0.1, 0.1, 0.5);

    FBPropertyInitTextureConnectable(this, ModelColor, "ModelColor" );
    ModelColor = FBColor(0.9, 0.8, 0.9);

    return true;
}

void ORHUDElementCustom::FBDestroy()
{
    ParentClass::FBDestroy();
}

void ORHUDElementCustom::GetIntrinsicDimension(FBViewingOptions* pViewingOptins, int& pWidth, int & pHeight)
{
    pWidth = 1;
    pHeight = 1;
}

extern void GRenderAfterRender();

void ORHUDElementCustom::DrawElement(FBViewingOptions* pViewingOption, Rect& pRect)
{
	GRenderAfterRender();

    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_POLYGON_BIT | GL_TRANSFORM_BIT | GL_DEPTH_BUFFER_BIT); 
    {
        glViewport(pRect.mX, pRect.mY, pRect.mWidth, pRect.mHeight);
        
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();

        const double lViewPortDim = 1.0;
        gluOrtho2D(-lViewPortDim, lViewPortDim, -lViewPortDim, lViewPortDim);

        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();

        //////////////////////////////////////////////////////////////////////////
        // Draw Background Rect 
        //////////////////////////////////////////////////////////////////////////
        FBColorAndAlpha lBGColor = BackgroundColor;
        glColor4dv(lBGColor);

        const bool lUseBlendForBackground = lBGColor[3] <= ( 254.0 / 255 );
        if( lUseBlendForBackground )
        {
            glEnable( GL_BLEND );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        }

        glRectd(-lViewPortDim, -lViewPortDim, lViewPortDim, lViewPortDim);

        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

        //////////////////////////////////////////////////////////////////////////
        // Draw the selected model in wire frame
        //////////////////////////////////////////////////////////////////////////
        FBModelList lSelectedModelList;
        FBGetSelectedModels( lSelectedModelList, NULL, true, true );
        if (lSelectedModelList.GetCount() && lSelectedModelList[0]->ModelVertexData)
        {
            FBModelVertexData* lModelVertexData = lSelectedModelList[0]->ModelVertexData;
            const int lSubRegionCount = lModelVertexData->GetSubRegionCount();
            if (lSubRegionCount)
            {
                FBVector3d lMax, lMin; 
                lSelectedModelList[0]->GetBoundingBox( lMin, lMax );

                FBVector3d lModelDim (lMax[0] - lMin[0], lMax[1] - lMin[1], lMax[2] - lMin[2]);
                double lMaxModelDim = sqrt(lModelDim[0]*lModelDim[0] + lModelDim[1]*lModelDim[1] + lModelDim[2]*lModelDim[2]);
                double lScaleFactor = lViewPortDim * 2 * 0.9 / lMaxModelDim;
                FBMatrix lRMatrix;  lSelectedModelList[0]->GetMatrix(lRMatrix, kModelRotation);

                glScalef(lScaleFactor, lScaleFactor, lScaleFactor);
                glMultMatrixd(lRMatrix);

                lModelVertexData->EnableOGLVertexData();  //Bind Vertex Array or Vertex Buffer Object.

                glLineWidth( 1.5 );
                FBColor lModelColor = ModelColor;
                glColor3dv(lModelColor);

                for(int lSubRegionIdx = 0; lSubRegionIdx < lSubRegionCount; ++lSubRegionIdx)
                {
                    lModelVertexData->DrawSubRegion(lSubRegionIdx, true); // Draw all the sub patches's wireframe inside this sub regions.
                }

                lModelVertexData->DisableOGLVertexData(); //Unbind Vertex Array or Vertex Buffer Object.
            }
        }
    }

    //Pop GL attributes.
    glPopAttrib();
}

bool ORHUDElementCustom::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
    return true;
}

bool ORHUDElementCustom::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
    return true;
}






