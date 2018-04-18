//
// Fragment shader - Downscale
//
//	Post Processing Toolkit
//	 Created by Sergey <Neill3d> Solokhin (c) 2018
//	Special for Les Androids Associes
//

uniform sampler2D 		sampler;
uniform vec2			texelSize;

varying vec2 centerTextureCoordinate;
varying vec2 oneStepLeftTextureCoordinate;
varying vec2 twoStepsLeftTextureCoordinate;
varying vec2 threeStepsLeftTextureCoordinate;
varying vec2 fourStepsLeftTextureCoordinate;
varying vec2 oneStepRightTextureCoordinate;
varying vec2 twoStepsRightTextureCoordinate;
varying vec2 threeStepsRightTextureCoordinate;
varying vec2 fourStepsRightTextureCoordinate;

// sinc(x) * sinc(x/a) = (a * sin(pi * x) * sin(pi * x / a)) / (pi^2 * x^2)
// Assuming a Lanczos constant of 2.0, and scaling values to max out at x = +/- 1.5

void main (void)
{
	//vec2 tx  = gl_TexCoord [0].xy;
	
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
	
	gl_FragColor = fragmentColor;
	
}
