
//
// Fragment Shader - Depth Of Field
//
//	Post Processing Toolkit
//
//	Sergei <Neill3d> Solokhin 2018-2026
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

const float PI = 3.14159265;
const float namount = 0.0001; //dither amount

uniform	sampler2D	colorSampler;
uniform sampler2D	depthSampler;
//uniform sampler2D	blurSampler; // TODO: do a CoC pre-pass (half res)
uniform sampler2D	maskSampler;
uniform sampler2D	randomSampler;

uniform float	useMasking;
uniform float	upperClip;
uniform float	lowerClip;

uniform float		focalDistance, focalRange;
uniform vec2		gResolution; //viewport resolution
uniform vec2 texelSize;

uniform float 		zNear;
uniform float 		zFar;

uniform float fstop; // = 1.4; // f/1.4 full blur

//-- debug variables
uniform float debugBlurValue;

// Debug visualization of blur amount:
// warm tint = low blur / near focus,
// cool tint = transition region.
uniform float debugShowFocus;
//--

uniform vec4 focusPoint; // where x,y screen space focus point on screen and w is a flag to use it (0.0 - 1.0)

//------------------------------------------
//user variables

uniform int samples; // = 3; //samples on the first ring
uniform int rings; // = 3; //ring count

uniform float blurForeground;

uniform float CoC; // = 0.03;//circle of confusion size in mm (35mm film = 0.03mm)
uniform float blurRadius;

// > 0.0 - use the original thin lens blur factor,
// in that mode focalRange is expected to be the camera focus angle (assigned as is)
uniform float legacyFocusAngle;

// > 0.0 - use the lens model of the MotionBuilder camera DOF, lensBlurScale is a scale of a blur width in pixels:
//  edge blur width = lensBlurScale * |depth - focal distance| / (depth * focal distance)
// the scale is 0 for a zero focus angle (no blur), so it can not tell if the model is used
uniform float useLensBlur;
uniform float lensBlurScale;

uniform float threshold; // = 0.5; //highlight threshold;
uniform float gain; // = 2.0; //highlight gain;

uniform float bias; // = 0.5; //bokeh edge bias
uniform float fringe; // = 0.7; //bokeh chromatic aberration/fringing

uniform bool useNoise;

/*
next part is experimental
not looking good with small sample and ring count
looks okay starting from samples = 4, rings = 4
*/

uniform bool pentagon; // = false; //use pentagon as bokeh shape?
uniform float feather; // = 0.4; //pentagon shape feather


//------------------------------------------

float pentaClip(vec2 p, float radius, float feather)
{
    const vec2 n0 = vec2( 0.0000000,  1.0000000);
    const vec2 n1 = vec2( 0.9510565,  0.3090170);
    const vec2 n2 = vec2( 0.5877853, -0.8090170);
    const vec2 n3 = vec2(-0.5877853, -0.8090170);
    const vec2 n4 = vec2(-0.9510565,  0.3090170);

    float d0 = dot(p, n0);
    float d1 = dot(p, n1);
    float d2 = dot(p, n2);
    float d3 = dot(p, n3);
    float d4 = dot(p, n4);

    float d = max(max(d0, d1), max(max(d2, d3), d4));

    return 1.0 - smoothstep(radius - feather, radius + feather, d);
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

vec3 debugFocus(vec3 col, float blur, float depth)
{
	float edge = 0.002*depth; //distance based edge smoothing
	float m = clamp(smoothstep(0.0,edge,blur),0.0,1.0);
	float e = clamp(smoothstep(1.0-edge,1.0,blur),0.0,1.0);
	
	col = mix(col,vec3(1.0,0.5,0.0),(1.0-m)*0.6);
	col = mix(col,vec3(0.0,0.5,1.0),((1.0-e)-(1.0-m))*0.2);

	return col;
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

// Original (legacy) blur factor, thin lens model on depths normalized by the far plane.
// focalRange is used as a focal length (camera focus angle), result is in 0..1
float LegacyBlurFactor(float depth, float fDepth)
{
	float f = focalRange * 0.05; // focal length in mm
	float d = (fDepth / zFar) * 1000.0; // focal plane in mm
	float o = (depth / zFar) * 1000.0; // depth in mm

	// keep the denominators away from zero
	float od = o - f;
	float dd = d - f;
	od = (abs(od) < 1e-4) ? ((od < 0.0) ? -1e-4 : 1e-4) : od;
	dd = (abs(dd) < 1e-4) ? ((dd < 0.0) ? -1e-4 : 1e-4) : dd;

	float a = (o * f) / od;
	float b = (d * f) / dd;
	float c = dd / (max(abs(d), 1e-4) * max(fstop, 1e-4) * max(CoC, 1e-4));

	return clamp(10.0 * abs(a - b) * c, 0.0, 1.0);
}

// Lens model of the MotionBuilder camera DOF:
//   width = lensBlurScale * |depth - fDepth| / (depth * fDepth),   lensBlurScale ~ aperture * focus angle * focal length (pixels)
// A gather kernel of the rings reaches about a half of the width in pixels (width ~ 1.6 * reach),
// the reach of a kernel is rings * blurRadius * blur, so blurRadius is a maximum reach of a ring
float LensBlurFactor(float depth, float fDepth, float apertureScale)
{
	float width = lensBlurScale * abs(depth - fDepth) / max(depth * fDepth, 1e-4);
	float reach = 0.625 * width * apertureScale;
	return clamp(reach / max(float(rings) * blurRadius, 1e-4), 0.0, 1.0);
}

// a blur amount in 0..1 (without a shaping of a band model) of a depth, used to look at neighbours and samples
float RawBlurFactor(float depth, float fDepth, float focusBand, float apertureScale)
{
	if (legacyFocusAngle > 0.0)
	{
		return LegacyBlurFactor(depth, fDepth);
	}
	if (useLensBlur > 0.0)
	{
		return LensBlurFactor(depth, fDepth, apertureScale);
	}
	return max(abs(depth - fDepth) - focusBand, 0.0) / focusBand * apertureScale;
}

void main()
{
	float width = gResolution.x; //texture width
	float height = gResolution.y; //texture height
	float focalLength = focalRange; //focal length in mm
	bool useLegacy = (legacyFocusAngle > 0.0);
	bool useLens = (useLensBlur > 0.0) && !useLegacy;

	if (texCoord.y < upperClip || texCoord.y > lowerClip)
	{
		FragColor = texture(colorSampler, texCoord);
		return;
	}

	//scene depth calculation
	
	float depth = ComputeDepth(texCoord);
	
	if (depth < 0.001)
	{
		FragColor = texture(colorSampler, texCoord);
		return;
	}

	//focal plane calculation
	
	float fDepth = focalDistance;// / zFar;
	
	// auto focus based on a point on screen
	if (focusPoint.w > 0.0)
	{
		fDepth = ComputeDepth(focusPoint.xy);
	}
	
	//
	//dof blur factor calculation
	
	float blur = 0.0;
	float delta = depth - fDepth;

	float absDelta = abs(delta);

	// CoC scales the in-focus zone width.
	// 0.03mm is standard 35mm film — at that value focusBand is unchanged.
	// Larger CoC (looser tolerance) → wider in-focus zone → blur starts further out.
	// Smaller CoC (tighter tolerance) → narrower in-focus zone → blur starts sooner.

	// focalRange controls the width of the in-focus zone
	float cocFactor = max(CoC / 0.03, 1e-4);
	float focusBand = max(focalRange * cocFactor, 1e-4);

	float refAperture = 1.4;
	float apertureScale = refAperture / max(fstop, 1e-4);

	if (useLegacy)
	{
		// original thin lens model, focal range is a camera focus angle
		blur = LegacyBlurFactor(depth, fDepth);
	}
	else if (useLens)
	{
		// lens model of the MotionBuilder camera DOF
		blur = LensBlurFactor(depth, fDepth, apertureScale);
	}
	else
	{
		// outside this band blur starts increasing
		float rawBlur = max(absDelta - focusBand, 0.0) / focusBand;
		blur = smoothstep(0.0, 1.0, rawBlur);

		// optional: slightly gentler background blur growth
		if (delta > 0.0)
		{
			blur *= 0.85;
		}

		blur *= apertureScale;
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
	
	// getting blur x and y step factor
	
	// after computing blur for the center pixel, before the blur > 0.05 check:
	float maxNeighborBlur = blur;
	float probeRadius = 3.0; //max(blurRadius * 0.25, 3.0);
	vec2 probeOffsets[4] = vec2[4](
	    vec2(texelSize.x * probeRadius, 0.0),
		vec2(-texelSize.x * probeRadius, 0.0),
		vec2(0.0, texelSize.y * probeRadius),
		vec2(0.0, -texelSize.y * probeRadius)
	);
	for (int k = 0; k < 4; ++k)
	{
	    float nd = ComputeDepth(texCoord + probeOffsets[k]);
	    float nb = RawBlurFactor(nd, fDepth, focusBand, apertureScale);
	    maxNeighborBlur = max(maxNeighborBlur, clamp(nb, 0.0, 1.0));
	}

	float spreadBlur = blur + (maxNeighborBlur - blur) * 0.15;
	vec2 blurStep = texelSize * spreadBlur * blurRadius;

	// calculation of pattern for ditering
	if (useNoise)
	{
		vec2 p = gl_FragCoord.xy;
	    vec2 noiseVec;
	    noiseVec.x = fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453) * 2.0 - 1.0;
	    noiseVec.y = fract(sin(dot(p, vec2(269.5, 183.3))) * 43758.5453) * 2.0 - 1.0;
	    blurStep += noiseVec * blurStep * 0.25;
	}
	
	// calculation of final color
	
	vec3 inputColor = texture(colorSampler, texCoord).rgb;
	vec3 col = inputColor;
	
	if(maxNeighborBlur > 0.01)
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
				vec2 ringCoord = vec2(x, y) * fi;          // ring-space
				vec2 shapeCoord = ringCoord / float(rings);
				vec2 pOffset = ringCoord * blurStep;        // UV/sample offset
				float aperture = 1.0;

			    if (pentagon)
			    {
			        aperture = pentaClip(shapeCoord, 1.0, feather);
			        
			        // reject only taps clearly outside
			        if (aperture < 0.01)
			        {
			            float nx = x * cs - y * sn;
			            float ny = x * sn + y * cs;
			            x = nx;
			            y = ny;
			            continue;
			        }

			        // optional: strengthen edge shaping
			        aperture = pow(aperture, 3.0);
			    }

			    vec2 sampleCoord = texCoord + pOffset;

			    float sampleDepth = ComputeDepth(sampleCoord);
			    float sampleBlur = RawBlurFactor(sampleDepth, fDepth, focusBand, apertureScale);
			    
			    // only suppress when sample is sharper than center (background leaking into blur)
				// allow when sample is more blurred than center (foreground trying to spread outward)
				float blurWeight = (sampleBlur < blur)
				    ? clamp(sampleBlur / max(blur, 0.01), 0.0, 1.0)
				    : 1.0;

			    float ringBias = mix(1.0, fi / float(rings), bias);
			    float sampleBias = ringBias * aperture * blurWeight;

			    vec3 sampleCol = sampleDOFColor(sampleCoord, blur) * sampleBias;
			    col += sampleCol;
			    s += sampleBias;

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