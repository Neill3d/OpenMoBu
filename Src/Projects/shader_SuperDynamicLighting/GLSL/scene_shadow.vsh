//
// Vertex shader - render a shadow map
//
//	OpenMoBu
//
// Sergei <Neill3d> Solokhin 2024
//
// GitHub page - https://github.com/Neill3d/OpenMoBu
// Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//

#version 430 compatibility

// vertex attributes
layout(location=0) in vec4 inPosition;
//layout(location=1) in vec2 inTexCoords;

// uniforms
uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

// output
out gl_PerVertex {
	vec4 gl_Position; // 16 bytes
};

layout(location=1) smooth out vec2 outTexCoords; // 8 bytes

void main(void)
{
    vec4 vertex = vec4(inPosition.xyz, 1.0);
    vertex = modelMatrix * vertex;
    vertex = viewMatrix * vertex;
    
    gl_Position = projMatrix * vertex;
    outTexCoords = vec2(0.0, 0.0); // inTexCoords;
}
