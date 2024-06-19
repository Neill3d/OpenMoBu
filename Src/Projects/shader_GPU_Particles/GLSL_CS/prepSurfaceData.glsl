
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: prepSurfaceData.cs
//
//	prepare surface data directly on GPU for a futher particles generation from a surface
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#version 430

layout (local_size_x = 1024, local_size_y = 1) in;

uniform int		numberOfTriangles;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TYPES AND DATA BUFFERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// emitter surface
struct TTriangle
{
	vec4	p[3];
	vec4	n;
	vec2	uv[3];
	vec2	temp;	// to align type
};

// gpu based model geometry data (positions)
layout (std430, binding = 0) readonly buffer PositionBuffer
{
	vec4	positions[];
} positionBuffer;

// normals
layout (std430, binding = 1) readonly buffer NormalBuffer
{
	vec4	normals[];
} normalBuffer;

// uv
layout (std430, binding = 2) readonly buffer UVBuffer
{
	vec2	uvs[];
} uvBuffer;

// indices
layout (std430, binding = 3) readonly buffer IndexBuffer
{
	int		indices[];
} indexBuffer;


layout (std430, binding = 4) buffer OutputBuffer
{
	TTriangle	tris[];
} outputBuffer;


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

		outputBuffer.tris[flattened_id].p[i] = vec4(positionBuffer.positions[index].xyz, 1.0);
		outputBuffer.tris[flattened_id].uv[i] = uvBuffer.uvs[index];
		n = normalBuffer.normals[index].xyz;
	}
	// average normal for the tri
	//n = normalize(n.xyz / 3.0);
	//vec4 edge0 = outputBuffer.tris[flattened_id].p[1] - outputBuffer.tris[flattened_id].p[0];
	//vec4 edge1 = outputBuffer.tris[flattened_id].p[2] - outputBuffer.tris[flattened_id].p[0];
	//vec3 n = normalize(cross(edge0.xyz, edge1.xyz));
	outputBuffer.tris[flattened_id].n = vec4(n.xyz, 1.0);
}