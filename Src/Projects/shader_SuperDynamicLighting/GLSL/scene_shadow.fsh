//
// Fragment shader - Shadow map with transparency map support
//
// OpenMoBu
//
// Sergei <Neill3d> Solokhin 2024
//
// GitHub page - https://github.com/Neill3d/OpenMoBu
// Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//

#version 430 compatibility

uniform float		useTransparency;
uniform mat4		transparencyTransform;
uniform sampler2D	samplerTransparency;

layout(location=1) smooth in vec2 inTexCoords;		// input 8 bytes
layout(location=0) out vec4 	outColor;

void main (void)
{
	vec4 color = vec4(1.0);

	if (useTransparency > 0.0)
	{
		vec4 coords = transparencyTransform * vec4(inTexCoords.x, inTexCoords.y, 0.0, 1.0);
		color.a = texture2D(samplerTransparency, coords.st).a;

		if (color.a < 0.5)
			discard;
	}

	outColor = color;
}
