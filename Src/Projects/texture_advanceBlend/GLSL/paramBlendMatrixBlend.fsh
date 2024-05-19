
/*
	Fragment shader for the Parametric blend texture in MotionBuilder
		MoPlugs Project (c) 2015
		
	Author Sergey Solohin (Neill3d)
	 e-mail to: s@neill3d.com
	 www.neill3d.com
*/


uniform	sampler2D		tex;

uniform float			countU;
uniform float			countV;

uniform float			paramU;
uniform float			paramV;

uniform float			premultAlpha;

void main (void)
{
	vec2 tx  = gl_TexCoord [0].xy;
	vec2 count = vec2(countU, countV);
	vec2 size = vec2(1.0) / count;
	vec2 halfSize = vec2(0.5) * size;
	vec2 realtx = tx * size;
	
	vec2 param = vec2(paramU, paramV);
	
	vec2 param1 = floor(param / size);
	vec2 param2 = floor( (param-halfSize) / size);
	
	param1 = clamp(param1, vec2(0.0), vec2(countU-1.0, countV-1.0));
	param2 = clamp(param2, vec2(0.0), vec2(countU-1.0, countV-1.0));
	
	float blendCol = (param.x / size.x - param1.x);
	float blendRow = (param.y / size.y - param1.y);
	
	param1.y = countV-1.0 - param1.y;
	param2.y = countV-1.0 - param2.y;
	
	tx = realtx + param1 * size;
	vec2 tx2 = realtx + param2 * size;
	
	vec4 color = texture2D( tex, tx );
	vec4 colorRow = texture2D( tex, vec2(tx.x, tx2.y));
	vec4 colorCol = texture2D( tex, vec2(tx2.x, tx.y));
	
	color = mix(colorRow, color, blendRow);
	color = mix(colorCol, color, blendCol);
	
	if (premultAlpha > 0.0)
		color.a *= mix(color.b, mix(color.r, color.g, 0.5), 0.5);

	gl_FragColor = color;
}
