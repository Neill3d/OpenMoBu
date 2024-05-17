
uniform	sampler2D		sampler;

void main (void)
{
	vec2 tx  = gl_TexCoord [0].xy;
	gl_FragColor = texture2D( sampler, tx );
}
