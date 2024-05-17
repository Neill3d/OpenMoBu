
///////////////////////////////////////////////////////////////////
//
// MoPLUGS Project (c) 2014
//  Shader for Composite Master Tool
//
// Shader from Geeks3D Shader Library (www.geeks3d.com)
//
// Author Sergey Solohin (Neill3d)
//		e-mail to: s@neill3d.com
//	www.neill3d.com
///////////////////////////////////////////////////////////////////

uniform	sampler2D	mainSampler;
uniform float		numColors;
uniform float		gamma;

void main (void)
{
	vec4 c4 = texture2D(mainSampler, gl_TexCoord[0].xy);
	vec3 c = c4.rgb;
	c = pow(c, vec3(gamma, gamma, gamma));
	c = c * numColors;
	c = floor(c);
	c = c / numColors;
	c = pow(c, vec3(1.0/gamma));
	gl_FragColor = vec4(c, c4.a);
}
