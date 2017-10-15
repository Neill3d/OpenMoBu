
#version 120

varying vec2 v_TextureCoord;

void main(void)
{
	gl_Position     = gl_ModelViewProjectionMatrix * gl_Vertex;
	//gl_TexCoord [0].st = gl_MultiTexCoord0.st;
	v_TextureCoord = gl_MultiTexCoord0.st;
}