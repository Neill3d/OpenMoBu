
// test fragment shader
// sampler2d, apply on previous layer

#version 140

in vec2 texCoord;
out vec4 FragColor;

uniform float ampl_slider; // slider(0-100)
uniform vec3 mycolor_color; // color rgb picker
uniform sampler2D inputSampler;
uniform sampler2D mySampler;

void main (void)
{
	vec2 uv = texCoord;
	vec4 input = texture2D( inputSampler, uv );
	vec4 texColor = texture2D( mySampler, uv );

	float value = 0.01 * ampl_slider;
	vec4 color = vec4( clamp(mix(input.rgb, texColor.rgb, value) + mycolor_color, vec3(0.0), vec3(1.0)), 1.0 ); 
	
	FragColor = color;
}