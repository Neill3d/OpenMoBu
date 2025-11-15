//
// Vertex shader - Downscale 
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

#version 120

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

void main(void)
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    
	vec2 firstOffset = 0.5 * texelSize;
	vec2 secondOffset = vec2(2.0 * texelSize.x, 2.0 * texelSize.y);
	vec2 thirdOffset = vec2(3.0 * texelSize.x, 3.0 * texelSize.y);
	vec2 fourthOffset = vec2(4.0 * texelSize.x, 4.0 * texelSize.y);
	
    vec2 inputTextureCoordinate = gl_MultiTexCoord0.st;
	centerTextureCoordinate = inputTextureCoordinate;
	oneStepLeftTextureCoordinate = inputTextureCoordinate - firstOffset;
	twoStepsLeftTextureCoordinate = inputTextureCoordinate - secondOffset;
	threeStepsLeftTextureCoordinate = inputTextureCoordinate - thirdOffset;
	fourStepsLeftTextureCoordinate = inputTextureCoordinate - fourthOffset;
	oneStepRightTextureCoordinate = inputTextureCoordinate + firstOffset;
	twoStepsRightTextureCoordinate = inputTextureCoordinate + secondOffset;
	threeStepsRightTextureCoordinate = inputTextureCoordinate + thirdOffset;
	fourStepsRightTextureCoordinate = inputTextureCoordinate + fourthOffset;
}
