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

uniform sampler2D sampler0;
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
	float dist = distance(gl_TexCoord[0].xy, vec2(0.5,0.5)); // gl_TexCoord[3].xy
	dist = smoothstep(vignout+(fstop/vignfade), vignin+(fstop/vignfade), dist);
	return clamp(dist,0.0,1.0);
}

void main (void)
{
	vec2 tx = gl_TexCoord [0].st;
	
	if (tx.y < upperClip || tx.y > lowerClip)
	{
		vec4 fragColor = texture2D(sampler0, tx);
		gl_FragData [0] =  fragColor;
		return;
	}
	
	vec4 color = texture2D( sampler0, tx ); 
	
	vec4 vigncolor = color * vignette();

	//
	// masking 
	float f = amount;

	if (useMasking > 0.0)
	{
		vec4 mask = texture2D( maskSampler, tx );
		f *= 1.0 - mask.r * useMasking;
	}
	
	color = mix(color, vigncolor, f);
	gl_FragData [0] =  color;
}
