
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergei Solokhin (Neill3d) 2014-2024
//  e-mail to: neill3d@gmail.com
//
/////////////////////////////////////////////////////////////////////////////////////////


// recomputer normals in real-time (for a good quality deformations) 
// 3 - normalize accumulated normals and don't forget about duplicates

#version 430

layout (local_size_x = 1024, local_size_y = 1) in;

uniform int		duplicateStart;
uniform int		duplicateCount;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TYPES AND DATA BUFFERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// inout
layout (std140, binding = 2) buffer OutputNormals
{
	vec4	normals[];
} outputNormals;

layout (std430, binding = 3) buffer Duplicates
{
	int		indices[];
} duplicates;

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
	if (flattened_id >= duplicateCount)
		return;

	int id = duplicates.indices[flattened_id];
	vec4 nor = outputNormals.normals[id];

	outputNormals.normals[duplicateStart + flattened_id] = nor; // mix(nor, outputNormals.normals[duplicateStart + flattened_id], 0.5);
}