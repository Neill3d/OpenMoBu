//
// Fragment shader - Fish Eye
//
//	Post Processing Toolkit
//	 Created by Sergey <Neill3d> Solokhin (c) 2018
//	Special for Les Androids Associes
//

#version 120
uniform sampler2D sampler0;

uniform float	upperClip;
uniform float	lowerClip;

varying vec4 Vertex_UV;
const float PI = 3.1415926535;

varying vec4 posDevSpace;
uniform float	amount;
uniform float lensradius; // 3
uniform float signcurvature; // 10

#define EPSILON 0.000011

void main (void)
{
	if (Vertex_UV.y < upperClip || Vertex_UV.y > lowerClip)
	{
		vec4 fragColor = texture2D(sampler0, Vertex_UV.st);
		gl_FragColor =  fragColor;
		return;
	}
	
    float curvature = abs(signcurvature);
    float extent = lensradius;
    float optics = extent / log2(curvature * extent + 1.0) / 1.4427;
    vec4 PP = posDevSpace - vec4 (1.5, 1.5, 0.0, 1.0);
    float P0 = PP[0];
    float P1 = PP[1];
    float radius = sqrt(P0 * P0 + P1 * P1);

    float cosangle = P0 / radius;
    float sinangle = P1 / radius;

    float rad1, rad2, newradius;
    rad1 = (exp2((radius / optics) * 1.4427) - 1.0) / curvature;
    rad2 = optics * log2(1.0 + curvature * radius) / 1.4427;
    newradius = signcurvature > 0.0 ? rad1 : rad2;

    vec4 FE = vec4 (0.0, 0.0, 0.0, 1.0);
    FE[0] = newradius * cosangle + 0.5;
    FE[1] = newradius * sinangle + 0.5;
    FE = radius <= extent ? FE : posDevSpace;
    FE = curvature < EPSILON ? posDevSpace : FE;

	vec2 texCoords = mix(Vertex_UV.st, vec2(FE), amount);
	vec4 color = texture2D(sampler0, texCoords);
    gl_FragColor = color;
	//gl_FragColor = texture2D(sampler0, Vertex_UV.st);
}
