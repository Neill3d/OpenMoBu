
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: model_collision_sphere.cxx
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "model_collision_sphere.h"

#include "checkglerror.h"
#include "ParticlesDrawHelper.h"
#include "FBCommon.h"
#include "math3d.h"

using namespace GPUParticles;

FBClassImplementation( CollisionSphere );								                //!< Register class
FBStorableCustomModelImplementation( CollisionSphere, ORMARKERCUSTOM__DESCSTR );			//!< Register to the store/retrieve system
FBElementClassImplementation2( CollisionSphere, "Collision Sphere", FB_DEFAULT_SDK_ICON );	                //!< Register to the asset system

/************************************************
*	Constructor.
************************************************/
CollisionSphere::CollisionSphere( const char* pName, HIObject pObject ) 
    : FBModelMarker( pName, pObject )
{
    FBClassInit;
}

/************************************************
*	FiLMBOX Constructor.
************************************************/
bool CollisionSphere::FBCreate()
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
	FBPropertyPublish( this, Friction, "Friction", nullptr, nullptr );
	FBPropertyPublish( this, Softness, "Softness", nullptr, nullptr );

	Enabled = true;
	Friction = 50.0;
	Softness = 50.0;

    return true;
}

/************************************************
*	FiLMBOX Destructor.
************************************************/
void CollisionSphere::FBDestroy()
{
    ParentClass::FBDestroy();
}

/** Custom display
*/
void CollisionSphere::CustomModelDisplay( FBCamera* pCamera, FBModelShadingMode pShadingMode, FBModelRenderPass pRenderPass, float pPickingAreaWidth, float pPickingAreaHeight)
{
    FBViewingOptions* lViewingOptions = FBSystem::TheOne().Renderer->GetViewingOptions();
    bool lIsSelectBufferPicking = (pRenderPass == kFBModelRenderPassPick) || lViewingOptions->IsInSelectionBufferPicking();
    bool lIsColorBufferPicking  = lViewingOptions->IsInColorBufferPicking();

	if (lViewingOptions->PickingMode() >=  kFBPickingModeModelsOnly)
		return;

	CHECK_GL_ERROR();

    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_POLYGON_BIT | GL_LINE_BIT); //Push Current GL states.
    {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        {
			FBMatrix tm;
			GetMatrix(tm);
			glMultMatrixd(tm);

			/*
            FBVector3d T = Translation;
			FBVector3d S = Scaling;
			glTranslated(T[0], T[1], T[2]);
			glScaled(S[0], S[1], S[2]);
			*/


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
				if (lIsColorBufferPicking)
				{
					FBColor lColor = UniqueColorId;
					glColor3dv(lColor);
				}
				else if (lIsSelectBufferPicking)
				{
					glLoadName(1);
				}
			}

            glLineWidth(3.0f);    //!< Draw line wider to easy picking.

            // Draw with Model's Unique ColorID  (or selection name (-1) set by Mobu Internally). 
            float lSize = (float) Size;
            
            DrawCircle( lSize, 16 );

			glRotatef( 90.0f, 1.0f, 0.0f, 0.0f );
			DrawCircle( lSize, 16 );

			glRotatef( 90.0f, 0.0f, 1.0f, 0.0f );
			DrawCircle( lSize, 16 );
        }
        glPopMatrix();
    }
    glPopAttrib();

	CHECK_GL_ERROR();
}

bool CollisionSphere::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
    return true;
}

bool CollisionSphere::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
    return true;
}

void CollisionSphere::FillCollisionData( TCollision	&data )
{
	FBVector3d T, vel, Scl;
	GetVector(T);
	GetVector(Scl, kModelScaling, false);
	vel = VectorSubtract( T, LastTranslation );
	LastTranslation = T;

	int iEnabled = (Enabled) ? PARTICLE_COLLISION_SPHERE_TYPE : PARTICLE_COLLISION_DISABLED;
	double maxscale = Scl[0];
	if (Scl[1] > maxscale) maxscale = Scl[1];
	if (Scl[2] > maxscale) maxscale = Scl[2];
	double friction = 0.01 * Friction;

	CollisionExchange::SetPosition( data, iEnabled, vec3( (float)T[0], (float)T[1], (float)T[2] ) );
	CollisionExchange::SetVelocity(data, vec4( (float)vel[0], (float)vel[1], (float)vel[2], maxscale ) );
	CollisionExchange::SetRadius(data, Size );
	CollisionExchange::SetFriction(data, friction);

	double dsoftness;
	Softness.GetData( &dsoftness, sizeof(double) );
	data.terrainScale.w = 0.01f * (float) dsoftness;

	//
	FBMatrix tm, invTM;
	GetMatrix(tm);
	GetMatrix(invTM, kModelInverse_Transformation);

	CollisionExchange::SetMatrix(data, tm, invTM);
}