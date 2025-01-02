//
// Vertex shader - Simple 
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

//layout(location = 0) in vec3 aPos;
//layout(location = 1) in vec2 aTexCoord;

varying vec2 TexCoord;

void main(void)
{
	gl_Position     = gl_ModelViewProjectionMatrix * gl_Vertex;
	TexCoord = gl_MultiTexCoord0.st;
}
