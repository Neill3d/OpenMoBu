//
// Fragment shader - Simple 
//
//	Post Processing Toolkit
//	 Created by Sergey <Neill3d> Solokhin (c) 2018
//	Special for Les Androids Associes
//

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
	vec2 tx = gl_TexCoord [0].st;
	
	float depth = texture2D(depthSampler, tx).x;
	float linear = reconstructCSZ(depth, gClipInfo);
	
	gl_FragData [0] =  vec4(linear);
}
