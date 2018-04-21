//
// Fragment shader - Mix (Multiply) 
//
//	Post Processing Toolkit
//	 Created by Sergey <Neill3d> Solokhin (c) 2018
//	Special for Les Androids Associes
//

uniform sampler2D sampler0;
uniform sampler2D sampler1;

void main (void)
{
	vec2 tx = gl_TexCoord [0].st;
	vec4 colorA = texture2D( sampler0, tx ); 
	vec4 colorB = texture2D( sampler1, tx );
	
	gl_FragData [0] =  colorA * colorB;
}
