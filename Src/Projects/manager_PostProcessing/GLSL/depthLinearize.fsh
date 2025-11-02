//
// Fragment shader - Simple 
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

#version 140

in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D depthSampler;

uniform vec4	gClipInfo;	// z_n * z_f,  z_n - z_f,  z_f, perspective = 1 : 0

// ----------------------------------

float reconstructCSZ(float d, vec4 clipInfo) {
  if (clipInfo[3] != 0.0) {
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

// -------------------------------------
void main (void)
{
	float depth = texture(depthSampler, texCoord).x;
	float linear = reconstructCSZ(depth, gClipInfo);
	
	FragColor = vec4(linear);
}
