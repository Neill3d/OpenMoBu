
//
// Fragment Shader - Color Correction
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

uniform	sampler2D	sampler0;
uniform sampler2D	maskSampler;

uniform float	useMasking;
uniform float	upperClip;
uniform float	lowerClip;

uniform vec2	gResolution;
uniform vec2 uInvResolution; // = 1.0 / resolution

// contrast, saturation, brightness, gamma
uniform vec4			gCSB;	

// hue, saturation, lightness, inverse state
uniform vec4			gHue;

// chromatic aberration
uniform vec4			gCA; // x-dir x; y-dir y; w-use chromatic aberration

/*
** Copyright (c) 2012, Romain Dura romain@shazbits.com
** 
** Permission to use, copy, modify, and/or distribute this software for any 
** purpose with or without fee is hereby granted, provided that the above 
** copyright notice and this permission notice appear in all copies.
** 
** THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES 
** WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF 
** MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY 
** SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
** WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN 
** ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR 
** IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/*
** Photoshop & misc math
** Blending modes, RGB/HSL/Contrast/Desaturate, levels control
**
** Romain Dura | Romz
** Blog: http://mouaif.wordpress.com
** Post: http://mouaif.wordpress.com/?p=94
*/

/*
** Contrast, saturation, brightness
** Code of this function is from TGM's shader pack
** http://irrlicht.sourceforge.net/phpBB2/viewtopic.php?t=21057
*/

// For all settings: 1.0 = 100% 0.5=50% 1.5 = 150%
vec3 ContrastSaturationBrightness(vec3 color, float brt, float sat, float con)
{
	// Increase or decrease theese values to adjust r, g and b color channels seperately
	const float AvgLumR = 0.5;
	const float AvgLumG = 0.5;
	const float AvgLumB = 0.5;
	
	const vec3 LumCoeff = vec3(0.2125, 0.7154, 0.0721);
	
	vec3 AvgLumin = vec3(AvgLumR, AvgLumG, AvgLumB);
	vec3 brtColor = color * brt;
	vec3 intensity = vec3(dot(brtColor, LumCoeff));
	vec3 satColor = mix(intensity, brtColor, sat);
	vec3 conColor = mix(AvgLumin, satColor, con);
	return conColor;
}

/*
** Gamma correction
** Details: http://blog.mouaif.org/2009/01/22/photoshop-gamma-correction-shader/
*/

#define GammaCorrection(color, gamma)								pow(color, 1.0 / gamma)

/*
** Levels control (input (+gamma), output)
** Details: http://blog.mouaif.org/2009/01/28/levels-control-shader/
*/

#define LevelsControlInputRange(color, minInput, maxInput)				min(max(color - vec3(minInput), vec3(0.0)) / (vec3(maxInput) - vec3(minInput)), vec3(1.0))
#define LevelsControlInput(color, minInput, gamma, maxInput)				GammaCorrection(LevelsControlInputRange(color, minInput, maxInput), gamma)
#define LevelsControlOutputRange(color, minOutput, maxOutput) 			mix(vec3(minOutput), vec3(maxOutput), color)
#define LevelsControl(color, minInput, gamma, maxInput, minOutput, maxOutput) 	LevelsControlOutputRange(LevelsControlInput(color, minInput, gamma, maxInput), minOutput, maxOutput)

// source from https://github.com/DOWNPOURDIGITAL/glsl-chromatic-aberration/blob/master/ca.glsl
vec4 ca( in sampler2D image, in vec2 uv, in vec2 resolution, in vec2 direction ) 
{
	// 4/3 scale factor, pre-multiplied by invResolution once
    vec2 off = (4.0/3.0) * direction * uInvResolution;

    vec2 uv1 = uv - off;
    vec2 uv2 = uv - off - off;

    // If you already have src.a from an earlier read, pass it in and skip .a here.
    vec4 c0 = texture(image, uv);   // center
    float g = texture(image, uv1).g;
    float b = texture(image, uv2).b;

    return vec4(c0.r, g, b, c0.a);
}

///////////////////////////////////////////////////////////////////////////////
// main

void main (void)
{
	vec2 tx  = texCoord;
	
	if (tx.y < upperClip || tx.y > lowerClip)
	{
		FragColor = texture2D(sampler0, tx);
		return;
	}
	
	vec4 srccolor;
	if (gCA.w > 0.0)
	{
		srccolor = ca(sampler0, tx, gResolution, gCA.st);
	}
	else
	{
		srccolor = texture(sampler0, tx);
	}
	
	vec3 color = ContrastSaturationBrightness( srccolor.xyz, gCSB.z, gCSB.y, gCSB.x );
	
	// Gamma once
	float invGamma = 1.0 / max(gCSB.w, 1e-6);
	color = pow(max(color, 0.0), vec3(invGamma));
	
	// Hue / saturation / lightness via YIQ rotate+scale
    float hue = gHue.x * 6.28318530718;
    float s = sin(hue), c = cos(hue);
    mat3 RGB2YIQ = mat3(0.299,0.587,0.114, 0.596,-0.274,-0.322, 0.211,-0.523,0.312);
    mat3 YIQ2RGB = mat3(1.0,0.956,0.621, 1.0,-0.272,-0.647, 1.0,-1.106,1.703);
    vec3 yiq = RGB2YIQ * color;
    vec2 IQ  = mat2(c,-s,s,c) * (yiq.yz * (1.0 + gHue.y));
    yiq.x   *= (1.0 + gHue.z);
    color    = clamp(YIQ2RGB * vec3(yiq.x, IQ), 0.0, 1.0);

    // Optional invert
    color = mix(color, vec3(1.0) - color, clamp(gHue.w, 0.0, 1.0));
	
	// Mask (one read only when used)
    if (useMasking > 0.0) {
        float m = texture(maskSampler, tx).r * useMasking;
        vec3 inputColor = texture(sampler0, tx).rgb; // If CA path used earlier, pass that value down instead of re-reading.
        color = mix(color, inputColor, m);
    }

    FragColor = vec4(color, 1.0);
}
