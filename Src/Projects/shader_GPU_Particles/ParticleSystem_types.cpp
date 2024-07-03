
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ParticleSystem_types.cpp
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "ParticleSystem_types.h"

using namespace GPUParticles;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
void EvaluationExchange::SetOrientation(evaluateBlock &data, const nv::vec3 &rot, const nv::vec3 &rotSpread, const nv::vec3 &vel, const nv::vec3 &velSpread)
{
	using namespace nv;
	data.gRotation = vec4( rot.x * 3.1415 / 180.0, 
		rot.y * 3.1415 / 180.0, 
		rot.z * 3.1415 / 180.0, 0.0 );
	data.gRotationSpread = vec4( rotSpread.x, rotSpread.y, rotSpread.z, 0.0 );
	data.gAngularVelocity = vec4( vel.x * 3.1415 / 180.0, 
		vel.y * 3.1415 / 180.0, 
		vel.z * 3.1415 / 180.0, 0.0 );
	data.gAngularVelocitySpread = vec4( velSpread.x, velSpread.y, velSpread.z, 0.0 );
}

void EvaluationExchange::SetDirection(evaluateBlock &data, const nv::vec3 &dir,
	const float spreadH, const float spreadV, bool useNormals)
{
	using namespace nv;
	vec3 ldir(dir);
	float mag = ldir.normalize();

	if ( nv_zero == mag )
	{
		data.gDirection = vec4( 1.0f, 0.0f, 0.0f, (useNormals) ? 1.0f : 0.0f );
	}
	else
	{
		data.gDirection = vec4( ldir, (useNormals) ? 1.0f : 0.0f );
	}
	
	data.gDirSpreadHor = spreadH;
	data.gDirSpreadVer = spreadV;
}
void EvaluationExchange::SetSpeed(evaluateBlock &data, const float speed, const float spread, 
	const nv::vec4 &emittervel, nv::vec4 emitterPivot, nv::vec4 angularVelocity, const double *TMdelta)
{
	data.gEmitSpeed = speed;
	data.gSpeedSpread = spread;
	data.gEmitterVelocity = emittervel;
	//data.gEmitterPivot = emitterPivot;
	//data.gAngularVelocity = angularVelocity;
	
	//for (int i=0; i<16; ++i)
	//	data.gEmitterDeltaTM.mat_array[i] = (float) TMdelta[i];
}

void EvaluationExchange::SetDynamicParameters(evaluateBlock &data, float mass, float damping)
{
	data.gDynamic[0] = mass;
	data.gDynamic[1] = damping;
}
void EvaluationExchange::SetGravity(evaluateBlock &data, const nv::vec3 &gravityDir, bool useGravity)
{
	data.gGravity[0] = gravityDir[0];
	data.gGravity[1] = gravityDir[1];
	data.gGravity[2] = gravityDir[2];
	data.gGravity[3] = (useGravity) ? 1.0f : 0.0f;
}
void EvaluationExchange::SetFlags(evaluateBlock &data, bool useForces, bool useCollisions, int emitterType)
{
	data.gFlags[0] = (useForces) ? 1.0f : 0.0f;
	data.gFlags[1] = (useCollisions) ? 1.0f : 0.0f;
	data.gFlags[2] = (float) emitterType;
}
void EvaluationExchange::SetTurbulence(evaluateBlock &data, const bool useTurbulence, const float freq, const float speed, const float amp)
{
	data.gTurbulence[3] = (useTurbulence) ? 1.0f : 0.0f;
	data.gTurbulence[0] = freq;
	data.gTurbulence[1] = speed;
	data.gTurbulence[2] = amp;
}
void EvaluationExchange::SetFloorParamaters(evaluateBlock &data, bool useFloor, float friction, float level)
{
	data.gFloor[3] = (useFloor) ? 1.0f : 0.0f;
	data.gFloor[1] = friction;
	data.gFloor[2] = level;
}
void EvaluationExchange::SetBoundingBox(evaluateBlock &data, const nv::vec3 &bmin, const nv::vec3 &bmax)
{
	data.gMin = bmin;
	data.gMax = bmax;
}


////////////////////////////////////////////////////////////////////////////
//

void CollisionExchange::SetPosition(TCollision &data, const int type, const nv::vec3 &pos)
{
	data.position[0]=pos[0];
	data.position[1]=pos[1];
	data.position[2]=pos[2];
	data.position[3]= (double) type;
}

void CollisionExchange::SetVelocity(TCollision &data, const nv::vec4 &value)
{
	data.velocity = value;
}

void CollisionExchange::SetRadius(TCollision &data, const double value)
{
	data.radius = (float) value;
}

void CollisionExchange::SetFriction(TCollision &data, const double value)
{
	data.friction = (float) value;
}

void CollisionExchange::SetMatrix(TCollision &data, const double *value, const double *invvalue)
{
	for (int i=0; i<16; ++i)
	{
		data.tm.mat_array[i] = (float) value[i];
		data.invtm.mat_array[i] = (float) invvalue[i];
	}
}

void CollisionExchange::SetTerrainScale(TCollision &data, const nv::vec4 &value)
{
	data.terrainScale = value;
}
void CollisionExchange::SetTerrainSize(TCollision &data, const nv::vec4 &value)
{
	data.terrainSize = value;
}
void CollisionExchange::SetTerrainTextureAddress(TCollision &data, const GLuint64 address)
{
	data.terrainAddress = address;
}

////////////////////////////////////////////////////////////////////////////
//

void ForceExchange::SetPosition(TForce &data, int type, const nv::vec3 &pos)
{
	data.position[0]=pos[0];
	data.position[1]=pos[1];
	data.position[2]=pos[2];
	data.position[3]= (float) type;
}

void ForceExchange::SetDirection(TForce &data, const nv::vec3 &dir, const double w)
{
	data.direction[0]=dir[0];
	data.direction[1]=dir[1];
	data.direction[2]=dir[2];
	data.direction[3]= w;
}

void ForceExchange::SetMagnitude(TForce &data, const float value)
{
	data.magnitude = value;
}

void ForceExchange::SetRadius(TForce &data, const double value)
{
	data.radius = (float) value;
}

void ForceExchange::SetTurbulence(TForce &data, const bool usage, const double amplitude, const double frequency)
{
	data.turbulence[0] = amplitude;
	data.turbulence[1] = frequency;
	data.turbulence[2] = 0.0f;
	data.turbulence[3] = (usage) ? 1.0f : 0.0f;
}
void ForceExchange::SetTurbulence(TForce &data,  const nv::vec3 &value, const double w)
{
	data.turbulence[0]=value[0];
	data.turbulence[1]=value[1];
	data.turbulence[2]=value[2];
	data.turbulence[3]= w;
}
void ForceExchange::SetWind1(TForce &data, const nv::vec3 &value, const double w)
{
	data.wind1[0]=value[0];
	data.wind1[1]=value[1];
	data.wind1[2]=value[2];
	data.wind1[3]= w;
}
void ForceExchange::SetWind2(TForce &data, const nv::vec3 &value, const double w)
{
	data.wind2[0]=value[0];
	data.wind2[1]=value[1];
	data.wind2[2]=value[2];
	data.wind2[3]= w;
}
void ForceExchange::SetNoise(TForce &data, const double _useNoise, const double _noiseFreq, const double _noiseSpeed)
{
	data.wind2[3]=_useNoise;
	data.noiseFreq = _noiseFreq;
	data.noiseSpeed = _noiseSpeed;
}