//
// Fragment shader - Model Buffer Id 
//
//	Post Processing Toolkit
//
// Sergei <Neill3d> Solokhin 2018
//
// GitHub page - https://github.com/Neill3d/OpenMoBu
// Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
//	Special for Les Androids Associes
//

uniform vec4 		ColorId;
uniform float		UseDiffuseSampler;
uniform sampler2D	sampler0;

void main (void)
{
	vec2 tx = gl_TexCoord [0].st;
	
	vec4 color = ColorId;
	
	if (UseDiffuseSampler > 0.0)
	{
		color.w *= texture2D(sampler0, tx).w;
	}
	//color = vec4(1.0, 0.0, 0.0, 1.0);
	gl_FragData [0] =  color;
}
