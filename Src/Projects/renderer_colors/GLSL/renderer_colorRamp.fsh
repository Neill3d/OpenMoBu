
#version 120     

in vec4			ColorValue;

void main()                                                                         
{
	gl_FragColor = vec4(ColorValue.xyz, 1.0);
}
