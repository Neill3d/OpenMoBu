
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BlendShapeToolkit_manip.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declarations
#include "ParticlesDrawHelper.h"
#include "BlendShapeToolkit_manip.h"
#include "BlendShapeToolkit_Helper.h"
#include "math3d.h"


#include "stylus.h"

//--- Registration defines
#define ORMANIPASSOCIATION__CLASS	ORMANIPASSOCIATION__CLASSNAME
#define ORMANIPASSOCIATION__DESC	"Sculpting brush"

//--- FiLMBOX implementation and registration
FBManipulatorImplementation	(	ORMANIPASSOCIATION__CLASS		);
FBRegisterManipulator		(	ORMANIPASSOCIATION__CLASS,
								ORMANIPASSOCIATION__LABEL,
								ORMANIPASSOCIATION__DESC,
								FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void SculptManip_SetColor(HIObject object, FBColor value)
{
	ORManip_Sculpt *manip = FBCast<ORManip_Sculpt>(object);
	if (manip)
	{
		manip->GetBrushManagerPtr()->SetColor(value);
		manip->Color.SetPropertyValue(value);
	}
}

static void SculptManip_SetRadius(HIObject object, double value)
{
	ORManip_Sculpt *manip = FBCast<ORManip_Sculpt>(object);
	if (manip)
	{
		manip->GetBrushManagerPtr()->SetRadius(value);
		manip->Radius.SetPropertyValue(value);
	}
}

static void SculptManip_SetStrength(HIObject object, double value)
{
	ORManip_Sculpt *manip = FBCast<ORManip_Sculpt>(object);
	if (manip)
	{
		manip->GetBrushManagerPtr()->SetStrength(value);
		manip->Strength.SetPropertyValue(value);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
//


//! a constructor
SculptUndo::SculptUndo(const OperationBuffer &buffer, ORManip_Sculpt *pManip)
	: mManip(pManip)
	, mBuffer(buffer)
	, mDifference(nullptr)
{}

/** Destructor.
*/
SculptUndo::~SculptUndo()
{
	if (mDifference)
	{
		delete [] mDifference;
		mDifference = nullptr;
	}
}

bool SculptUndo::CalculateDifference( const OperationBuffer &buffer )
{
	bool res = false;

	if (mDifference)
	{
		delete [] mDifference;
		mDifference = nullptr;
	}

	if (mBuffer.vertices.size() == buffer.vertices.size() )
	{
		int count = buffer.vertices.size();
		mDifference = new FBVector3<float>[count];

		for (int i=0; i<count; ++i)
		{
			FBVector3d dif;
			dif = VectorSubtract(buffer.vertices[i].position, mBuffer.vertices[i].position);

			mDifference[i][0] = dif[0];
			mDifference[i][1] = dif[1];
			mDifference[i][2] = dif[2];

			double len = VectorLength(dif);
			if (len > 0.001) res = true;
		}
	}

	return res;
}

/** Callback function for undo custom action.
*/
void SculptUndo::Undo()
{
	// restore mesh vertices from buffer
	if (mBuffer.pModel.Ok() )
	{
		if (mManip->IsModelDeformed() )
		{
			OperationBuffer temp = mManip->GetBuffer();
			mManip->GetBuffer() = mBuffer;
			mBuffer = temp;
		}
		else
		{
			ORManip_Sculpt::CopyBufferToGeometry( mBuffer, mBuffer.pModel );
			ORManip_Sculpt::CopyGeometryToBuffer( mBuffer.pModel, mManip->GetBuffer() );
		
			FBMesh *pMesh = (FBMesh *) (FBGeometry*) mBuffer.pModel->Geometry;
			pMesh->ComputeVertexNormals();
		}
	}
}

/** Callback function for redo custom action.
*/
void SculptUndo::Redo()
{
	// restore mesh vertices from buffer + difference
	if (mBuffer.pModel.Ok() )
	{
		if (mManip->IsModelDeformed() )
		{
			OperationBuffer temp = mManip->GetBuffer();
			mManip->GetBuffer() = mBuffer;
			mBuffer = temp;
		}
		else
		{
			ORManip_Sculpt::CopyBufferToGeometry( mBuffer, mBuffer.pModel, mDifference );
			ORManip_Sculpt::CopyGeometryToBuffer( mBuffer.pModel, mManip->GetBuffer() );

			FBMesh *pMesh = (FBMesh *) (FBGeometry*) mBuffer.pModel->Geometry;
			pMesh->ComputeVertexNormals();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ORManip_Sculpt::FBCreate()
{
	if( FBManipulator::FBCreate() )
	{
		ConfigBrushManager();

		mUsePenPressure = true;
		mTabletSupported = false;
		mLastPressure = 1.0f;
		/*
		if ( !LoadWintab( ) )
		{
			ShowError( "Wintab not available" );
			mTabletSupported = false;
		}

		// check if WinTab available.
		if (!gpWTInfoA(0, 0, NULL)) 
		{
			ShowError( "WinTab Services Not Available." );
			mTabletSupported = false;
		}
		*/

		//
		//
	
		// Properties
		DefaultBehavior		= true;
		ViewerText			= "Sculpting brush";

		// Members
		mRenderInModelSpace = true;
		mActivated			= false;

		mPrepStage = false;

		FBPropertyPublish( this, UseConstraint, "Use Constraint", nullptr, nullptr );
		FBPropertyPublish( this, Color, "Color", nullptr, SculptManip_SetColor );
		FBPropertyPublish( this, Radius, "Radius", nullptr, SculptManip_SetRadius );
		FBPropertyPublish( this, Strength, "Strength", nullptr, SculptManip_SetStrength );

		FBPropertyPublish( this, Inverted, "Inverted", nullptr, nullptr );

		FBPropertyPublish( this, RadiusSens, "Radius Sensivity", nullptr, nullptr );
		FBPropertyPublish( this, StrengthSens, "Strength Sensivity", nullptr, nullptr );

		FBPropertyPublish( this, BrushColor, "Brush Color", nullptr, nullptr );
		FBPropertyPublish( this, DirectionMode, "Direction Mode", nullptr, nullptr );
		FBPropertyPublish( this, AffectMode, "Affect Mode", nullptr, nullptr );
		FBPropertyPublish( this, DisplayAffect, "Display Affect", nullptr, nullptr );

		FBPropertyPublish( this, ScreenInfluence, "Screen Influence", nullptr, nullptr );
		FBPropertyPublish( this, ScreenInfluenceRadius, "Screen Influence Radius", nullptr, nullptr );

		FBPropertyPublish( this, UseTablet, "Use Tablet", nullptr, nullptr );
		FBPropertyPublish( this, UsePenPressure, "Use Tablet", nullptr, nullptr );
		FBPropertyPublish( this, UsePenEraser, "Use Pen Eraser", nullptr, nullptr );
		FBPropertyPublish( this, MaxPenPressure, "Max Pen Pressure", nullptr, nullptr );
		FBPropertyPublish( this, DisplayPressure, "Display pressure", nullptr, nullptr );

		FBPropertyPublish( this, AutoBlendshape, "Auto Blendshape", nullptr, nullptr );
		FBPropertyPublish( this, AutoKeyframe, "Auto Keyframe", nullptr, nullptr );
		FBPropertyPublish( this, UseKeyframeRange, "Use Keyframe Range", nullptr, nullptr );
		FBPropertyPublish( this, KeyframeIn, "Keyframe In", nullptr, nullptr );
		FBPropertyPublish( this, KeyframeLen, "Keyframe Len", nullptr, nullptr );
		FBPropertyPublish( this, KeyframeOut, "Keyframe Out", nullptr, nullptr );

		UseConstraint = false;
		Color = FBColor(1.0, 1.0, 1.0);
		Radius = 50.0;
		Strength = 50.0;

		Inverted = false;

		RadiusSens = 0.3;
		StrengthSens = 0.3;

		BrushColor = FBColor(1.0, 1.0, 0.0);
		DirectionMode.SetPropertyValue(kFBBrushScreen);
		AffectMode.SetPropertyValue(kFBBrushAffectOnSurface);
		DisplayAffect = true;

		ScreenInfluence = false;
		ScreenInfluenceRadius = 100.0;

		UseTablet = true;
		UsePenPressure = true;
		UsePenEraser = false;
		MaxPenPressure = 0;
		DisplayPressure = true;

		UsePenPressure.SetEnable(false);
		UsePenEraser.SetEnable(false);
		MaxPenPressure.ModifyPropertyFlag(kFBPropertyFlagReadOnly, true);
		DisplayPressure.SetEnable(false);

		AutoBlendshape = true;
		AutoKeyframe = true;
		UseKeyframeRange = true;
		KeyframeIn = 0;
		KeyframeLen = 1;
		KeyframeOut = 0;

		//
		//
		//

		mUndo = nullptr;
		//mDevice = nullptr;

		mGLFont.InitFont( wglGetCurrentDC() );

		mTheSystem.Scene->OnChange.Add( this, (FBCallback) &ORManip_Sculpt::EventSceneChange );
		mTheApplication.OnFileNewCompleted.Add( this, (FBCallback) &ORManip_Sculpt::EventFileNewOrOpen );
		mTheApplication.OnFileOpenCompleted.Add( this, (FBCallback) &ORManip_Sculpt::EventFileNewOrOpen );

		return true;
	}
	return false;
}

void ORManip_Sculpt::ConfigBrushManager()
{
	// add some brushes
	mBrushManager.AddBrush( new BrushDrag(this) );
	mBrushManager.AddBrush( new BrushPush(this) );
	mBrushManager.AddBrush( new BrushFreeze(this) );
	mSmoothBrushIndex = mBrushManager.AddBrush( new BrushSmooth(this) );
	mBrushManager.AddBrush( new BrushErase(this) );
	mBrushManager.AddBrush( new BrushPaint(this) );
	mBrushManager.SetCurrentBrush( 0 );

	// add some falloffs
	mBrushManager.AddFalloff( new FalloffSmooth() );
	mBrushManager.AddFalloff( new FalloffLinear() );
	mBrushManager.AddFalloff( new FalloffHard() );
	mBrushManager.AddFalloff( new FalloffPoint() );
	mBrushManager.SetCurrentFalloff( 0 );
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void ORManip_Sculpt::FBDestroy()
{
	//ReleaseRTS();
	TabletCleanup();
	/*
	if (g_hctx)
    {
		gpWTClose(g_hctx);
    }
	
	if (mTabletSupported)
		UnloadWintab();
*/
	mTheSystem.Scene->OnChange.Remove( this, (FBCallback) &ORManip_Sculpt::EventSceneChange );
	mTheApplication.OnFileNewCompleted.Remove( this, (FBCallback) &ORManip_Sculpt::EventFileNewOrOpen );
	mTheApplication.OnFileOpenCompleted.Remove( this, (FBCallback) &ORManip_Sculpt::EventFileNewOrOpen );

	FreeFreezeState();

	mBrushManager.FBDestroy();

	FBManipulator::FBDestroy();
}

/************************************************
 *	FiLMBOX Draw.
 ************************************************/

void ORManip_Sculpt::SetModel(FBComponent* pFBModel, OperationBuffer *bufferZero)
{
	//printf( "> begin setModel\n" );

	mPrepStage = true;

	FBModel *pModel = (FBModel*) pFBModel;

	if (mBrushData.GetModelPtr() != pModel)
	{
		mBrushData.SetModel( pModel );
	
		//
		mDeformed = (pModel && (pModel->IsDeformable == true));

		// find deformer constraint
		BlendShapeDeformerConstraint *pConstraint = nullptr;
		bool isAssigned = false;
		if (pModel)
			for (int j=0; j<pModel->Deformers.GetCount(); ++j)
				if ( FBIS(pModel->Deformers[j], FBDeformerCorrective) )
				{
					FBDeformerCorrective *pDeformer = (FBDeformerCorrective*) pModel->Deformers[j];
					if (pDeformer->Constraint.GetCount() )
					{
						pConstraint = FBCast<BlendShapeDeformerConstraint>(pDeformer->Constraint.GetAt(0));
				
						if (pConstraint == static_cast<BlendShapeDeformerConstraint*>(mCurrentConstraint))
							isAssigned = true;
					}
				}

		if (isAssigned == false)
			mCurrentConstraint = pConstraint;

		if ( OnDataUpdate.empty() == false )
			OnDataUpdate();
	}


	if (bufferZero != nullptr)
	{
		mBufferZero = *bufferZero;
		mBuffer = *bufferZero;
	}
	else
	{
		if (mBrushData.IsModelOk() )
		{
			CopyGeometryToBuffer( mBrushData.GetModelPtr(), mBufferZero );
			CopyGeometryToBuffer( mBrushData.GetModelPtr(), mBuffer );
		}
	}

	// !
	RestoreFreezeState();

	mPrepStage = false;
}

const char *ORManip_Sculpt::GetModelName()
{
	if (mBrushData.IsModelOk() )
		return mBrushData.GetModelPtr()->Name;

	return nullptr;
}

FBModel *ORManip_Sculpt::GetModelPtr()
{
	if (mBrushData.IsModelOk() )
		return mBrushData.GetModelPtr();

	return nullptr;
}

void ORManip_Sculpt::ChangeNotify()
{
	if ( mBrushData.IsModelOk() && mBrushData.GetModelPtr()->Geometry )
	{
		FBMatrix pMatrix;
		mBrushData.GetModelPtr()->GetMatrix( pMatrix, kModelTransformation_Geometry );
		mBrushData.GetEdgesGraph().CalculateDistances( pMatrix, (FBMesh*) (FBGeometry*) mBrushData.GetModelPtr()->Geometry, true );
	}
}

/************************************************
 *	Draw function for manipulator
 ************************************************/
/************************************************
 *	calculate view plane for current camera
 ************************************************/
void ORManip_Sculpt::BuildViewPlane(const FBTVector &pos)
{
	FBVector3d W;
	FBTVector unitZ(0.0, 0.0, 1.0, 1.0);
	FBTVector mViewUp;
	FBVector3d unitX(1.0, 0.0, 0.0);
	//FBVector3d mWorkPoint(0.0, 0.0, 0.0);

	FBMatrix m;
	FBMatrixInverse(m, modelview);
	/*
	W = MatrixToEuler( modelview );

	FBMatrix mRotX = rotateX( DEG2RAD( W[2] ) );
	FBMatrix mRotY = rotateY( DEG2RAD( -W[1] ) );
	FBMatrix mRotZ = rotateZ( DEG2RAD( W[0] ) );

	VectorTransform33( unitZ, mRotZ, mViewUp );
	VectorTransform33( mViewUp, mRotY, mViewUp );
	VectorTransform33( mViewUp, mRotX, mViewUp );

	VectorTransform33( unitX, mRotZ, mViewRight );
	VectorTransform33( mViewRight, mRotY, mViewRight );
	VectorTransform33( mViewRight, mRotX, mViewRight );

	VectorNormalize( mViewUp );
	mViewPlane[0] = mViewUp[0];
	mViewPlane[1] = mViewUp[1];
	mViewPlane[2] = mViewUp[2];
	*/

	m[12] = 0.0;
	m[13] = 0.0;
	m[14] = 0.0;

	FBVectorMatrixMult(mViewUp, m, unitZ);
	FBMult(mViewUp, mViewUp, 1.0 / FBLength(mViewUp) );

	mViewPlane[0] = mViewUp[0];
	mViewPlane[1] = mViewUp[1];
	mViewPlane[2] = mViewUp[2];
	mViewPlane[3] = -FBDot(mViewUp, pos );
	//mViewPlane[3] = -DotProduct( mViewUp, mBrushData.pos );

	//mViewPlane = PlaneMake( FBVector3d(mBrushData.pos), FBVector3d(mViewUp) );
}

//! update viewport
/*!
	\param currentCamera - current viewport camera (manipulator variable)
*/
void ORManip_Sculpt::UpdateViewPlane(FBCamera *currentCamera, BrushCameraData *pCameraData)
{
	glGetIntegerv(GL_VIEWPORT,lViewport);
	/*
	lViewport[0] = GetPanePosX();
	lViewport[1] = GetPanePosY();
	lViewport[2] = GetPaneWidth();
	lViewport[3] = GetPaneHeight();
	*/
	lViewport[0] = currentCamera->CameraViewportX;
	lViewport[1] = currentCamera->CameraViewportY;
	lViewport[2] = currentCamera->CameraViewportWidth;
	lViewport[3] = currentCamera->CameraViewportHeight;


	currentCamera->GetCameraMatrix(modelview, kFBModelView);
	currentCamera->GetCameraMatrix(projection, kFBProjection);

	mNearDist = currentCamera->NearPlaneDistance;
	mFarDist = currentCamera->FarPlaneDistance;
	mPerspective = (currentCamera->Type.GetPropertyValue() == kFBCameraTypePerspective);

	mScreenHeight = lViewport[3];
	mScreenWidth = lViewport[2];
	mFOV = currentCamera->FieldOfView;

	mSystem = true;
	if (!currentCamera->SystemCamera)
	{
		if (currentCamera->Interest)
		{
			currentCamera->GetVector(mCamera);
			currentCamera->Interest->GetVector(mInterest);

			mSystem = false;
		}
	}

	if (pCameraData)
		BuildViewPlane(pCameraData->pos);
}

//! draw debug lines
void ORManip_Sculpt::DrawViewPlane(BrushCameraData *pCameraData)
{
	if (pCameraData == nullptr)
		return;

	FBMatrix m;
	FBMatrixInverse(m, modelview);
	m[12] = pCameraData->pos[0];
	m[13] = pCameraData->pos[1];
	m[14] = pCameraData->pos[2];

	const double factor = 50.0;

	glPushMatrix();

	glMultMatrixd(m);
	
	glColor3f(0.0, 1.0, 1.0);
	glBegin(GL_LINES);
		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(factor, 0.0, 0.0);
			
		glColor3f(0.0, 1.0, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, factor, 0.0);
			
		glColor3f(0.0, 0.0, 1.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, 0.0, factor);
	glEnd();

	glPopMatrix();
}

void ORManip_Sculpt::ViewExpose()
{
	static int	lValue		= 0;
	static int	lStep		= 1;
	static bool lForward	= true;
	

	FBPlayerControl &lPlayerControl = FBPlayerControl::TheOne();
	if (lPlayerControl.IsPlaying || lPlayerControl.IsRecording || lPlayerControl.IsPlotting)
		return;
	if (mPrepStage == true)
		return;

	FBVector3d lVector;
	FBModelList	lList;

	if( mBrushData.IsModelOk() && mInputCamera == CurrentCamera )
	{
		BrushCameraData *pCameraData = mBrushData.GetCameraData( CurrentCamera );
		if (pCameraData == nullptr)
			return;

		mBrushData.GetModelPtr()->GetVector(lVector);
		mActivated = true;

		int paneX = GetPanePosX();
		int paneY = GetPanePosY();
		int paneWidth = GetPaneWidth();
		int paneHeight = GetPaneHeight();

		int cameraX = mInputCamera->CameraViewportX;
		int cameraY = mInputCamera->CameraViewportY;
		int cameraW = mInputCamera->CameraViewportWidth;
		int cameraH = mInputCamera->CameraViewportHeight;

		//int realY = /*GetViewerHeight() -*/ pCameraData->mouseY;
		//int localX = pCameraData->mouseX - paneX;
		//int localY = realY - paneY;

		int localX = pCameraData->absMouseX;
		int localY = pCameraData->absMouseY;

		glViewport( cameraX, cameraY, cameraW, cameraH );

		if( mRenderInModelSpace )
		{
			FBModelRenderBegin( NULL );
		}

		//
		mBrushManager.Draw( kFBDrawBefore );

		//
		///
		//
		double lStrength = (mBrushManager.IsOverride() ) ? mBrushManager.GetStrength() : Strength;
		lStrength *= 0.1;

		if (UseTablet && DisplayPressure)
		{
			lStrength *= mLastPressure;
		}

		lVector[0] = pCameraData->pos[0] + pCameraData->nor[0] * lStrength;
		lVector[1] = pCameraData->pos[1] + pCameraData->nor[1] * lStrength;
		lVector[2] = pCameraData->pos[2] + pCameraData->nor[2] * lStrength;

		FBColor lColor = BrushColor;
		glColor3dv( lColor );
		glBegin(GL_LINES);
		glVertex3dv( pCameraData->pos );
		glVertex3dv( lVector );
		glEnd();
		
		
		glPointSize(5.0f);
		
		/*
		glBegin(GL_POINTS);
			glVertex3dv( orig );
			glVertex3dv( dir );

			glColor3d(0.0, 0.0, 1.0);
			glVertex3dv( mPointInViewSpace );
			glColor3d(1.0, 0.0, 0.0);
			glVertex3dv( mBrushData.pos );
		glEnd();
		*/
		
		if (ScreenInfluence == false)
		{
			double lRadius = (mBrushManager.IsOverride() ) ? mBrushManager.GetRadius() : Radius;
			lRadius *= 0.1;

			FBMatrix m;
			
			m = NormalToRotation( pCameraData->nor );
			memcpy( &m[12], pCameraData->pos, sizeof(double)*3 );

			DrawCircleBillboard( lRadius , 32, m);
		}

		//
		// display affected vertices
		//

		if (DisplayAffect)
		{
			RenderWeights();
		}

		//DrawViewPlane();

		//
		mBrushManager.Draw( kFBDrawAfter );

		// draw 2d view
		glViewport( paneX, GetViewerHeight() - (paneY+paneHeight), paneWidth, paneHeight );
		
			GLint lViewport[4];
			glGetIntegerv( GL_VIEWPORT, &lViewport[0] );

			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(0.0, lViewport[2], lViewport[3], 0.0, -1.0, 1.0); 
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			//-------------------------
	
			glColor3dv( lColor );
			mGLFont.PositionText( localX+2, localY+2 );
			
			if (mBrushData.inverted)
			{
				FBString brush_name( "Inv. ", GetBrushManagerPtr()->GetCurrentBrushPtr()->GetCaption() );
				mGLFont.glDrawText( brush_name, brush_name.GetLen() );
			}
			else
			{
				FBString brush_name(GetBrushManagerPtr()->GetCurrentBrushPtr()->GetCaption() );
				mGLFont.glDrawText( brush_name, brush_name.GetLen() );
			}

			if (ScreenInfluence)
			{
				const double lRadius = ScreenInfluenceRadius;
				FBMatrix m;
				m.Identity();
				m[12] = localX;
				m[13] = localY;
				DrawCircleBillboard( lRadius , 32, m);
			}

			glMatrixMode(GL_PROJECTION);
			glPopMatrix();

			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
		

		/*
		// draw mesh normals (base and deformed)

		FBModelVertexData	*pVertexData = mBrushData.GetModelPtr()->ModelVertexData;

		float scale = 1.0f;

		pVertexData->VertexArrayMappingRequest();

		int vertCount = pVertexData->GetVertexCount();
		FBVertex *positions = (FBVertex*) pVertexData->GetVertexArray( kFBGeometryArrayID_Point, true ); // before deform
		FBNormal *normalsT = (FBNormal*) pVertexData->GetVertexArray( kFBGeometryArrayID_Normal, false );
		FBNormal *normals = (FBNormal*) pVertexData->GetVertexArray( kFBGeometryArrayID_Normal, true );

		pVertexData->VertexArrayMappingRelease();

		//FBMatrix m;
		mBrushData.GetModelPtr()->GetMatrix(m);

		glPushMatrix();

		glMultMatrixd( m );

		glBegin(GL_LINES);
		for (int i=0; i<vertCount; ++i)
		{
			glColor3f( 0.0f, 0.0f, 1.0f );
			glVertex3fv( positions[i] );
			glVertex3f( positions[i][0] + normalsT[i][0]*scale, positions[i][1] + normalsT[i][1]*scale, positions[i][2] + normalsT[i][2]*scale );

			glColor3f( 1.0f, 0.0f, 0.0f );
			glVertex3fv( positions[i] );
			glVertex3f( positions[i][0] + normals[i][0]*scale, positions[i][1] + normals[i][1]*scale, positions[i][2] + normals[i][2]*scale );
		}
		glEnd();

		glPopMatrix();
		*/

		glFlush();

		if( mRenderInModelSpace )
		{
			FBModelRenderEnd();
		}
	}
}

/************************************************
 *	Deal with manipulator input.
 ************************************************/
bool ORManip_Sculpt::ViewInput(int pMouseX, int pMouseY, FBInputType pAction, int pButtonKey, int pModifier)
{
	static bool lMouseButtonPressed = false;
	static bool lArrowSelected = false;
	static int	lPrevMouseX = 0;

	FBPlayerControl &lPlayerControl = FBPlayerControl::TheOne();
	if (lPlayerControl.IsPlaying || lPlayerControl.IsRecording || lPlayerControl.IsPlotting)
		return false;
	if (mPrepStage == true) 
		return false;

	if ( mDeformer.Ok() && (mDeformer->Active==false) && (pAction == kFBMotionNotify || pAction == kFBButtonPress) )
		mDeformer->Active = true;

	int paneX = GetPanePosX();
	int paneY = GetPanePosY();
	int paneWidth = GetPaneWidth();
	int paneHeight = GetPaneHeight();
	int realY = GetViewerHeight() - pMouseY;
	realY = pMouseY;

	FBCamera* lCamera = CurrentCamera;
	if (!lCamera)
		return false;

	if ( FBIS(lCamera, FBCameraSwitcher) )
		lCamera = FBCast<FBCameraSwitcher>(lCamera)->CurrentCamera;
	
	if (!lCamera)
		return false;

	if (pMouseX > paneX && pMouseX < (paneX+paneWidth) && realY > paneY && realY < (paneY+paneHeight) )
	{
		mInputCamera = lCamera;
	}
	else
	{
		return false;
	}
	
	int cameraX = lCamera->CameraViewportX;
	int cameraY = lCamera->CameraViewportY;
	//int cameraW = lCamera->CameraViewportWidth;
	int cameraH = lCamera->CameraViewportHeight;

	int minusX = cameraX;
	int minusY = (paneY > 0) ? cameraH - cameraY : paneY;

	int localX = pMouseX - minusX;
	int localY = pMouseY - minusY;

	BrushCameraData *pCameraData = mBrushData.GetOrCreateCameraData(lCamera);
	if (pCameraData == nullptr)
		return false;

	if (lMouseButtonPressed == true && (pButtonKey > 1) )
	{
	}
	else
	{
		pCameraData->absMouseX = pMouseX - paneX;
		pCameraData->absMouseY = pMouseY - paneY;

		pCameraData->mouseX = localX;
		pCameraData->mouseY = localY;
	}

	// TODO: get input device pressure
	double lPressure = 1.0;
	
	if (UseTablet)
	{
		UseTablet = false;
		if( !mTabletSupported )
		{
			if (false == TabletInit() )
			{
				UseTablet = false;
				mTabletSupported = false;
			}
			else
			{
				UseTablet = true;
				mTabletSupported = true;
			}
		}
		
		//

		if (mTabletSupported && (TabletPumpMessages() == true))
		{
			StylusData data;
			GetStylusData(data);

			if (data.wintab && data.type == StylusData::kStylusType_Pen)
			{
				lPressure = data.pressure;
			}
		}

	}
	mLastPressure = lPressure;

	// switch to smooth operation
	
	mBrushData.inverted = Inverted || (pModifier == kFBKeyCtrl);
	double InvertValue = (mBrushData.inverted) ? -1.0 : 1.0;

	mBrushManager.ToggleSmooth( (pModifier == kFBKeyShift), mSmoothBrushIndex);
	
	/*
	if ((pModifier & VK_CONTROL) == VK_CONTROL )
	{
		// inverse influence if supported

	}
	*/
	FBVector3d	lVector;
	if( mActivated )
	{
		switch( pAction )
		{
			case kFBKeyPress:
			{
				// When a keyboard key is pressed.
			}
			break;
			case kFBKeyRelease:
			{
				// When a keyboard key is released.
			}
			break;
			case kFBButtonDoubleClick:
			{
				// Mouse button double-clicked.
			}
			break;
			case kFBButtonPress:
			{
				lMouseButtonPressed = true;

				if (mUndo)
				{
					if (mUndoManager.TransactionIsOpen())
					{
						mUndoManager.TransactionAdd(mUndo);
						mUndoManager.TransactionEnd();
					}

					//delete mUndo;
					mUndo = nullptr;
				}

				mUndo = new SculptUndo(mBuffer, this);

				mUndoManager.TransactionBegin("sculpting");
				
			}
			break;
			case kFBButtonRelease:
			{
				// Mouse button released.
				lMouseButtonPressed = false;

				if (mUndo) 
				{
					if (mUndo->CalculateDifference( mBuffer ) )
					{
						mUndoManager.TransactionAdd( mUndo );

						if (mBrushData.IsModelOk() && (mDeformed == false) )
						{
							FBMesh *pMesh = (FBMesh*) (FBGeometry*) mBrushData.GetModelPtr()->Geometry;
							pMesh->ComputeVertexNormals();
						}

						// update edges distances
						ChangeNotify();
					}

					//delete mUndo;
					mUndo = nullptr;
				}

				if (mUndoManager.TransactionIsOpen() )
					mUndoManager.TransactionEnd();
			}
			break;
			case kFBMotionNotify:
			{
				
				if (lMouseButtonPressed == false)
				{
					UpdateViewPlane( lCamera, pCameraData );
				}

				double nearPlane = lCamera->NearPlaneDistance;
				double farPlane = lCamera->FarPlaneDistance;

				orig = lCamera->InverseProjection( localX, localY, nearPlane, true );
				dir = lCamera->InverseProjection( localX, localY, farPlane, true );
				
				//nearPlane = 1.0; // lCamera->NearPlaneDistance
				//gluUnProject( pMouseX, lViewport[3]-pMouseY, nearPlane, modelview, projection, lViewport, &mPointInViewSpace[0], &mPointInViewSpace[1], &mPointInViewSpace[2] );
				//mBrushData.deltaView = VectorSubtract( mPointInViewSpace, mPrevPointInViewSpace );
				
				double scale = 1.0;
				FBVector3d orig3d(orig);
				FBVector3d dir3d(dir);
				
				if (RayIntersection(mViewPlane, orig3d, dir3d, scale) )
				{
					FBVector3d delta(dir3d);					
					VectorMult( delta, scale );
					mPointInViewSpace = VectorAdd(orig3d, delta);
					
					pCameraData->deltaView = VectorSubtract( mPointInViewSpace, mPrevPointInViewSpace );
				}
				

				if (mBrushData.IsModelOk() && (pButtonKey < 2) )
				{
					FBTVector pos, n;
					FBTVector lOrig, lDir;

					FBMatrix m, mInv;
					mBrushData.GetModelPtr()->GetMatrix(m, kModelTransformation_Geometry);
					FBMatrixInverse(mInv, m);

					if (lCamera->Type.GetPropertyValue() == kFBCameraTypeOrthogonal)
					{
						nearPlane = 1.0; //lCamera->NearPlaneDistance;
						farPlane = 3500.0; //lCamera->FarPlaneDistance;

						orig = lCamera->InverseProjection( localX, localY, nearPlane, true );
						dir = lCamera->InverseProjection( localX, localY, farPlane, true );
					}

					FBVectorMatrixMult( lOrig, mInv, orig );
					FBVectorMatrixMult( lDir, mInv, dir );

					if (mBrushData.GetModelPtr()->ClosestRayIntersection( lOrig, lDir, pos, (FBNormal&) n ) )
					{
						// brush position on the model
						FBVectorMatrixMult( pCameraData->pos, m, pos );
						pCameraData->nor = n;
					}
				}

				//
				// display affected vertices
				//

				if ( mBrushManager.WantToReacalcWeights() || (lMouseButtonPressed == false) )
				{
					CalculateBufferWeights(pCameraData);
				}

				switch(pButtonKey)
				{
				case 1: // left mouse button
					{
						if (mBrushData.IsModelOk() )
						{
							if (lMouseButtonPressed)
							{
								pCameraData->viewPlane = mViewPlane;
								pCameraData->direction = DirectionMode.GetPropertyValue();
								if (ScreenInfluence)
								{
									mBrushData.radius = ScreenInfluenceRadius;
								}
								else
								{
									mBrushData.radius = (mBrushManager.IsOverride() ) ? mBrushManager.GetRadius() : Radius;
								}
								FBColor c = (mBrushManager.IsOverride() ) ? mBrushManager.GetColor() : Color;
								mBrushData.color = FBColorF( (float) c[0], (float) c[1], (float) c[2] );
								mBrushData.strength = InvertValue * 0.01 * lPressure * ((mBrushManager.IsOverride() ) ? mBrushManager.GetStrength() : Strength);
								pCameraData->mouseX = localX;
								pCameraData->mouseY = localY;
								pCameraData->prevMouseX = lPrevMouseX;
								pCameraData->deltaMouseX = (localX - lPrevMouseX);
								mBrushData.fillMode = false;

								// update buffer
								if (mBrushManager.Process( mBrushData, pCameraData, mBufferZero, mBuffer ) )
								{
									// update geometry according to buffer
									if (mBrushData.IsModelOk() && (mDeformed == false) )
										CopyBufferToGeometry(mBuffer, mBrushData.GetModelPtr() );
								}
							}
						}
					} break;
				case 2: // middle mouse button
					{
						if (lMouseButtonPressed)
						{
							double lSens = StrengthSens;
							if (mBrushManager.IsOverride() )
							{
								mBrushManager.SetCurrentBrushStrength( mBrushManager.GetStrength() + lSens * (localX - lPrevMouseX) ); 
							}
							else
							{
								Strength = Strength + lSens * (pMouseX - lPrevMouseX);
							}
						}
					} break;
				case 3: // right mouse button
					{
						if (lMouseButtonPressed)
						{
							double lSens = RadiusSens;
						
							if (ScreenInfluence)
							{
								ScreenInfluenceRadius = ScreenInfluenceRadius + lSens * (localX - lPrevMouseX);
							}
							else
							{
								if (mBrushManager.IsOverride() )
								{
									mBrushManager.SetCurrentBrushRadius( mBrushManager.GetRadius() + lSens * (localX - lPrevMouseX) );
								}
								else
								{
									Radius = Radius + lSens * (localX - lPrevMouseX);
								}
							}
						}
					} break;
				}
			}
			break;
			case kFBDragging:
			{
				// Items are being dragged.
			}
			break;
			case kFBDropping:
			{
				// Items are being dropped.
			}
			break;
		}
	}
	lPrevMouseX = localX;

	mPrevPointInViewSpace = mPointInViewSpace;

	//
	///
	//
	return mBrushManager.Input( localX, localY, pAction, pButtonKey, pModifier );
}


void ORManip_Sculpt::SetCurrentBrush( const int index )
{
	mBrushManager.SetCurrentBrush( index );
}
void ORManip_Sculpt::SetCurrentFalloff( const int index )
{
	mBrushManager.SetCurrentFalloff( index );
}

// vertCount should be always 3
/*
	lPosition - point position (will check this point)
	plane - triangle plane
	vertCount - should be always 3
	triIndices - 3 indices for each vert
*/
bool IsPointInsideTri(FBVector3d lPosition, FBVector4d plane, const int vertCount, const FBVector3d *triPositions)
{
	bool res = false;

	FBVector3d		a,b,c;
	FBVector3d polyN(plane);

	double side = plane[0]*lPosition[0] + plane[1]*lPosition[1] + plane[2]*lPosition[2] + plane[3];

	if ( abs(side ) < 0.001 )
	{
		int				n1, n2;

		res = true;

		// same orientation (in same plane)
		for (int j=0; j<vertCount-1; ++j)
		{
			if (j==vertCount-1)
			{
				n1 = j;
				n2 = 0;
			}
			else
			{
				n1 = j;
				n2 = j+1;
			}
					
			a = triPositions[n1];
			b = triPositions[n2];
			c = VectorAdd( b, polyN );

			plane = PlaneMake(a,b,c);

			side = plane[0]*lPosition[0] + plane[1]*lPosition[1] + plane[2]*lPosition[2] + plane[3];

			if (side > 0)
			{
				res = false;
				break;
			}
		}
	}

	return res;
}

void ORManip_Sculpt::DistributeSurfaceWeights( std::stack<VertEdge> &stack, const float lRadius )
{
	BaseFalloff *pFalloff = mBrushManager.GetCurrentFalloffPtr();
	if (pFalloff == nullptr) return;

	MeshEdgesGraph &graph = mBrushData.GetEdgesGraph();

	std::stack<VertEdge> newItems;
	FBVector3d lPosition;

	while( !stack.empty() )
	{
		while( !stack.empty() )
		{
			const VertEdge vertedge = stack.top();

			double weight = (vertedge.dist <= lRadius) ? (1.0f - vertedge.dist/(lRadius+0.001f)) : 0.0f;
			mBuffer.vertices[vertedge.vert].weight =  pFalloff->Calculate(weight);
			graph.SetFlag( vertedge.vert );

			// compute weights
			if (weight > 0.0)
			{
				lPosition = mBuffer.vertices[vertedge.vert].position;

				// populate new items ( neighboards without flag )
				IntSet &neighboars = graph.GetVertexNeighbores( vertedge.vert );
				float *distances = graph.GetVertexDistances( vertedge.vert );
				if (distances == nullptr)
				{
					printf( "wrong distances array\n" );
				}

				int idx = 0;
				for (auto iter=neighboars.begin(); iter!=neighboars.end(); ++iter)
				{
					if ( false == graph.IsFlag( *iter ) )
					{
						float length = distances[idx];
					
						newItems.push( VertEdge( *iter, length + vertedge.dist ) );
						graph.SetFlag( *iter );
					}

					idx++;
				}
			}

			stack.pop();
		}

		while ( !newItems.empty() )
		{
			stack.push( newItems.top() );
			newItems.pop();
		}
	}

}


void ORManip_Sculpt::CalculateBufferWeights(BrushCameraData *pCameraData)
{
	if (pCameraData == nullptr)
		return;

	double lRadius;
	
	if (ScreenInfluence)
	{
		lRadius = ScreenInfluenceRadius;
	}
	else
	{
		lRadius = (mBrushManager.IsOverride() ) ? mBrushManager.GetRadius() : Radius;
		lRadius = 0.1 * abs(lRadius);
	}

	if (mPrepStage == true) return;
	if (mBrushData.IsModelOk() == false) return;
	if (mBuffer.pModel.Ok() == false) return;
	BaseFalloff *pFalloff = mBrushManager.GetCurrentFalloffPtr();
	if (pFalloff == nullptr) return;


	FBVector3d lPosition (pCameraData->pos);

	//FBMatrix m;
	//mBuffer.pModel->GetMatrix(m, kModelInverse_Transformation_Geometry);

	//VectorTransform( lPosition, m, lPosition );

	if (ScreenInfluence)
	{
		//FBMatrix m;
		FBVector3d pos, res;
		//double x,y,z;

		double length, weight;

		//glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
		//glGetDoublev( GL_PROJECTION_MATRIX, projection );

		lPosition = FBVector3d( pCameraData->mouseX, lViewport[3] - pCameraData->mouseY, 0.0 );
		//mBuffer.pModel->GetMatrix(m, kModelTransformation_Geometry);

		const size_t count = mBuffer.vertices.size();
		for (size_t i=0; i<count; ++i)
		{
			//VectorTransform( mBuffer.vertices[i].position, m, pos );
			pos = mBuffer.vertices[i].position;
			gluProject( pos[0], pos[1], pos[2], modelview, projection, lViewport, &res[0], &res[1], &res[2] ); 
			res[0] = res[0] - lViewport[0];
			res[1] = /*lViewport[3] -*/ res[1] - lViewport[1];
			res[2] = 0.0;

			length = VectorLength( VectorSubtract( lPosition, res ) );
			weight = (length <= lRadius) ? (1.0 - length/(lRadius+0.001)) : 0.0;
			mBuffer.vertices[i].weight = (weight > 0.0) ? pFalloff->Calculate(weight) : 0.0;
		}
	}
	else
	if (AffectMode.AsInt() == kFBBrushAffectOnVolume)
	{
		double length, weight;

		const size_t count = mBuffer.vertices.size();
		for (size_t i=0; i<count; ++i)
		{
			length = VectorLength( VectorSubtract(lPosition, mBuffer.vertices[i].position) );
			weight = (length <= lRadius) ? (1.0 - length/(lRadius+0.001)) : 0.0;
			mBuffer.vertices[i].weight = (weight > 0.0) ? pFalloff->Calculate(weight) : 0.0;
		}
	}
	else
	{
		//
		// for surface mode
		//
		// 1 - determine the face under cursor
		// 2 - find all connected face (can be pre cached when assign a model)
		// 3 - assign weights only inside connected faces

		const size_t count = mBuffer.vertices.size();
		for (size_t i=0; i<count; ++i)
		{
			mBuffer.vertices[i].weight = 0.0;
		}

		FBMesh *pMesh = (FBMesh*) ((FBGeometry*) mBuffer.pModel->Geometry);

		int polyCount = pMesh->PolygonCount();

		for (int i=0; i<polyCount; ++i)
		{
			bool res = false;
			
			int polyVertCount = pMesh->PolygonVertexCount(i);

			// calculate polygon center, min max and radius
			// do simple bounding test

			FBVector3d v, min, max;

			if (polyVertCount > 0)
			{
				min = mBuffer.vertices[pMesh->PolygonVertexIndex(i, 0)].position;
				max = min;

				for (int j=0; j<polyVertCount; ++j)
				{
					v = mBuffer.vertices[pMesh->PolygonVertexIndex(i, j)].position;

					if (min[0] > v[0]) min[0] = v[0];
					if (min[1] > v[1]) min[1] = v[1];
					if (min[2] > v[2]) min[2] = v[2];

					if (max[0] < v[0]) max[0] = v[0];
					if (max[1] < v[1]) max[1] = v[1];
					if (max[2] < v[2]) max[2] = v[2];
				}
			}

			// bounding box test
			if (lPosition[0] < min[0] || lPosition[0] > max[0]
				|| lPosition[1] < min[1] || lPosition[1] > max[1]
				|| lPosition[2] < min[2] || lPosition[2] > max[2] )
				{
					continue;
				}

			if (polyVertCount > 3)
			{
				FBVector4d plane = PlaneMake(	mBuffer.vertices[pMesh->PolygonVertexIndex(i, 2)].position,
					mBuffer.vertices[pMesh->PolygonVertexIndex(i, 3)].position,
					mBuffer.vertices[pMesh->PolygonVertexIndex(i, 0)].position );

				FBVector3d triPositions[3] = { mBuffer.vertices[ pMesh->PolygonVertexIndex(i,2)].position,
											mBuffer.vertices[ pMesh->PolygonVertexIndex(i,3)].position,
											mBuffer.vertices[ pMesh->PolygonVertexIndex(i,0)].position };

				if ( IsPointInsideTri( lPosition, plane, 3, triPositions ) )
					res = true;
			}

			if ( (res == false) && (polyVertCount > 2) )
			{
				FBVector4d plane = PlaneMake(	mBuffer.vertices[pMesh->PolygonVertexIndex(i, 0)].position,
					mBuffer.vertices[pMesh->PolygonVertexIndex(i, 1)].position,
					mBuffer.vertices[pMesh->PolygonVertexIndex(i, 2)].position );

				FBVector3d triPositions[3] = { mBuffer.vertices[ pMesh->PolygonVertexIndex(i,0)].position,
											mBuffer.vertices[ pMesh->PolygonVertexIndex(i,1)].position,
											mBuffer.vertices[ pMesh->PolygonVertexIndex(i,2)].position };

				if ( IsPointInsideTri( lPosition, plane, 3, triPositions ) )
					res = true;
			}
			
			if (res)
			{
				MeshEdgesGraph &graph = mBrushData.GetEdgesGraph();
				if (graph.data.size() == 0) break;

				// 1 - zero all flags except poly verts
				graph.ZeroFlags();

				// 2 - set flags for poly verts
				for (int j=0; j<polyVertCount; ++j)
				{
					graph.SetFlag( pMesh->PolygonVertexIndex(i, j) );
				}

				// start iterating through neighboars
				std::stack<VertEdge> lStack;

				for (int j=0; j<polyVertCount; ++j)
				{
					const int index = pMesh->PolygonVertexIndex(i, j);
					float length = (float) VectorLength( VectorSubtract(lPosition, mBuffer.vertices[index].position) );	

					lStack.push( VertEdge(index, length) );
				}

				DistributeSurfaceWeights( lStack, (float) lRadius );

				break;
			}
		}
	}
}

void ORManip_Sculpt::RenderWeights()
{
	if (mBuffer.pModel.Ok() == false) return;

	const bool paintMode = ((mBrushManager.GetCurrentBrushPtr() != nullptr) ? ( strcmp(mBrushManager.GetCurrentBrushPtr()->GetCaption(), "Paint") == 0) : false);

	//FBMatrix m;
	//mBuffer.pModel->GetMatrix(m);

	const size_t count = mBuffer.vertices.size();

	//glPushMatrix();

	//glMultMatrixd(m);

	//
	// draw freezed
	
	glEnable(GL_DEPTH_TEST);
	glBegin(GL_POINTS);
	
	// draw colors
	if (paintMode)
	{
		for (size_t i=0; i<count; ++i)
		{
			glColor3fv( mBuffer.vertices[i].color );
			glVertex3dv( mBuffer.vertices[i].position );
		}
	}
	
	for (int i=0; i<count; ++i)
	{
		const float freeze = mBuffer.vertices[i].freeze;
		if (freeze > 0.0)
		{
			glColor3f( 1.0f - freeze, 0.0f, 1.0f );
			glVertex3dv( mBuffer.vertices[i].position );
		}
	}
	glEnd();
	glDisable(GL_DEPTH_TEST);

	//
	// draw weights points

	glBegin(GL_POINTS);
	for (int i=0; i<count; ++i)
	{
		const float weight = mBuffer.vertices[i].weight;	
		if (weight > 0.0)
		{
			glColor3f( weight, weight, weight );
			glVertex3dv( mBuffer.vertices[i].position );
		}
	}
	glEnd();

	//glPopMatrix();
}

void ORManip_Sculpt::CopyGeometryToBuffer(FBModel *pModel, OperationBuffer &buffer)
{
	if (pModel)
	{
		buffer.pModel = pModel;
		FBGeometry *pGeometry = buffer.pModel->Geometry;

		FBMatrix tm;
		pModel->GetMatrix(tm, kModelTransformation_Geometry);

		int count = pGeometry->VertexCount();
		FBVertex *pVertices = pGeometry->GetVertexes();
		
		buffer.vertices.resize(count);

		FBVertex vertex;

		for (int i=0; i<count; ++i)
		{
			FBVertexMatrixMult( vertex, tm, pVertices[i] );
			//vertex = pVertices[i];

			buffer.vertices[i].position = FBVector3d( vertex[0], vertex[1], vertex[2] );
			buffer.vertices[i].normal = pGeometry->VertexNormalGet(i);
			buffer.vertices[i].color = pGeometry->VertexColorGet(i);
			buffer.vertices[i].uv = pGeometry->VertexUVGet(i);
			buffer.vertices[i].freeze = 0.0;
			buffer.vertices[i].weight = 0.0;
		}
	}
}

void ORManip_Sculpt::ApplyBufferDifferenceToGeometry(const OperationBuffer &bufferA, const OperationBuffer &bufferB, FBModel *pModel)
{
	if (pModel && pModel->Geometry)
	{
		FBGeometry *pGeometry = pModel->Geometry;

		FBMatrix invTM;
		pModel->GetMatrix( invTM, kModelInverse_Transformation_Geometry );

		
		//pGeometry->VertexInit( buffer.vertices.GetCount(), false, true, true );	// enable holding vertex color
		pGeometry->GeometryBegin();
		
		// TODO: only if model was painted
		//pGeometry->VertexArrayInit( buffer.vertices.GetCount(), true, kFBGeometryArrayID_Color );

		int count = 0;
		FBVertex *pVertices = pGeometry->GetPositionsArray(count);

		FBVertex vertexA, vertexB;
		FBNormal normal, normalA, normalB;

		for (int i=0; i<count; ++i)
		{
			vertexA = FBVertex(bufferA.vertices[i].position[0], bufferA.vertices[i].position[1], bufferA.vertices[i].position[2], 1.0);
			vertexB = FBVertex(bufferB.vertices[i].position[0], bufferB.vertices[i].position[1], bufferB.vertices[i].position[2], 1.0);

			FBVertexMatrixMult( vertexA, invTM, vertexA );
			FBVertexMatrixMult( vertexB, invTM, vertexB );

			pVertices[i][0] += vertexB[0] - vertexA[0];
			pVertices[i][1] += vertexB[1] - vertexA[1];
			pVertices[i][2] += vertexB[2] - vertexA[2];

			normal = pGeometry->VertexNormalGet(i);
			normalA = bufferA.vertices[i].normal;
			normalB = bufferB.vertices[i].normal;

			normal[0] += normalB[0] - normalA[0];
			normal[1] += normalB[1] - normalA[1];
			normal[2] += normalB[2] - normalA[2];

			pGeometry->VertexNormalSet( normal, i );
		}
		
		pGeometry->GeometryEnd();
		pGeometry->ModifyNotify();
	}
}

void ORManip_Sculpt::CopyBufferToGeometry(const OperationBuffer &buffer, FBModel *pModel, const FBVector3<float> *difference)
{
	if (pModel && FBIS(pModel, FBModelPath3D) )
	{
		//((FBModelPath3D*) pModel)->UpdateGeometry();
		//( (FBModelPath3D*) pModel)->PathKeySetLeftTangent(0, FBVector4d(0.1, 0.0, 0.0, 1.0), true);
	}
	else
	if (pModel && pModel->Geometry)
	{
		FBGeometry *pGeometry = pModel->Geometry;

		FBMatrix invTM;
		pModel->GetMatrix( invTM, kModelInverse_Transformation_Geometry );

		
		//pGeometry->VertexInit( buffer.vertices.GetCount(), false, true, true );	// enable holding vertex color
		pGeometry->GeometryBegin();
		
		// TODO: only if model was painted
		//pGeometry->VertexArrayInit( buffer.vertices.GetCount(), true, kFBGeometryArrayID_Color );

		int count = 0;
		FBVertex *pVertices = pGeometry->GetPositionsArray(count);

		if (difference)
		{
			for (int i=0; i<count; ++i)
			{
				pVertices[i][0] = buffer.vertices[i].position[0] + difference[i][0];
				pVertices[i][1] = buffer.vertices[i].position[1] + difference[i][1];
				pVertices[i][2] = buffer.vertices[i].position[2] + difference[i][2];

				FBVertexMatrixMult( pVertices[i], invTM, pVertices[i] );
			}
		}
		else
		{
			for (int i=0; i<count; ++i)
			{
				pVertices[i][0] = buffer.vertices[i].position[0];
				pVertices[i][1] = buffer.vertices[i].position[1];
				pVertices[i][2] = buffer.vertices[i].position[2];

				FBVertexMatrixMult( pVertices[i], invTM, pVertices[i] );

				pGeometry->VertexNormalSet( buffer.vertices[i].normal, i );
				pGeometry->VertexColorSet( buffer.vertices[i].color, i );
				//pGeometry->VertexUVSet( buffer.vertices[i].uv, i );
			}
		}

		pGeometry->GeometryEnd();

		pGeometry->ModifyNotify();
	}
}
/*
void ORManip_Sculpt::AssignInputDevice( FBDevice *pDevice )
{
	mDevice = pDevice;
}
*/
void ORManip_Sculpt::SetUseTabletPenPressure( bool useflag )
{
	mUsePenPressure = useflag;
}

void ORManip_Sculpt::AssignDeformerConstraint( BlendShapeDeformerConstraint *pDeformer )
{
	mDeformer = pDeformer;
}

void ORManip_Sculpt::FreezeAll()
{
	if (mBuffer.pModel.Ok() == false) return;

	const size_t count = mBuffer.vertices.size();
	for (size_t i=0; i<count; ++i)
	{
		mBuffer.vertices[i].freeze = 1.0;
	}
}

void ORManip_Sculpt::FreezeInvert()
{
	if (mBuffer.pModel.Ok() == false) return;

	const size_t count = mBuffer.vertices.size();
	for (size_t i=0; i<count; ++i)
	{
		mBuffer.vertices[i].freeze = 1.0 - mBuffer.vertices[i].freeze;
	}
}

void ORManip_Sculpt::FreezeNone()
{
	if (mBuffer.pModel.Ok() == false) return;

	const size_t count = mBuffer.vertices.size();
	for (size_t i=0; i<count; ++i)
	{
		mBuffer.vertices[i].freeze = 0.0;
	}
}

void ORManip_Sculpt::StoreFreezeState()
{
	if (mBuffer.pModel.Ok() == false) return;

	ModelStateHolder *pHolder = nullptr;
	auto iter = mFreezeStates.find( mBuffer.pModel );
	if (iter != mFreezeStates.end() )
		pHolder = iter->second;

	if (pHolder == nullptr)
		pHolder = new ModelStateHolder();

	if (pHolder)
	{
		const size_t count = mBuffer.vertices.size();
		pHolder->flags.resize( count );
		for (size_t i=0; i<count; ++i)
		{
			pHolder->flags[i] = mBuffer.vertices[i].freeze;
		}

		mFreezeStates[mBuffer.pModel] = pHolder;
	}
}

void ORManip_Sculpt::RestoreFreezeState()
{
	if (mBuffer.pModel.Ok() == false) return;

	ModelStateHolder *pHolder = nullptr;
	auto iter = mFreezeStates.find( mBuffer.pModel );
	if (iter != mFreezeStates.end() )
		pHolder = iter->second;

	if (pHolder && mBuffer.vertices.size() == pHolder->flags.size())
	{
		const size_t count = mBuffer.vertices.size();
		for (size_t i=0; i<count; ++i)
		{
			mBuffer.vertices[i].freeze = pHolder->flags[i];
		}
	}
}

void ORManip_Sculpt::FreeFreezeState()
{
	for (auto iter=mFreezeStates.begin(); iter!=mFreezeStates.end(); ++iter)
	{
		ModelStateHolder *pHolder = iter->second;
		if (pHolder)
		{
			delete pHolder;
			pHolder = nullptr;
		}
	}

	mFreezeStates.clear();
}

void ORManip_Sculpt::Fill()
{
	BrushCameraData *pData = mBrushData.GetCameraData( CurrentCamera );
	if (pData == nullptr)
		return;

	pData->viewPlane = mViewPlane;
	pData->direction = DirectionMode.GetPropertyValue();
	if (ScreenInfluence)
	{
		mBrushData.radius = ScreenInfluenceRadius;
	}
	else
	{
		mBrushData.radius = (mBrushManager.IsOverride() ) ? mBrushManager.GetRadius() : Radius;
	}
	FBColor c = (mBrushManager.IsOverride() ) ? mBrushManager.GetColor() : Color;
	mBrushData.color = FBColorF( (float) c[0], (float) c[1], (float) c[2] );
	mBrushData.strength = 0.01 * ((mBrushManager.IsOverride() ) ? mBrushManager.GetStrength() : Strength);
	mBrushData.fillMode = true;

	// update buffer
	if (mBrushManager.Process( mBrushData, pData, mBufferZero, mBuffer ) )
	{
		// update geometry according to buffer
		if (mBrushData.IsModelOk() && (mDeformed == false) )
			CopyBufferToGeometry(mBuffer, mBrushData.GetModelPtr() );
	}
}

void ORManip_Sculpt::Reset()
{
	mBuffer = mBufferZero;

	if ( mDeformer.Ok() )
		mDeformer->Reset();
	else
		CopyBufferToGeometry(mBuffer, mBrushData.GetModelPtr() );
}

void FBVector3dToVertex( const FBVector3d &v, FBVertex &vert )
{
	vert[0] = (float) v[0];
	vert[1] = (float) v[1];
	vert[2] = (float) v[2];
}

void ORManip_Sculpt::GlobalAddBlendShape(ORManip_Sculpt *pManipulator, const bool overrideExisting)
{
	pManipulator->Active = false;
	
	//
	// bool setKeyframe, timeIn, timeOut
	FBTime curTime( FBSystem::TheOne().LocalTime );
	
	double keyIn=0.0, keyLen=0.0, keyOut=0.0;
	if (pManipulator->UseKeyframeRange)
	{
		keyIn = pManipulator->KeyframeIn;
		keyLen = pManipulator->KeyframeLen;
		keyOut = pManipulator->KeyframeOut;
	}

	pManipulator->AddBlendShape( curTime, 
		pManipulator->UseConstraint, 
		pManipulator->AutoKeyframe , 
		keyIn, 
		keyLen, 
		keyOut, 
		overrideExisting );
	
	// reset our temp deformer
	pManipulator->Reset();
	
	FBSystem::TheOne().Scene->Evaluate();
	FBSystem::TheOne().Scene->EvaluateDeformations();

	pManipulator->Active = true;
}

void ORManip_Sculpt::AddBlendShape(	const FBTime &curTime, 
									const bool useContraint, 
									const bool setKeyframe, 
									const double timeIn, 
									const double timeLen, 
									const double timeOut, 
									const bool replaceExisting)
{
	
	FBModel *pModel = nullptr;
	
	if (mBuffer.pModel.Ok() )
		pModel = (FBModel*) mBuffer.pModel.GetPlug();

	if (pModel == nullptr)
		return;

	if (useContraint)
	{
		// UI option - create a new constraint
		if (mCurrentConstraint.Ok() == false)
		{
			// 1 - delete a temp deformer
			FreeTempDeformer();

			// 2 - create a new constraint
			BlendShapeDeformerConstraint *pConstraint = (BlendShapeDeformerConstraint*) FBConstraintManager::TheOne().TypeCreateConstraint( "BlendShape Deformer" );
			pConstraint->ReferenceAdd(0, pModel);
			pConstraint->Active = true;
			mCurrentConstraint = pConstraint;

			// 3 - create a temp deformer
			InitTempDeformer(pModel, this);
		}

		AddBlendShapeConstraint(curTime, mCurrentConstraint, setKeyframe, timeIn, timeLen, timeOut, replaceExisting);
	}
	else
	{
		
		AddBlendShapeDynamic(curTime, setKeyframe, timeIn, timeLen, timeOut, replaceExisting);

	}
}

void ORManip_Sculpt::AddBlendShapeConstraint(const FBTime &curTime, BlendShapeDeformerConstraint *pConstraint, const bool setKeyframe, const double timeIn, const double timeLen, const double timeOut, const bool replaceExisting)
{
	FBModel *pModel = (FBModel*) mBuffer.pModel.GetPlug();
	
	pConstraint->AddBlendShape( curTime, pModel, mBufferZero, mBuffer, setKeyframe, timeIn, timeLen, timeOut, replaceExisting );
}

void ORManip_Sculpt::AddBlendShapeDynamic(const FBTime &_currTime, const bool setKeyframe, const double timeIn, const double timeLen, const double timeOut, const bool replaceExisting)
{
	FBModel *pModel = (FBModel*) mBuffer.pModel.GetPlug();
	FBGeometry *pGeom = pModel->Geometry;
	// 1 - calculate differences

	const int numberOfBaseVertices = pGeom->VertexCount();
	const int numberOfVertices = (numberOfBaseVertices < mBuffer.vertices.size()) ? numberOfBaseVertices : mBuffer.vertices.size() - 1;

	if (numberOfVertices == 0) 
		return;

	int difCount = 0;

	FBNormal n1, n2;


	FBArrayTemplate<double>		difLength;

	FBArrayTemplate<int>		difIndex;
	FBArrayTemplate<FBVertex>	difPositions;
	FBArrayTemplate<FBNormal>	difNormals;

	difIndex.SetCount(numberOfVertices);
	difPositions.SetCount(numberOfVertices);
	difNormals.SetCount(numberOfVertices);

	for (int i=0; i<numberOfBaseVertices; ++i)
	{
		FBVector3dToVertex( mBuffer.vertices[i].position, difPositions[i] );
		difNormals[i] = mBuffer.vertices[i].normal;
	}

	// calcualate which vertices have real effect in that blendshape
	difLength.SetCount( numberOfVertices );
	for (int i=0; i<numberOfVertices; ++i)
	{
		const FBVector3d &va = mBufferZero.vertices[i].position;
		const FBVector3d &vb = mBuffer.vertices[i].position;
		FBVector3d vdif(vb[0]-va[0], vb[1]-va[1], vb[2]-va[2]);
		difLength[i] = sqrt( vdif[0]*vdif[0] + vdif[1]*vdif[1] + vdif[2]*vdif[2] );
	}

	//
	CalculateDeltaMesh( pModel, numberOfVertices, difLength.GetArray(), difPositions.GetArray(), difNormals.GetArray(), difPositions.GetArray(), difNormals.GetArray() );

	//FBMatrix tm;
	//pModel->GetMatrix(tm, kModelInverse_Transformation_Geometry);

	// calculate difference from the base geometry
	FBVertex *positionsBase = pGeom->GetVertexes();
	for (int i=0; i<numberOfVertices; ++i)
	{
		difIndex[i] = -1;

		const FBVertex &v1 = difPositions[i];
		const FBVertex &v2 = positionsBase[i];

		if (difLength[i] > 0.001)
		{
			difIndex[i] = i;
			difPositions[i] = FBVertex(v1[0]-v2[0], v1[1]-v2[1], v1[2]-v2[2]);

			n1 = difNormals[i];
			n2 = pGeom->VertexNormalGet(i);

			difNormals[i] = FBNormal(	n1[0]-n2[0],
										n1[1]-n2[1],
										n1[2]-n2[2] );

			difCount++;
		}
	}


	// 2 - add a new blendshape
	int shapeId = ::AddBlendShape( pModel,
						numberOfVertices, 
						difIndex.GetArray(), 
						difCount, 
						difPositions.GetArray(),
						difNormals.GetArray() );

	// 3 - find shape with that id and set keyframes
	if ( setKeyframe && (shapeId >= 0))
	{
		FBGeometry *pGeometry = pModel->Geometry;
		FBString shapeName = pGeometry->ShapeGetName(shapeId);	
		FBProperty *pProperty = pModel->PropertyList.Find( shapeName );

		double ValueOne = 100.0;
		double ValueZero = 0.0;

		FBTime lTimeIn(0,0,0, timeIn);
		FBTime lTimeOut(0,0,0, timeOut);
		FBTime lA(_currTime);
		FBTime lB(_currTime);
		FBTime lLen(0,0,0, timeLen * 0.5);

		lA -= lLen;
		lB += lLen;

		lTimeIn = lA - lTimeIn;
		lTimeOut = lB+ lTimeOut;

		if (pProperty != nullptr && pProperty->IsAnimatable() )
		{
			FBPropertyAnimatable *pPropAnim = (FBPropertyAnimatable*) pProperty;

			pPropAnim->SetAnimated(true);
			FBAnimationNode *pNode = pPropAnim->GetAnimationNode();

			if (pNode)
			{
				pNode->KeyAdd(lA, &ValueOne, kFBInterpolationCubic, kFBTangentModeUser);

				if (lA != lB)
					pNode->KeyAdd(lB, &ValueOne, kFBInterpolationCubic, kFBTangentModeUser);
				if (lA != lTimeIn)
					pNode->KeyAdd(lTimeIn, &ValueZero, kFBInterpolationCubic, kFBTangentModeUser);
				if (lA != lTimeOut)
					pNode->KeyAdd(lTimeOut, &ValueZero, kFBInterpolationCubic, kFBTangentModeUser);
			}	

			pProperty->SetData( &ValueOne );
			pPropAnim->SetFocus(true);
		}
	}
}

void ORManip_Sculpt::ApplyGeometryChanges()
{
	if (mBuffer.pModel.Ok() == false)
		return;

	FBModel *pModel = (FBModel*) mBuffer.pModel.GetPlug();

	if (UseConstraint == false && pModel->SkeletonDeformable == false)
		ApplyBufferDifferenceToGeometry( mBufferZero, mBuffer, pModel );

	//
	StoreFreezeState();
}

void ORManip_Sculpt::EventSceneChange(HISender pSender, HKEvent pEvent)
{
	FBEventSceneChange	lEvent(pEvent);

	if (lEvent.Type.AsInt() == kFBSceneChangeDetach && FBIS(lEvent.ChildComponent, BlendShapeDeformerConstraint))
	{
		SetCurrentConstraint(nullptr);
	}
}

void ORManip_Sculpt::EventFileNewOrOpen(HISender pSender, HKEvent pEvent)
{
	FreeFreezeState();
}
