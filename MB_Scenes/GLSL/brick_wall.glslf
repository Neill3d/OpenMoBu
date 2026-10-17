
// procedural brick wall, a test shader for the Texture User Effect (works in the effects chain as well)
//
// the result is a seamless tile: texture coordinates 0..1 hold a whole number of bricks,
//  an even number of rows (the running bond offset matches at the top and bottom edges)
//  and all the noise is periodic, so the texture can be repeated without visible seams
//
// user properties (a new property starts at 0, so 0 means "use the default"):
//  brick        - brick color, default red-brown
//  mortar       - mortar color, default light grey
//  rows         - number of brick rows on the tile, rounded up to an even number, default 12
//  mortarWidth  - mortar thickness in percents of a brick height, default 12
//  variation    - per brick color variation in percents, default 35
//  grain        - surface noise in percents, default 40
//  bevel        - shade brick edges (checkbox)
//  mixWithInput - percents of the input image (texture layer) to blend in, 0 is bricks only

#version 140

in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D inputSampler;   // system: the input image (the texture layer or a blank background color)
uniform vec2      gResolution;    // system: render target size in pixels

uniform vec3  brick_color;
uniform vec3  mortar_color;
uniform float rows_slider;
uniform float mortarWidth_slider;
uniform float variation_slider;
uniform float grain_slider;
uniform float bevel_flag;
uniform float mixWithInput_slider;

// --- hash and periodic value noise ---

float Hash12(vec2 p)
{
	vec3 p3 = fract(vec3(p.xyx) * 0.1031);
	p3 += dot(p3, p3.yzx + 33.33);
	return fract((p3.x + p3.y) * p3.z);
}

// value noise that repeats every 'period' units (period has to be a whole number)
float ValueNoise(vec2 p, vec2 period)
{
	vec2 i = floor(p);
	vec2 f = fract(p);
	vec2 u = f * f * (3.0 - 2.0 * f);

	vec2 i0 = mod(i, period);
	vec2 i1 = mod(i + 1.0, period);

	float a = Hash12(i0);
	float b = Hash12(vec2(i1.x, i0.y));
	float c = Hash12(vec2(i0.x, i1.y));
	float d = Hash12(i1);
	return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

// fractal noise with the same period, every octave doubles the frequency and the period
float Fbm(vec2 p, vec2 period)
{
	float value = 0.0;
	float amplitude = 0.5;
	for (int i = 0; i < 4; ++i)
	{
		value += amplitude * ValueNoise(p, period);
		p *= 2.0;
		period *= 2.0;
		amplitude *= 0.5;
	}
	return value;
}

// a property value or its default when the property is still 0
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
	const float PREFERRED_BRICK_ASPECT = 2.0; // brick width / height

	float mortar = 0.01 * OrDefault(mortarWidth_slider, 12.0);
	float variation = 0.01 * OrDefault(variation_slider, 35.0);
	float grain = 0.01 * OrDefault(grain_slider, 40.0);
	vec3 brickBase = OrDefault(brick_color, vec3(0.56, 0.24, 0.17));
	vec3 mortarBase = OrDefault(mortar_color, vec3(0.72, 0.70, 0.66));

	// tile layout, a whole number of bricks: an even number of rows and an integer number of columns
	float rows = 2.0 * ceil(0.5 * max(OrDefault(rows_slider, 12.0), 2.0));
	float aspect = (gResolution.y > 0.0) ? gResolution.x / gResolution.y : 1.0;
	float columns = max(1.0, floor(rows * aspect / PREFERRED_BRICK_ASPECT + 0.5));
	// the real brick aspect on the texture, close to the preferred one
	float brickAspect = rows * aspect / columns;

	vec2 tile = vec2(columns, rows);

	// brick space, x is in bricks and y is in rows
	vec2 p = texCoord * tile;

	// running bond, every second row is shifted by half a brick
	float row = floor(p.y);
	p.x += 0.5 * mod(row, 2.0);

	// wrap the brick id, so a brick cut by the tile edge has the same look on both sides
	vec2 brickId = mod(floor(p), tile);
	vec2 local = fract(p); // [0; 1] inside a brick

	// distance to the brick edge in brick height units
	vec2 edge = min(local, 1.0 - local) * vec2(brickAspect, 1.0);
	float edgeDistance = min(edge.x, edge.y);

	// mortar mask with a soft edge, a bit of noise to make the joints uneven
	float halfMortar = 0.5 * mortar;
	float jitter = (ValueNoise(p * 9.0, tile * 9.0) - 0.5) * 0.04;
	float aa = 1.5 * rows / max(gResolution.y, 1.0); // about one pixel in brick units
	float brickMask = smoothstep(halfMortar - aa, halfMortar + aa, edgeDistance + jitter);

	// per brick color variation
	float brickRandom = Hash12(brickId);
	vec3 brick = brickBase * (1.0 + variation * (brickRandom - 0.5));
	brick = mix(brick, brick * vec3(1.08, 0.95, 0.9), Hash12(brickId + 17.0) * variation);

	// surface grain and small dark spots, a per brick offset makes every brick a bit different
	vec2 brickOffset = floor(brickId * 7.0);
	float surface = Fbm(p * vec2(6.0, 12.0) + brickOffset, tile * vec2(6.0, 12.0));
	brick *= 1.0 + grain * (surface - 0.5);
	float spots = smoothstep(0.72, 0.8, ValueNoise(p * 24.0 + brickOffset, tile * 24.0));
	brick *= 1.0 - 0.25 * grain * spots;

	// optional bevel, darker towards the brick edges
	if (bevel_flag > 0.5)
	{
		float bevel = smoothstep(halfMortar, halfMortar + 0.12, edgeDistance);
		brick *= mix(0.7, 1.0, bevel);
	}

	vec3 mortarColor = mortarBase * (0.9 + 0.2 * Fbm(texCoord * tile * 8.0, tile * 8.0));

	vec3 color = mix(mortarColor, brick, brickMask);

	// optional blend with the input image (a texture layer or a previous effect)
	float inputAmount = clamp(0.01 * mixWithInput_slider, 0.0, 1.0);
	if (inputAmount > 0.0)
	{
		vec3 inputColor = texture(inputSampler, texCoord).rgb;
		color = mix(color, color * inputColor * 1.5, inputAmount);
	}

	FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
