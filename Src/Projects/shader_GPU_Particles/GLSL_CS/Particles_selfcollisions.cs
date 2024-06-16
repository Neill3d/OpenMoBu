
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Particles_selfcollisions.cs
//
//	Author Sergey Solokhin (Neill3d)
//
// GPU Particles self collision
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#version 430

layout (local_size_x = 256, local_size_y = 1) in;

uniform int		gNumParticles;
uniform float	DeltaTimeSecs;

#define		ACCELERATION_LIMIT		15.0

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

layout (std430, binding = 0) buffer ParticleBuffer
{
	TParticle particles[];
} particleBuffer;

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
shared vec4 tmp[gl_WorkGroupSize.x];
shared vec4 tmpVel[gl_WorkGroupSize.x];

void main()
{
	// particle id in the array
	uint flattened_id = get_invocation();
	
	// ?! skip unused part of the array
	if (flattened_id >= gNumParticles)
		return;
	
	vec3 acceleration = vec3(0.0);

	// Read position and velocity
	vec4 pos = particleBuffer.particles[flattened_id].Pos;
	vec4 vel = particleBuffer.particles[flattened_id].Vel;
	vec4 packedColor = particleBuffer.particles[flattened_id].Color;
	float Age = packedColor.z;

	float mass = 1.0;
	float radius1 = pos.w;

	if (Age > 0.0)
	{
		//int N = int(gl_NumWorkGroups.x*gl_WorkGroupSize.x);
	
		//
		// linear calculation
		
		for (int i=0; i<gNumParticles; ++i)
		{
			
			if ( i == flattened_id )
				continue;
			
			vec4 other = particleBuffer.particles[i].Pos;
			vec4 othervel = particleBuffer.particles[i].Vel;
			vec4 otherColor = particleBuffer.particles[i].Color;

			vec3 n = pos.xyz - other.xyz;
			float udiff = length(n);
			float radsum = radius1 + other.w;

			if ( otherColor.z > 0.0 && udiff < radsum )
			{
				n = normalize(n);

				float a1 = dot(vel.xyz, n);
				float a2 = dot(othervel.xyz, n);

				float optimizedP = (2.0 * (a1 - a2)) / (mass + mass); 

				// calculate v1', the new movement vector of circle1
				//vel.xyz = vel.xyz - optimizedP * mass * n;
				acceleration = acceleration - optimizedP * mass * n;
			}
		}
		
		/*
		int N = int(gl_NumWorkGroups.x*gl_WorkGroupSize.x);
		int groupSize = int(gl_WorkGroupSize.x);
	
		for(int tile = 0; tile<N; tile+=groupSize) 
		{
			int id = tile + int(gl_LocalInvocationIndex);
			//tmpIndex[gl_LocalInvocationIndex] = tile + int(gl_LocalInvocationIndex);
			if (id < gNumParticles)
			{
				tmp[gl_LocalInvocationIndex] = particleBuffer.particles[id].Pos;
				tmpVel[gl_LocalInvocationIndex] = particleBuffer.particles[id].Vel;
				tmpVel[gl_LocalInvocationIndex].w = particleBuffer.particles[id].Color.z; // Age is stored in packedColor.z
			}
			else
			{
				tmp[gl_LocalInvocationIndex] = vec4(0.0);
				tmpVel[gl_LocalInvocationIndex] = vec4(0.0);
			}
			
			groupMemoryBarrier();
			barrier();
			for(int i=0; i<groupSize; ++i) 
			{
				vec4 other = tmp[i];
				vec4 othervel = tmpVel[i];

				vec3 n = pos.xyz - other.xyz;
				float udiff = length(n);
				float radsum = radius1 + other.w;
				
				if ( othervel.w > 0.0 && udiff < radsum && udiff > 0.0 )
				{
					n = normalize(n);

					float a1 = dot(vel.xyz, n);
					float a2 = dot(othervel.xyz, n);

					float optimizedP = (2.0 * (a1 - a2)) / (mass + mass); 

					// calculate v1', the new movement vector of circle1
					//vel.xyz = vel.xyz - optimizedP * mass * n;
					acceleration = acceleration - optimizedP * mass * n;
				}
			}
			groupMemoryBarrier();
			barrier();
		}
		*/
	}

	float accLen = length(acceleration);
	if (accLen > ACCELERATION_LIMIT)
		acceleration = ACCELERATION_LIMIT * normalize(acceleration);
	

	particleBuffer.particles[flattened_id].Vel = vec4(vel.xyz + acceleration, vel.w);	// in w we individual birth randomF
	//particleBuffer.particles[flattened_id].RotVel = vec4(acceleration.xyz, rotVel.w);
}