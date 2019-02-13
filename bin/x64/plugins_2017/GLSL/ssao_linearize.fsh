
//
// Fragment Shader - SSAO
//
//	Post Processing Toolkit
//
//	Sergei <Neill3d> Solokhin 2018
//
//	GitHub page - https://github.com/Neill3d/OpenMoBu
//	Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
//	Special for Les Androids Associes
//

#version 120

uniform	sampler2D	colorSampler;
uniform sampler2D	depthSampler;

uniform vec4	clipInfo;	// z_n * z_f,  z_n - z_f,  z_f, perspective = 1 : 0

uniform float 	distanceThreshold;	// = 10.5;
uniform vec2	filterRadius;		// = vec2( 0.02, 0.02 );
uniform float		only_ao;

uniform mat4	g_matInvProjection;

uniform float 		zNear = 10.0;
uniform float 		zFar = 4000.0;

uniform float		gamma = 1.0;
uniform float		contrast = 1.0;


float reconstructCSZ(float d, vec4 clipInfo) {
  if (clipInfo[3] != 0) {
    return (clipInfo[0] / (clipInfo[1] * d + clipInfo[2]));
  }
  else {
    return (clipInfo[1]+clipInfo[2] - d * clipInfo[1]);
  }
}
/*
    if (in_perspective == 1.0) // perspective
    {
        ze = (zNear * zFar) / (zFar - zb * (zFar - zNear)); 
    }
    else // orthographic proj 
    {
        ze  = zNear + zb  * (zFar - zNear);
    }
*/

void main()
{
	float depth = texture2D(depthSampler, gl_TexCoord[0].st).x;
	float linear = reconstructCSZ(depth, clipInfo);
	//linear = depth;
	gl_FragColor = vec4(linear, linear, linear, 1.0);
}