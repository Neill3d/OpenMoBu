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

#version 140

in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D sampler0;
uniform sampler2D maskSampler;

uniform float	useMasking;
uniform float	upperClip;
uniform float	lowerClip;

uniform float amount;

uniform vec2 iResolution;

uniform float iTime;
uniform vec4  light_pos;

uniform vec4  tint;
uniform float	inner;
uniform float	outer;

uniform float fadeToBorders; // = 1.0;
uniform float borderWidth;
uniform float feather; // = 1.0;

uniform float flareSeed;


//
// FONCTION PARTAGEES
//
vec3 cc(vec3 color, float factor,float factor2) // color modifier
{
	float w = color.x+color.y+color.z;
	return mix(color,vec3(w)*factor,w*factor2);
}


//
//FONCTIONS DU FLARE 3 
//

const vec3 suncolor = vec3(0.643,0.494,0.867);

float getSun(vec2 uv){
    return length(uv) < 0.009 ? 1.0 : 0.0;
}



//from: https://www.shadertoy.com/view/XdfXRX
vec3 lensflare2(vec2 uv, vec2 pos, out vec3 sunflare, out vec3 lensflare)
{
	vec2 main = uv-pos;
	vec2 uvd = uv*(length(uv));

	float ang = atan(main.y, main.x);
	float dist = length(main);
    dist = pow(dist, 0.1);

	float f0 = 1.0/(length(uv-pos)*25.0+1.0);
	f0 = pow(f0, 2.0);

	f0 = f0+f0*(sin((ang+1.0/18.0)*12.0)*.1+dist*.1+.8);

	float f2 = max(1.0/(1.0+32.0*pow(length(uvd+0.8*pos),2.0)),.0)*00.25;
	float f22 = max(1.0/(1.0+32.0*pow(length(uvd+0.85*pos),2.0)),.0)*00.23;
	float f23 = max(1.0/(1.0+32.0*pow(length(uvd+0.9*pos),2.0)),.0)*00.21;

	vec2 uvx = mix(uv,uvd,-0.5);

	float f4 = max(0.01-pow(length(uvx+0.4*pos),2.4),.0)*6.0;
	float f42 = max(0.01-pow(length(uvx+0.45*pos),2.4),.0)*5.0;
	float f43 = max(0.01-pow(length(uvx+0.5*pos),2.4),.0)*3.0;

	uvx = mix(uv,uvd,-.4);

	float f5 = max(0.01-pow(length(uvx+0.2*pos),5.5),.0)*2.0;
	float f52 = max(0.01-pow(length(uvx+0.4*pos),5.5),.0)*2.0;
	float f53 = max(0.01-pow(length(uvx+0.6*pos),5.5),.0)*2.0;

	uvx = mix(uv,uvd,-0.5);

	float f6 = max(0.01-pow(length(uvx-0.3*pos),1.6),.0)*6.0;
	float f62 = max(0.01-pow(length(uvx-0.325*pos),1.6),.0)*3.0;
	float f63 = max(0.01-pow(length(uvx-0.35*pos),1.6),.0)*5.0;

    sunflare = vec3(f0);
    lensflare = vec3(f2+f4+f5+f6, f22+f42+f52+f62, f23+f43+f53+f63);

	return sunflare+lensflare;
}


vec3 anflares(vec2 uv, float threshold, float intensity, float stretch, float brightness)
{
    threshold = 1.0 - threshold;

    vec3 hdr = vec3(getSun(uv));
    hdr = vec3(floor(threshold+pow(hdr.r, 1.0)));

    float d = intensity;
    float c = intensity*stretch;

    for (float i=c; i>-1.0; i--){
        float texL = getSun(uv+vec2(i/d, 0.0));
        float texR = getSun(uv-vec2(i/d, 0.0));
        
        hdr += floor(threshold+pow(max(texL,texR), 4.0))*(1.0-i/c);
    }
    
    return hdr*brightness;
}


vec3 anflares(vec2 uv, float intensity, float stretch, float brightness)
{
    uv.x *= 1.0/(intensity*stretch);
    uv.y *= 0.5;
    return vec3(smoothstep(0.009, 0.0, length(uv)))*brightness;
}


void main(void)
{
	vec2 tx = texCoord.st;
	
	if (tx.y < upperClip || tx.y > lowerClip)
	{
		FragColor = texture2D(sampler0, tx);
		return;
	}
	
	vec4 color = texture2D( sampler0, tx ); 
	
	vec2 fragCoord = gl_FragCoord.xy;
	vec3 iMouse = vec3(light_pos.x * iResolution.x, light_pos.y * iResolution.y, light_pos.z);
	
	vec2 uv = fragCoord.xy / iResolution.xy - 0.5;
	uv.x *= iResolution.x/iResolution.y; //fix aspect ratio
	vec3 mouse = vec3(iMouse.xy/iResolution.xy - 0.5,iMouse.z-.5);
	float att = 1.0 - clamp(2.0 * (light_pos.z - 0.5), 0.0, 1.0);
	mouse.z = clamp(0.275 * log(100.0 * att), 0.0, 1.0);
	mouse.x *= iResolution.x/iResolution.y; //fix aspect ratio
	vec3 col = vec3(0.0);
	vec3 sun = vec3(0.0);
	vec3 sunflare, lensflare;
	vec3 flare = lensflare2(uv*1.5, mouse.xy*1.5, sunflare, lensflare);
	
	vec3 anflare = pow(anflares(uv-mouse.xy, 0.5, 400.0, 0.9 + (flareSeed*0.05 - 1), 0.1), vec3(4.0));
    sun += getSun(uv-mouse.xy) + (flare + anflare)*suncolor*2.0;
    col += sun;
    col = pow(col, vec3(1.0/2.2));
    
	vec3 flareColor = cc(col,outer,inner); 
	flareColor *= tint.rgb;
	
	float f = mix(amount, amount * mouse.z, light_pos.w);

	if (fadeToBorders > 0.0)
	{
		float distToBorder = light_pos.x + borderWidth;
		distToBorder = min(distToBorder, iResolution.x + borderWidth - light_pos.x);
	
		distToBorder = min(distToBorder, light_pos.y + borderWidth);
		distToBorder = min(distToBorder, iResolution.y + borderWidth - light_pos.y);
	
		distToBorder *= 0.01;
		distToBorder = clamp(distToBorder, 0.0, 1.0);
		
		f *= smoothstep(0.0, feather, distToBorder);
	}
	
	//
	// masking 

	vec4 mask = vec4(0.0, 0.0, 0.0, 0.0);
	if (useMasking > 0.0)
	{
		mask = texture2D( maskSampler, tx );
	}
	color.rgb = mix(flareColor * f + color.rgb, color.rgb, mask.r * useMasking);
	FragColor = color;
}
