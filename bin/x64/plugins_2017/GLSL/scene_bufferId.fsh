//
// Fragment shader - Model Buffer Id 
//
//	Post Processing Toolkit
//	 Created by Sergey <Neill3d> Solokhin (c) 2018
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
