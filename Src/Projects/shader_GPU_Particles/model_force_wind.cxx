
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: model_force_wind.cxx
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "model_force_wind.h"

#include "checkglerror.h"
#include "ParticlesDrawHelper.h"
#include "FBCommon.h"
#include "math3d.h"

using namespace GPUParticles;

FBClassImplementation( ForceWind );								                //!< Register class
FBStorableCustomModelImplementation( ForceWind, FORCEWIND__DESCSTR );			//!< Register to the store/retrieve system
FBElementClassImplementation2( ForceWind, "Force Wind", FB_DEFAULT_SDK_ICON );	                //!< Register to the asset system

void AddPropertyViewForForceWind(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(FORCEWIND__CLASSSTR, pPropertyName, pHierarchy);
}
void ForceWind::AddPropertiesToPropertyViewManager()
{
	
	AddPropertyViewForForceWind("Enabled", "");
	AddPropertyViewForForceWind("Show Debug", "");

	//
	AddPropertyViewForForceWind("Common", "", true);
	AddPropertyViewForForceWind("Magnitude", "Common");
	AddPropertyViewForForceWind("Local influence", "Common");
	AddPropertyViewForForceWind("Influence Radius", "Common");
	AddPropertyViewForForceWind("Cone Angle", "Common");
	AddPropertyViewForForceWind("Use Noise", "Common");
	AddPropertyViewForForceWind("Noise Frequency", "Common");
	AddPropertyViewForForceWind("Noise Speed", "Common");

	// 
	AddPropertyViewForForceWind("Turbulence", "", true);
	AddPropertyViewForForceWind("Use Turbulence", "Turbulence");
	AddPropertyViewForForceWind("Amplitude", "Turbulence");
	AddPropertyViewForForceWind("Frequency", "Turbulence");
}

/************************************************
*	Constructor.
************************************************/
ForceWind::ForceWind( const char* pName, HIObject pObject ) 
    : FBModelMarker( pName, pObject )
{
    FBClassInit;
}

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool ForceWind::FBCreate()
{
    ShadingMode = kFBModelShadingTexture;
    Size = 50.0;
	Color = FBColor(1.0, 0.0, 0.0);
    Length = 1.0;
    ResLevel = kFBMarkerMediumResolution;
    Look = kFBMarkerLookCube;
    Type = kFBMarkerTypeStandard;
    Show = true;
    mPickedSubItem = -1;

	FBPropertyPublish( this, Enabled, "Enabled", nullptr, nullptr );
	FBPropertyPublish( this, ShowDebug, "Show Debug", nullptr, nullptr );
	FBPropertyPublish( this, Magnitude, "Magnitude", nullptr, nullptr );
	FBPropertyPublish( this, UseNoise, "Use Noise", nullptr, nullptr );
	FBPropertyPublish( this, NoiseFrequency, "Noise Frequency", nullptr, nullptr );
	FBPropertyPublish( this, NoiseSpeed, "Noise Speed", nullptr, nullptr );
	FBPropertyPublish( this, LocalInfluence, "Local influence", nullptr, nullptr );
	FBPropertyPublish( this, InfluenceRadius, "Influence Radius", nullptr, nullptr );
	FBPropertyPublish( this, ConeAngle, "Cone Angle", nullptr, nullptr );
	FBPropertyPublish( this, Turbulence, "Use Turbulence", nullptr, nullptr );
	FBPropertyPublish( this, Amplitude, "Amplitude", nullptr, nullptr );
	FBPropertyPublish( this, Frequency, "Frequency", nullptr, nullptr );

	Enabled = true;
	ShowDebug = false;
	Magnitude = 1.0;
	LocalInfluence = false;
	InfluenceRadius = 100.0;
	ConeAngle = 40.0;
	Turbulence = false;
	Amplitude = 8.0;
	Frequency = 0.2;

	NoiseFrequency = 0.01;
	NoiseSpeed = 10.0;

    return true;
}

/************************************************
*	FiLMBOX Destructor.
************************************************/
void ForceWind::FBDestroy()
{
    ParentClass::FBDestroy();
}

void ForceWind::SetWindVectors( const FBVector4d wind1, const FBVector4d wind2, const FBVector4d wind3, const FBVector4d wind4 )
{
	mWind1 = wind1;
	mWind2 = wind2;
	mWind3 = wind3;
	mWind4 = wind4;
}

/** Custom display
*/
void ForceWind::CustomModelDisplay( FBCamera* pCamera, FBModelShadingMode pShadingMode, FBModelRenderPass pRenderPass, float pPickingAreaWidth, float pPickingAreaHeight)
{
    FBMatrix		 MatrixView;
    FBMatrix		 MatrixProjection;

    FBViewingOptions* lViewingOptions = FBSystem::TheOne().Renderer->GetViewingOptions();
    bool lIsSelectBufferPicking = (pRenderPass == kFBModelRenderPassPick) || lViewingOptions->IsInSelectionBufferPicking();
    bool lIsColorBufferPicking  = lViewingOptions->IsInColorBufferPicking();

	if (lViewingOptions->PickingMode() >=  kFBPickingModeModelsOnly)
		return;

    //double lScale = 100;
	FBMatrix m;

    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_POLYGON_BIT | GL_TRANSFORM_BIT | GL_LINE_BIT); //Push Current GL states.
    {
		double lSize = Size;

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        {
			GetMatrix(m);
			glMultMatrixd(m);
            
            if (!lIsSelectBufferPicking && !lIsColorBufferPicking) 
            { 
                if ((bool)Selected) 
                {
					glColor3d( 0.0, 1.0, 0.0 );   //!< Otherwise, BLUE.
                } 
                else 
                {
					FBColor lColor = Color;
                    glColor3dv(lColor);
                }
            }
			else
			{
				FBColor lColor = UniqueColorId;
				glColor3dv(lColor);
				
				glLoadName(1);
			}

            glLineWidth(3.0);    //!< Draw line wider to easy picking.

            // Draw with Model's Unique ColorID  (or selection name (-1) set by Mobu Internally). 
			
			double lArrowLength = lSize * 2.0;
			double lArrowSize = lSize * 0.2;
			
			glBegin(GL_LINES);

				// draw arrow
				glVertex3d(0.0, 0.0, 0.0);
				glVertex3d(0.0, 0.0, lArrowLength);

				glVertex3d(0.0, 0.0, lArrowLength);
				glVertex3d(lArrowSize, 0.0, lArrowLength - lArrowSize);

				glVertex3d(0.0, 0.0, lArrowLength);
				glVertex3d(-lArrowSize, 0.0, lArrowLength - lArrowSize);

				glVertex3d(0.0, 0.0, lArrowLength);
				glVertex3d(0.0, lArrowSize, lArrowLength - lArrowSize);

				glVertex3d(0.0, 0.0, lArrowLength);
				glVertex3d(0.0, -lArrowSize, lArrowLength - lArrowSize);

				// draw work plane
				glVertex3d(-lSize, -lSize, 0.0);
				glVertex3d(-lSize, lSize, 0.0);

				glVertex3d(-lSize, lSize, 0.0);
				glVertex3d(lSize, lSize, 0.0);

				glVertex3d(lSize, lSize, 0.0);
				glVertex3d(lSize, -lSize, 0.0);

				glVertex3d(lSize, -lSize, 0.0);
				glVertex3d(-lSize, -lSize, 0.0);

            glEnd();

			
        }
        glPopMatrix();

		if (ShowDebug == true)
		{
			glColor3d(1.0, 0.0, 0.0);

			glBegin(GL_LINES);

				glVertex3d(0.0, 0.0, 0.0);
				glVertex3d( mWind1[0] * lSize, mWind1[1] * lSize, mWind1[2] * lSize );

				glVertex3d(0.0, 0.0, 0.0);
				glVertex3d( mWind2[0] * lSize, mWind2[1] * lSize, mWind2[2] * lSize );

				glVertex3d(0.0, 0.0, 0.0);
				glVertex3d (mWind3[0] * lSize, mWind3[1] * lSize, mWind3[2] * lSize );

				glVertex3d(0.0, 0.0, 0.0);
				glVertex3d( mWind4[0] * lSize, mWind4[1] * lSize, mWind4[2] * lSize );

			glEnd();
		}

		if (Selected && LocalInfluence)
		{
			FBVector3d T;
			GetVector(T);
			DrawCircleBillboard(pCamera, InfluenceRadius, 16, T);
		}
    }
    glPopAttrib();

	CHECK_GL_ERROR();
}

bool ForceWind::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
    return true;
}

bool ForceWind::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
    return true;
}


void SetRotation(const FBVector3d axis, const double angle_radian, FBQuaternion &quat)
{
	// This function assumes that the axis vector has been normalized.
	double halfAng = 0.5 * angle_radian;
    double sinHalf = sin(halfAng);
	quat[3] = cos(halfAng);

    quat[0] = sinHalf * axis[0];
    quat[1] = sinHalf * axis[1];
    quat[2] = sinHalf * axis[2];
}

const float MATH_PI2 = 3.14159265359f;
#define DEG_TO_RAD2(d) (d * MATH_PI2 / 180)

FBVector3d QuatMultVect(const FBQuaternion &q, const FBVector3d &p)
{
	FBVector3d uv, uuv;
	FBVector3d qvec(q[0], q[1], q[2]);
	uv = CrossProduct(qvec, p);
	uuv = CrossProduct(qvec, uv);

	VectorMult( uv, 2.0 * q[3] );
	VectorMult( uuv, 2.0 );

	FBVector3d res;
	res = VectorAdd(p, uv);
	res = VectorAdd(res, uuv);

	return res;
}

void ForceWind::FillForceData( TForce &data, const float elapsedTime )
{
	using namespace nv;
	FBVector4d wind1, wind2, wind3, wind4;

	FBVector3d P, T(0.0, 0.0, 1.0);
	FBMatrix m;
	GetVector(P);
	GetMatrix(m);
	VectorTransform33( T, m, T );
			
	m[12] = 0.0;
	m[13] = 0.0;
	m[14] = 0.0;

	// ! Wind type is 1, write it to the position w component

	const float windMag = 0.01f * (float) Magnitude;

	float wM = windMag * (pow( sin(0.05f*elapsedTime), 2.0f ) + 0.5f);

	FBVector3d windDirN(P);
	VectorNormalize(windDirN);
				
	FBVector3d XAxis(0.0, 0.0, 1.0);
	FBVector3d xCrossW = CrossProduct(XAxis, windDirN);
	xCrossW = XAxis;

	FBQuaternion rotFromXAxisToWindDir;
	rotFromXAxisToWindDir.Init();
	rotFromXAxisToWindDir[0] = rotFromXAxisToWindDir[1] = rotFromXAxisToWindDir[2] = 0.0;
	rotFromXAxisToWindDir[3] = 1.0;

	float angle = asin(VectorLength(xCrossW));

	if ( angle > 0.001 )
	{
		VectorNormalize(xCrossW);
		SetRotation(xCrossW, angle, rotFromXAxisToWindDir);
	}

	float angleToWideWindCone = DEG_TO_RAD2( ConeAngle );

	{
		FBVector3d rotAxis(0.0, 1.0, 0.0);
			
		FBQuaternion rot;
		SetRotation(rotAxis, angleToWideWindCone, rot);
					
		//FBQMult( rot, rotFromXAxisToWindDir, rot );
		FBVector3d newWindDir = QuatMultVect(rot, XAxis);

		wind1 = FBVector4d(newWindDir[0]*wM, newWindDir[1]*wM, newWindDir[2]*wM);
		FBVectorMatrixMult( wind1, m, wind1 );
		ForceExchange::SetDirection( data, vec3( (float)wind1[0], (float)wind1[1], (float)wind1[2]), elapsedTime );
	}

	{
		FBVector3d rotAxis(0.0, -1.0, 0.0);

		FBQuaternion rot;
		SetRotation(rotAxis, angleToWideWindCone, rot);
					
		//FBQMult( rot, rotFromXAxisToWindDir, rot );
		FBVector3d newWindDir = QuatMultVect(rot, XAxis);

		wind2 = FBVector4d(newWindDir[0]*wM, newWindDir[1]*wM, newWindDir[2]*wM);
		FBVectorMatrixMult( wind2, m, wind2 );
		ForceExchange::SetTurbulence( data, vec3( (float)wind2[0], (float)wind2[1], (float)wind2[2]), elapsedTime);
	}

	{
		FBVector3d rotAxis(1.0, 0.0, 0.0);

		FBQuaternion rot;
		SetRotation(rotAxis, angleToWideWindCone, rot);
					
		//FBQMult( rot, rotFromXAxisToWindDir, rot );
		FBVector3d newWindDir = QuatMultVect(rot, XAxis);

		wind3 = FBVector4d(newWindDir[0]*wM, newWindDir[1]*wM, newWindDir[2]*wM);
		FBVectorMatrixMult( wind3, m, wind3 );
		ForceExchange::SetWind1( data, vec3( (float)wind3[0], (float)wind3[1], (float)wind3[2]), elapsedTime );
	}

	{
		FBVector3d	rotAxis(-1.0, 0.0, 0.0);

		FBQuaternion rot;
		SetRotation(rotAxis, angleToWideWindCone, rot);
					
		//FBQMult( rot, rotFromXAxisToWindDir, rot );
		FBVector3d newWindDir = QuatMultVect(rot, XAxis);

		wind4 = FBVector4d(newWindDir[0]*wM, newWindDir[1]*wM, newWindDir[2]*wM);
		FBVectorMatrixMult( wind4, m, wind4 );
		ForceExchange::SetWind2( data, nv::vec3( (float)wind4[0], (float)wind4[1], (float)wind4[2]), elapsedTime );
	}

	SetWindVectors( wind1, wind2, wind3, wind4 );

	int iEnabled = (Enabled) ? PARTICLE_FORCE_WIND_TYPE : PARTICLE_FORCE_DISABLED;

	ForceExchange::SetPosition( data, iEnabled, vec3( (float)P[0], (float)P[1], (float)P[2]) );
	//data[i].SetDirection( vec3( T[0], T[1], T[2]), 0.0 );
	ForceExchange::SetMagnitude( data, Magnitude );
	ForceExchange::SetRadius( data, (LocalInfluence) ? InfluenceRadius : 0.0 );
	ForceExchange::SetNoise( data, (UseNoise) ? 1.0 : 0.0, NoiseFrequency, NoiseSpeed );
	//data[i].SetTurbulence( pWind->Turbulence, pWind->Amplitude, pWind->Frequency );
	//data[i].SetWind( gWind, 0.0 );
}