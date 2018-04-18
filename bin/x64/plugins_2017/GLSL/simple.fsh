//
// Fragment shader - Simple 
//
//	Post Processing Toolkit
//	 Created by Sergey <Neill3d> Solokhin (c) 2018
//	Special for Les Androids Associes
//

uniform sampler2D sampler0;

void main (void)
{
	vec2 tx = gl_TexCoord [0].st;
	vec4 color = texture2D( sampler0, tx ); 
	
	gl_FragData [0] =  color;
}
