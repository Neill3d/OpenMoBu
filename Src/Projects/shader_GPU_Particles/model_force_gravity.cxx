
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: model_force_gravity.cxx
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "model_force_gravity.h"
#include "checkglerror.h"
#include "ParticlesDrawHelper.h"
#include "FBCommon.h"
#include "math3d.h"

using namespace GPUParticles;

FBClassImplementation( ForceGravity );								                //!< Register class
FBStorableCustomModelImplementation( ForceGravity, FORCEGRAVITY__DESCSTR );			//!< Register to the store/retrieve system
FBElementClassImplementation2( ForceGravity, "Force Gravity", FB_DEFAULT_SDK_ICON );	                //!< Register to the asset system

void AddPropertyViewForForceDrag(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(FORCEGRAVITY__CLASSSTR, pPropertyName, pHierarchy);
}

void ForceGravity::AddPropertiesToPropertyViewManager()
{
	
	AddPropertyViewForForceDrag("Enabled", "");

	//
	AddPropertyViewForForceDrag("Common", "", true);
	AddPropertyViewForForceDrag("Magnitude", "Common");
	AddPropertyViewForForceDrag("Local influence", "Common");
	AddPropertyViewForForceDrag("Influence Radius", "Common");

	// 
	AddPropertyViewForForceDrag("Turbulence", "", true);
	AddPropertyViewForForceDrag("Use Turbulence", "Turbulence");
	AddPropertyViewForForceDrag("Amplitude", "Turbulence");
	AddPropertyViewForForceDrag("Frequency", "Turbulence");
}

/************************************************
*	Constructor.
************************************************/
ForceGravity::ForceGravity( const char* pName, HIObject pObject ) 
    : FBModelMarker( pName, pObject )
{
    FBClassInit;
}

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool ForceGravity::FBCreate()
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
	FBPropertyPublish( this, Magnitude, "Magnitude", nullptr, nullptr );
	FBPropertyPublish( this, LocalInfluence, "Local influence", nullptr, nullptr );
	FBPropertyPublish( this, InfluenceRadius, "Influence Radius", nullptr, nullptr );
	FBPropertyPublish( this, Turbulence, "Use Turbulence", nullptr, nullptr );
	FBPropertyPublish( this, Amplitude, "Amplitude", nullptr, nullptr );
	FBPropertyPublish( this, Frequency, "Frequency", nullptr, nullptr );

	FBPropertyPublish( this, UserDirection, "User Direction", nullptr, nullptr );
	FBPropertyPublish( this, Direction, "Direction", nullptr, nullptr );

	Enabled = true;
	Magnitude = 1.0;
	LocalInfluence = false;
	InfluenceRadius = 100.0;
	Turbulence = false;
	Amplitude = 8.0;
	Frequency = 0.2;

	UserDirection = false;
	Direction = FBVector3d(0.0, 1.0, 0.0);

    return true;
}

/************************************************
*	FiLMBOX Destructor.
************************************************/
void ForceGravity::FBDestroy()
{
    ParentClass::FBDestroy();
}

void DrawArrow(const int type, const double lineLen, const double arrowlen)
{
	glBegin(GL_LINES);

		switch (type)
		{
		case 0:
			// draw arrow
			glVertex3d(0.0, 0.0, 0.0);
			glVertex3d(0.0, 0.0, lineLen);

			glVertex3d(0.0, 0.0, lineLen);
			glVertex3d(arrowlen, 0.0, lineLen - arrowlen);

			glVertex3d(0.0, 0.0, lineLen);
			glVertex3d(-arrowlen, 0.0, lineLen - arrowlen);

			glVertex3d(0.0, 0.0, lineLen);
			glVertex3d(0.0, arrowlen, lineLen - arrowlen);

			glVertex3d(0.0, 0.0, lineLen);
			glVertex3d(0.0, -arrowlen, lineLen - arrowlen);

			//
			glVertex3d(0.0, 0.0, 0.0);
			glVertex3d(0.0, 0.0, -lineLen);

			glVertex3d(0.0, 0.0, -lineLen);
			glVertex3d(arrowlen, 0.0, -lineLen + arrowlen);

			glVertex3d(0.0, 0.0, -lineLen);
			glVertex3d(-arrowlen, 0.0, -lineLen + arrowlen);

			glVertex3d(0.0, 0.0, -lineLen);
			glVertex3d(0.0, arrowlen, -lineLen + arrowlen);

			glVertex3d(0.0, 0.0, -lineLen);
			glVertex3d(0.0, -arrowlen, -lineLen + arrowlen);
			break;
		case 1:
			// draw arrow
			glVertex3d(0.0, 0.0, 0.0);
			glVertex3d(lineLen, 0.0, 0.0);

			glVertex3d(lineLen, 0.0, 0.0);
			glVertex3d(lineLen - arrowlen, 0.0, arrowlen);

			glVertex3d(lineLen, 0.0, 0.0);
			glVertex3d(lineLen - arrowlen, 0.0, -arrowlen);

			glVertex3d(lineLen, 0.0, 0.0);
			glVertex3d(lineLen - arrowlen, arrowlen, 0.0);

			glVertex3d(lineLen, 0.0, 0.0);
			glVertex3d(lineLen - arrowlen, -arrowlen, 0.0);

			//
			glVertex3d(0.0, 0.0, 0.0);
			glVertex3d(-lineLen, 0.0, 0.0);

			glVertex3d(-lineLen, 0.0, 0.0);
			glVertex3d(-lineLen + arrowlen, 0.0, arrowlen);

			glVertex3d(-lineLen, 0.0, 0.0);
			glVertex3d(-lineLen + arrowlen, 0.0, -arrowlen);

			glVertex3d(-lineLen, 0.0, 0.0);
			glVertex3d(-lineLen + arrowlen, arrowlen, 0.0);

			glVertex3d(-lineLen, 0.0, 0.0);
			glVertex3d(-lineLen + arrowlen, -arrowlen, 0.0);
			break;
		case 2:
			// draw arrow
			glVertex3d(0.0, 0.0, 0.0);
			glVertex3d(0.0, lineLen, 0.0);

			glVertex3d(0.0, lineLen, 0.0);
			glVertex3d(0.0, lineLen - arrowlen, arrowlen);

			glVertex3d(0.0, lineLen, 0.0);
			glVertex3d(0.0, lineLen - arrowlen, -arrowlen);

			glVertex3d(0.0, lineLen, 0.0);
			glVertex3d(arrowlen, lineLen - arrowlen, 0.0);

			glVertex3d(0.0, lineLen, 0.0);
			glVertex3d(-arrowlen, lineLen - arrowlen, 0.0);

			//
			glVertex3d(0.0, 0.0, 0.0);
			glVertex3d(0.0, -lineLen, 0.0);

			glVertex3d(0.0, -lineLen, 0.0);
			glVertex3d(0.0, -lineLen + arrowlen, arrowlen);

			glVertex3d(0.0, -lineLen, 0.0);
			glVertex3d(0.0, -lineLen + arrowlen, -arrowlen);

			glVertex3d(0.0, -lineLen, 0.0);
			glVertex3d(arrowlen, -lineLen + arrowlen, 0.0);

			glVertex3d(0.0, -lineLen, 0.0);
			glVertex3d(-arrowlen, -lineLen + arrowlen, 0.0);
			break;
		}

	glEnd();
}

/** Custom display
*/
void ForceGravity::CustomModelDisplay( FBCamera* pCamera, FBModelShadingMode pShadingMode, FBModelRenderPass pRenderPass, float pPickingAreaWidth, float pPickingAreaHeight)
{
    FBMatrix		 MatrixView;
    FBMatrix		 MatrixProjection;

    FBViewingOptions* lViewingOptions = FBSystem::TheOne().Renderer->GetViewingOptions();
    bool lIsSelectBufferPicking = (pRenderPass == kFBModelRenderPassPick) || lViewingOptions->IsInSelectionBufferPicking();
    bool lIsColorBufferPicking  = lViewingOptions->IsInColorBufferPicking();

	if (lViewingOptions->PickingMode() >=  kFBPickingModeModelsOnly)
		return;
   
	FBMatrix m;

    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_POLYGON_BIT | GL_TRANSFORM_BIT | GL_LINE_BIT); //Push Current GL states.
    {
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
			double lSize = Size;
			double lArrowLength = lSize;
			double lArrowSize = lSize * 0.2;
			
			DrawArrow(0, lArrowLength, lArrowSize);
			DrawArrow(1, lArrowLength, lArrowSize);
			DrawArrow(2, lArrowLength, lArrowSize);
        }
        glPopMatrix();

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

bool ForceGravity::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
    return true;
}

bool ForceGravity::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
    return true;
}


void ForceGravity::FillForceData( TForce &data )
{
	
	FBVector3d P, T(0.0, 0.0, 1.0);
	FBMatrix m;

	GetVector(P);
	//pDrag->GetMatrix(m);
	//VectorTransform33( T, m, T );
				
	T = VectorSubtract(P, LastTranslation);
	LastTranslation = P;

	int iEnabled = (Enabled) ? PARTICLE_FORCE_GRAVITY_TYPE : PARTICLE_FORCE_DISABLED;
	
	if (UserDirection)
	{
		T = Direction;
	}

	ForceExchange::SetPosition( data, iEnabled, vec3((float)P[0], (float)P[1], (float)P[2]) );
	ForceExchange::SetDirection( data, vec3((float)T[0], (float)T[1], (float)T[2]), 0.0);
	ForceExchange::SetMagnitude( data, (float) Magnitude );
	ForceExchange::SetRadius( data, (LocalInfluence) ? InfluenceRadius : 0.0 );
	ForceExchange::SetTurbulence( data, Turbulence, Amplitude, Frequency );
}