
#version 330                                                                       

layout (location = 0) in vec4 Position;
layout (location = 1) in vec4 BindPosition;

uniform mat4 		MVP; 
uniform vec4 		CubeMin;
uniform vec4		CubeSize;

out vec4			ColorValue;

void main()                                                                         
{                      
    gl_Position     = MVP * Position;
	
	ColorValue = (Position - CubeMin) / CubeSize;
} 
