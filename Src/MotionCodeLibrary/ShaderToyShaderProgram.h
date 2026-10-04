#pragma once
/*
* ShaderToyShaderProgram.h
* Sergei <Neill3d> Solokhin 2026
*		
* GitHub page - https://github.com/Neill3d/OpenMoBu
* Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//#include <windows.h>
#include <stdio.h>
#include <GL\glew.h>

///
/// profile for GLSL, to compile fragment shader code compatible with ShaderToy
///
class ShaderToyShaderProgram : public GLSLShaderProgram
{
public:
	ShaderToyShaderProgram() = default;
	virtual ~ShaderToyShaderProgram() = default;

protected:

	void OnShaderCodeReadyToCompile(std::string& shaderCodeInOut, bool isFragmentShader) const override
	{
		if (!isFragmentShader)
			return;

		// Replace embedded NULs with spaces
		for (char& c : shaderCodeInOut)
		{
			if (c == '\0')
				c = ' ';
		}

		// --- GLSL 1.40 fragment header ---
		const char* header = R"(
#version 140

uniform vec2  iResolution;
uniform float iTime;

uniform sampler2D iChannel0;

in vec2 texCoord;
out vec4 FragColor;

)";

		// --- GLSL main() wrapper ---
		const char* footer = R"(
void main()
{
    vec2 fragCoord = texCoord * iResolution;
    mainImage(FragColor, fragCoord);
}
)";

		shaderCodeInOut = std::string(header) + shaderCodeInOut + std::string(footer);
	}
};