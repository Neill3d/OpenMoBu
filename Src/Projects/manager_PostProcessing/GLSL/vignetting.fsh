//
// Fragment shader - Vignetting
//
//	Post Processing Toolkit
//
//	Sergei <Neill3d> Solokhin 2018
//
//	GitHub page - https://github.com/Neill3d/OpenMoBu
//	Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
//	Special for Les Androids Associes
//

#version 140

in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D colorSampler;
uniform sampler2D maskSampler;

uniform float	useMasking;
uniform float	upperClip;
uniform float	lowerClip;

float fstop = 0.5; //f-stop value

uniform float amount; // = true; //use optical lens vignetting?
uniform float vignout; // = 1.3; //vignetting outer border
uniform float vignin; // = 0.0; //vignetting inner border
uniform float vignfade; // = 22.0; //f-stops till vignete fades

float vignette()
{
	float dist = distance(texCoord.xy, vec2(0.5,0.5));
	dist = smoothstep(vignout+(fstop/vignfade), vignin+(fstop/vignfade), dist);
	return clamp(dist,0.0,1.0);
}

void main (void)
{
	vec2 tx = texCoord;
	
	if (tx.y < upperClip || tx.y > lowerClip)
	{
		vec4 fragColor = texture2D(colorSampler, tx);
		FragColor =  fragColor;
		return;
	}
	
	vec4 color = texture2D(colorSampler, tx); 
	
	vec4 vigncolor = color * vignette();

	//
	// masking 
	float f = amount;

	if (useMasking > 0.0)
	{
		vec4 mask = texture2D(maskSampler, tx);
		f *= 1.0 - mask.r * useMasking;
	}
	
	color = mix(color, vigncolor, f);
	FragColor = color;
}
