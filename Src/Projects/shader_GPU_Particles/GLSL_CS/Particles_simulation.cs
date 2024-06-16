//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Particles_simulation.cs
//
//	Author Sergey Solokhin (Neill3d)
//
// Main GPU Particles compute shader
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#version 430

layout (local_size_x = 1024, local_size_y = 1) in;

uniform	int		gNumParticles;

uniform float	DeltaTimeSecs;
uniform float	gTime;

uniform mat4	gTM;	// emitter transform

uniform vec4	gDynamic;
uniform vec4	gTurbulence;
uniform vec4	gGravity;			// in w - use gravity force
uniform vec4	gFloor;				// in x - floor friction, in y - level, in w - use floor level

uniform	int		gNumForces;
uniform	int		gNumCollisions;

uniform int		gUseSizeAttenuation;
uniform int		gUseColorAttenuation;

uniform int		gUpdatePosition;
uniform int		gEmitterPointCount;


	#define PARTICLE_TYPE_LAUNCHER 0.0f                                                 
	#define PARTICLE_TYPE_SHELL 1.0f                                                    
	#define PARTICLE_TYPE_SECONDARY_SHELL 2.0f  

#define		USE_FLOOR			gFloor.w
#define		FLOOR_FRICTION		gFloor.y
#define		FLOOR_LEVEL			gFloor.z

#define		MASS				gDynamic.x
#define		DAMPING				gDynamic.y
#define 	CONSTRAINT_MAGN		gDynamic.z

#define		USE_GRAVITY			gGravity.w
#define		GRAVITY				gGravity.xyz

#define 	USE_TURBULENCE		gTurbulence.w
#define		NOISE_FREQ			gTurbulence.x
#define		NOISE_SPEED			gTurbulence.y
#define		NOISE_AMP			gTurbulence.z

#define		USE_FORCES			gNumForces > 0
#define		USE_COLLISIONS		gNumCollisions > 0
#define		EMITTER_TYPE		gFlags.z

#define		EMITTER_TYPE_VERTICES	0.0
#define		EMITTER_TYPE_VOLUME		1.0

#define		FORCE_WIND				1.0
#define		FORCE_GRAVITY			2.0
#define		FORCE_MOTOR				3.0
#define		FORCE_VORTEX			4.0

#define		COLLISION_SPHERE		1.0
#define		COLLISION_TERRIAN		4.0

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TYPES AND DATA BUFFERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct TParticle
{ 
	vec4				Pos;				// in w hold lifetime from 0.0 to 1.0 (normalized)
	vec4				Vel;				// in w - individual birth randomF
	// color packed in x. inherit color from the emitter surface, custom color simulation
	vec4				Color;				// in y - total lifetime, z - AgeMillis, w - Index
	vec4				Rot;				// 
	vec4 				RotVel;				// 
};

struct TCollision
{
	vec4			position;			// use w as collision type
	vec4			velocity;			// .w - maxscale dimention
	
	vec4			terrainScale;		// .w - softness
	vec4			terrainSize;		// texture dimentions
	
	float 			radius;
	float			friction;
	
	uvec2			terrainAddress;
	
	mat4			tm;
	mat4			invtm;
};

struct TForce
{
	vec4			position;
	vec4			direction;		// use w as a force type
	float			magnitude;
	float			radius;
	float			noiseFreq;
	float 			noiseSpeed;
	vec4			turbulence;		// w - use turbulence or not, x-amplitude, y-frequency
	vec4			wind1;			// special wind pre-calculated force
	vec4			wind2;	
};

// emitter surface
struct TTriangle
{
	vec4	p[3];
	vec4	n;
	vec2	uv[3];
		
	vec2	temp;	// to align type
};

layout (std430, binding = 0) buffer ParticleBuffer
{
	TParticle particles[];
} particleBuffer;

layout (std430, binding = 1) readonly buffer ForcesBuffer
{
	TForce forces[];
} forceBuffer;

layout (std430, binding = 2) readonly buffer CollisionBuffer
{
	TCollision collisions[];
} collisionBuffer;

layout (std430, binding = 3) readonly buffer MeshBuffer
{
	TTriangle	mesh[];
} meshBuffer;

// terrain depth
layout(binding=0) uniform sampler2D 	TerrainSampler;
// particle size attenuation
layout(binding=5) uniform sampler1D 	SizeSampler;

const vec2 randN1 = vec2(0.14, -0.07);
const vec2 randN2 = vec2(0.77, 1.01);
const vec2 randN3 = vec2(-0.38, 0.15);

const float PiPi = 6.2831853;
const float PI = 3.14159265;
const float PI_2 = 1.57079632;
const float PI_4 = 0.785398163;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HELPER FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

highp float rand(vec2 co)
{
	highp float a = 12.9898;
	highp float b = 78.233;
	highp float c = 43758.5453;
	highp float dt= dot(co.xy ,vec2(a,b));
	highp float sn= mod(dt,3.14);
	return fract(sin(sn) * c);
}

mat3 rotationMatrix(vec3 axisIn, float angle)
{
	vec3 axis = normalize(axisIn);
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1.0 - c;
	
	return mat3(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
				oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
				oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c);
}

vec4 quat_mul(vec4 q0, vec4 q1) {
		  vec4 d;
		  d.x = q0.w * q1.x + q0.x * q1.w + q0.y * q1.z - q0.z * q1.y;
		  d.y = q0.w * q1.y - q0.x * q1.z + q0.y * q1.w + q0.z * q1.x;
		  d.z = q0.w * q1.z + q0.x * q1.y - q0.y * q1.x + q0.z * q1.w;
		  d.w = q0.w * q1.w - q0.x * q1.x - q0.y * q1.y - q0.z * q1.z;
		  return d;
		}

uint get_invocation()
{
   //uint work_group = gl_WorkGroupID.x * gl_NumWorkGroups.y * gl_NumWorkGroups.z + gl_WorkGroupID.y * gl_NumWorkGroups.z + gl_WorkGroupID.z;
   //return work_group * gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z + gl_LocalInvocationIndex;

   // uint work_group = gl_WorkGroupID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x + gl_WorkGroupID.x * gl_WorkGroupSize.x + gl_LocalInvocationIndex;
   uint work_group = gl_GlobalInvocationID.y * (gl_NumWorkGroups.x * gl_WorkGroupSize.x) + gl_GlobalInvocationID.x;
   return work_group;
}

//
// Description : Array and textureless GLSL 2D/3D/4D simplex 
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
// 

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
	 return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
  { 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i); 
  vec4 p = permute( permute( permute( 
			 i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
		   + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
		   + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
								dot(p2,x2), dot(p3,x3) ) );
}

//////////////////////////////////////////////////////////////////////
//
// out float r, out float theta, out float phi

void ConvertUnitVectorToSpherical(const vec4 v, out vec3 sv)
{
	sv.x = sqrt(v.x*v.x + v.y*v.y + v.z*v.z); // r
	sv.y = atan( v.y,  v.x ); // theta
	sv.z = atan( sqrt(v.x*v.x+v.y*v.y), v.z ); // phi
}

// const float r, const float theta, const float phi
void ConvertSphericalToUnitVector(vec3 sv, out vec4 v)
{
	v.x = sv.x * cos(sv.y) * sin(sv.z);
	v.y = sv.x * sin(sv.y) * sin(sv.z);
	v.z = sv.x * cos(sv.z);
	v.w = 1.0;
}

void GetRandomDir(in vec4 inDir, in vec2 dirRnd, out vec4 dir)                                   
{
	//float r, theta, phi;
	vec3 sv;

	ConvertUnitVectorToSpherical(inDir, sv);

	sv.y += dirRnd.x * PI;
	sv.z += dirRnd.y * PiPi;
			
	ConvertSphericalToUnitVector(sv, dir);
} 

void GetRandomDir(in vec3 inDir, in vec2 dirRnd, out vec3 dir)                                        {
	//float r, theta, phi;
	vec3 sv;

	ConvertUnitVectorToSpherical(vec4(inDir, 1.0), sv);

	sv.y += dirRnd.x * PI;
	sv.z += dirRnd.y * PiPi;
	
	vec4 result;
	ConvertSphericalToUnitVector(sv, result);
	dir = result.xyz;
}  

	
vec4 Color_UnPack (float x)
{
	float a,b,c,d;
	a = floor(x*255.0/64.0)*64.0/255.0;
	x -= a;
	b = floor(x*255.0/16.0)*16.0/255.0;
	x -= b;
	b *= 4.0;
	c = floor(x*255.0/4.0)*4.0/255.0;
	x -= c;
	c *= 16.0;
	d = x*255.0 * 64.0 / 255.0; // scan be simplified to just x*64.0
			
	return vec4(a,b,c,d);
}


float Color_Pack (vec4 colour)
{
	float x = 1.0/255.0 * (floor(colour.x*255.0/64.0)*64.0 + floor(colour.y*255.0/64.0)*16.0 + floor(colour.z*255.0/64.0)*4.0 + floor(colour.a*255.0/64.0));
	return x;
}

void GetEmitPos(in vec2 randN, out vec4 pos, out int vertIndex, out vec3 bary)
{
			
	float rnd = rand(randN) * gEmitterPointCount;
	int triIndex = int(rnd);

	// barycentric coords
	float rnd1 = rand(randN+randN1);
	float rnd2 = rand(randN+randN2);

	bary.x = 1.0 - sqrt(rnd1);
	bary.y = sqrt(rnd1) * (1.0 - rnd2);
	bary.z = sqrt(rnd1) * rnd2; 

	TTriangle tri = meshBuffer.mesh[triIndex];
	vec4 p0 = tri.p[0];
	vec4 p1 = tri.p[1];
	vec4 p2 = tri.p[2];

	p0 *= bary.x;
	p1 *= bary.y;
	p2 *= bary.z;

	vec4 P = p0 + p1 + p2;
	vertIndex = triIndex * 3;

	pos = gTM * vec4(P.xyz, 1.0);
	//pos = vec4(P.xyz, 1.0);
	// TODO: extrusion dist is not used !
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRAINT AND COLLIDE
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SphereCollide(TCollision data, in float size, in vec3 x, inout vec3 vel)
{
	vec3 delta = x - data.position.xyz;
	float dist = length(delta);
	float radius = data.radius * data.velocity.w; // + size;

	if (dist < radius) {
		vec4 untransformed = data.invtm * vec4(x, 1.0);
		dist = length(untransformed.xyz);
		radius = data.radius; // + size / data.velocity.w;

		if (dist < radius)
		{
			vec3 newvel = vel * clamp(dist / radius, 0.0, 1.0) * data.friction;
			newvel = reflect(newvel, normalize(untransformed.xyz));
			vel = mix(newvel, vel, data.terrainScale.w);
			vel += (1.0 - data.terrainScale.w) * data.velocity.xyz;
		}
	}
}

// constrain particle to be outside volume of a sphere
void SphereConstraint(TCollision data, in float size, inout vec3 x)
{
	vec3 delta = x - data.position.xyz;
	float dist = length(delta);
	float radius = data.radius * data.velocity.w; // + size;

	if (dist < radius) {
		vec4 untransformed = data.invtm * vec4(x, 1.0);
		dist = length(untransformed.xyz);
		radius = data.radius; // + size / data.velocity.w;

		if (dist < radius)
		{
			vec3 newx = radius * normalize(untransformed.xyz);
			untransformed = data.tm * vec4(newx, 1.0);

			x = mix(untransformed.xyz, x, data.terrainScale.w);
		}
	}
}

// constrain particle to heightfield stored in texture
void TerrainConstraint(TCollision data, inout vec3 pos)
{
	vec3 scale = data.terrainScale.xyz;
	vec3 offset = data.position.xyz;
	
	
	if (pos.x < offset.x || pos.x > offset.x+scale.x || pos.z < offset.z || pos.z > offset.z+scale.z
		|| pos.y < offset.y || pos.y > data.terrainSize.z )
	{
		return;
	}
	
	vec2 uv = (pos.xz - offset.xz) / scale.xz;

	float h = texture(TerrainSampler, uv).x;
	
	if (pos.y < h) {
		pos.y = h;
	}
}

// constrain particle to be above floor
void FloorConstraint(inout vec3 x, float level)
{
	if (x.y < level) {
		x.y = level;
	}
}

void FloorCollide(inout vec3 x, inout vec3 vel, const float rndF, float level, float friction, const float dt)
{
	if (x.y < level) {
//        x.y = level;
//        force.y += -vel.y*friction;
		
		vel.xyz = friction * vel.xyz;
		vel.xyz = reflect( vel.xyz, vec3(0.0, 1.0, 0.0) );
		//vel.y += -vel.y*friction * rand(vec2(rndF, 0.123)) * dt;
		//vel.x += rand(vec2(rndF, -0.123)) * friction * dt;
		//vel.z += rand(vec2(rndF, -0.543)) * friction * dt;
	}
}

void TerrainCollide(TCollision data, vec3 pos, const float rndF, inout vec3 vel)
{
	vec3 offset = data.position.xyz;
	vec3 scale = data.terrainScale.xyz;
	
	// should be predefined in shader
	if (pos.x < offset.x || pos.x > offset.x+scale.x || pos.z < offset.z || pos.z > offset.z+scale.z
		|| pos.y < offset.y || pos.y > data.terrainSize.z )
	{
		return;
	}
	 
	vec2 texelSize = vec2( 1.0 / data.terrainSize.x, 1.0 / data.terrainSize.y );
	vec2 uv = (pos.xz - offset.xz) / scale.xz;
	
	float h0 = texture(TerrainSampler, uv).x;
	
	if (pos.y < h0) {
		// calculate normal (could precalc this)
		float h1 = texture(TerrainSampler, uv + texelSize*vec2(1.0, 0.0) ).r;
		float h2 = texture(TerrainSampler, uv + texelSize*vec2(0.0, 1.0) ).r;
		
		vec3 N = cross( vec3(scale.x*texelSize.x, h1-h0, 0.0), vec3(0.0, h2-h0, scale.z*texelSize.y) );
		//vec3 N = cross( vec3(scale.x*texelSize.x, (h1-h0)*scale.y, 0), vec3(0, (h2-h0)*scale.y, scale.z*texelSize.y) );
		N = normalize(N);
		
		//GetRandomDir(N, vec2(0.1*rand(vec2(rndF, 0.11)), 0.2*rand(vec2(rndF, -0.05))), N);
		
		vel = reflect(vel, N);
		vel *= data.friction;

		vec3 newvel = vel * data.friction;
		newvel = reflect(newvel, N);
		vel = mix(newvel, vel, data.terrainScale.w);
	}
}

////////////////////////////////////////////////////////////////////////
// FORCES
////////////////////////////////////////////////////////////////////////


void ApplyWindForce2(TForce data, in float time, in vec3 pos_next, inout vec3 force, inout vec3 vel, float dt)
{
	// combining four winds.
	
	float a = snoise( vec3(data.noiseSpeed * time) + data.noiseFreq * pos_next );
	vec3 w = a*data.direction.xyz + (1.0f-a)*data.turbulence.xyz + a*data.wind1.xyz + (1.0f-a)*data.wind2.xyz;
	vec3 lforce = data.magnitude * normalize(w);
	
	float r = data.radius;
	if (r > 0.0f)
	{
		vec3 lpos = data.position.xyz;
		float len = length(lpos - pos_next);
		len = clamp(len, 0.0, r);
		len = 1.0f - len / r;
		lforce *= len;
	}
	
	vel += lforce*dt*dt;
}

void ApplyDragForce(TForce data, in float time, in vec3 pos_next, inout vec3 force, inout vec3 vel)
{
	// here direction is a drag velocity
	vec3 lforce = data.magnitude * data.direction.xyz;
	float r = data.radius;
	
	if (r > 0.0)
	{
		vec3 lpos = data.position.xyz;
		float len = length(lpos - pos_next);
		if (len <= r)
		{
			len = 1.0 - len / (r + 0.001);
			force += lforce * len;
			//force += lforce * (1.0 / (len*len + 0.001) );
		}
	}
	else
	{
		force +=  lforce;
	}
}

void ApplyDragForce2(TForce data, in float time, in vec3 pos_next, inout vec3 force, inout vec3 vel)
{
	// here direction is a drag velocity
	vec3 lforce = data.magnitude * data.direction.xyz;
	lforce = data.magnitude * normalize(pos_next - data.position.xyz);
	lforce.x = 0.0;
	lforce.z = 0.0;
	float r = data.radius;
	
	if (r > 0.0)
	{
		vec3 lpos = data.position.xyz;
		float len = length(lpos - pos_next);
		if (len <= r)
		{
			len = 1.0 - len / (r + 0.001);
			force += lforce * len;
			//force += lforce * (1.0 / (len*len + 0.001) );
		}
	}
	else
	{
		force +=  lforce;
	}
}

// Thanks for paper - Particle Systems Using 3D Vector Fields with OpenGL Compute Shaders. Johan Anderdahl Alice Darner

void ApplyGravityForce(TForce data, in float time, in vec3 pos_next, inout vec3 force, inout vec3 vel)
{
	vec3 center = data.position.xyz;
	float power = data.magnitude;
	vec3 dir = center - pos_next;

	float range = data.radius;
	
	if (range > 0.0)
	{
		
		float distance = length(dir);
		float percent = ((range-distance) / range);
		percent = clamp(percent, 0.0, 1.0);
		dir = normalize(dir);
		
		force += dir * percent * power;
	}
	else
	{
		force += dir * power;
	}
}

void ApplyMotorForce(TForce data, in float time, in vec3 pos_next, inout vec3 force, inout vec3 vel)
{
	vec3 lpos = data.position.xyz;
	
	mat3 mat = rotationMatrix( data.direction.xyz, data.direction.w * 3.14 / 180.0 );
	
	vec3 v = pos_next - lpos;
	
	// TODO: we can make a force to move to/from the center of motor
	
	vec3 tmpPos = pos_next - lpos;
	vec3 point = (dot(tmpPos, data.direction.xyz)/dot(data.direction.xyz, data.direction.xyz) * data.direction.xyz);
	vec3 pointVec = tmpPos - point;
	
	vec3 direction = lpos - pos_next;
	float distance = length(direction);
	direction /= distance;

	//vel += direction * max(0.01, (1.0 / (distance*distance))); // * data.magnitude;
	
	vec3 v2 = v * mat;
	//v += lpos;
	vec3 lforce = vec3(0.0); // 0.1 * data.magnitude * (v - pos_next);
	vec3 spinVec = cross(direction, pointVec);
	//lforce = spinVec * data.magnitude;
	lforce += (v2 - v + direction) * max(0.01, (1.0 / (distance*distance))) * data.magnitude * 10.0;
	//vec3 lforce = data.magnitude * data.direction.xyz;
	float r = data.radius;
	
	if (r > 0.0)
	{
		
		float len = length(lpos - pos_next);
		if (len <= r)
		{
			len = 1.0 - len / (r + 0.001);
			vel = lforce * len;
			//force += lforce * (1.0 / (len*len + 0.001) );
		}
	}
	else
	{
		vel = lforce;
	}
	
}

void ApplyVortex(TForce data, in float time, in vec3 particlePos, inout vec3 force, inout vec3 vel)
{
	float height = data.radius;
	float range = data.radius;
	float curve = 1.0;
	float downPower = 1.0;
	
	vec3 center = data.position.xyz;
	vec3 direction = data.direction.xyz;
	
	vec3 tmpPos = particlePos - center;
	vec3 point = (dot(tmpPos, data.direction.xyz)/dot(data.direction.xyz, data.direction.xyz) * data.direction.xyz);

	// if the particle we are testing against is above the vortex it shouldn't affect that particle
	bool cut = bool(clamp(dot(point, direction), 0.0, 1.0));
	
	// TODO: we can make a force to move to/from the center of motor

	vec3 pointVec = tmpPos - point;
	vec3 pullVec = pointVec;
	
	float vort = length(point);
	float percentVort = ((height - vort)/height);
	range *= clamp(pow(percentVort, curve), 0.0, 1.0);
	
	float dist = length(pointVec);
	float downDist = length(point);
	
	float downPercent = ((height - downDist)/height);
	float rangePercent = ((range - dist)/range);
	
	rangePercent = clamp(rangePercent, 0.0, 1.0);
	downPercent = clamp(downPercent, 0.0, 1.0);
	
	vec3 spinVec = cross(direction, pointVec);
	vec3 downVec = normalize(direction);
	
	normalize(spinVec);
	normalize(pullVec);
	
	force += (spinVec * data.direction.w - pullVec * data.magnitude
		+ downVec * downPower) * rangePercent * float(cut);
	/*
	float r = data.radius;
	
	if (r > 0.0)
	{
		
		float len = length(center - particlePos);
		if (len <= r)
		{
			len = 1.0 - len / (r + 0.001);
			force = lforce * len;
			//force += lforce * (1.0 / (len*len + 0.001) );
		}
	}
	else
	{
		force =  lforce;
	}
	*/
}



void ApplyConstraint(in float time, in float randomF, inout vec3 force, inout vec3 vel, inout vec3 pos)
{
	vec4 dst;
	int vertIndex = 0;
	vec3 baryCoords;
			
	GetEmitPos(vec2(randomF, 0.487), dst, vertIndex, baryCoords);

	//vec4 dst = gTM * vec4(particleHold, 1.0);
	vec3 direction = pos - dst.xyz;
	float distance = length(direction);
	
	if (distance != 0.0)
	{
		
		pos = mix(pos, dst.xyz, clamp(CONSTRAINT_MAGN * time, 0.0, 1.0)) ;
		
		direction /= distance;
		vel *= 1.0 - CONSTRAINT_MAGN; // * max(1.0, (1.0 / (distance*distance))); // * data.magnitude;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void main()
{
	// particle id in the array
	uint flattened_id = get_invocation();
	
	// ?! skip unused part of the array
	if (flattened_id >= gNumParticles)
		return;
	
	// Read position and velocity
	vec4 pos = particleBuffer.particles[flattened_id].Pos;
	vec4 packedColor = particleBuffer.particles[flattened_id].Color;
	vec4 vel = particleBuffer.particles[flattened_id].Vel;
	vec4 rot = particleBuffer.particles[flattened_id].Rot;
	vec4 rotVel = particleBuffer.particles[flattened_id].RotVel;

	float lifetime = packedColor.y;
	
	if (lifetime == 0.0)
		return;
	
	float Age = packedColor.z + DeltaTimeSecs; 
	packedColor.z = Age;

	// launcher has a negative size and negative lifetime value
	if (pos.w < 0.0 || lifetime < 0.0) // PARTICLE_TYPE_LAUNCHER
	{
		particleBuffer.particles[flattened_id].Color = packedColor;
		return;
	}
	else if (Age >= lifetime)
	{
		// dead particle, don't process it
		packedColor.y = 0.0;
		particleBuffer.particles[flattened_id].Color = packedColor;	
		return;
	}
	
	// animate size
	if (gUseSizeAttenuation > 0)
	{
		float normLife = Age / lifetime;
		pos.w = texture( SizeSampler, normLife ).r;
		//pos.w = normLife * 10.0;
	}

	float randomF = gTime + float(flattened_id);
	
	// predicted position next timestep
	vec3 pos_next = pos.xyz + vel.xyz * DeltaTimeSecs; 

	// accumulate rotation by angular velocity
	
	vec4 Qupdate = vec4(rotVel.xyz * 0.5 * DeltaTimeSecs, 0.0);
	rot += quat_mul(Qupdate, rot);

	// update velocity - gravity force
	vec3 force = GRAVITY * USE_GRAVITY;	// in w - use gravity flag
	
	if (USE_FORCES)
	{
		for (int i=0; i<gNumForces; ++i)
		{
			float type = forceBuffer.forces[i].position.w;
			
			if (type == FORCE_WIND) 
				ApplyWindForce2(forceBuffer.forces[i], gTime * 0.01, pos_next, force, vel.xyz, DeltaTimeSecs);
			else if (type == FORCE_GRAVITY) 
				ApplyGravityForce(forceBuffer.forces[i], gTime * 0.01, pos_next, force, vel.xyz);
			else if (type == FORCE_MOTOR)
				ApplyMotorForce(forceBuffer.forces[i], gTime * 0.01, pos_next, force, vel.xyz);
			else if (type == FORCE_VORTEX) 
				ApplyVortex(forceBuffer.forces[i], gTime * 0.01, pos_next, force, vel.xyz);
		}
	}
	
	if (USE_FLOOR > 0.0) 
		FloorCollide(pos_next, vel.xyz, randomF, FLOOR_LEVEL, FLOOR_FRICTION, DeltaTimeSecs);
	
	//
	// Process all collisions
	//
	
	if (USE_COLLISIONS)
	{
		for(int i=0; i<gNumCollisions; ++i)
		{
			float coltype = collisionBuffer.collisions[i].position.w;
			
			if ( COLLISION_SPHERE == coltype ) 
			{
				SphereCollide(collisionBuffer.collisions[i], pos.w, pos_next, vel.xyz); // collisions_inuse[i]);
			}
			else if ( COLLISION_TERRIAN == coltype )
			{
				TerrainCollide(collisionBuffer.collisions[i], pos_next, randomF, vel.xyz);
			}
		}
	}	
	
	
	
	float inv_mass = 1.0 / MASS;
	float damping = DAMPING;
	vel.xyz += force * inv_mass * DeltaTimeSecs; // F = ma
	damping = 1.0 - (1.0 - damping) * DeltaTimeSecs;
	vel.xyz *= damping;
	//vel.xyz += force;
	
	// turbulence behaviour

	if (USE_TURBULENCE > 0.0)
	{
		float f2 = cos(gTime) - 2.2;
		float f3 = sin(gTime) + 0.5;
	
		vec3 noiseVel = vec3( 	snoise(pos.xyz*NOISE_FREQ + gTime*NOISE_SPEED), 
							snoise(pos.xyz*NOISE_FREQ + gTime*NOISE_SPEED+f2), 
							snoise(pos.xyz*NOISE_FREQ + gTime*NOISE_SPEED+f3) );
							
		vel.xyz += noiseVel * NOISE_AMP;
	}
	
	// update position
	//
	// new position = old position + velocity * deltaTime
	if (gUpdatePosition > 0)
	{
		//pos.xyz += vel.xyz * DeltaTimeSecs;
		pos.xyz = pos_next;
		
		if (CONSTRAINT_MAGN > 0.0)
		{
			ApplyConstraint(gTime * 0.01, vel.w, force, vel.xyz, pos.xyz);
		}
		
		if (USE_COLLISIONS)
		{
			for(int i=0; i<gNumCollisions; ++i)
			{
				//if ( inuse < 1.0 ) // collisions_inuse[i] < 1.0 )
				//	continue;
				float coltype = collisionBuffer.collisions[i].position.w;
				
				if ( COLLISION_SPHERE == coltype) 
				{
					SphereConstraint(collisionBuffer.collisions[i], pos.w, pos.xyz);
				}
				else if ( COLLISION_TERRIAN == coltype )
				{
					TerrainConstraint(collisionBuffer.collisions[i], pos.xyz);
				}
			}
		}
		
		if (USE_FLOOR > 0.0) 
			FloorConstraint(pos.xyz, FLOOR_LEVEL);
	}	
	
	
	
	// write back
	particleBuffer.particles[flattened_id].Pos = pos; // pos.w - particle size
	particleBuffer.particles[flattened_id].Vel = vel;
	particleBuffer.particles[flattened_id].Color = packedColor;	// in y we store lifetime, in z - Age, in W - Index
	particleBuffer.particles[flattened_id].Rot = rot;

	/*
	// noise3 based color
	
	vec4 color = particleBuffer.particles[flattened_id].Color;
	float a = clamp(1.0 - snoise( 0.03 * pos.xyz + 0.01 * vel.xyz ), 0.0, 1.0);
	particleBuffer.particles[flattened_id].Color = vec4(a, a, a, color.w);
	*/
}
