//
// Fragment shader - Simple 
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

uniform sampler2D sampler0;

void main (void)
{
	vec2 tx = gl_TexCoord [0].st;
	vec4 color = texture2D( sampler0, tx ); 
	
	gl_FragData [0] =  color;
}
