
// object slot sample, a soft glow on screen at the position of a connected scene object
//  connect a light (or any model) to the "sun" property of the user effect

#version 140

in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D inputSampler;
uniform vec2 gResolution;

uniform vec3  sun_object_screen;     // (u, v, view depth) of the connected object
uniform vec3  sun_object_color;      // light color, white for a non-light
uniform float sun_object_intensity;  // light intensity, 1.0 for the default 100
uniform float sun_object_connected;  // 1 when an object is connected

uniform float radius_slider;  // glow radius in percents of the screen height, 0 is the default 20 percents

void main(void)
{
	vec4 color = texture(inputSampler, texCoord);

	// aspect corrected distance to the object on screen
	vec2 delta = (texCoord - sun_object_screen.xy) * vec2(gResolution.x / gResolution.y, 1.0);
	float radius = (radius_slider > 0.0) ? 0.01 * radius_slider : 0.2;
	float glow = 1.0 - smoothstep(0.0, radius, length(delta));

	// no glow when nothing is connected or the object is behind the camera
	float visible = sun_object_connected * step(0.0, sun_object_screen.z);

	color.rgb += sun_object_color * sun_object_intensity * glow * visible;
	FragColor = color;
}
