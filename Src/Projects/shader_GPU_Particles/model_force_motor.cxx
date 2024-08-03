
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: model_force_motor.cxx
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "model_force_motor.h"

#include "checkglerror.h"
#include "ParticlesDrawHelper.h"
#include "FBCommon.h"
#include "math3d.h"

using namespace GPUParticles;

FBClassImplementation( ForceMotor );								                //!< Register class
FBStorableCustomModelImplementation( ForceMotor, FORCEMOTOR__CLASSSTR );			//!< Register to the store/retrieve system
FBElementClassImplementation2( ForceMotor, "Force Motor", FB_DEFAULT_SDK_ICON );	                //!< Register to the asset system

/************************************************
*	Constructor.
************************************************/
ForceMotor::ForceMotor( const char* pName, HIObject pObject ) 
    : FBModelMarker( pName, pObject )
{
    FBClassInit;
}

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool ForceMotor::FBCreate()
{
    ShadingMode = kFBModelShadingTexture;
    Size = 50.0;
    Length = 1.0;
    ResLevel = kFBMarkerMediumResolution;
    Look = kFBMarkerLookCube;
    Type = kFBMarkerTypeStandard;
    Show = true;
    mPickedSubItem = -1;

	FBPropertyPublish( this, Enabled, "Enabled", nullptr, nullptr );
	FBPropertyPublish( this, NewVortexMath, "New Vortex Math", nullptr, nullptr );
	FBPropertyPublish( this, Magnitude, "Magnitude", nullptr, nullptr );
	FBPropertyPublish( this, RotationSpeed, "Rotation Speed", nullptr, nullptr );
	FBPropertyPublish( this, LocalInfluence, "Local influence", nullptr, nullptr );
	FBPropertyPublish( this, InfluenceRadius, "Influence Radius", nullptr, nullptr );
	FBPropertyPublish( this, Turbulence, "Use Turbulence", nullptr, nullptr );
	FBPropertyPublish( this, Amplitude, "Amplitude", nullptr, nullptr );
	FBPropertyPublish( this, Frequency, "Frequency", nullptr, nullptr );


	Enabled = true;
	NewVortexMath = true;
	Magnitude = 1.0;
	RotationSpeed = 1.0;
	LocalInfluence = false;
	InfluenceRadius = 100.0;
	Turbulence = false;
	Amplitude = 8.0;
	Frequency = 0.2;

    return true;
}

/************************************************
*	FiLMBOX Destructor.
************************************************/
void ForceMotor::FBDestroy()
{
    ParentClass::FBDestroy();
}

/** Custom display
*/
void ForceMotor::CustomModelDisplay( FBCamera* pCamera, FBModelShadingMode pShadingMode, FBModelRenderPass pRenderPass, float pPickingAreaWidth, float pPickingAreaHeight)
{
    FBMatrix		 MatrixView;
    FBMatrix		 MatrixProjection;

    FBViewingOptions* lViewingOptions = FBSystem::TheOne().Renderer->GetViewingOptions();
    bool lIsSelectBufferPicking = (pRenderPass == kFBModelRenderPassPick) || lViewingOptions->IsInSelectionBufferPicking();
    bool lIsColorBufferPicking  = lViewingOptions->IsInColorBufferPicking();

	if (lViewingOptions->PickingMode() >=  kFBPickingModeModelsOnly)
		return;

    //double lScale = 100;

    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_POLYGON_BIT | GL_TRANSFORM_BIT | GL_LINE_BIT); //Push Current GL states.
    {
        glMatrixMode(GL_MODELVIEW);
        
		glPushMatrix();

			FBMatrix m;
			GetMatrix(m);
			glMultMatrixd(m);
            
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
			else
			{
				FBColor lColor = UniqueColorId;
				glColor3dv(lColor);
				
				glLoadName(1);
			}

			glLineWidth(3.0);    //!< Draw line wider to easy picking.

			// Draw with Model's Unique ColorID  (or selection name (-1) set by Mobu Internally). 
           
			double lSize = Size;
			double lArroSize = lSize * 0.3;

			glBegin(GL_LINES);
				glVertex3d(lSize, 0.0, lSize);
				glVertex3d(lSize, 0.0, -lSize);

				glVertex3d(-lSize, 0.0, lSize);
				glVertex3d(-lSize, 0.0, -lSize);

				glVertex3d(0.0, lSize, lSize);
				glVertex3d(0.0, lSize, -lSize);

				glVertex3d(0.0, -lSize, lSize);
				glVertex3d(0.0, -lSize, -lSize);
			glEnd();

			glTranslatef(0.0f, 0.0f, (float) -lSize);
			DrawCircle( lSize, 16 );

			glBegin(GL_LINES);
				glVertex3d(lSize, 0.0, 0.0);
				glVertex3d(lSize-lArroSize, -lArroSize, 0.0);

				glVertex3d(lSize, 0.0, 0.0);
				glVertex3d(lSize+lArroSize, -lArroSize, 0.0);
			glEnd();

			glTranslatef(0.0f, 0.0f, (float) 2.0 * lSize);
			DrawCircle( lSize, 16 );

			glBegin(GL_LINES);
				glVertex3d(lSize, 0.0, 0.0);
				glVertex3d(lSize-lArroSize, -lArroSize, 0.0);

				glVertex3d(lSize, 0.0, 0.0);
				glVertex3d(lSize+lArroSize, -lArroSize, 0.0);
			glEnd();

		glPopMatrix();

		if (Selected && LocalInfluence)
		{
			FBVector3d T;
			GetVector(T);
			DrawCircleBillboard( pCamera, InfluenceRadius, 16, T );
		}
    }
    glPopAttrib();

	CHECK_GL_ERROR();
}

bool ForceMotor::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
    return true;
}

bool ForceMotor::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
    return true;
}


void ForceMotor::FillForceData( TForce &data )
{
	FBVector3d P, T(0.0, 0.0, 1.0);
	FBMatrix m;
	GetVector(P);
	GetMatrix(m);
	VectorTransform33( T, m, T );

	int iEnabled = PARTICLE_FORCE_DISABLED;

	if (Enabled)
	{
		iEnabled = (NewVortexMath) ? PARTICLE_FORCE_VORTEX_TYPE : PARTICLE_FORCE_MOTOR_TYPE;
	}
	
	//float fTurbulence = (Turbulence) ? 1.0f : 0.0f;

	ForceExchange::SetPosition( data, iEnabled, nv::vec3( (float)P[0], (float)P[1], (float)P[2]) );
	ForceExchange::SetDirection( data, nv::vec3( (float)T[0], (float)T[1], (float)T[2]), RotationSpeed );
	ForceExchange::SetMagnitude( data, Magnitude );
	ForceExchange::SetRadius( data, (LocalInfluence) ? InfluenceRadius : 0.0 );
	ForceExchange::SetTurbulence( data, Turbulence, Amplitude, Frequency );
}