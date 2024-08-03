//
// Vertex shader - Model Phong Shading 
//
//	Post Processing Toolkit
//
// Sergei <Neill3d> Solokhin 2018
//
// GitHub page - https://github.com/Neill3d/OpenMoBu
// Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
//	Special for Les Androids Associes
//

#version 430 compatibility


/////////////////////////////////////////////

	layout(location=0) in vec4 inPosition;
	layout(location=2) in vec4 inNormal;

	layout(location=0) smooth out vec3 outNw;	// 12 bytes
	layout(location=2) smooth out vec3 outPw;	// 12 bytes

out gl_PerVertex {
	vec4 gl_Position;		// 16 bytes
};

///////////////////////////////////////////////////////////////////////////////
//

layout(location = 3) uniform mat4 viewMatrix;
layout(location = 4) uniform mat4 projMatrix;
layout(location = 5) uniform mat4 modelMatrix;
layout(location = 6) uniform mat4 normalMatrix;
layout(location = 7) uniform vec4 eyePos;

void main(void)
{
	// surface normal in eye space
	vec4 N = vec4(inNormal.xyz, 1.0);

	if (eyePos.w > 0.0)
	{
		N = normalMatrix * N;
	}
	else
	{
		N = normalMatrix * -N;
	}
	
	
	vec4 vertex = vec4(inPosition.xyz, 1.0);
	vec4 Po    =  modelMatrix * vertex;
	vec4 vPosition4 = viewMatrix * Po;
	vec3 vPosition3 = vPosition4.xyz;
	
	gl_Position = projMatrix * vPosition4;
	
	outNw           = N.xyz; // / N.w;
	outPw           = vPosition3; // / vPosition4.w;
}
