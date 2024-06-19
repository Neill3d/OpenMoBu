


#version 120
uniform samplerCube 	cubeMapSampler;

#define PI 		3.1415926535897932384626433832795
#define HalfPI 	1.5707963267948966192313216916398
#define TWOPI 	6.283185307179587

void main()
{

	vec2 tc = gl_TexCoord[0].st;// / vec2(2.0) + 0.5;  //only line modified from the shader toy example
	vec2 thetaphi = ((tc * 2.0) - vec2(1.0)) * vec2(PI, HalfPI);
	
	vec3 rayDirection;
	rayDirection.x = cos(thetaphi.y) * cos(thetaphi.x);
	rayDirection.y = sin(thetaphi.y);
	rayDirection.z = cos(thetaphi.y) * sin(thetaphi.x);

	vec4 color = textureCube(cubeMapSampler, rayDirection);
	gl_FragColor = color;
}