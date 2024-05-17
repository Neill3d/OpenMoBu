uniform	sampler2D	mainTex;
uniform vec4		scale;
void main (void)
{
	vec2 tx  = gl_TexCoord [0].xy;
	vec2 dx  = scale.xy;
	vec2 sdx = dx;
	vec4 sum = texture2D ( mainTex, tx ) * 0.134598;

	sum += (texture2D ( mainTex, tx + sdx ) + texture2D ( mainTex, tx - sdx ) )* 0.127325; 
	sdx += dx;
	sum += (texture2D ( mainTex, tx + sdx ) + texture2D ( mainTex, tx - sdx ) )* 0.107778; 
	sdx += dx;
	sum += (texture2D ( mainTex, tx + sdx ) + texture2D ( mainTex, tx - sdx ) )* 0.081638; 
	sdx += dx;
	sum += (texture2D ( mainTex, tx + sdx ) + texture2D ( mainTex, tx - sdx ) )* 0.055335; 
	sdx += dx;
	sum += (texture2D ( mainTex, tx + sdx ) + texture2D ( mainTex, tx - sdx ) )* 0.033562; 
	sdx += dx;
	sum += (texture2D ( mainTex, tx + sdx ) + texture2D ( mainTex, tx - sdx ) )* 0.018216; 
	sdx += dx;
	sum += (texture2D ( mainTex, tx + sdx ) + texture2D ( mainTex, tx - sdx ) )* 0.008847; 
	sdx += dx;

	gl_FragColor = sum;
}
