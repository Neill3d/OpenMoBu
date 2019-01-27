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

void main(void)
{
	gl_Position     = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord [0] = gl_MultiTexCoord0;
}
