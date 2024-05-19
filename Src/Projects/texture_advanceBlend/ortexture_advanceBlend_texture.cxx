
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ortexture_advanceBlend_texture.cxx
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/OpenMoBu
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "ortexture_advanceBlend_texture.h"
#include <gl\glew.h>

FBClassImplementation( ORTextureAdvanceBlend );					        //Register class
FBStorableCustomTextureImplementation( ORTextureAdvanceBlend, Texture );	//Register to the store/retrieve system
FBShadingElementClassImplementationWithLabel( ORTextureAdvanceBlend, "Advanced Blend", "browsing/template_layeredtexture.png" );	//Register to the asset browser shading element category folder.


bool glewFirstRun = true;

/************************************************
 *	Constructor.
 ************************************************/
ORTextureAdvanceBlend::ORTextureAdvanceBlend( const char* pName, HIObject pObject ) 
	: FBLayeredTexture( pName, pObject )
{
	FBClassInit;
	mSupported = true;
	if (glewFirstRun)
	{
		glewInit();
		glewFirstRun = false;

		if (glewIsExtensionSupported( "GL_NV_blend_equation_advanced" ) == false )
		{
			FBMessageBox( "Texture Advanced", "Graphics extension with an advanced list of modes are not supported!", "Ok" );
			mSupported = false;
		}
	}
}

static void ORTextureAdvanceBlend_CustomCompositionSet(HIObject pMbObject, bool pValue)
{
    ORTextureAdvanceBlend* pFbObject = FBCast<ORTextureAdvanceBlend>( pMbObject );
    pFbObject->CustomComposition.SetPropertyValue(pValue);
    pFbObject->SetLayerConfigDirty();
}

static void ORTextureAdvanceBlend_UseMaskSet(HIObject pMbObject, bool pValue)
{
    ORTextureAdvanceBlend* pFbObject = FBCast<ORTextureAdvanceBlend>( pMbObject );
    pFbObject->UseMask.SetPropertyValue(pValue);
    pFbObject->SetLayerConfigDirty();
}

static void ORTextureAdvanceBlend_BlendMode2Set(HIObject pMbObject, ECompositeBlendType pValue)
{
    ORTextureAdvanceBlend* pFbObject = FBCast<ORTextureAdvanceBlend>( pMbObject );
    pFbObject->BlendMode.SetPropertyValue(pValue);
    pFbObject->SetLayerConfigDirty();
}

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ORTextureAdvanceBlend::FBCreate()
{
	FBPropertyInitTextureConnectable(this, BackgroundColor, "Background Color" );
    FBPropertyPublish(this, CustomComposition, "CustomComposition", nullptr, ORTextureAdvanceBlend_CustomCompositionSet);

	FBPropertyPublish(this, BlendMode, "Blend Mode Advanced", nullptr, ORTextureAdvanceBlend_BlendMode2Set );
	
	FBPropertyPublish(this, UseMask, "Use Mask", nullptr, ORTextureAdvanceBlend_UseMaskSet );
	FBPropertyPublish(this, Mask, "Mask", nullptr, nullptr );
	

	BackgroundColor = FBColorAndAlpha(1.0, 1.0, 1.0, 1.0);
    CustomComposition = true;

	BlendMode = eCompositeBlendAdd;

	UseMask = false;

	Mask.SetFilter( FBTexture::GetInternalClassId() );
	Mask.SetSingleConnect(true);

	mLoaded = true;

    return true;
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void ORTextureAdvanceBlend::FBDestroy()
{
	ParentClass::FBDestroy();
}

bool ORTextureAdvanceBlend::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
    ParentClass::FbxStore(pFbxObject, pStoreWhat);

	return true;
}

bool ORTextureAdvanceBlend::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
    ParentClass::FbxRetrieve(pFbxObject, pStoreWhat);

	return true;
}

bool ORTextureAdvanceBlend::PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData,void* pDataOld,int pDataSize)
{
    if (pThis == &BackgroundColor)
    {
        switch (pAction)
        {
        case kFBCandidated:
            {
                SetLayerConfigDirty();
            }
            break;
        default:
            break;
        }
    }

    return ParentClass::PlugDataNotify(pAction, pThis, pData, pDataOld, pDataSize);
}

bool ORTextureAdvanceBlend::EvaluateAnimationNodes( FBEvaluateInfo* pEvaluateInfo )
{
    ParentClass::EvaluateAnimationNodes(pEvaluateInfo);

    if (CustomComposition && BackgroundColor.IsAnimated())
    {
        // Compute animatable property value in background evaluation thread.
        FBPropertyAnimatableColorAndAlpha::ValueType lTmpValue;
        BackgroundColor.GetData(lTmpValue.mValue, sizeof(lTmpValue.mValue), pEvaluateInfo);

        // Trigger composition per frame.
        SetLayerConfigDirty();
    }

    return true;
}

void ORTextureAdvanceBlend::TextureLayerComposition(FBTime pTime,FBTime pTimeInCurrentTimeRef, int pWidth, int pHeight)
{
    // 
    // Here we simply demo the default and customized the texture layers blend operations. 
    //  Actually nothing is preventing user to do a more sophisticated real-time composition (keying and etc.,) 
    //  or even a full scene (or shadow map) RTT for advanced fancy tasks. 
    //
	if (Layers.GetCount() <= 1 || !mSupported || !CustomComposition)
    {
        ParentClass::TextureLayerComposition(pTime, pTimeInCurrentTimeRef, pWidth, pHeight);
    }
	else if (mLoaded && Layers.GetCount() == 2)
    {
        //
        //Render-To-Texture already setup. 
        //

        // Push GL states.
        glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT);

        // Clear Buffers.

        // Default implementation use BackgroundColor property. Here AuxLayer is used for demo purpose. 
        // FBColorAndAlpha lBgColor = BackgroundColor;
        const FBColorAndAlpha lBgColor = BackgroundColor;
        glClearColor(lBgColor[0], lBgColor[1], lBgColor[2], lBgColor[3]);
        glClear(GL_COLOR_BUFFER_BIT);

        glViewport(0,0, pWidth, pHeight);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 1, 1, 0, -1, 1);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glDisable(GL_FOG);
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
		
		// base, back layer

		const ECompositeShader shaderType = ECompositeShader(eCompositeShaderBlendNormal + BlendMode);

		FBTexture *lMask = (Mask.GetCount()>0) ? FBCast<FBTexture>(Mask.GetAt(0)) : nullptr;
		const bool lUseMask = UseMask && (lMask != nullptr);

		if ( !mShaderManager.CheckAndLoadShader( shaderType, UseMask ) )
		{
			mLoaded = false;
			return;
		}

		FBTexture* lTexture0 = Layers[0];
		FBTexture* lTexture1 = Layers[1];
		
        // Binding the texture with proper parameters and matrix.
        
		const GLuint textureId0 = GetTextureId(lTexture0);
		const GLuint textureId1 = GetTextureId(lTexture1);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureId0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textureId1);

		if (lUseMask)
		{
			const GLuint maskId = GetTextureId(lMask);

			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, maskId);
		}

		mShaderManager.Bind(shaderType, lUseMask);
		float blendColor[4] = {1.0, 1.0, 1.0, 1.0};
		mShaderManager.SetBlendUniforms(Layers[1]->Alpha, false, blendColor);

        // TODO: replace this. Draw quad for blending.
        glBegin(GL_POLYGON);
        glTexCoord2f(0.0, 1.0);
        glVertex2f(0.0, 0.0);
        glTexCoord2f(0.0, 0.0);
        glVertex2f(0.0, 1.0);
        glTexCoord2f(1.0, 0.0);
        glVertex2f(1.0, 1.0);
        glTexCoord2f(1.0, 1.0);
        glVertex2f(1.0, 0.0);						
        glEnd();
		
		mShaderManager.UnBind();

		if (lUseMask)
		{
			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		//

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        // Pop GL states.
        glPopAttrib();
    }
}

void ORTextureAdvanceBlend::SetUpBlendMode( const ECompositeBlendType mode )
{
	switch( mode )
	{

	case eCompositeBlendNormal: glBlendEquation( GL_DST_NV );
		break;
	case eCompositeBlendLighten: glBlendEquation( GL_LIGHTEN_NV );
		break;
	case eCompositeBlendDarken: glBlendEquation( GL_DARKEN_NV );
		break;
	case eCompositeBlendMultiply: glBlendEquation( GL_MULTIPLY_NV );
		break;
	case eCompositeBlendAverage: glBlendEquation( GL_HARDMIX_NV );
		break;
	case eCompositeBlendAdd: glBlendEquation( GL_PLUS_NV );
		break;
	case eCompositeBlendSubstract: glBlendEquation( GL_EXCLUSION_NV );
		break;
	case eCompositeBlendDifference: glBlendEquation( GL_DIFFERENCE_NV );
		break;
	case eCompositeBlendNegation: glBlendEquation( GL_EXCLUSION_NV );
		break;
	case eCompositeBlendExclusion: glBlendEquation( GL_EXCLUSION_NV );
		break;
	case eCompositeBlendScreen: glBlendEquation( GL_SCREEN_NV );
		break;
	case eCompositeBlendOverlay: glBlendEquation( GL_OVERLAY_NV );
		break;
	case eCompositeBlendSoftLight: glBlendEquation( GL_SOFTLIGHT_NV);
		break;
	case eCompositeBlendHardLight: glBlendEquation( GL_HARDLIGHT_NV );
		break;
	case eCompositeBlendColorDodge: glBlendEquation( GL_COLORDODGE_NV );
		break;
	case eCompositeBlendColorBurn: glBlendEquation( GL_COLORBURN_NV );
		break;
	case eCompositeBlendLinearDodge: glBlendEquation( GL_LINEARDODGE_NV );
		break;
	case eCompositeBlendLinearBurn: glBlendEquation( GL_LINEARBURN_NV );
		break;
	case eCompositeBlendLinearLight: glBlendEquation( GL_LINEARLIGHT_NV );
		break;
	case eCompositeBlendVividLight: glBlendEquation( GL_VIVIDLIGHT_NV );
		break;
	case eCompositeBlendPinLight: glBlendEquation( GL_PINLIGHT_NV );
		break;
	case eCompositeBlendHardMix: glBlendEquation( GL_HARDMIX_NV );
		break;
	case eCompositeBlendReflect: glBlendEquation( GL_PLUS_NV );
		break;
	case eCompositeBlendGlow: glBlendEquation( GL_PLUS_NV );
		break;
	case eCompositeBlendPhoenix: glBlendEquation( GL_PLUS_NV );
		break;
	}

}


const GLuint ORTextureAdvanceBlend::GetTextureId( FBTexture *pTexture )
{
	GLuint id = pTexture->TextureOGLId;

	if (0 == id)
	{
		pTexture->OGLInit();
		id = pTexture->TextureOGLId;
	}

	return id;
}