
///////////////////////////////////////////////////////////////////
//
// MoPLUGS Project (c) 2014
//  Shader for Composite Master Tool
// Author Sergey Solohin (Neill3d)
//		e-mail to: s@neill3d.com
//	www.neill3d.com
///////////////////////////////////////////////////////////////////

uniform	sampler2D	mainTex;
uniform vec4		colorX;
uniform vec4		colorY;
uniform vec4		colorZ;

void main (void)
{
	vec2 tx  = gl_TexCoord [0].xy;
	vec4 normal = texture2D ( mainTex, tx );
	
	vec4 finalColor = normal.x * colorX + normal.y * colorY + normal.z * colorZ;
	finalColor.a = normal.a;
	
	gl_FragColor = finalColor;
}
