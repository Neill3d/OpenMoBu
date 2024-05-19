
/*
	Fragment shader for a sprite sheet texture in MotionBuilder
		MoPlugs Project (c) 2015
		
	Author Sergey Solohin (Neill3d)
	 e-mail to: s@neill3d.com
	 www.neill3d.com
*/

uniform	sampler2D		tex;

uniform float			countU;
uniform float			countV;

uniform float			paramU;

uniform float			premultAlpha;

void main (void)
{
	vec2 tx  = gl_TexCoord [0].xy;
	vec2 count = vec2(countU, countV);
	vec2 size = vec2(1.0) / count;
	vec2 realtx = tx * size;
	
	float u = 1.0 - paramU;
	
	float sizeInLine = 1.0 / countV;
	float row = floor(u / sizeInLine);
	row = clamp(row, 0.0, countV-1.0);
	
	float col = u - row * sizeInLine;
	float sizeInCol = sizeInLine / countU;
	col = floor(col / sizeInCol);
	col = clamp(col, 0.0, countU-1.0);
	
	vec2 param = vec2(col, countV-row);
	tx = realtx + param * size;
	
	vec4 color = texture2D( tex, tx );
	
	if (premultAlpha > 0.0)
		color.a *= mix(color.b, mix(color.r, color.g, 0.5), 0.5);

	gl_FragColor = color;
}
