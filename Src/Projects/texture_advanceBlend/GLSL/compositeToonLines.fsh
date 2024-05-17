uniform	sampler2D	colorSampler;
uniform sampler2D	normalSampler;
uniform sampler2D	depthSampler;

uniform float 		zNear;	// camera near plane
uniform float 		zFar;	// camera far plane

uniform float		falloff;

uniform vec2		textureCoordOffset[9];

uniform float		logarithmicDepth;
uniform float		highQualityDepth;

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
*/

void ComputeDepth(in vec2 uv, out float depth)
{
	float d = texture2D(depthSampler, uv).x;
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
/*
float decodeDepth(in vec2 uv)
{
	if (logarithmicDepth > 0.0)
	{
		return LogarithmicDepth(texture2D(depthSampler, uv).x);
	}
	else
	{
		return LinearizeDepth(texture2D(depthSampler, uv).x);
	}
}
*/
vec3 decodeNormal(in vec2 normal)
{
	// restore normal
	return normalize(texture2D(normalSampler, normal).xyz);
}

// The fragment shader loop
void main(void)
{
	// Assign these variables now because they will be used next
	vec4 texcol = texture2D(colorSampler, gl_TexCoord[0].st);

	// Current fragment depth
	float base = 0.0;
	float colDifForce = 0.0;
	
	float Depth[9];
	vec3 Normal[9];
	
	vec2 screenPos = gl_TexCoord[0].st;
	for (int i=0; i<9; ++i)
	{
		vec2 uv = screenPos + textureCoordOffset[i];
		//Depth[i] = decodeDepth(uv); // Retrieve depth
		ComputeDepth(uv, Depth[i]);
		Normal[i] = decodeNormal(uv);
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

	if (falloff > 0.0)
	{
		float lfalloff = falloff / zFar;
		float depth = Depth[0];
		depth = clamp(depth, 0.0, lfalloff);
		edge *= (1.0 - depth / lfalloff);
	}
		  
	gl_FragColor = mix(texcol, vec4(0.0, 0.0, 0.0, 1.0), edge );
}
