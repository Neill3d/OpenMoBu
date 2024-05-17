
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
uniform vec4		color1;
uniform vec4		replace1;
uniform vec4		weights1;	// w as multiplier !

uniform vec4		flag;	// x - change color2, y - change color3

uniform vec4		color2;
uniform vec4		replace2;
uniform vec4		weights2;

uniform vec4		color3;
uniform vec4		replace3;
uniform vec4		weights3;

vec3 rgb2hsv(vec3 rgb)
{
	float Cmax = max(rgb.r, max(rgb.g, rgb.b));
	float Cmin = min(rgb.r, min(rgb.g, rgb.b));
    float delta = Cmax - Cmin;

	vec3 hsv = vec3(0., 0., Cmax);
	
	if (Cmax > Cmin)
	{
		hsv.y = delta / Cmax;

		if (rgb.r == Cmax)
			hsv.x = (rgb.g - rgb.b) / delta;
		else
		{
			if (rgb.g == Cmax)
				hsv.x = 2. + (rgb.b - rgb.r) / delta;
			else
				hsv.x = 4. + (rgb.r - rgb.g) / delta;
		}
		hsv.x = fract(hsv.x / 6.);
	}
	return hsv;
}

float chromaKey(vec3 color, vec3 keyColor, vec4 inWeights)
{
	//vec3 backgroundColor = vec3(0.157, 0.576, 0.129);
	vec3 weights = inWeights.xyz * vec3(0.01, 0.01, 0.01) * inWeights.w;

	vec3 hsv = rgb2hsv(color);
	vec3 target = rgb2hsv(keyColor);
	float dist = length(weights * (target - hsv));
	return 1. - clamp(3.0 * dist - 1.5, 0.0, 1.0);
}

void main (void)
{
	vec3 eps = vec3(0.009, 0.009, 0.009);

	vec4 texel = texture2D(mainSampler, gl_TexCoord[0].xy);
	vec3 color = texel.rgb;

	float incrustation = chromaKey(color, color1.rgb, weights1);
	color = mix(color, replace1.rgb, incrustation);

	if (flag.x > 0.0)
	{
		incrustation = chromaKey(color, color2.rgb, weights2);
		color = mix(color, replace2.rgb, incrustation);
	}
	
	if (flag.y > 0.0)
	{
		incrustation = chromaKey(color, color3.rgb, weights3);
		color = mix(color, replace3.rgb, incrustation);
	}
	
	gl_FragColor = vec4(color, texel.a);
}
