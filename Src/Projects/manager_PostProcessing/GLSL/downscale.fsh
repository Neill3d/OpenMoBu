//
// Fragment shader - Downscale
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

uniform sampler2D 		sampler;

in vec2 centerTextureCoordinate;
in vec2 oneStepLeftTextureCoordinate;
in vec2 twoStepsLeftTextureCoordinate;
in vec2 threeStepsLeftTextureCoordinate;
in vec2 fourStepsLeftTextureCoordinate;
in vec2 oneStepRightTextureCoordinate;
in vec2 twoStepsRightTextureCoordinate;
in vec2 threeStepsRightTextureCoordinate;
in vec2 fourStepsRightTextureCoordinate;

out vec4 FragColor;

// sinc(x) * sinc(x/a) = (a * sin(pi * x) * sin(pi * x / a)) / (pi^2 * x^2)
// Assuming a Lanczos constant of 2.0, and scaling values to max out at x = +/- 1.5

void main (void)
{
	vec4 fragmentColor = texture2D(sampler, centerTextureCoordinate) * 0.38026;

	fragmentColor += texture2D(sampler, oneStepLeftTextureCoordinate) * 0.27667;
	fragmentColor += texture2D(sampler, oneStepRightTextureCoordinate) * 0.27667;

	fragmentColor += texture2D(sampler, twoStepsLeftTextureCoordinate) * 0.08074;
	fragmentColor += texture2D(sampler, twoStepsRightTextureCoordinate) * 0.08074;

	fragmentColor += texture2D(sampler, threeStepsLeftTextureCoordinate) * -0.02612;
	fragmentColor += texture2D(sampler, threeStepsRightTextureCoordinate) * -0.02612;

	fragmentColor += texture2D(sampler, fourStepsLeftTextureCoordinate) * -0.02143;
	fragmentColor += texture2D(sampler, fourStepsRightTextureCoordinate) * -0.02143;

	fragmentColor.a = 1.0;
	
	FragColor = fragmentColor;
}