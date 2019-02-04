
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solokhin (Neill3d) 2014-2017
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////

// recomputer normals in real-time (for a good quality deformations) 

#version 430
#extension GL_NV_shader_atomic_float : enable

layout (local_size_x = 1024, local_size_y = 1) in;

uniform int		numberOfTriangles;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TYPES AND DATA BUFFERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// INPUT from mobu mesh data
layout (std430, binding = 0) readonly buffer PositionBuffer
{
	vec4	positions[];
} positionBuffer;

layout (std430, binding = 1) readonly buffer IndicesBuffer
{
	int		indices[];
} indicesBuffer;

// output
layout (std140, binding = 2) coherent buffer OutputNormals
{
	vec4	normals[];
} outputNormals;


////////////////////////////////////////////////////////
//

uint get_invocation()
{
   uint work_group = gl_GlobalInvocationID.y * (gl_NumWorkGroups.x * gl_WorkGroupSize.x) + gl_GlobalInvocationID.x;
   return work_group;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void main()
{
	// id in the array
	uint flattened_id = get_invocation();
	
	// ?! skip unused part of the array
	if (flattened_id >= numberOfTriangles)
		return;
	
	int id1 = indicesBuffer.indices[flattened_id * 3];
	int id2 = indicesBuffer.indices[flattened_id * 3 + 1];
	int id3 = indicesBuffer.indices[flattened_id * 3 + 2];
	
	vec4 pos1 = positionBuffer.positions[id1];
	vec4 pos2 = positionBuffer.positions[id2];
	vec4 pos3 = positionBuffer.positions[id3];
	
	vec4 pos21 = pos2-pos1;
	vec4 pos31 = pos3-pos1;
	
	vec3 n = cross(pos21.xyz, pos31.xyz);
	n = normalize(n);
	
	barrier();
	
	// accumulate vertex normal by connected face normal
	atomicAdd(outputNormals.normals[id1].x, n.x);
	atomicAdd(outputNormals.normals[id1].y, n.y);
	atomicAdd(outputNormals.normals[id1].z, n.z);
	
	atomicAdd(outputNormals.normals[id2].x, n.x);
	atomicAdd(outputNormals.normals[id2].y, n.y);
	atomicAdd(outputNormals.normals[id2].z, n.z);
	
	atomicAdd(outputNormals.normals[id3].x, n.x);
	atomicAdd(outputNormals.normals[id3].y, n.y);
	atomicAdd(outputNormals.normals[id3].z, n.z);
}