
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: resetParticles.cs
//
//	prepare emitters (rate count) and particles (pre-count), define start properties (pos, vel, color)
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#version 430

layout (local_size_x = 1024, local_size_y = 1) in;

uniform int		rate;
uniform int		preCount;
uniform float	extrudeDist;
uniform int		randomSeed;
uniform int		totalCount;

uniform int		numberOfTriangles;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TYPES AND DATA BUFFERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct TParticle
{ 
	vec4				Pos;				// in w hold lifetime from 0.0 to 1.0 (normalized)
	vec4				Vel;				// in w hold total lifetime
	vec4				Color;				// inherit color from the emitter surface, custom color simulation
	vec4				Rot;				// in w - float				AgeMillis;			// current Age
	vec4 				RotVel;				// in w - float				Index;				// individual assigned index from 0.0 to 1.0 (normalized)
};

// emitter surface
struct TTriangle
{
	vec4	p[3];
	vec4	n;
	vec2	uv[3];
	vec2	temp;	// to align type
};

// write directly to main particles array
layout (std430, binding = 0) buffer ParticlesBuffer
{
	TParticle	particles[];
} particlesBuffer;

// surface data, to generate particles from it
layout (std430, binding = 1) buffer SurfaceBuffer
{
	TTriangle	tris[];
} surfaceBuffer;

uint get_invocation()
{
   //uint work_group = gl_WorkGroupID.x * gl_NumWorkGroups.y * gl_NumWorkGroups.z + gl_WorkGroupID.y * gl_NumWorkGroups.z + gl_WorkGroupID.z;
   //return work_group * gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z + gl_LocalInvocationIndex;

   // uint work_group = gl_WorkGroupID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x + gl_WorkGroupID.x * gl_WorkGroupSize.x + gl_LocalInvocationIndex;
   uint work_group = gl_GlobalInvocationID.y * (gl_NumWorkGroups.x * gl_WorkGroupSize.x) + gl_GlobalInvocationID.x;
   return work_group;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void main()
{
	// particle id in the array
	uint flattened_id = get_invocation();
	
	// ?! skip unused part of the array
	if (flattened_id >= numberOfTriangles)
		return;
	
	vec3	n = vec3(0.0, 0.0, 0.0);

	for (int i=0; i<3; ++i)
	{
		int index = indexBuffer.indices[flattened_id * 3 + i];

		outputBuffer.tris[flattened_id].p[i] = positionBuffer.positions[index];
		outputBuffer.tris[flattened_id].uv[i] = uvBuffer.uvs[index];
		n = n + normalBuffer.normals[index].xyz;
	}
	// average normal for the tri
	n = normalize(n.xyz / 3.0);
	outputBuffer.tris[flattened_id].n = vec4(n.xyz, 1.0);
}