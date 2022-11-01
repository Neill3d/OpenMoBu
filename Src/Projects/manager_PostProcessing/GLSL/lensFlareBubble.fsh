//
// Fragment shader - Lens Flare
//
//	Post Processing Toolkit
//
//	Sergei <Neill3d> Solokhin 2018-2019
//
//	GitHub page - https://github.com/Neill3d/OpenMoBu
//	Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
//	Special for Les Androids Associes
//

// musk's lense flare, modified by icecool.
// See the original at: https://www.shadertoy.com/view/4sX3Rs 



uniform sampler2D sampler0;

uniform float	upperClip;
uniform float	lowerClip;

uniform float amount;

uniform float textureWidth;
uniform float textureHeight;

uniform float iTime;
uniform vec4  light_pos;

uniform vec4  tint;
uniform float	inner;
uniform float	outer;

uniform float fadeToBorders; // = 1.0;
uniform float borderWidth;
uniform float feather; // = 1.0;

uniform float flareSeed;

float width = textureWidth; //texture width
float height = textureHeight; //texture height


//
// FONCTION PARTAGEES
//
vec3 cc(vec3 color, float factor,float factor2) // color modifier
{
	float w = color.x+color.y+color.z;
	return mix(color,vec3(w)*factor,w*factor2);
}



//
//FONCTIONS DU FLARE 1
//
vec4 noise1(float t){	
	
	vec2 coord = vec2(t, t);
    
    coord -= floor(coord / 289.0) * 289.0;
    coord += vec2(223.35734, 550.56781);
    coord *= coord;
    
    float xy = coord.x * coord.y;
    
    return vec4(fract(xy * 0.00000012),fract(xy * 0.00000543),fract(xy * 0.00000192),fract(xy * 0.00000423));
}

vec3 lensflare1(vec2 uv, vec2 pos, float seed, float size)
{
	vec4 gn = noise1(seed-1.0);
	gn.x = size;
	vec3 c = vec3(.0);
	vec2 p = pos;
	vec2 d = uv-p;
	
	
	c += (0.01+gn.x*.2)/(length(d));
	
	c += vec3(noise1(atan(d.x,d.y)*256.9+pos.x*2.0).y*.25)*c;
	
	float fltr = length(uv);
	fltr = (fltr*fltr)*.5+.5;
	fltr = min(fltr,1.0);
	
	for (float i=.0; i<20.; i++)
	{
		
		vec4 n = noise1(seed+i);
		vec4 n2 = noise1(seed+i*2.1);
		vec4 nc = noise1 (seed+i*3.3);
		nc+=vec4(length(nc));
		nc*=.65;
		
		for (int i=0; i<3; i++)
		{
			float ip = n.x*3.0+float(i)*.1*n2.y*n2.y*n2.y;
			float is = n.y*n.y*4.5*gn.x+.1;
			float ia = (n.z*4.0-2.0)*n2.x*n.y;
			vec2 iuv = (uv*(mix(1.0,length(uv),n.w*n.w)))*mat2(cos(ia),sin(ia),-sin(ia),cos(ia));
			vec2 id = mix(iuv-p,iuv+p,ip);
			c[i] += pow(max(.0,is-(length(id))),.45)/is*.1*gn.x*nc[i]*fltr;
		}
		
	}
	
	
	return c;
}


//Main du flare 1
void main(void)
{
	vec2 tx = gl_TexCoord [0].st;
	
	if (tx.y < upperClip || tx.y > lowerClip)
	{
		vec4 fragColor = texture2D(sampler0, tx);
		gl_FragData [0] =  fragColor;
		return;
	}
	
	vec4 color = texture2D( sampler0, tx ); 
	
	vec2 fragCoord = gl_FragCoord.xy;
	vec2 iResolution = vec2(textureWidth, textureHeight);
	vec3 iMouse = vec3(light_pos.x * textureWidth, light_pos.y * textureHeight, light_pos.z);
	
	vec2 uv = fragCoord.xy / iResolution.xy - 0.5;
	uv.x *= iResolution.x/iResolution.y; //fix aspect ratio
	vec3 mouse = vec3(iMouse.xy/iResolution.xy - 0.5,iMouse.z-.5);
	float att = 1.0 - clamp(2.0 * (light_pos.z - 0.5), 0.0, 1.0);
	mouse.z = clamp(0.275 * log(100.0 * att), 0.0, 1.0);
	mouse.x *= iResolution.x/iResolution.y; //fix aspect ratio
	
	vec3 flareColor = lensflare1(uv,mouse.xy, flareSeed, 0.15);

	flareColor = cc(flareColor,outer,inner); 
	flareColor *= tint.rgb;
	
	float f = mix(amount, amount * mouse.z, light_pos.w);
	
	if (fadeToBorders > 0.0)
	{
		float distToBorder = light_pos.x + borderWidth;
		distToBorder = min(distToBorder, width + borderWidth - light_pos.x);
	
		distToBorder = min(distToBorder, light_pos.y + borderWidth);
		distToBorder = min(distToBorder, height + borderWidth - light_pos.y);
	
		distToBorder *= 0.01;
		distToBorder = clamp(distToBorder, 0.0, 1.0);
		
		f *= smoothstep(0.0, feather, distToBorder);
	}
	
	color.rgb = color.rgb + f * flareColor;
	gl_FragData [0] =  color;
}