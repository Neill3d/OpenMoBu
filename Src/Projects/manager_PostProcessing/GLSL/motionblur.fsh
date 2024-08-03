
//
// Fragment Shader - Motion Blur
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

#version 130

uniform	sampler2D	colorSampler;
uniform sampler2D	depthSampler;
uniform sampler2D	maskSampler;

uniform float		useMasking;
uniform float 		zNear;
uniform float 		zFar;

uniform float		dt;

uniform float	upperClip;
uniform float	lowerClip;

uniform vec4	gClipInfo;	// z_n * z_f,  z_n - z_f,  z_f, perspective = 1 : 0

uniform vec4 projInfo;
uniform int projOrtho;
uniform vec2 InvQuarterResolution;
uniform vec2 InvFullResolution;

uniform mat4 uInverseModelViewMat;	// inverse model->view-space
uniform mat4 uPrevModelViewProj; 	// previous model->view->projection



void ComputeDepth(out float depth, in vec2 texCoord)
{
	float d = texture2D(depthSampler, texCoord).x;
	
	float C = 1.0;
	float z = (exp(d*log(C*zFar+1.0)) - 1.0)/C;
	
	float n = zNear;
	float f = zFar;
	float lz = d;
	
	// nvidia depth range doesn't need that !
	lz = 2.0 * d - 1.0; 
	lz = (2.0 * n) / (f + n - lz * (f - n));	
	
	depth = lz;
}

// -----------------------------------------------------------------------

void main()
{
	vec2 uv = gl_TexCoord[0].st;
	
	float zOverW = 0.0;
	ComputeDepth(zOverW, uv);
	
	vec4 H = vec4(uv.s * 2.0 - 1.0, uv.t*2.0-1.0, zOverW, 1.0);
	// transfrom by the view-projection inverse.
	vec4 D = uInverseModelViewMat * H;
	
	// Divide by w to get the world position.
	vec4 worldPos = D / D.w;
	
	// current viewport position
	vec4 currentPos = H;
	// use the world position, and transform by the previous view-projection matrix
	vec4 previousPos = uPrevModelViewProj * worldPos;
	// convert to nonhomogeneous points[-1,1] by dividing by w.
	previousPos /= previousPos.w;
	//previousPos.xy = previousPos.xy * 0.5 + 0.5;
	// use this frame's position and last frame's to compute the pixel velocity
	vec2 blurVec = (currentPos.xy - previousPos.xy) * dt;
	
	//
	
	vec4 inputColor = texture2D( colorSampler, uv );
	vec4 outcolor = inputColor;

	const int nSamples = 8;
	for (int i=0; i<nSamples; ++i)
	{
		// get offset in range[-0.5; 0.5]
		vec2 offset = blurVec * (float(i) / float(nSamples-1) - 0.5);
		
		// sample and add to result:
		outcolor += texture2D(colorSampler, uv + offset);
	}
	outcolor /= float(nSamples);
	
	//
	// masking 

	vec4 mask = vec4(0.0, 0.0, 0.0, 0.0);
	if (useMasking > 0.0)
	{
		mask = texture2D( maskSampler, uv );
	}
	outcolor.rgb = mix(outcolor.rgb, inputColor.rgb, mask.r * useMasking);

	gl_FragColor = outcolor;	
}