
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: MAIN.cxx
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>

#ifdef KARCH_ENV_WIN
	#include <windows.h>
#endif
#include <stdio.h>
#include <stdarg.h>

#include "fb_shader_particles_shader.h"
#include "model_force_gravity.h"
#include "model_force_wind.h"
#include "model_collision_terrain.h"

#include "Shader_ParticleSystem.h"
#include "FileUtils.h"

#define PARTICLES_EFFECT				"Particles.glslfx"
#define PREP_SURFACE_COMPUTE_SHADER		"\\GLSL_CS\\prepSurfaceData.glsl"

//--- Library declaration
FBLibraryDeclare( gpushader_particles )
{
	FBLibraryRegister( GPUshader_Particles );
	FBLibraryRegister( GPUshader_ParticlesLayout );

	FBLibraryRegisterStorable(ForceWind);
	FBLibraryRegisterElement(ForceWind);

	FBLibraryRegisterStorable(ForceMotor);
	FBLibraryRegisterElement(ForceMotor);

	FBLibraryRegisterStorable(ForceGravity);
	FBLibraryRegisterElement(ForceGravity);

	FBLibraryRegisterStorable(CollisionSphere);
	FBLibraryRegisterElement(CollisionSphere);

	FBLibraryRegisterStorable(CollisionTerrain);
	FBLibraryRegisterElement(CollisionTerrain);

	FBLibraryRegister( KShaderParticlesAssociation );
}
FBLibraryDeclareEnd;

static void NVFXErrorCallback(const char* errMsg)
{
	FBTrace("[NVFX ERROR] %s\n", errMsg);
}

static void NVFXMessageCallback(const char* msg)
{
	FBTrace(msg);
}

/// <summary>
/// a method to transfer shared library logs into motionbuilder logs output
/// </summary>
void LOGE(const char* pFormatString, ...)
{
	char buffer[256];
	va_list args;
	va_start(args, pFormatString);
	vsnprintf(buffer, 255, pFormatString, args);

	FBTrace(buffer);

	va_end(args);
}

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()	{ return true; }
bool FBLibrary::LibOpen()	{ return true; }
bool FBLibrary::LibReady()	{ 
	
	glewInit();
	GPUshader_Particles::AddPropertiesToPropertyViewManager();
	ForceWind::AddPropertiesToPropertyViewManager();
	ForceGravity::AddPropertiesToPropertyViewManager();
	CollisionTerrain::AddPropertiesToPropertyViewManager();

	//
	nvFX::setErrorCallback(NVFXErrorCallback);
	nvFX::setMessageCallback(NVFXMessageCallback);

	//
	char buffer[256];

	FBString particleEffectPath("\\GLSL_FX\\", PARTICLES_EFFECT);
	if ( FindEffectLocation( particleEffectPath, buffer, 256 ) )
	{
		FBString effectFullName(buffer, particleEffectPath);
		GPUParticles::ParticleShaderFX::SetShaderEffectLocation( effectFullName, effectFullName.GetLen() );

		FBString computeLocation(buffer, "\\GLSL_CS\\Particles_simulation.glsl");
		GPUParticles::ParticleShaderFX::SetComputeShaderLocation( computeLocation, computeLocation.GetLen() );

		FBString computeSelfCollisionsLocation(buffer, "\\GLSL_CS\\Particles_selfcollisions.glsl");
		GPUParticles::ParticleShaderFX::SetComputeSelfCollisionsShaderLocation( computeSelfCollisionsLocation, computeSelfCollisionsLocation.GetLen() );

		FBString computeIntegrateLocation(buffer, "\\GLSL_CS\\Particles_integrate.glsl");
		GPUParticles::ParticleShaderFX::SetComputeIntegrateLocation( computeIntegrateLocation, computeIntegrateLocation.GetLen() );

		FBString strComputeSurfaceData(buffer, PREP_SURFACE_COMPUTE_SHADER);
		GPUParticles::ParticleShaderFX::SetComputeSurfaceDataPath(strComputeSurfaceData, strComputeSurfaceData.GetLen() );
	}

	return true; 
}
bool FBLibrary::LibClose()	{ return true; }
bool FBLibrary::LibRelease(){ return true; }
