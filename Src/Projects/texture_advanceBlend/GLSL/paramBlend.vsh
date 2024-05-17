
/*
	Vertex shader for the Parametric Blend Texture plugin in MotionBuilder
		MoPlugs Project (c) 2015
		
	Author Sergey Solohin (Neill3d)
	 e-mail to: s@neill3d.com
	 www.neill3d.com
*/

void main(void)
{
	gl_Position     = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord [0] = gl_MultiTexCoord0;
}
