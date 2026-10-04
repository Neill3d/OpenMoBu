//
// Fragment shader - Mix (Multiply) 
//
//	Post Processing Toolkit
//
//	Sergei <Neill3d> Solokhin 2018
//
//	GitHub page - https://github.com/Neill3d/OpenMoBu
//	Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
//	Special for Les Androids Associes
//

#version 140

in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D sampler0;
uniform sampler2D sampler1;

uniform vec4 gBloom; // x-tone, y-stretch, z-invert term, w-use bloom

void main (void)
{
	vec2 tx = texCoord.st;
	vec4 colorA = texture2D( sampler0, tx ); 
	vec4 colorB = texture2D( sampler1, tx );
	
	if (gBloom.w > 0.0)
	{
		FragColor =  colorA * gBloom.x + colorB * gBloom.y;
	}
	else if (gBloom.z > 0.0)
	{
		FragColor = gBloom.z - ((gBloom.z - colorA) * colorB);
	}
	else
	{
		FragColor = colorA * colorB;
	}
}
