
uniform	sampler2D		colorTex;
uniform	sampler2D		maskTex;

void main (void)
{
	vec2 tx  = gl_TexCoord [0].xy;
	vec4 aColor = texture2D ( colorTex, tx );
	vec4 bColor = texture2D ( maskTex, tx );
	
	gl_FragColor = aColor * bColor;
	gl_FragColor.w = bColor.r;	// copy mask as alpha
}
