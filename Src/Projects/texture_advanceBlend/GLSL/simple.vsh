
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: simple.vsh
//
//	Vertex GLSL shader
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

void main(void)
{
	gl_Position     = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord [0] = gl_MultiTexCoord0;
}
