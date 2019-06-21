
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////

//--- Class declaration
#include "volume_calculator_model_display.h"
#include <fbsdk\fbsdk-opengl.h>

FBClassImplementation( ModelVolumeCalculator );					//Register class
FBStorableCustomModelImplementation( ModelVolumeCalculator, ORMODELCUSTOMDISPLAY__DESCSTR );	//Register to the store/retrieve system
FBElementClassImplementation( ModelVolumeCalculator, "volume_calculator.tif" );			//Register to the asset system


static void Volume_Solve(HIObject pObject, bool value) {
	ModelVolumeCalculator *model = FBCast<ModelVolumeCalculator>(pObject);
	if (value && model)	model->DoVolumeSolve();
}

static void Volume_SwitchToCamera(HIObject pObject, bool value) {
	ModelVolumeCalculator *model = FBCast<ModelVolumeCalculator>(pObject);
	if (value && model)	{

		FBModelList list;
		FBGetSelectedModels(list);

		if (list.GetCount() && list.GetAt(0)->Is( FBCamera::TypeInfo ) )
		{
			FBScene *pScene = FBSystem::TheOne().Scene;
			for (int i=0; i<pScene->Cameras.GetCount(); ++i)
				pScene->Cameras[i]->ViewNearFarPlane = false;

			FBCamera *pCamera = (FBCamera*) list.GetAt(0);
			pCamera->ViewNearFarPlane = true;
			pScene->Renderer->CurrentCamera = pCamera;
		}
		else
		{
			FBMessageBox( "Volume Calculator", "Please, selected one of the capture cameras.", "Ok" );
		}
	}
}

static void Volume_SetPreset(HIObject object, kCECameraPresets pValue)
{
	ModelVolumeCalculator *model = FBCast<ModelVolumeCalculator>(object);
	if (model) 
	{
		if (model->Cameras.GetCount() )
		{
			for (int i=0; i<model->Cameras.GetCount(); ++i)
			{
				FBCamera *pCamera = (FBCamera*) model->Cameras.GetAt(i);

				switch(pValue)
				{
				case kCECameraPreset_V100R2:
					pCamera->FrameSizeMode = kFBFrameSizeFixedResolution;
					pCamera->ResolutionWidth = 640;
					pCamera->ResolutionHeight = 680;
					pCamera->PixelAspectRatio = 1.33;
					break;
				case kCECameraPreset_S250E:
					pCamera->FrameSizeMode = kFBFrameSizeFixedResolution;
					pCamera->ResolutionWidth = 832;
					pCamera->ResolutionHeight = 832;
					pCamera->PixelAspectRatio = 1.0;
					break;
				case kCECameraPreset_Flex13:
					pCamera->FrameSizeMode = kFBFrameSizeFixedResolution;
					pCamera->ResolutionWidth = 1280;
					pCamera->ResolutionHeight = 1024;
					pCamera->PixelAspectRatio = 1.25;
					break;
				case kCECameraPreset_Prime41:
					pCamera->FrameSizeMode = kFBFrameSizeFixedResolution;
					pCamera->ResolutionWidth = 2048;
					pCamera->ResolutionHeight = 2048;
					pCamera->PixelAspectRatio = 1.0;
					break;
				}

				
			}
		}
		else
		{
			FBMessageBox( "Volume Calculator", "There is no any connected camera", "Ok" );
		}
	}
}

static void Volume_SetLensPreset(HIObject object, kCELensPresets pValue)
{
	ModelVolumeCalculator *model = FBCast<ModelVolumeCalculator>(object);
	if (model) 
	{
		if (model->Cameras.GetCount() )
		{
			for (int i=0; i<model->Cameras.GetCount(); ++i)
			{
				FBCamera *pCamera = (FBCamera*) model->Cameras.GetAt(i);

				switch (pValue)
				{
				case kCELensPreset_38:
					pCamera->FieldOfView = 38.0;
					break;
				case kCELensPreset_42:
					pCamera->FieldOfView = 42.0;
					break;
				case kCELensPreset_43:
					pCamera->FieldOfView = 43.0;
					break;
				case kCELensPreset_46:
					pCamera->FieldOfView = 46.0;
					break;
				case kCELensPreset_51:
					pCamera->FieldOfView = 51.0;
					break;
				case kCELensPreset_56:
					pCamera->FieldOfView = 56.0;
					break;
				case kCELensPreset_58:
					pCamera->FieldOfView = 58.0;
					break;
				}

			}
		}
	}
}

static void Volume_About(HIObject pObject, bool value) {
	ModelVolumeCalculator *model = FBCast<ModelVolumeCalculator>(pObject);
	if (value && model)	{
		FBMessageBox( "About", "Volume calculator created by Sergey Solohin (Neill3d)\n"
			"e-mail to: s@neill3d.com\n"
			"home page: www.neill3d.com\n\n",
			"Ok" );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////// PROCS

void ModelVolumeCalculator::AddPropertyView(const char* pPropertyName, const char* pHierarchy, bool pIsFolder)
{
	FBPropertyViewManager::TheOne().AddPropertyView(ORMODELCUSTOMDISPLAY__CLASSSTR, pPropertyName, pHierarchy);
}
void ModelVolumeCalculator::AddPropertiesToPropertyViewManager()
{
	AddPropertyView("Switch To Camera", "");
	AddPropertyView("Resolution presets", "");
	AddPropertyView("Lens presets", "");

	AddPropertyView("Length", "");
	AddPropertyView("Width", "");
	AddPropertyView("Height", "");
	AddPropertyView("Show room", "");
	AddPropertyView("Show Zone", "");
	AddPropertyView("Circle Zone", "");

	AddPropertyView("Zone X Offset", "");
	AddPropertyView("Zone Z Offset", "");
	AddPropertyView("Zone Length", "");
	AddPropertyView("Zone Width", "");
	AddPropertyView("Show points", "");
	AddPropertyView("Step", "");
	AddPropertyView("Min Coverage", "");
	AddPropertyView("Marker", "");
	AddPropertyView("Cameras", "");
	AddPropertyView("Solve", "");
	AddPropertyView("Marks", "");

	AddPropertyView("", "Display Settings", true);
	AddPropertyView("Room Color", "Display Settings");
	AddPropertyView("Zone Color", "Display Settings");
	AddPropertyView("Point Color", "Display Settings");
	AddPropertyView("Ray Color", "Display Settings");

	AddPropertyView("About", "");
}

/************************************************
 *	Constructor.
 ************************************************/
ModelVolumeCalculator::ModelVolumeCalculator( const char* pName, HIObject pObject ) : FBModel( pName, pObject )
{
	FBClassInit;

	FBPropertyPublish( this, Length, "Length", NULL, NULL );
	FBPropertyPublish( this, Width, "Width", NULL, NULL );
	FBPropertyPublish( this, Height, "Height", NULL, NULL );
	FBPropertyPublish( this, DisplayRoom, "Show room", NULL, NULL );
	FBPropertyPublish( this, DisplayZone, "Show Zone", NULL, NULL );
	FBPropertyPublish( this, IsZoneCircle, "Circle Zone", NULL, NULL );
	FBPropertyPublish( this, ZoneX, "Zone X Offset", NULL, NULL );
	FBPropertyPublish( this, ZoneZ, "Zone Z Offset", NULL, NULL );
	FBPropertyPublish( this, ZoneLength, "Zone Length", NULL, NULL );
	FBPropertyPublish( this, ZoneWidth, "Zone Width", NULL, NULL );
	FBPropertyPublish( this, DisplayPoints, "Show points", NULL, NULL );
	FBPropertyPublish( this, SolveStep, "Step", NULL, NULL );
	FBPropertyPublish( this, CameraCoverage, "Min Coverage", NULL, NULL );
	FBPropertyPublish( this, Marker, "Marker", NULL, NULL );
	FBPropertyPublish( this, Cameras, "Cameras", NULL, NULL );
	FBPropertyPublish( this, Solve, "Solve", NULL, Volume_Solve );
	FBPropertyPublish( this, Marks, "Marks", NULL, NULL );

	FBPropertyPublish(this, RoomColor, "Room Color", nullptr, nullptr);
	FBPropertyPublish(this, ZoneColor, "Zone Color", nullptr, nullptr);
	FBPropertyPublish(this, PointColor, "Point Color", nullptr, nullptr);
	FBPropertyPublish(this, RayColor, "Ray Color", nullptr, nullptr);

	FBPropertyPublish( this, SwitchToCamera, "Switch To Camera", NULL, Volume_SwitchToCamera );
	FBPropertyPublish( this, Presets, "Resolution presets", NULL, Volume_SetPreset );
	FBPropertyPublish( this, LensPresets, "Lens presets", NULL, Volume_SetLensPreset );
	FBPropertyPublish( this, About, "About", NULL, Volume_About );

	Length = 600.0;
	Width = 500.0;
	Height = 300.0;

	DisplayRoom = true;
	DisplayPoints = false;
	
	DisplayZone = false;
	IsZoneCircle = false;
	ZoneX = 0.0;
	ZoneZ = 0.0;
	ZoneLength = 200.0;
	ZoneWidth = 200.0;

	SolveStep = 50.0;
	CameraCoverage = 3;
	Marks = 0;

	RoomColor = FBColor(0.8, 0.8, 0.8);
	ZoneColor = FBColor(0.9, 0.9, 0.9);
	PointColor = FBColor(0.5, 0.5, 0.5);
	RayColor = FBColor(1.0, 0.0, 0.0);
}

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ModelVolumeCalculator::FBCreate()
{
	ShadingMode = kFBModelShadingTexture;

	return true;
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void ModelVolumeCalculator::FBDestroy()
{
}

// X axis is a width
// Z axis is a length

void ModelVolumeCalculator::DrawRoom(double l, double w, double h, const bool is_pick)
{
	const double hl = l * 0.5;
	const double hw = w * 0.5;
	
	glBegin(GL_LINE_LOOP);
	glVertex3d( -hw, 0.0, hl );
	glVertex3d( -hw, 0.0, -hl );
	glVertex3d( hw, 0.0, -hl );
	glVertex3d( hw, 0.0, hl );
	glEnd();

	glLineWidth(2.0f);

	if (!is_pick)
	{
		const FBColor room_color = RoomColor;
		glColor3dv(room_color);
	}
	
	glBegin(GL_LINE_LOOP);
	glVertex3d( -hw, h, hl );
	glVertex3d( -hw, h, -hl );
	glVertex3d( hw, h, -hl );
	glVertex3d( hw, h, hl );
	glEnd();
}

void ModelVolumeCalculator::DrawActiveZone(const bool is_pick)
{
	double hl = Length * 0.5;
	double hw = Width * 0.5;
	//double hh = h * 0.5;

	if (!is_pick)
	{
		const FBColor zone_color = ZoneColor;
		glColor3dv(zone_color);
	}
	
	glBegin(GL_LINES);
	glVertex3d( -hw, 0.0, 0.0);
	glVertex3d( hw, 0.0, 0.0);

	glVertex3d( 0.0, 0.0, -hl);
	glVertex3d( 0.0, 0.0, hl);
	glEnd();

	if (IsZoneCircle)
	{
		float t = 0.0f;
		float maxt = 2 * 3.1415;
		float step = maxt / 16.0f;
		float radiusz = ZoneLength * 0.5;
		float radiusx = ZoneWidth * 0.5;

		FBVector3d W;
	
		glBegin(GL_LINE_LOOP);
		while(t < maxt)
		{
			W = FBVector3d(radiusx*cos(t), 0.0, radiusz*sin(t));

			glVertex3dv(W);
			t += step;
		}
		glEnd();
	
	}
	else
	{
		hl = ZoneLength * 0.5;
		hw = ZoneWidth * 0.5;

		glBegin(GL_LINE_LOOP);
		glVertex3d( -hw, 0.0, hl );
		glVertex3d( -hw, 0.0, -hl );
		glVertex3d( hw, 0.0, -hl );
		glVertex3d( hw, 0.0, hl );
		glEnd();
	}
}

void ModelVolumeCalculator::DrawVolumePoints(const bool is_pick)
{
	if (!mPoints.size() ) return;

	glPointSize(3.0f);

	if (!is_pick)
	{
		const FBColor point_color = PointColor;
		glColor3dv(point_color);
	}
	
	glBegin(GL_POINTS);
	for (size_t i=0; i<mPoints.size(); ++i)
		glVertex3dv(mPoints[i]);
	glEnd();
	glPointSize(1.0f);
}

/** Custom display
*/
void ModelVolumeCalculator::CustomModelDisplay( FBCamera* pCamera, FBModelShadingMode pShadingMode, FBModelRenderPass pRenderPass, float pPickingAreaWidth, float pPickingAreaHeight)
{
  FBMatrix		 MatrixView;
  FBMatrix		 MatrixProjection;

  glLineWidth(4.0f);

	const bool is_pick = (pRenderPass == kFBModelRenderPassPick);
	const bool is_selected = Selected.AsInt() > 0;

	if (is_selected) {
		glColor3d(0.0,1.0,0.0);
	} else {
		const FBColor room_color = RoomColor;
		glColor3dv(room_color);
	}

	if (is_pick)
	{
		const FBColor color_id = UniqueColorId;
		glColor3dv(color_id);
	}

	FBVector3d T = Translation;
	glTranslated(T[0],T[1],T[2]);

	// show the room
	if (DisplayRoom)
	{
		if (is_pick) glLoadName(3);
		DrawRoom(Length, Width, Height, is_pick);
	}

	if (DisplayZone)
	{
		if (is_pick) glLoadName(4);
		glPushMatrix();
		glTranslatef( ZoneX, 0.0, ZoneZ );
		DrawActiveZone(is_pick);
		glPopMatrix();
	}

	if (DisplayPoints)
	{
		DrawVolumePoints(is_pick);
	}

	glLineWidth(1.0f);
	DrawMarkerRays(is_pick);

	glLineWidth(1.0f);
}

/** Custom picking for selection
*/
bool ModelVolumeCalculator::CustomModelPicking(	int pNbHits, unsigned int *pSelectBuffer, FBCamera* pCamera, 
									int pMouseX,int pMouseY,
									FBTVector* localRaySrc, FBTVector* localRayDir,
									FBTVector* pWorldRaySrc, FBTVector* pWorldRayDir,
									FBMatrix* pGlobalInverseMatrix,
									FBTVector* pOutPickedPoint)
{
  int			 NameCount = 0;
  int			 NameIndex;
  bool			 Result = false;

	// no hits 
   	if(!pNbHits) { return false;}

 	for (int Count=0; Count<pNbHits; Count++) {

	  NameCount = *pSelectBuffer++;
	  // Skip two items
	  pSelectBuffer+=2; 

		// Fetch the name 
		NameIndex = *pSelectBuffer++;

		if(NameIndex==1)
		{
			// it is first line
			Result = true;
			if(pOutPickedPoint)
			{
				*pOutPickedPoint = FBTVector(100,100,100,0);
			}
		} else if(NameIndex==2)
		{
			// it is second line
			Result = true;
			if(pOutPickedPoint)
			{
				*pOutPickedPoint = FBTVector(-100,100,100,0);
			}
		}
	}
	return Result; 
}

bool ModelVolumeCalculator::FbxStore(FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}

bool ModelVolumeCalculator::FbxRetrieve(FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat)
{
	return true;
}

struct Camera
{
	void UpdateFrustum(FBCamera *pCamera)
	{
		if (!pCamera) return;

		GLfloat	clip[16];
		FBMatrix proj;
		FBMatrix modl;
		GLfloat	t;

		// Get The Current PROJECTION Matrix From OpenGL
		//glGetFloatv( GL_PROJECTION_MATRIX, proj );
		pCamera->GetCameraMatrix(proj, kFBProjection);
		//proj = pCamera->GetMatrix(kFBProjection);

		// Get The Current MODELVIEW Matrix From OpenGL
		//glGetFloatv( GL_MODELVIEW_MATRIX, modl );
		pCamera->GetCameraMatrix(modl, kFBModelView);
		//modl = pCamera->GetMatrix(kFBModelView);

		// Combine The Two Matrices (Multiply Projection By Modelview)
		clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
		clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
		clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
		clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

		clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
		clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
		clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
		clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

		clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
		clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
		clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
		clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

		clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
		clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
		clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
		clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];

		// Extract The Numbers For The RIGHT Plane
		m_Frustum[0][0] = clip[ 3] - clip[ 0];
		m_Frustum[0][1] = clip[ 7] - clip[ 4];
		m_Frustum[0][2] = clip[11] - clip[ 8];
		m_Frustum[0][3] = clip[15] - clip[12];

		// Normalize The Result
		t = GLfloat(sqrt( m_Frustum[0][0] * m_Frustum[0][0] + m_Frustum[0][1] * m_Frustum[0][1] + m_Frustum[0][2] * m_Frustum[0][2] ));
		m_Frustum[0][0] /= t;
		m_Frustum[0][1] /= t;
		m_Frustum[0][2] /= t;
		m_Frustum[0][3] /= t;

		// Extract The Numbers For The LEFT Plane
		m_Frustum[1][0] = clip[ 3] + clip[ 0];
		m_Frustum[1][1] = clip[ 7] + clip[ 4];
		m_Frustum[1][2] = clip[11] + clip[ 8];
		m_Frustum[1][3] = clip[15] + clip[12];

		// Normalize The Result
		t = GLfloat(sqrt( m_Frustum[1][0] * m_Frustum[1][0] + m_Frustum[1][1] * m_Frustum[1][1] + m_Frustum[1][2] * m_Frustum[1][2] ));
		m_Frustum[1][0] /= t;
		m_Frustum[1][1] /= t;
		m_Frustum[1][2] /= t;
		m_Frustum[1][3] /= t;

		// Extract The BOTTOM Plane
		m_Frustum[2][0] = clip[ 3] + clip[ 1];
		m_Frustum[2][1] = clip[ 7] + clip[ 5];
		m_Frustum[2][2] = clip[11] + clip[ 9];
		m_Frustum[2][3] = clip[15] + clip[13];

		// Normalize The Result
		t = GLfloat(sqrt( m_Frustum[2][0] * m_Frustum[2][0] + m_Frustum[2][1] * m_Frustum[2][1] + m_Frustum[2][2] * m_Frustum[2][2] ));
		m_Frustum[2][0] /= t;
		m_Frustum[2][1] /= t;
		m_Frustum[2][2] /= t;
		m_Frustum[2][3] /= t;

		// Extract The TOP Plane
		m_Frustum[3][0] = clip[ 3] - clip[ 1];
		m_Frustum[3][1] = clip[ 7] - clip[ 5];
		m_Frustum[3][2] = clip[11] - clip[ 9];
		m_Frustum[3][3] = clip[15] - clip[13];

		// Normalize The Result
		t = GLfloat(sqrt( m_Frustum[3][0] * m_Frustum[3][0] + m_Frustum[3][1] * m_Frustum[3][1] + m_Frustum[3][2] * m_Frustum[3][2] ));
		m_Frustum[3][0] /= t;
		m_Frustum[3][1] /= t;
		m_Frustum[3][2] /= t;
		m_Frustum[3][3] /= t;

		// Extract The FAR Plane
		m_Frustum[4][0] = clip[ 3] - clip[ 2];
		m_Frustum[4][1] = clip[ 7] - clip[ 6];
		m_Frustum[4][2] = clip[11] - clip[10];
		m_Frustum[4][3] = clip[15] - clip[14];

		// Normalize The Result
		t = GLfloat(sqrt( m_Frustum[4][0] * m_Frustum[4][0] + m_Frustum[4][1] * m_Frustum[4][1] + m_Frustum[4][2] * m_Frustum[4][2] ));
		m_Frustum[4][0] /= t;
		m_Frustum[4][1] /= t;
		m_Frustum[4][2] /= t;
		m_Frustum[4][3] /= t;

		// Extract The NEAR Plane
		m_Frustum[5][0] = clip[ 3] + clip[ 2];
		m_Frustum[5][1] = clip[ 7] + clip[ 6];
		m_Frustum[5][2] = clip[11] + clip[10];
		m_Frustum[5][3] = clip[15] + clip[14];

		// Normalize The Result
		t = GLfloat(sqrt( m_Frustum[5][0] * m_Frustum[5][0] + m_Frustum[5][1] * m_Frustum[5][1] + m_Frustum[5][2] * m_Frustum[5][2] ));
		m_Frustum[5][0] /= t;
		m_Frustum[5][1] /= t;
		m_Frustum[5][2] /= t;
		m_Frustum[5][3] /= t;
	}

	bool PointInFrustum(FBVector3d p)
	{
		int i;
		// The Idea Behind This Algorithum Is That If The Point
		// Is Inside All 6 Clipping Planes Then It Is Inside Our
		// Viewing Volume So We Can Return True.
		for(i = 0; i < 6; i++)
		{
			if(m_Frustum[i][0] * p[0] + m_Frustum[i][1] * p[1] + m_Frustum[i][2] * p[2] + m_Frustum[i][3] <= 0)
			{
				return false;
			}
		}
		return true;
	}

	public:
		double m_Frustum[6][4];

		FBVector3d position;

};

void ModelVolumeCalculator::PrepareVolumePoints(double l, double w, double h, double step)
{
	const double hl = l * 0.5;
	const double hw = w * 0.5;
	
	double sl = -hl;
	double sw = -hw;
	double sh = 0.0;

	std::vector<Camera>	cameras;
	if (Cameras.GetCount() ) {
		cameras.resize(Cameras.GetCount() );

		for (int i=0; i<cameras.size(); ++i)
			cameras[i].UpdateFrustum( (FBCamera*) Cameras.GetAt(i) );
	}

	// lets calculate the number of points in volume
	long count = l * w * h / step;
	if (!count) return;

	long index = 0;
	mPoints.resize(count);
	FBVector3d v;

	for( sl=-hl; sl < hl; sl+=step)
		for (sw=-hw; sw<hw; sw+=step)
			for (sh=0.0; sh<h; sh+=step)
			{
				if (index < count) {
					v = FBVector3d(sw, sh, sl);

					int tests = 0;
					for (size_t i=0; i<cameras.size(); ++i)
						if (cameras[i].PointInFrustum(v) ) tests++;

					if (tests >= CameraCoverage) {
						mPoints[index] = v;
						index++;
					}
				}
			}

	if (index < count) mPoints.resize(index);
}

void ModelVolumeCalculator::DrawMarkerRays(const bool is_pick)
{
	if (!Marker.GetCount() ) return;

	FBModel *pModel = (FBModel*) Marker.GetAt(0);
	FBVector3d v, v2;
	pModel->GetVector(v);

	std::vector<Camera>	cameras;
	if (Cameras.GetCount() ) {
		cameras.resize(Cameras.GetCount() );

		for (int i=0; i<cameras.size(); ++i)
		{
			cameras[i].UpdateFrustum( (FBCamera*) Cameras.GetAt(i) );
			pModel = (FBModel*) Cameras.GetAt(i);
			pModel->GetVector(cameras[i].position);
		}
	}

	if (!is_pick)
	{
		const FBColor ray_color = RayColor;
		glColor3dv(ray_color);
	}
	
	glBegin(GL_LINES);

	for (size_t i=0; i<cameras.size(); ++i)
		if (cameras[i].PointInFrustum(v) ) {
			glVertex3dv(v);
			glVertex3dv(cameras[i].position);
		}

	glEnd();
}

void ModelVolumeCalculator::DoVolumeSolve()
{
	PrepareVolumePoints(Length, Width, Height, SolveStep);
	Marks = mPoints.size();
}
