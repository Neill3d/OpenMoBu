//
// Vertex shader - Model Phong Shading 
//
//	Post Processing Toolkit
//
// Sergei <Neill3d> Solokhin 2018-2024
//
// GitHub page - https://github.com/Neill3d/OpenMoBu
// Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
//	Special thanks for Les Androids Associes
//

#version 430 compatibility


struct TTransform
{
	mat4 	m4_World;
	mat4	m4_View;
	mat4	m4_Proj;
	mat4	m4_Model;
	
	mat4	normalMatrix;
	
	vec4	eyePos;
};

layout (std430, binding = 0) buffer TransformBuffer
{
	TTransform transform;
	
} transformBuffer;

/////////////////////////////////////////////

	layout(location=0) in vec4 inPosition;
	layout(location=1) in vec2 inTexCoords;
	layout(location=2) in vec4 inNormal;
	layout(location=3) in vec4 inTangent;
	layout(location=4) in vec2 inTexCoords2;
	
	layout(location=0) smooth out vec3 outNw;	// 12 bytes
	layout(location=1) smooth out vec2 outTC;	// 8 bytes
	layout(location=2) smooth out vec3 outPw;	// 12 bytes
	layout(location=3) smooth out vec3 outWV;	// 12 bytes
		
	layout(location=4) smooth out vec3	n_eye;
	layout(location=5) smooth out vec3 	outTangent;
	layout(location=6) smooth out vec2 outTC2;
	
	layout(location=7) smooth out float fogAmount;
	layout(location=8) smooth out vec3 viewPos;

out gl_PerVertex {
	vec4 gl_Position;		// 16 bytes
};

///////////////////////////////////////////////////////////////////////////////
//

uniform mat4			displacementMatrix;
uniform vec4			displacementOption;

uniform vec4		fogOptions;

uniform sampler2D		samplerDisplacement;

float fogFactorLinear(
  const float dist,
  const float start,
  const float end
) {
  return 1.0 - clamp((end - dist) / (end - start), 0.0, 1.0);
}

void main(void)
{
	// surface normal in eye space
	mat4 normalMatrix = transformBuffer.transform.normalMatrix;
	vec4 N = vec4(inNormal.xyz, 1.0);

	if (transformBuffer.transform.eyePos.w > 0.0)
	{
		N = normalMatrix * N;
	}
	else
	{
		N = normalMatrix * -N;
		
	}
	
	vec3 offset = vec3(0.0);
	
	if (displacementOption.x > 0.0)
	{
		vec2 dispUV = (displacementMatrix * vec4(inTexCoords.st, 0.0, 1.0)).xy;
		offset = inNormal.xyz * (texture2D(samplerDisplacement, dispUV.st).r - displacementOption.y);
		offset *= displacementOption.x;
	}
	vec4 vertex = vec4(inPosition.xyz + offset, 1.0);
	vec4 Po    =  transformBuffer.transform.m4_Model * vertex;
	vec4 vPosition4 = transformBuffer.transform.m4_View * Po;
	vec3 vPosition3 = vPosition4.xyz; // / vPosition4.w;
	
	if (transformBuffer.transform.eyePos.w < 0.0)
	{
		//N.xyz = reflect(N.xyz, normalize(vPosition3));
		N.xyz = reflect(N.xyz, vec3(0.0, 0.0, 1.0));
	}
	
	gl_Position = transformBuffer.transform.m4_Proj * vPosition4;
	float fogDistance = length(gl_Position.xyz);
	fogAmount = fogFactorLinear(fogDistance, fogOptions.x, fogOptions.y);
	
	outNw           = N.xyz; // / N.w;
	outTangent = (normalMatrix * vec4(inTangent.xyz, 1.0)).xyz;
	outTC           = inTexCoords;
	outTC2 			= inTexCoords2;
	outPw           = vPosition3; // / vPosition4.w;
    outWV = Po.xyz; //transformBuffer.transform.eyePos.xyz - Po.xyz; // / Po.w; //(Po.xyz - eyePos);
	
	normalMatrix = transpose(inverse(transformBuffer.transform.m4_World * transformBuffer.transform.m4_Model));
	n_eye = (normalMatrix * vec4(inNormal.xyz, 1.0)).xyz; // inNormal.xyz;
	viewPos = transformBuffer.transform.eyePos.xyz;
}
