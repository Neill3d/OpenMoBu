
// dissolve effect for a texture with alpha, a test shader for the Texture User Effect
//
// connect a character texture as the input layer, the result is the same texture with an alpha
//  that dissolves by a noise pattern and a glowing burn edge along the dissolve border.
//  Without an input layer the result is the dissolve mask (alpha) over the texture background color.
// Use the result texture in a material with transparency (for example Transparency set to Accurate,
//  the alpha of the texture is the transparency).
//
// user properties (animatable, a new property starts at 0):
//  amount     - 0 is fully solid, 100 is fully dissolved, key it to animate the dissolve
//  edgeWidth  - width of the burning edge in percents of the noise range, 0 is the default 6
//  edge       - edge glow color, default orange (black means default)
//  edgeGlow   - edge brightness in percents, 0 is the default 300
//  softness   - softness of the alpha border in percents, 0 is a hard (clip like) border
//  scale      - noise cells on the texture (a whole number, the pattern tiles), 0 is the default 6
//  seed       - changes the noise pattern
//  invert     - checkbox, dissolve from the opposite noise values

#version 140

in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D inputSampler;   // system: the input image (texture layer) or a blank background color

uniform float amount_slider;
uniform float edgeWidth_slider;
uniform vec3  edge_color;
uniform float edgeGlow_slider;
uniform float softness_slider;
uniform float scale_slider;
uniform float seed_slider;
uniform float invert_flag;

// --- hash and periodic value noise (the pattern repeats over 0..1 texture coordinates) ---

float Hash12(vec2 p)
{
	vec3 p3 = fract(vec3(p.xyx) * 0.1031);
	p3 += dot(p3, p3.yzx + 33.33);
	return fract((p3.x + p3.y) * p3.z);
}

float ValueNoise(vec2 p, vec2 period, float seed)
{
	vec2 i = floor(p);
	vec2 f = fract(p);
	vec2 u = f * f * (3.0 - 2.0 * f);

	vec2 i0 = mod(i, period);
	vec2 i1 = mod(i + 1.0, period);
	vec2 s = vec2(seed * 17.13, seed * 31.71);

	float a = Hash12(i0 + s);
	float b = Hash12(vec2(i1.x, i0.y) + s);
	float c = Hash12(vec2(i0.x, i1.y) + s);
	float d = Hash12(i1 + s);
	return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

float Fbm(vec2 p, vec2 period, float seed)
{
	float value = 0.0;
	float amplitude = 0.5;
	float total = 0.0;
	for (int i = 0; i < 5; ++i)
	{
		value += amplitude * ValueNoise(p, period, seed);
		total += amplitude;
		p *= 2.0;
		period *= 2.0;
		amplitude *= 0.5;
	}
	return value / total;
}

float OrDefault(float value, float defaultValue)
{
	return (value > 0.0) ? value : defaultValue;
}

vec3 OrDefault(vec3 value, vec3 defaultValue)
{
	return (dot(value, value) > 0.0) ? value : defaultValue;
}

void main(void)
{
	vec4 inputColor = texture(inputSampler, texCoord);

	float amount = clamp(0.01 * amount_slider, 0.0, 1.0);
	float edgeWidth = 0.01 * OrDefault(edgeWidth_slider, 6.0);
	vec3 edgeColor = OrDefault(edge_color, vec3(1.0, 0.45, 0.08));
	float edgeGlow = 0.01 * OrDefault(edgeGlow_slider, 300.0);
	float softness = 0.01 * softness_slider;
	float cells = floor(OrDefault(scale_slider, 6.0));

	// dissolve noise, stretched to use the whole [0; 1] range
	float n = Fbm(texCoord * cells, vec2(cells), floor(seed_slider));
	n = clamp((n - 0.5) * 2.2 + 0.5, 0.0, 1.0);
	if (invert_flag > 0.5)
	{
		n = 1.0 - n;
	}

	// the threshold goes a bit beyond [0; 1], so 0 is fully solid and 1 is fully dissolved including the edge
	// NOTE: a small margin keeps the noise extremes (0 and 1) out of the border at both ends of the slider
	const float MARGIN = 0.002;
	float threshold = mix(-edgeWidth - softness - MARGIN, 1.0 + softness + MARGIN, amount);

	// alpha of the dissolve, 1 is solid
	float halfSoft = max(0.5 * softness, 0.0005);
	float dissolveAlpha = smoothstep(threshold - halfSoft, threshold + halfSoft, n);

	// burning edge on the solid side of the border
	float edgeMask = 1.0 - smoothstep(threshold, threshold + edgeWidth, n);
	edgeMask *= dissolveAlpha;
	edgeMask *= step(0.0001, amount); // no edge when fully solid

	vec3 color = mix(inputColor.rgb, edgeColor * edgeGlow, edgeMask);

	FragColor = vec4(color, inputColor.a * dissolveAlpha);
}
