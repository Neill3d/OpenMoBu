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
uniform sampler2D maskSampler;

uniform float	useMasking;
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

float width = textureWidth; //texture width
float height = textureHeight; //texture height

/*
float noise(float t)
{
	return texture(iChannel0,vec2(t, 0.0) / iChannelResolution[0].xy).x;
}
float noise(vec2 t)
{
	return texture(iChannel0,(t + vec2(iTime)) / iChannelResolution[0].xy).x;
}
*/

float noise(float t) //generating noise/pattern texture for dithering
{
	vec2 coord = vec2(t, t);
	float noiseX = ((fract(1.0-coord.s*(width/2.0))*0.25)+(fract(coord.t*(height/2.0))*0.75))*2.0-1.0;
	float noiseY = ((fract(1.0-coord.s*(width/2.0))*0.75)+(fract(coord.t*(height/2.0))*0.25))*2.0-1.0;
	
	//if (noise)
	//{
		noiseX = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233))) * 43758.5453),0.0,1.0)*2.0-1.0;
		noiseY = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233)*2.0)) * 43758.5453),0.0,1.0)*2.0-1.0;
	//}
	return noiseX; // vec2(noiseX,noiseY);
}

float noise(vec2 coord) //generating noise/pattern texture for dithering
{
	float noiseX = ((fract(1.0-coord.s*(width/2.0))*0.25)+(fract(coord.t*(height/2.0))*0.75))*2.0-1.0;
	float noiseY = ((fract(1.0-coord.s*(width/2.0))*0.75)+(fract(coord.t*(height/2.0))*0.25))*2.0-1.0;
	
	//if (noise)
	//{
		noiseX = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233))) * 43758.5453),0.0,1.0)*2.0-1.0;
		noiseY = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233)*2.0)) * 43758.5453),0.0,1.0)*2.0-1.0;
	//}
	return noiseX; // vec2(noiseX,noiseY);
}

vec3 lensflare(vec2 uv,vec2 pos)
{
	vec2 main = uv-pos;
	vec2 uvd = uv*(length(uv));
	
	float ang = atan(main.y, main.x);
	float dist=length(main); dist = pow(dist,.1);
	float n = noise(vec2((ang-iTime/9.0)*16.0,dist*32.0));
	
	float f0 = 1.0/(length(uv-pos)*16.0+1.0);
	
	f0 = f0+f0*(sin((ang+iTime/18.0 + noise(abs(ang)+n/2.0)*2.0)*12.0)*.1+dist*.1+.8);

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
	
	vec3 c = vec3(.0);
	
	c.r+=f2+f4+f5+f6; c.g+=f22+f42+f52+f62; c.b+=f23+f43+f53+f63;
	c+=vec3(f0);
	
	return c;
}

vec3 cc(vec3 color, float factor,float factor2) // color modifier
{
	float w = color.x+color.y+color.z;
	return mix(color,vec3(w)*factor,w*factor2);
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
	vec4 mask = vec4(0.0, 0.0, 0.0, 0.0);
	if (useMasking > 0.0)
	{
		mask = texture2D( maskSampler, tx );
	}
	
	vec2 fragCoord = gl_FragCoord.xy;
	vec2 iResolution = vec2(textureWidth, textureHeight);
	vec3 iMouse = vec3(light_pos.x * textureWidth, light_pos.y * textureHeight, light_pos.z);
	
	vec2 uv = fragCoord.xy / iResolution.xy - 0.5;
	uv.x *= iResolution.x/iResolution.y; //fix aspect ratio
	vec3 mouse = vec3(iMouse.xy/iResolution.xy - 0.5,iMouse.z-.5);
	float att = 1.0 - clamp(2.0 * (light_pos.z - 0.5), 0.0, 1.0);
	mouse.z = clamp(0.275 * log(100.0 * att), 0.0, 1.0);
	mouse.x *= iResolution.x/iResolution.y; //fix aspect ratio
	
	vec3 flareColor = lensflare(uv,mouse.xy); // vec3(1.4,1.2,1.0)
	
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
	
	color.rgb = mix(color.rgb + f * flareColor, color, mask.r * useMasking);
	gl_FragData [0] =  color;
}
