
#extension GL_ARB_texture_multisample : enable

uniform	sampler2D	colorSampler;
uniform sampler2DMS	normalSampler;
uniform sampler2DMS	depthSampler;

uniform float 		zNear;	// camera near plane
uniform float 		zFar;	// camera far plane

uniform float		falloff;

uniform vec2		textureCoordOffset[9];

uniform float		logarithmicDepth;
uniform float		highQualityDepth;

uniform int			numberOfSamples=4;	// AA samples

uniform int			SCREEN_WIDTH;
uniform int			SCREEN_HEIGHT;
/*
float LogarithmicDepth(float d)
{
	//float d = texture2D(depthTex, gl_TexCoord[0].st).x;
	float C = 1.0;
	float z = (exp(d*log(C*zFar+1.0)) - 1.0)/C;
	return z;
}

float LinearizeDepth(float z)
{
  float n = zNear; // camera z near
  float f = zFar; // camera z far
  //float z = texture2D(depthTex, gl_TexCoord[0].st).x;
  //return (2.0 * n) / (f + n - z * (f - n));	
  
  //float z_b = texture2D(depthBuffTex, vTexCoord).x;
  float z_b = z;
  float z_n = 2.0 * z_b - 1.0;
  float z_e = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
  return z_e;
}

float decodeDepthMS(in ivec2 uv, in int nsample)
{
	if (logarithmicDepth > 0.0)
	{
		return LogarithmicDepth(texelFetch(depthSampler, uv, nsample).x);
	}
	else
	{
		return LinearizeDepth(texelFetch(depthSampler, uv, nsample).x);
	}
}
*/

void ComputeDepth(in ivec2 uv, in int nsample, out float depth)
{
	float d = texelFetch(depthSampler, uv, nsample).x;
	float C = 1.0;
	float z = (exp(d*log(C*zFar+1.0)) - 1.0)/C;
	
	float n = zNear;
	float f = zFar;
	float lz = d;
	
	// nvidia depth range doesn't need that !
	if (highQualityDepth == 0.0)
	{
		lz = 2.0 * d - 1.0; 
	}
	lz = (2.0 * n) / (f + n - lz * (f - n));	
	
	if (logarithmicDepth > 0.0)
	{
		depth = z;
		depth /= zFar;
	}
	else
	{
		depth = lz;
		//depth *= zFar;
	}
}

vec3 decodeNormalMS(in ivec2 uv, in int nsample)
{
	// restore normal
	return normalize(texelFetch(normalSampler, uv, nsample).xyz);
}

float EdgeDetection(in vec2 screen, in vec2 rasterPos, in int nsample)
{
	// Current fragment depth
	float base = 0.0;
	float colDifForce = 0.0;
	
	float Depth[9];
	vec3 Normal[9];
	
	
	for (int i=0; i<9; ++i)
	{
		vec2 uv = rasterPos + screen * textureCoordOffset[i];
		//Depth[i] = decodeDepthMS( ivec2( int(uv.s), int(uv.t) ), nsample); // Retrieve depth
		ComputeDepth( ivec2( int(uv.s), int(uv.t) ), nsample, Depth[i] );
		Normal[i] = decodeNormalMS( ivec2( int(uv.s), int(uv.t) ), nsample);
	}
	
	// The result fragment sample matrix is as below, where x is the current fragment(4)
	// 0 1 2
	// 3 x 5
	// 6 7 8

	//Compute Deltas in Depth.  
	vec4 Deltas1;  
	vec4 Deltas2;  
	Deltas1.x = Depth[1];  
	Deltas1.y = Depth[2];  
	Deltas1.z = Depth[3];  
	Deltas1.w = Depth[4];  
	Deltas2.x = Depth[5];  
	Deltas2.y = Depth[6];  
	Deltas2.z = Depth[7];  
	Deltas2.w = Depth[8];  
	//Compute absolute gradients from center.  
	Deltas1 = abs(Deltas1 - Depth[0]);  
	Deltas2 = abs(Depth[0] - Deltas2);  
	//Find min and max gradient, ensuring min != 0  
	vec4 maxDeltas = max(Deltas1, Deltas2);  
	vec4 minDeltas = max(min(Deltas1, Deltas2), 0.00001);  
	// Compare change in gradients, flagging ones that change  
	// significantly.  
	// How severe the change must be to get flagged is a function of the  
	// minimum gradient. It is not resolution dependent. The constant  
	// number here would change based on how the depth values are stored  
	// and how sensitive the edge detection should be.  
	vec4 depthResults = step(minDeltas * 25.0, maxDeltas);  
	//Compute change in the cosine of the angle between normals.  
	Deltas1.x = dot(Normal[1], Normal[0]);  
	Deltas1.y = dot(Normal[2], Normal[0]);  
	Deltas1.z = dot(Normal[3], Normal[0]);  
	Deltas1.w = dot(Normal[4], Normal[0]);  
	Deltas2.x = dot(Normal[5], Normal[0]);  
	Deltas2.y = dot(Normal[6], Normal[0]);  
	Deltas2.z = dot(Normal[7], Normal[0]);  
	Deltas2.w = dot(Normal[8], Normal[0]);  
	Deltas1 = abs(Deltas1 - Deltas2);  
	
	// Compare change in the cosine of the angles, flagging changes  
	// above some constant threshold. The cosine of the angle is not a  
	// linear function of the angle, so to have the flagging be  
	// independent of the angles involved, an arccos function would be  
	// required.  
	vec4 normalResults = step(0.4, Deltas1);  
	normalResults = max(normalResults, depthResults);  
	float edge = (normalResults.x + normalResults.y +  
          normalResults.z + normalResults.w) * 0.25; 
		  
	return edge;
}

// The fragment shader loop
void main(void)
{
	vec2 screenPos = gl_TexCoord[0].st;
	vec2 screenDimentions = vec2( SCREEN_WIDTH, SCREEN_HEIGHT );
	vec2 rasterPos = screenDimentions * screenPos;

	// Assign these variables now because they will be used next
	vec4 texcol = texture2D(colorSampler, gl_TexCoord[0].st);
	
	float result = 0.0;
	for (int nSample=0; nSample<numberOfSamples; ++nSample)
	{
		result += EdgeDetection(screenDimentions, rasterPos, nSample);
	}

	result /= float(numberOfSamples);
	
	if (falloff > 0.0)
	{
		float depth = 0.0;
		float lfalloff = falloff / zFar;
		//decodeDepthMS( ivec2( int(rasterPos.s), int(rasterPos.t) ), 0);
		ComputeDepth( ivec2( int(rasterPos.s), int(rasterPos.t) ), 0, depth );
		depth = clamp(depth, 0.0, lfalloff);
		result *= (1.0 - depth / lfalloff);
	}
	
	gl_FragColor = mix(texcol, vec4(0.0, 0.0, 0.0, 1.0), result );
}
