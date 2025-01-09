#version 140

in vec2 texCoord;
out vec4 fragColor;

uniform sampler2D inputSampler;
uniform sampler2D maskSampler;

uniform float	useMasking;
uniform float	upperClip;
uniform float	lowerClip;