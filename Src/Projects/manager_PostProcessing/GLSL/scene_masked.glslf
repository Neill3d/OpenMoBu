//
// Fragment shader - Render 3d Model for masking
//
//	Post Processing Toolkit
//
// Sergei <Neill3d> Solokhin 2018-2024
//
// GitHub page - https://github.com/Neill3d/OpenMoBu
// Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
//	Special for Les Androids Associes
//

#version 430 compatibility


/////////////////////////////////////////////////////
// Rim effect, could be used as a falloff for the mask appearance

uniform vec4		baseColor;
uniform vec4		rimOptions;
uniform vec4		rimColor;


void ApplyRim(in vec3 Nn, in vec3 inPw, in vec4 rimOptions, in vec4 rimColor, inout vec4 difColor)
{
	if( rimOptions.x > 0. ) {
		float f = rimOptions.y * abs( dot( Nn, normalize(inPw) ) );
		f = rimOptions.x * ( 1. - smoothstep( 0.0, 1., f ) );
		
		difColor.rgb = mix(difColor.rgb, rimColor.rgb, f);
	}
}

//////////////////////////////////////////////////////////////////
// input and output attributes

	layout(location=0) smooth in vec3 inNw;
	layout(location=2) smooth in vec3 inPw;
	
	layout(location=0) out vec4 	outColor;

void main (void)
{
	vec3 Nn = normalize(inNw);
	vec4 color = baseColor;
	
	ApplyRim(Nn, inPw, rimOptions, rimColor, color);
	
	outColor = color;
}
