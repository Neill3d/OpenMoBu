//
// Fragment shader - Vignetting
//
//	Post Processing Toolkit
//	 Created by Sergey <Neill3d> Solokhin (c) 2018
//	Special for Les Androids Associes
//

uniform sampler2D sampler0;

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
	color = mix(color, vigncolor, amount);
	
	gl_FragData [0] =  color;
}
