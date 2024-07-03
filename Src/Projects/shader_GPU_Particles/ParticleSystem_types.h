
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ParticleSystem_types.h
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "GL\glew.h"
#include "math3d.h"
#include "nv_math.h"

namespace GPUParticles
{

#ifndef PARTICLE_EMIT_FROM_VERTICES
	#define PARTICLE_EMIT_FROM_VERTICES			0.0f
	#define PARTICLE_EMIT_FROM_VOLUME			1.0f
	#define PARTICLE_EMIT_FROM_SURFACE			2.0f
#endif

#ifndef PARTICLE_TYPE_LAUNCHER

	#define PARTICLE_TYPE_LAUNCHER			0.0f
	#define PARTICLE_TYPE_SHELL				1.0f
	#define PARTICLE_TYPE_SECONDARY_SHELL	2.0f
	#define PARTICLE_TYPE_DEAD	-1.0f

#endif

#ifndef PARTICLE_FORCE_WIND_TYPE
	#define		PARTICLE_FORCE_DISABLED			0	
	#define		PARTICLE_FORCE_WIND_TYPE		1
	#define		PARTICLE_FORCE_GRAVITY_TYPE		2
	#define		PARTICLE_FORCE_MOTOR_TYPE		3
	#define		PARTICLE_FORCE_VORTEX_TYPE		4
#endif

#ifndef	PARTICLE_COLLISION_SPHERE_TYPE
	#define PARTICLE_COLLISION_DISABLED			0
	#define PARTICLE_COLLISION_SPHERE_TYPE		1
	#define PARTICLE_COLLISION_TERRAIN_TYPE		4
#endif

// particle attributes
struct Particle
{
//    vec4				OldPos;				// in w store Particle Type  
	nv::vec4				Pos;				// in w - particle size (radius)
	nv::vec4				Vel;				// in w hold total lifetime
	// pack color into one float, and we have 3 free floats !

	// r - packed color, g - total lifetime, b - age, a - index 

	nv::vec4				Color;				// inherit color from the emitter surface, custom color simulation
    
	// we store rotations in quaternions
	nv::vec4				Rot;				// in w - float				AgeMillis;			// current Age
	nv::vec4				RotVel;				// in w - float				Index;				// individual assigned index from 0.0 to 1.0 (normalized)
};

struct evaluateBlock
{
	nv::mat4					gTM;			// emitter transform
	nv::mat4					gRotationTM;	// only rotation
	nv::mat4					gNormalTM;
	nv::mat4					gTextureTM;

	nv::vec4					gDirection;		// vec3 - direction, 4th - use normals as dir or not

	float					gDirSpreadHor;	// spread direction vector in horizontal plane
	float					gDirSpreadVer;	// spread in vertical plane
	float					gEmitSpeed;
	float					gSpeedSpread;

	//vec4					gDirRandom;
	//vec4					gVelocity;		// start particle velocity
	//vec4					gVelRandom;		// randomize start velocity
	nv::vec4					gEmitterVelocity; // use this velocity to inherit emitter speed
	//vec4					gEmitterPivot;
	//vec4					gAngularVelocity;
	//mat4					gEmitterDeltaTM;	// computed diff between two frames
	
	nv::vec4					gRotation;
	nv::vec4					gRotationSpread;
	nv::vec4					gAngularVelocity;
	nv::vec4					gAngularVelocitySpread;

	nv::vec4					gDynamic;		// 1st - mass, 2nd - damping
	nv::vec4					gGravity;		// vec3 - gravity direction XYZ, 4-th component - use or not to use gravity
	nv::vec4					gFlags;			// 1st - useForces, 2nd - useCollisions, 3rd - emitter type(0.0-vertices, 1.0-volume)
	nv::vec4					gTurbulence;
	nv::vec4					gFloor;			// 1st - use/not use, 2nd - friction, 3rd - Y level
	nv::vec4					gMin;
	nv::vec4					gMax;

	int						gPositionCount;	// number of position and normal vertices (emitter positions)
	int						gNumCollisions;
	int						gNumForces;
	int						gUseEmitterTexture;
	
	float 					gLauncherLifetime;                                                    
	float 					gShellLifetime;  
	float					gShellLifetimeVariation;	// a percent of randomization                                                     
	float 					gSecondaryShellLifetime;   

	int						gUseSizeAttenuation;
	int						gUseColorAttenuation;
	float					gSizeVariation;
	float					gColorVariation;	
	
	float					gSize;
	float					gInheritEmitterColor;
	float					gUseEmitterMask;
	float					gSkipAlphaLimit;	// don't emit particles from a transparency pixel
	
	nv::vec4					gEmitColor;
	nv::vec4					gEmitColor2;
	nv::vec4					gEmitColor3;

	float					gGenerateOnMotionLimit;
	float					gUseEmitColor2;
	float					gUseEmitColor3;
	float					gEvalTemp3;
};

struct	renderBlock
{

	nv::mat4		gMV;
	nv::mat4		gVP;
	nv::mat4		gInvTransposeMV;
	nv::mat4		gTexMatrix;

	nv::vec4		gCameraPos;
	nv::vec4		gScreenSize;
	
	nv::vec4		gColor;

	float		gPointFalloff;
	float		gUseSizeCurve;
	float		gUseColorCurve;
	float		gTransparencyFactor;

	float		gMinPointScale;
	float		gMaxPointScale;
	float		gPointScaleDistance;
	float		gUseColorMap;
};

//
struct terrainBlock
{
	nv::mat4 			gTerrainVP;

	nv::vec4 			gTerrainOffset;
	nv::vec4 			gTerrainScale;
	
	//uniform mat4 gVP;       
	//GLuint64		gTerrainColorAddress;  
	float 			gTerrainFarPlane{ 1.0f };
};

// emitter surface
struct TTriangle
{
	nv::vec4	p0;
	nv::vec4	p1;
	nv::vec4	p2;

	nv::vec4	n;

	nv::vec2	uv0;
	nv::vec2	uv1;
	nv::vec2	uv2;

	nv::vec2	temp;	// to align type
};

// instance model
struct TInstanceVertexStream
{
	unsigned int	vertexCount;

	GLuint			positionId;
	GLuint			normalId;
	GLuint			uvId;

	GLuint			indexId;

	void			*positionOffset;
	void			*normalOffset;
	void			*uvOffset;
	void			*indexOffset;
};

struct TMeshPatch
{
	unsigned int	textureId;
	unsigned int	offset;
	unsigned int	size;
};

// TODO: !! use rotation and rotation velocity to move particles !!
// TODO: !! terrainAddress could be also a 3d texture of mesh voxels !!
struct TCollision
{
	nv::vec4			position;			// use .W as collision TYPE
	nv::vec4			velocity;
	
	//vec4			terrainOffset;	// stores in position
	nv::vec4			terrainScale;
	nv::vec4			terrainSize;		// texture dimentions

	float 			radius;
	float			friction;

	// terrain texture pointer (bindless texture)
	GLuint64		terrainAddress;

	nv::mat4			tm;
	nv::mat4			invtm;
};

struct	TForce
{
	nv::vec4			position;
	nv::vec4			direction;		// use w as a force type
	float			magnitude;
	float			radius;
	float			noiseFreq;
	float 			noiseSpeed;
	nv::vec4			turbulence;		// w - use turbulence or not, x-amplitude, y-frequency
	nv::vec4			wind1;			// special wind pre-calculated force
	nv::vec4			wind2;

};


////////////////////////////////////////////////////////////////////////////////////////////////////
// exchange data types

//vec4 Color_UnPack (float depth);
//float Color_Pack (const vec4 &colour);


struct EvaluationExchange
{
	//
	static void SetOrientation(evaluateBlock &data, const nv::vec3 &rot, const nv::vec3 &rotSpread, const nv::vec3 &vel, const nv::vec3 &velSpread);
	static void SetDirection(evaluateBlock &data, const nv::vec3 &dir, const float spreadH, const float spreadV, bool useNormals);
	static void SetSpeed(evaluateBlock &data, const float speed, const float spread, 
		const nv::vec4 &emittervel, nv::vec4 pivot, nv::vec4 angular, const double *TMdelta);
	//static void SetDirection(evaluateBlock &data, const vec3 &dir, const vec3 &random, bool useNormals);
	//static void SetVelocity(evaluateBlock &data, const vec3 &vel, const vec3 &random, const vec4 &emittervel);
	static void SetDynamicParameters(evaluateBlock &data, float mass, float damping);
	static void SetGravity(evaluateBlock &data, const nv::vec3 &gravityDir, bool useGravity);
	static void SetFlags(evaluateBlock &data, bool useForces, bool useCollisions, int emitterType);
	static void SetTurbulence(evaluateBlock &data, const bool useTurbulence, const float freq, const float speed, const float amp);
	static void SetFloorParamaters(evaluateBlock &data, bool useFloor, float friction, float level);
	static void SetBoundingBox(evaluateBlock &data, const nv::vec3 &bmin, const nv::vec3 &bmax);
};

//
struct CollisionExchange
{
	static void SetPosition(TCollision &data, const int type, const nv::vec3 &pos);
	static void SetVelocity(TCollision &data, const nv::vec4 &value);
	static void SetRadius(TCollision &data, const double value);
	static void SetFriction(TCollision &data, const double value);
	static void SetMatrix(TCollision &data, const double *value, const double *invvalue);

	static void SetTerrainScale(TCollision &data, const nv::vec4 &value);
	static void SetTerrainSize(TCollision &data, const nv::vec4 &value);
	static void SetTerrainTextureAddress(TCollision &data, const GLuint64 address);
};

//
struct ForceExchange
{
	static void SetPosition(TForce &data, int type, const nv::vec3 &pos);
	static void SetDirection(TForce &data, const nv::vec3 &dir, const double w);
	static void SetMagnitude(TForce &data, const float value);
	static void SetRadius(TForce &data, const double value);
	static void SetTurbulence(TForce &data, const bool usage, const double amplitude, const double frequency);
	static void SetTurbulence(TForce &data,  const nv::vec3 &value, const double w);
	static void SetWind1(TForce &data, const nv::vec3 &value, const double w);
	static void SetWind2(TForce &data, const nv::vec3 &value, const double w);
	static void SetNoise(TForce &data, const double _useNoise, const double _noiseFreq, const double _noiseSpeed);
};

};