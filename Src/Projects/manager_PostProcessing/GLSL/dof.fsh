
//
// Fragment Shader - Depth Of Field
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
// based on the original work of Martin Upitis (martinsh) (devlog-martinsh.blogspot.com)

#version 140

in vec2 texCoord;
out vec4 FragColor;

uniform	sampler2D	colorSampler;
uniform sampler2D	depthSampler;
uniform sampler2D	blurSampler;
uniform sampler2D	maskSampler;

uniform float	useMasking;
uniform float	upperClip;
uniform float	lowerClip;

uniform float		focalDistance, focalRange;
uniform vec2		gResolution; //viewport resolution
uniform vec2 texelSize;

uniform float 		zNear;
uniform float 		zFar;

const float PI = 3.14159265;

uniform float fstop; // = 0.5; //f-stop value

//-- debug variables
uniform float debugBlurValue;
uniform float debugShowFocus; //show debug focus point and focal range (red = focal point, green = focal range)
//--

uniform vec4 focusPoint;

//------------------------------------------
//user variables

uniform int samples; // = 3; //samples on the first ring
uniform int rings; // = 3; //ring count

uniform float blurForeground;

uniform bool manualdof; // = false; //manual dof calculation
uniform float ndofstart; // = 1.0; //near dof blur start
uniform float ndofdist; // = 2.0; //near dof blur falloff distance
uniform float fdofstart; // = 1.0; //far dof blur start
uniform float fdofdist; // = 3.0; //far dof blur falloff distance

uniform float CoC; // = 0.03;//circle of confusion size in mm (35mm film = 0.03mm)

uniform bool autofocus; // = false; //use autofocus in shader? disable if you use external focalDepth value
uniform vec2 focus; // = vec2(0.5,0.5); // autofocus point on screen (0.0,0.0 - left lower corner, 1.0,1.0 - upper right)
float maxblur = 1.0; //clamp value of max blur (0.0 = no blur,1.0 default)

uniform float threshold; // = 0.5; //highlight threshold;
uniform float gain; // = 2.0; //highlight gain;

uniform float bias; // = 0.5; //bokeh edge bias
uniform float fringe; // = 0.7; //bokeh chromatic aberration/fringing

uniform bool noise; // = true; //use noise instead of pattern for sample dithering
float namount = 0.0001; //dither amount

bool depthblur = false; //blur the depth buffer?
float dbsize = 1.25; //depthblursize

/*
next part is experimental
not looking good with small sample and ring count
looks okay starting from samples = 4, rings = 4
*/

uniform bool pentagon; // = false; //use pentagon as bokeh shape?
uniform float feather; // = 0.4; //pentagon shape feather


//------------------------------------------


float penta(vec2 coords) //pentagonal shape
{
	float scale = float(rings) - 1.3;
	vec4  HS0 = vec4( 1.0,         0.0,         0.0,  1.0);
	vec4  HS1 = vec4( 0.309016994, 0.951056516, 0.0,  1.0);
	vec4  HS2 = vec4(-0.809016994, 0.587785252, 0.0,  1.0);
	vec4  HS3 = vec4(-0.809016994,-0.587785252, 0.0,  1.0);
	vec4  HS4 = vec4( 0.309016994,-0.951056516, 0.0,  1.0);
	vec4  HS5 = vec4( 0.0        ,0.0         , 1.0,  1.0);
	
	vec4  one = vec4( 1.0 );
	
	vec4 P = vec4((coords),vec2(scale, scale)); 
	
	vec4 dist = vec4(0.0);
	float inorout = -4.0;
	
	dist.x = dot( P, HS0 );
	dist.y = dot( P, HS1 );
	dist.z = dot( P, HS2 );
	dist.w = dot( P, HS3 );
	
	dist = smoothstep( -feather, feather, dist );
	
	inorout += dot( dist, one );
	
	dist.x = dot( P, HS4 );
	dist.y = HS5.w - abs( P.z );
	
	dist = smoothstep( -feather, feather, dist );
	inorout += dist.x;
	
	return clamp( inorout, 0.0, 1.0 );
}

float bdepth(vec2 coords) //blurring depth
{
	float d = 0.0;
	float kernel[9];
	vec2 offset[9];
	
	vec2 wh = vec2(texelSize.x, texelSize.y) * dbsize;
	
	offset[0] = vec2(-wh.x,-wh.y);
	offset[1] = vec2( 0.0, -wh.y);
	offset[2] = vec2( wh.x, -wh.y);
	
	offset[3] = vec2(-wh.x,  0.0);
	offset[4] = vec2( 0.0,   0.0);
	offset[5] = vec2( wh.x,  0.0);
	
	offset[6] = vec2(-wh.x, wh.y);
	offset[7] = vec2( 0.0,  wh.y);
	offset[8] = vec2( wh.x, wh.y);
	
	kernel[0] = 1.0/16.0;   kernel[1] = 2.0/16.0;   kernel[2] = 1.0/16.0;
	kernel[3] = 2.0/16.0;   kernel[4] = 4.0/16.0;   kernel[5] = 2.0/16.0;
	kernel[6] = 1.0/16.0;   kernel[7] = 2.0/16.0;   kernel[8] = 1.0/16.0;
	
	
	for( int i=0; i<9; i++ )
	{
		float tmp = texture(depthSampler, coords + offset[i]).r;
		d += tmp * kernel[i];
	}
	
	return d;
}

const vec2 kFringeDirR = vec2(0.0, 1.0);
const vec2 kFringeDirG = vec2(-0.8660254, -0.5);
const vec2 kFringeDirB = vec2(0.8660254, -0.5);
const vec3 kLumCoeff = vec3(0.2126, 0.7152, 0.0722);
const float kHighlightKnee = 0.2;

vec3 sampleDOFColor(vec2 coords, float blur) //processing the sample
{
	vec3 col = vec3(0.0);
	
	vec2 offset = texelSize * fringe * blur;
	col.r = texture(colorSampler, coords + offset * kFringeDirR).r;
	col.g = texture(colorSampler, coords + offset * kFringeDirG).g;
	col.b = texture(colorSampler, coords + offset * kFringeDirB).b;
	
	float lum = dot(col, kLumCoeff);
	
	float highlight = smoothstep(threshold, threshold + kHighlightKnee, lum);
    float boost = 1.0 + 0.15 * gain * highlight * blur;

    return col * boost;
}

vec2 rand(vec2 coord, float width, float height) //generating noise/pattern texture for dithering
{
	float noiseX = ((fract(1.0-coord.s*(width/2.0))*0.25)+(fract(coord.t*(height/2.0))*0.75))*2.0-1.0;
	float noiseY = ((fract(1.0-coord.s*(width/2.0))*0.75)+(fract(coord.t*(height/2.0))*0.25))*2.0-1.0;
	
	if (noise)
	{
		noiseX = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233))) * 43758.5453),0.0,1.0)*2.0-1.0;
		noiseY = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233)*2.0)) * 43758.5453),0.0,1.0)*2.0-1.0;
	}
	return vec2(noiseX,noiseY);
}

vec3 debugFocus(vec3 col, float blur, float depth)
{
	float edge = 0.002*depth; //distance based edge smoothing
	float m = clamp(smoothstep(0.0,edge,blur),0.0,1.0);
	float e = clamp(smoothstep(1.0-edge,1.0,blur),0.0,1.0);
	
	col = mix(col,vec3(1.0,0.5,0.0),(1.0-m)*0.6);
	col = mix(col,vec3(0.0,0.5,1.0),((1.0-e)-(1.0-m))*0.2);

	return col;
}

float linearize(float depth)
{
	return -zFar * zNear / (depth * (zFar - zNear) - zFar);
}

float LinearizeDepth(float depthSample)
{
    float z = depthSample * 2.0 - 1.0;
    return (2.0 * zNear * zFar) / (zNear + zFar - z * (zFar - zNear));
}

float ComputeDepth(in vec2 texCoord)
{
	float d = texture(depthSampler, texCoord).x;
	return LinearizeDepth(d);
}

void main() 
{
	float width = gResolution.x; //texture width
	float height = gResolution.y; //texture height
	float focalLength = focalRange; //focal length in mm

	if (texCoord.y < upperClip || texCoord.y > lowerClip)
	{
		FragColor = texture(colorSampler, texCoord);
		return;
	}

	//scene depth calculation
	
	float depth = ComputeDepth(texCoord);
	
	//focal plane calculation
	
	float fDepth = focalDistance;// / zFar;
	
	if (focusPoint.w > 0.0)
	{
		fDepth = ComputeDepth(focusPoint.xy);
	}
	
	/*
	if (autofocus)
	{
		fDepth = linearize(texture(depthSampler,focus).x);
	}
	*/
	
	//
	//dof blur factor calculation
	
	float blur = 0.0;
	float delta = depth - fDepth;

	if (manualdof)
	{
		float nearBlur = max((-delta) - ndofstart, 0.0) / max(ndofdist, 1e-5);
		float farBlur  = max(( delta) - fdofstart, 0.0) / max(fdofdist, 1e-5);
		blur = (delta < 0.0) ? nearBlur : farBlur;
	}
	else
	{
		float absDelta = abs(delta);

		// focalRange controls the width of the in-focus zone
		float focusBand = max(focalRange, 1e-4);

		// outside this band blur starts increasing
		blur = max(absDelta - focusBand, 0.0) / focusBand;

		// optional: slightly gentler background blur growth
		if (delta > 0.0)
		{
			blur *= 0.85;
		}

		float apertureScale = 1.0 / max(fstop, 1e-4);
		float cocScale = max(CoC / 0.03, 0.01);

		blur *= apertureScale * cocScale;
		blur = pow(clamp(blur, 0.0, 1.0), 1.35);
	}

	if (blurForeground == 0.0 && delta < 0.0)
	{
		blur = 0.0;
	}

	blur = clamp(blur, 0.0, 1.0);
	
	if (debugBlurValue > 0.0)
	{
		vec4 outColor2 = vec4(blur, blur, blur, 1.0);
		FragColor = outColor2;
		return;
	}
	
	// calculation of pattern for ditering
	
	vec2 noise = rand(texCoord.xy, width, height) * namount * blur;
	
	// getting blur x and y step factor
	
	vec2 blurStep = vec2(1.0/width, 1.0/height) * blur * maxblur + noise;

	// calculation of final color
	
	vec3 inputColor = texture(colorSampler, texCoord).rgb;
	vec3 col = inputColor;
	
	if(blur > 0.05) //some optimization thingy
	{
		float s = 1.0;
		int ringsamples;
		
		for (int i = 1; i <= rings; i += 1)
		{   
			float fi = float(i);
			ringsamples = i * samples;
			
			float step = PI * 2.0 / float(ringsamples);
			float cs = cos(step);
			float sn = sin(step);

			float x = 1.0;
			float y = 0.0;

			for (int j = 0 ; j < ringsamples ; j += 1)   
			{
				vec2 pOffset = vec2(x * fi, y * fi) * blurStep;
				float p = 1.0;
				if (pentagon)
				{ 
					p = penta(pOffset);
				}
				float sampleBias = mix(1.0, fi / float(rings), bias) * p;

				vec3 sampleCol = sampleDOFColor(texCoord + pOffset, blur) * sampleBias;
				col += sampleCol;
				s += sampleBias;   

				// rotate (x, y)
				float nx = x * cs - y * sn;
				float ny = x * sn + y * cs;
				x = nx;
				y = ny;
			}
		}
		col /= s; //divide by sample count
	}
	
	if (debugShowFocus > 0.0)
	{
		col = debugFocus(col, blur, depth);
	}
	
	vec4 mask = vec4(0.0);
	if (useMasking > 0.0)
	{
		mask = texture(maskSampler, texCoord);
	}

	FragColor.rgb = mix(col, inputColor, mask.r * useMasking);
	FragColor.a = 1.0;
}