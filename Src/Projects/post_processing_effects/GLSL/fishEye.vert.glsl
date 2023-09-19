//
// Vertex shader - Fish Eye
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
varying vec4 Vertex_UV;
varying vec4 posDevSpace;
// uniform mat4 gxl3d_ModelViewProjectionMatrix;

void main(void)
{
	gl_Position     = gl_ModelViewProjectionMatrix * gl_Vertex;
	//gl_TexCoord [0] = gl_MultiTexCoord0;
	
	Vertex_UV = gl_MultiTexCoord0;
	posDevSpace = gl_MultiTexCoord0 + vec4(1.0, 1.0, 0.0, 0.0); // gl_Position; // / gl_Position.w;
	//posDevSpace = vec4(0.5, 0.5, 1.0, 1.0) + (posDevSpace + vec4(1.0, 1.0, 0.0, 0.0));
}