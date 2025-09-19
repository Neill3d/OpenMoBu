
// shadertoy - https://www.shadertoy.com/view/WtsfDH

// FXAA by Dave Hoskins, FXAA algorithm from NVIDIA
// http://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf
// https://catlikecoding.com/unity/tutorials/advanced-rendering/fxaa/

#version 140

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D iChannel0;
uniform vec2 iResolution;
uniform float iTime;

#define EDGE_STEP_COUNT 10
#define EDGE_GUESS 8.0f
#define EDGE_STEPS 1.0f, 1.5f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 4.0f
const float edgeSteps[EDGE_STEP_COUNT] = float[EDGE_STEP_COUNT]( EDGE_STEPS );

float _ContrastThreshold = 0.0312f;
float _RelativeThreshold = 0.063f;
float _SubpixelBlending = 1.0f;

vec4 Sample (sampler2D  tex2D, vec2 uv) {
	return texture(tex2D, uv);
}

float SampleLuminance (sampler2D tex2D, vec2 uv) {			
	return dot(Sample(tex2D, uv).rgb, vec3(0.3f, 0.59f, 0.11f));
}

float SampleLuminance (sampler2D tex2D, vec2 texSize, vec2 uv, float uOffset, float vOffset) {
	uv += texSize * vec2(uOffset, vOffset);
	return SampleLuminance(tex2D, uv);
}

struct LuminanceData {
	float m, n, e, s, w;
	float ne, nw, se, sw;
	float highest, lowest, contrast;
};

LuminanceData SampleLuminanceNeighborhood (sampler2D tex2D, vec2 texSize, vec2 uv) {
	LuminanceData l;
	l.m = SampleLuminance(tex2D, uv);
	l.n = SampleLuminance(tex2D, texSize, uv,  0.0f,  1.0f);
	l.e = SampleLuminance(tex2D, texSize, uv,  1.0f,  0.0f);
	l.s = SampleLuminance(tex2D, texSize, uv,  0.0f, -1.0f);
	l.w = SampleLuminance(tex2D, texSize, uv, -1.0f,  0.0f);

	l.ne = SampleLuminance(tex2D, texSize, uv,  1.0f,  1.0f);
	l.nw = SampleLuminance(tex2D, texSize, uv, -1.0f,  1.0f);
	l.se = SampleLuminance(tex2D, texSize, uv,  1.0f, -1.0f);
	l.sw = SampleLuminance(tex2D, texSize, uv, -1.0f, -1.0f);

	l.highest = max(max(max(max(l.n, l.e), l.s), l.w), l.m);
	l.lowest = min(min(min(min(l.n, l.e), l.s), l.w), l.m);
	l.contrast = l.highest - l.lowest;
	return l;
}

bool ShouldSkipPixel (LuminanceData l) {
	float threshold =
		max(_ContrastThreshold, _RelativeThreshold * l.highest);
	return l.contrast < threshold;
}

float DeterminePixelBlendFactor (LuminanceData l) {
	float f = 2.0f * (l.n + l.e + l.s + l.w);
	f += l.ne + l.nw + l.se + l.sw;
	f *= 1.0f / 12.0f;
	f = abs(f - l.m);
	f = clamp(f / l.contrast, 0.0, 1.0);

	float blendFactor = smoothstep(0.0f, 1.0f, f);
	return blendFactor * blendFactor * _SubpixelBlending;
}

struct EdgeData {
	bool isHorizontal;
	float pixelStep;
	float oppositeLuminance, gradient;
};

EdgeData DetermineEdge (vec2 texSize, LuminanceData l) {
	EdgeData e;
	float horizontal =
		abs(l.n + l.s - 2.0f * l.m) * 2.0f +
		abs(l.ne + l.se - 2.0f * l.e) +
		abs(l.nw + l.sw - 2.0f * l.w);
	float vertical =
		abs(l.e + l.w - 2.0f * l.m) * 2.0f +
		abs(l.ne + l.nw - 2.0f * l.n) +
		abs(l.se + l.sw - 2.0f * l.s);
	e.isHorizontal = horizontal >= vertical;

	float pLuminance = e.isHorizontal ? l.n : l.e;
	float nLuminance = e.isHorizontal ? l.s : l.w;
	float pGradient = abs(pLuminance - l.m);
	float nGradient = abs(nLuminance - l.m);

	e.pixelStep =
		e.isHorizontal ? texSize.y : texSize.x;
	
	if (pGradient < nGradient) {
		e.pixelStep = -e.pixelStep;
		e.oppositeLuminance = nLuminance;
		e.gradient = nGradient;
	}
	else {
		e.oppositeLuminance = pLuminance;
		e.gradient = pGradient;
	}

	return e;
}

float DetermineEdgeBlendFactor (sampler2D  tex2D, vec2 texSize, LuminanceData l, EdgeData e, vec2 uv) {
	vec2 uvEdge = uv;
	vec2 edgeStep;
	if (e.isHorizontal) {
		uvEdge.y += e.pixelStep * 0.5f;
		edgeStep = vec2(texSize.x, 0.0f);
	}
	else {
		uvEdge.x += e.pixelStep * 0.5f;
		edgeStep = vec2(0.0f, texSize.y);
	}

	float edgeLuminance = (l.m + e.oppositeLuminance) * 0.5f;
	float gradientThreshold = e.gradient * 0.25f;

	vec2 puv = uvEdge + edgeStep * edgeSteps[0];
	float pLuminanceDelta = SampleLuminance(tex2D, puv) - edgeLuminance;
	bool pAtEnd = abs(pLuminanceDelta) >= gradientThreshold;

	for (int i = 1; i < EDGE_STEP_COUNT && !pAtEnd; i++) {
		puv += edgeStep * edgeSteps[i];
		pLuminanceDelta = SampleLuminance(tex2D, puv) - edgeLuminance;
		pAtEnd = abs(pLuminanceDelta) >= gradientThreshold;
	}
	if (!pAtEnd) {
		puv += edgeStep * EDGE_GUESS;
	}

	vec2 nuv = uvEdge - edgeStep * edgeSteps[0];
	float nLuminanceDelta = SampleLuminance(tex2D, nuv) - edgeLuminance;
	bool nAtEnd = abs(nLuminanceDelta) >= gradientThreshold;

	for (int i = 1; i < EDGE_STEP_COUNT && !nAtEnd; i++) {
		nuv -= edgeStep * edgeSteps[i];
		nLuminanceDelta = SampleLuminance(tex2D, nuv) - edgeLuminance;
		nAtEnd = abs(nLuminanceDelta) >= gradientThreshold;
	}
	if (!nAtEnd) {
		nuv -= edgeStep * EDGE_GUESS;
	}

	float pDistance, nDistance;
	if (e.isHorizontal) {
		pDistance = puv.x - uv.x;
		nDistance = uv.x - nuv.x;
	}
	else {
		pDistance = puv.y - uv.y;
		nDistance = uv.y - nuv.y;
	}

	float shortestDistance;
	bool deltaSign;
	if (pDistance <= nDistance) {
		shortestDistance = pDistance;
		deltaSign = pLuminanceDelta >= 0.0f;
	}
	else {
		shortestDistance = nDistance;
		deltaSign = nLuminanceDelta >= 0.0f;
	}

	if (deltaSign == (l.m - edgeLuminance >= 0.0f)) {
		return 0.0f;
	}
	return 0.5f - shortestDistance / (pDistance + nDistance);
}

vec4 ApplyFXAA (sampler2D  tex2D, vec2 texSize, vec2 uv) {
	LuminanceData l = SampleLuminanceNeighborhood(tex2D, texSize, uv);
	if (ShouldSkipPixel(l)) {
		return Sample(tex2D, uv);
	}

	float pixelBlend = DeterminePixelBlendFactor(l);
	EdgeData e = DetermineEdge(texSize, l);
	float edgeBlend = DetermineEdgeBlendFactor(tex2D, texSize, l, e, uv);
	float finalBlend = max(pixelBlend, edgeBlend);

	if (e.isHorizontal) {
		uv.y += e.pixelStep * finalBlend;
	}
	else {
		uv.x += e.pixelStep * finalBlend;
	}
	return vec4(Sample(tex2D, uv).rgb, l.m);
}

//void mainImage( out vec4 fragColor, in vec2 fragCoord )
void main(void)
{
    vec2 uv = TexCoord;
    vec2 texSize = vec2(1.0) / iResolution.xy;
    float t = 0.5f + sin(iTime) * 0.5;
    if(uv.x < t) {
    	FragColor = ApplyFXAA(iChannel0,texSize, uv);
    } else {
        FragColor = texture(iChannel0, uv);
    }
    FragColor += step(abs(uv.x - t), texSize.x);
}