
void main(void)
{
	gl_Position     = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord [0].st = vec2(-gl_MultiTexCoord0.s, 1.0-gl_MultiTexCoord0.t);
}