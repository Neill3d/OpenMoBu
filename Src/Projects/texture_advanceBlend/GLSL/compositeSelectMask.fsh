
///////////////////////////////////////////////////////////////////
//
// MoPLUGS Project (c) 2014
//  Shader for Composite Master Tool
//
// Author Sergey Solohin (Neill3d)
//		e-mail to: s@neill3d.com
//	www.neill3d.com
///////////////////////////////////////////////////////////////////

uniform	sampler2D	mainSampler;
uniform vec4		maskCode;
uniform int			inverse;


void main (void)
{
	

	vec4 texel = texture2D(mainSampler, gl_TexCoord[0].xy);
	
	vec4 color = vec4(0.0);
	if (texel == maskCode)
	{
		color = (1.0, 1.0, 1.0, 1.0);
	}	
	
	gl_FragColor = color;
}
