
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: model_collision_terrain.cxx
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "model_collision_terrain.h"

#include "checkglerror.h"
#include "ParticlesDrawHelper.h"
#include "math3d.h"
#include "FBCommon.h"
#include "FileUtils.h"

#include "Shader_ParticleSystem.h"

using namespace GPUParticles;

FBClassImplementation( CollisionTerrain );								                //!< Register class
FBStorableCustomModelImplementation( CollisionTerrain, COLLISIONTERRAIN__DESCSTR );			//!< Register to the store/retrieve system
FBElementClassImplementation2( CollisionTerrain, "Collision Terrain", FB_DEFAULT_SDK_ICON );	                //!< Register to the asset system


static void CollisionTerrain_ResetAction(HIObject pObject, bool value) {
	CollisionTerrain *p = FBCast<CollisionTerrain>(pObject);
	if (value && p)	p->DoUpdate();
}

void AddPropertyViewForCollisionTerrain(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(COLLISIONTERRAIN__CLASSSTR, pPropertyName, pHierarchy);
}

void CollisionTerrain::AddPropertiesToPropertyViewManager()
{
	AddPropertyViewForCollisionTerrain("Enabled", "");
	AddPropertyViewForCollisionTerrain("Size", "");

	// 
	AddPropertyViewForCollisionTerrain("Simulation properties", "", true);
	AddPropertyViewForCollisionTerrain("Friction", "Simulation properties");

	//
	AddPropertyViewForCollisionTerrain("Dynamic generation", "", true);
	AddPropertyViewForCollisionTerrain("Generate", "Dynamic generation");
	AddPropertyViewForCollisionTerrain("Update", "Dynamic generation");
	AddPropertyViewForCollisionTerrain("Texture resolution", "Dynamic generation");
	AddPropertyViewForCollisionTerrain("Objects", "Dynamic generation");
	AddPropertyViewForCollisionTerrain("Auto Calculate Z", "Dynamic generation");
	AddPropertyViewForCollisionTerrain("Min Z", "Dynamic generation");
	AddPropertyViewForCollisionTerrain("Max Z", "Dynamic generation");
	AddPropertyViewForCollisionTerrain("Density", "Dynamic generation");

	// 
	AddPropertyViewForCollisionTerrain("Render preview", "", true);
	AddPropertyViewForCollisionTerrain("Preview", "Render preview");
	AddPropertyViewForCollisionTerrain("Debug Normals", "Render preview");
}

/************************************************
*	Constructor.
************************************************/
CollisionTerrain::CollisionTerrain( const char* pName, HIObject pObject ) 
    : FBModelMarker( pName, pObject )
	, mBuffer(1, 1, FrameBuffer::eCreateColorTexture | FrameBuffer::eDeleteFramebufferOnCleanup | FrameBuffer::eCreateDepthRenderbuffer)
{
    FBClassInit;
}

/************************************************
*	FiLMBOX Constructor.
************************************************/

extern void DebugOGL_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void*userParam);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
bool CollisionTerrain::FBCreate()
{
	mLastContext = 0;

	/*
#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback( DebugOGL_Callback, nullptr );
#endif
	*/
	//
	mShader = nullptr;
	mShaderData.gTerrainFarPlane = 5.0f;
	mShaderData.gTerrainOffset = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	mShaderData.gTerrainScale = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	//mShaderData.gTerrainColorAddress = 0;
	mShaderData.gTerrainVP.identity();

	mTextureId = 0;
	mTextureAddress = 0;

	//
    ShadingMode = kFBModelShadingTexture;
	/*
    Size = 50.0;
    Length = 1.0;
    ResLevel = kFBMarkerMediumResolution;
    Look = kFBMarkerLookCube;
    Type = kFBMarkerTypeStandard;
	*/
    Show = true;

	FBPropertyPublish( this, Enabled, "Enabled", nullptr, nullptr );
	FBPropertyPublish( this, Friction, "Friction", nullptr, nullptr );

	//FBPropertyPublish( this, Size, "Size", nullptr, nullptr );
	//FBPropertyPublish( this, Color, "Color", nullptr, nullptr );

	// dynamic generation
	FBPropertyPublish( this, ManualUpdate, "Manual Update", nullptr, nullptr );
	FBPropertyPublish( this, Update, "Update", nullptr, CollisionTerrain_ResetAction );
	FBPropertyPublish( this, Objects, "Objects", nullptr, nullptr );

	FBPropertyPublish( this, TextureResolution, "Texture resolution", nullptr, nullptr );

	FBPropertyPublish( this, AutoCalculateZ, "Auto Calculate Z", nullptr, nullptr );
	FBPropertyPublish( this, MinZ, "Min Z", nullptr, nullptr );
	FBPropertyPublish( this, MaxZ, "Max Z", nullptr, nullptr );

	FBPropertyPublish( this, Density, "Density", nullptr, nullptr );

	FBPropertyPublish( this, Preview, "Preview", nullptr, nullptr );
	FBPropertyPublish( this, DebugNormals, "Debug Normals", nullptr, nullptr );

	Enabled = true;
	Friction = 50.0;

	Size = 50.0;
	Color = FBColor(0.4, 0.3, 0.8);

	ManualUpdate = true;
	Objects.SetFilter( FBModel::GetInternalClassId() );

	TextureResolution = kFBTerrainTexture128;

	AutoCalculateZ = true;
	MinZ = 0.1;
	//MinZ.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	MaxZ = 50.0;

	Density = FBVector3d(1.0, 1.0, 1.0);
	Density.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );

	Preview = false;
	DebugNormals = false;

	mNeedUpdate = true;
	mLastTimelineTime = FBTime::MinusInfinity;

	FBEvaluateManager::TheOne().OnRenderingPipelineEvent.Add( this, (FBCallback) &CollisionTerrain::OnRenderEvent);

	CreateGeometry();

    return true;
}

/** CreateGeometry
* The purpose of this method is solely to give the model a valid bounding box
* for the purposes of framing. The bounding box is invalid if no geometry exists.
*/

void CollisionTerrain::CreateGeometry()
{
    // create *fake* geometry for bounding box computation.
    FBMesh* lMesh = new FBMesh("Model Custom");
    this->Geometry = lMesh;

    double lScale = 10.0; //Scale;

    lMesh->GeometryBegin();

    lMesh->VertexAdd(	0.0,	0.0,	0.0	);
    lMesh->VertexAdd(	lScale,	lScale,	lScale	);
    lMesh->VertexAdd(	-lScale,	-lScale,	-lScale	);

    lMesh->GeometryEnd();
}

/************************************************
*	FiLMBOX Destructor.
************************************************/
void CollisionTerrain::FBDestroy()
{
	FBEvaluateManager::TheOne().OnRenderingPipelineEvent.Remove( this, (FBCallback) &CollisionTerrain::OnRenderEvent);

    ParentClass::FBDestroy();

	GPUParticles::FreeShader();
}

void DrawPlaneSolid(const float size, const float level)
{
	const float positions[] = {-size, level, -size,
								size, level, -size,
								size, level, size,
								-size, level, size };

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, positions);                        // position
	glDrawArrays(GL_QUADS, 0, 4);
	glDisableVertexAttribArray(0);
}

void DrawPlaneWire(const float size, const float level)
{
	const float positions[] = {-size, level, -size,
								size, level, -size,
								size, level, size,
								-size, level, size };

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, positions);                        // position
	glDrawArrays(GL_LINE_LOOP, 0, 4);
	glDisableVertexAttribArray(0);
}

void RenderModel(FBModel *pModel)
{
	FBModelVertexData* lModelVertexData = pModel->ModelVertexData;
	if (lModelVertexData)
	{
		
		//Get number of region mapped by different materials.
		const int lSubRegionCount = lModelVertexData->GetSubRegionCount();
		if (lSubRegionCount)
		{
			//Set up vertex buffer object (VBO) or vertex array
			lModelVertexData->EnableOGLVertexData();

			const GLuint id = lModelVertexData->GetVertexArrayVBOId( kFBGeometryArrayID_Point );
			const GLvoid* positionOffset = lModelVertexData->GetVertexArrayVBOOffset(kFBGeometryArrayID_Point);
			glBindBuffer( GL_ARRAY_BUFFER, id );
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) positionOffset ); 

			glEnableVertexAttribArray(0);	

			for (int lSubRegionIndex = 0; lSubRegionIndex < lSubRegionCount; lSubRegionIndex++)
			{
				// Setup material, texture, shader, parameters here.
				/* 
				FBMaterial* lMaterial = lModelVertexData->GetSubRegionMaterial(lSubRegionIndex);
				*/
				lModelVertexData->DrawSubRegion(lSubRegionIndex);

				//Cleanup material, texture, shader, parameters here
			}

			glDisableVertexAttribArray(0);

			lModelVertexData->DisableOGLVertexData();
		}
	}
}

void CollisionTerrain::CalculateDensity()
{
	FBTerrainTextureResolution res = TextureResolution;
	
	int texRes = 1;

	switch(res)
	{
	case kFBTerrainTexture128: texRes = 128;
		break;
	case kFBTerrainTexture256: texRes = 256;
		break;
	case kFBTerrainTexture512: texRes = 512;
		break;
	case kFBTerrainTexture1024: texRes = 1024;
		break;
	case kFBTerrainTexture2048: texRes = 2048;
		break;
	case kFBTerrainTexture4096: texRes = 4096;
		break;
	}

	double size = Size;
	double sizeZ = MaxZ - MinZ;

	Density = FBVector3d( size/texRes, size/texRes, sizeZ );
}

void CollisionTerrain::ReSize()
{
	if (mShader == nullptr || mShader->IsInitialized() == false)
	{
		return;
	}

	FBTerrainTextureResolution res = TextureResolution;
	
	int texRes = 1;

	switch(res)
	{
	case kFBTerrainTexture128: texRes = 128;
		break;
	case kFBTerrainTexture256: texRes = 256;
		break;
	case kFBTerrainTexture512: texRes = 512;
		break;
	case kFBTerrainTexture1024: texRes = 1024;
		break;
	case kFBTerrainTexture2048: texRes = 2048;
		break;
	case kFBTerrainTexture4096: texRes = 4096;
		break;
	}

	if (mBuffer.GetFrameBuffer() == 0 || mBuffer.GetWidth() != texRes)
	{
		mBuffer.SetColorFormat(0, GL_RED);
		mBuffer.SetColorInternalFormat(0, GL_R32F );  // GL_LUMINANCE32F_ARB);
		mBuffer.SetColorType(0, GL_FLOAT);
		mBuffer.SetFilter(0, mBuffer.filterNearest);
		mBuffer.SetClamp(0, GL_CLAMP_TO_EDGE);

		mBuffer.ReSize(texRes, texRes);

		GLuint texId = mBuffer.GetColorObject();
		GLuint64 handle = 0; 
		
		if (mShader->IsBindlessTexturesSupported() )
			handle = glGetTextureHandleARB(texId);

		mTextureId = texId;
		mTextureAddress = handle;
	}
}

void setothographicmat(float l, float r, float t, float b, float n, float f, FBMatrix &mat)
{
    mat(0,0) = 2 / (r - l);
    mat(0,1) = 0;
    mat(0,2) = 0;
    mat(0,3) = 0;
 
    mat(1,0) = 0;
    mat(1,1) = 2 / (t - b);
    mat(1,2) = 0;
    mat(1,3) = 0;
 
    mat(2,0) = 0;
    mat(2,1) = 0;
    mat(2,2) = -1 / (f - n);
    mat(2,3) = 0;
 
    mat(3,0) = -(r + l) / (r - l);
    mat(3,1) = -(t + b) / (t - b);
    mat(3,2) = -n / (f - n);
    mat(3,3) = 1;
}

bool CollisionTerrain::RenderToHeightMap()
{

	if (mShader == nullptr || mShader->IsInitialized() == false)
	{
		if (mShader)
		{
			FreeShader();
			mShader = nullptr;
		}

		//
		//
		mShader = QueryShader();

		if (mShader->IsInitialized() == false)
		if (false == mShader->Initialize())
		{
			FreeShader();
			mShader = nullptr;
			Enabled = false;

			return false;
		}

		CHECK_GL_ERROR();
	}

	// if needed - prepare framebuffer
	ReSize();

	CHECK_GL_ERROR();
	
	glPushClientAttrib(GL_ALL_ATTRIB_BITS);

	//
	// render to texture
	//
	mBuffer.Bind();

	FBMatrix viewMatrix, projectionMatrix;
	viewMatrix.Identity();
	
	int x = 0;
	int y = 0;
	int w = mBuffer.GetWidth();
	int h = mBuffer.GetHeight();
	
	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	glViewport(x, y, w, h);

	double size = Size;
	setothographicmat(0, size*2, 0, 2*size, MinZ, MaxZ, projectionMatrix );
		
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	
	CHECK_GL_ERROR();

	glDisable(GL_CULL_FACE);

	FBVector3d T;
	GetVector(T);

	float farPlane = (float)MaxZ;

	FBMatrix m(projectionMatrix);
	
	FBMatrix localM;
	mat4	fLocalM;

	localM.Identity();
	localM[12] = -T[0] + Size;
	localM[13] = T[2] + Size;
	localM[14] = -T[1];

	FBMatrixMult( m, m, localM );

	for (int i=0; i<16; ++i)
		mShaderData.gTerrainVP.mat_array[i] = (float) m[i];

	mShaderData.gTerrainFarPlane = farPlane;
	mShader->UploadTerrainDataBlock(mShaderData);

	mShader->BindTerrainDepth();

	fLocalM.identity();
	mShader->UpdateTerrainModelTM(fLocalM);

	CHECK_GL_ERROR();
	
	for (int i=0, count=Objects.GetCount(); i<count; ++i)
	{
		if ( FBIS(Objects[i], FBModel) )
		{
			FBModel *pModel = (FBModel*) Objects[i];
			//if (pModel->Show == false || pModel->Visibility == false) continue; // skip hidden objects
				
			pModel->GetMatrix(localM);

			for (int ii=0; ii<16; ++ii)
				fLocalM.mat_array[ii] = (float) localM[ii];

			mShader->UpdateTerrainModelTM(fLocalM);

			RenderModel(pModel);
		}
	}

	mShader->UnBindTerrainDepth();

	mBuffer.UnBind();

	glViewport(vp[0], vp[1], vp[2], vp[3]);

	glPopClientAttrib();

	CHECK_GL_ERROR();

	return true;
}

/** Custom display
*/
void CollisionTerrain::CustomModelDisplay( FBCamera* pCamera, FBModelShadingMode pShadingMode, FBModelRenderPass pRenderPass, float pPickingAreaWidth, float pPickingAreaHeight)
{
    FBViewingOptions* lViewingOptions = FBSystem::TheOne().Renderer->GetViewingOptions();
    bool lIsSelectBufferPicking = lViewingOptions->IsInSelectionBufferPicking();
    bool lIsColorBufferPicking  = lViewingOptions->IsInColorBufferPicking();
	
	if (lViewingOptions->PickingMode() >=  kFBPickingModeModelsOnly)
		return;
	
	int displayWhat = lViewingOptions->DisplayWhat();
	if ( (displayWhat & kFBDisplayMarker) == 0)
		return;
	
	
	FBVector3d T;
	FBVector3d S;

	GetVector(T);
	GetVector(S, kModelScaling);

	float lSize = (float) Size;


	if (pRenderPass == kFBModelRenderPassPick || lIsSelectBufferPicking || lIsColorBufferPicking)
	{
		FBColor uniqueColor = UniqueColorId;

		if (lIsColorBufferPicking)
			// Draw with subitem additional ColorID
			glColor3dv(uniqueColor);
		else if (pRenderPass == kFBModelRenderPassPick || lIsSelectBufferPicking) 
			// Draw with subitem selection name 1;
			glLoadName(1);

		//glColor3dv(uniqueColor);

		glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        {        
			glTranslated(T[0], T[1], T[2]);
			glScaled(S[0], S[1], S[2]);

			DrawPlaneWire(lSize, 0.0);

			glLineWidth(1.0f);
			DrawPlaneWire(lSize, MinZ);
			DrawPlaneWire(lSize, MaxZ);

			glBegin(GL_LINES);
				glVertex3d(0.0, 0.0, 0.0);
				glVertex3d(0.0, lSize, 0.0);
			glEnd();
		}
		glPopMatrix();

		return;
	}

	// update statistics
	CalculateDensity();

	//
	// main drawing
	//


    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_POLYGON_BIT | GL_LINE_BIT); //Push Current GL states.
    {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        {
			glTranslated(T[0], T[1], T[2]);
			glScaled(S[0], S[1], S[2]);

            if (!lIsSelectBufferPicking && !lIsColorBufferPicking) 
            { 
                if ((bool)Selected) 
                {
                    glColor3d(0.0, 1.0, 0.0);   //!< Otherwise, BLUE.
                } 
                else 
                {
                    FBColor lColor = Color;
                    glColor3dv(lColor);
                }
            }

            glLineWidth(3.0f);    //!< Draw line wider to easy picking.

			// TODO: draw points array and apply displacement according to the extracted depth
			CHECK_GL_ERROR();
			//FBModelShadingMode mode = ShadingMode;

			const bool bindlessSupported = mShader->IsBindlessTexturesSupported();
			bool readyToDisplay = ((bindlessSupported && mTextureAddress > 0)
				|| (!bindlessSupported && mTextureId > 0) );

			if (Preview && readyToDisplay)
			{
				FBMatrix m;
				pCamera->GetCameraMatrix(m, kFBModelViewProj);
				FBMatrix localM;

				FBTRSToMatrix( localM, FBTVector(T[0], T[1], T[2], 1.0), FBRVector(), FBSVector(S[0], S[1], S[2]) );
				FBMatrixMult( m, m, localM );

				for (int i=0; i<16; ++i)
					mShaderData.gTerrainVP.mat_array[i] = (float) m[i];
				mShaderData.gTerrainOffset = vec4(-lSize, 0.0, -lSize, 1.0f);
				mShaderData.gTerrainScale = vec4(lSize*2, 0.0, lSize*2, 1.0f);
			
				mShader->UploadTerrainDataBlock(mShaderData);

				//if (mShaderData.gTerrainColorAddress > 0)
				//	glMakeTextureHandleResidentARB(mShaderData.gTerrainColorAddress);

				CHECK_GL_ERROR();

				if (bindlessSupported)
				{
					glMakeTextureHandleResidentARB(mTextureAddress);
				}
				else
				{
					glBindTexture(GL_TEXTURE_2D, mTextureId);
				}

				mShader->BindTerrainPreview();
				CHECK_GL_ERROR();
				mShader->SetUniformTerrainTextureAddress(mTextureAddress);
				CHECK_GL_ERROR();
				{
					DrawPlaneSolid(lSize, 0.0);
				}
				mShader->UnBindTerrainPreview();

				//if (mShaderData.gTerrainColorAddress > 0)
				//	glMakeTextureHandleNonResidentARB(mShaderData.gTerrainColorAddress);

				if (bindlessSupported)
				{
					glMakeTextureHandleNonResidentARB(mTextureAddress);
				}
				else
				{
					glBindTexture(GL_TEXTURE_2D, 0);
				}

				CHECK_GL_ERROR();
			}

			DrawPlaneWire(lSize, 0.0);

			glLineWidth(1.0f);
			DrawPlaneWire(lSize, MinZ);
			DrawPlaneWire(lSize, MaxZ);


			glBegin(GL_LINES);

				glVertex3d(0.0, 0.0, 0.0);
				glVertex3d(0.0, lSize, 0.0);

			glEnd();
        }
        glPopMatrix();
    }
    glPopAttrib();

	CHECK_GL_ERROR();
}

bool CollisionTerrain::CustomModelPicking(    int pNbHits, unsigned int *pSelectBuffer, FBCamera* pCamera, 
        int pMouseX,int pMouseY,
        FBTVector* pLocalRaySrc, FBTVector* pLocalRayDir,
        FBTVector* pWorldRaySrc, FBTVector* pWorldRayDir,
        FBMatrix* pGlobalInverseMatrix,
        FBTVector* pOutPickedPoint)
{
	// no hits 
    if( pNbHits <= 0) 
        return false;
	/*
    mPickedSubItem = -1;

    for (int Count=0; Count<pNbHits; Count++) 
    {
        

        unsigned int NameCount = *pSelectBuffer++;
        assert(NameCount == 1); 

        // Skip two items  (Min & Max Depth)
        pSelectBuffer+=2; 

        // Fetch the first name from stack.
        unsigned int NameIndex = *pSelectBuffer++;

        if(NameIndex==1)
        {
            mPickedSubItem = 0;

            if(pOutPickedPoint)
            {
                *pOutPickedPoint = FBTVector(100,100,100,0);
            }
        } 
        else if(NameIndex==2)
        {
            mPickedSubItem = 1;

            if(pOutPickedPoint)
            {
                *pOutPickedPoint = FBTVector(-100,100,100,0);
            }
        }
    }

    FBTrace("Select %s SubItem[%d]", GetFullName(), mPickedSubItem);
	*/
    return true; 
}

bool CollisionTerrain::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
    return true;
}

bool CollisionTerrain::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
    return true;
}

void CollisionTerrain::DoUpdate()
{
	mNeedUpdate = true;
}

void CollisionTerrain::FillCollisionData( TCollision &data )
{
	float size = (float) Size;
	double z = MaxZ;
	int res = 1;

	FBVector3d T;
	GetVector(T);

	switch(TextureResolution)
	{
	case kFBTerrainTexture128: res=128;
		break;
	case kFBTerrainTexture256: res=256;
		break;
	case kFBTerrainTexture512: res=512;
		break;
	case kFBTerrainTexture1024: res=1024;
		break;
	case kFBTerrainTexture2048: res=2048;
		break;
	case kFBTerrainTexture4096: res=4096;
		break;
	}

	int iEnabled = (Enabled) ? PARTICLE_COLLISION_TERRAIN_TYPE : PARTICLE_COLLISION_DISABLED;
	double friction = 0.01 * Friction;

	CollisionExchange::SetPosition( data, iEnabled, vec3( -size+(float)T[0], -z+(float)T[1], -size+(float)T[2]) );
	CollisionExchange::SetFriction( data, friction );
	CollisionExchange::SetTerrainScale( data, vec4(2.0f*size, 2.0f*z, 2.0f*size, 1.0f) );
	CollisionExchange::SetTerrainSize( data, vec4(res, res, 0.9f*z+T[1], 1.0f) );

	// TODO: assign terrain texture address
	CollisionExchange::SetTerrainTextureAddress( data, 0 );

}

void CollisionTerrain::OnRenderEvent(HISender pSender, HKEvent pEvent)
{

	FBEventEvalGlobalCallback	levent(pEvent);

	if (Enabled && levent.GetTiming() == kFBGlobalEvalCallbackBeforeRender)
	{
		FBVector3d T;
		GetVector(T);

		FBTime currTime = mSystem.LocalTime;

		bool proceedToUpdate = false;

		if (mNeedUpdate)
			proceedToUpdate = true;
		else if (!ManualUpdate && (T[0] != mLastTranslation[0] || T[1] != mLastTranslation[1] || T[2] != mLastTranslation[2]) )
			proceedToUpdate = true;
		else if (!ManualUpdate && currTime != mLastTimelineTime)
			proceedToUpdate = true;

		if (proceedToUpdate)
		{
			HGLRC	currentContext = wglGetCurrentContext();

			if (mLastContext == 0)
				mLastContext = currentContext;

			if (currentContext != mLastContext)
			{
				mShader->ChangeContext();
				mBuffer.Cleanup();

				mLastContext = currentContext;
			}

			RenderToHeightMap();
			
			mNeedUpdate = false;
			mLastTranslation = T;
			mLastTimelineTime = currTime;
		}
	}

}
/*
bool CollisionTerrain::PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug )
{
	if (pThis == &Objects)
	{
		if (pAction == kFBConnectedSrc)
		{
			Components.Add( (FBComponent*)pPlug);
			return true;
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			Components.Remove( (FBComponent*)pPlug);
			return true;
		}
	}
	
	return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
}
*/