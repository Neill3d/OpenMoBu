
//
// Fragment Shader - SSAO
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
// Based on NVidia gl_ssao (horizon-based ambient occlusion) https://github.com/nvpro-samples/gl_ssao

#version 130

in vec2 texCoord;
out vec4 FragColor;

uniform	sampler2D	colorSampler;
uniform sampler2D	linearDepthSampler;
uniform sampler2D	texRandom;
uniform sampler2D	maskSampler;

uniform float	useMasking;
uniform float	upperClip;
uniform float	lowerClip;

//uniform vec4	gClipInfo;	// z_n * z_f,  z_n - z_f,  z_f, perspective = 1 : 0

uniform vec4 projInfo;
uniform int projOrtho;
//uniform vec2 InvQuarterResolution;
uniform vec2 InvFullResolution;

uniform float RadiusToScreen;		// radius
//uniform float R2;					// 1 / radius
uniform float NegInvR2;				// radius * radius
uniform float NDotVBias;			

uniform float	AOMultiplier;
uniform float 	PowExponent;

//uniform vec4 	g_Jitter;

uniform float	OnlyAO;		// display only AO when > 0.0

#define AO_RANDOMTEX_SIZE 4
#define M_PI 3.14159265f

// tweakables
const float  NUM_STEPS = 4.0;
const float  NUM_DIRECTIONS = 8.0; // texRandom/g_Jitter initialization depends on this


vec3 UVToView(vec2 uv, float eye_z)
{
  return vec3((uv * projInfo.xy + projInfo.zw) * (projOrtho != 0 ? 1. : eye_z), eye_z);
}

vec3 FetchViewPos(vec2 UV)
{
  float ViewDepth = textureLod(linearDepthSampler, UV, 0).x;	// texLinearDepth
  return UVToView(UV, ViewDepth);
}

vec3 MinDiff(vec3 P, vec3 Pr, vec3 Pl)
{
  vec3 V1 = Pr - P;
  vec3 V2 = P - Pl;
  return (dot(V1,V1) < dot(V2,V2)) ? V1 : V2;
}

vec3 ReconstructNormal(vec2 UV, vec3 P)
{
  vec3 Pr = FetchViewPos(UV + vec2(InvFullResolution.x, 0));
  vec3 Pl = FetchViewPos(UV + vec2(-InvFullResolution.x, 0));
  vec3 Pt = FetchViewPos(UV + vec2(0, InvFullResolution.y));
  vec3 Pb = FetchViewPos(UV + vec2(0, -InvFullResolution.y));
  
  return normalize(cross(MinDiff(P, Pr, Pl), MinDiff(P, Pt, Pb)));
}


//----------------------------------------------------------------------------------
float Falloff(float DistanceSquare)
{
  // 1 scalar mad instruction
  return DistanceSquare * NegInvR2 + 1.0;
}

//----------------------------------------------------------------------------------
// P = view-space position at the kernel center
// N = view-space normal at the kernel center
// S = view-space position of the current sample
//----------------------------------------------------------------------------------
float ComputeAO(vec3 P, vec3 N, vec3 S)
{
  vec3 V = S - P;
  float VdotV = dot(V, V);
  float NdotV = dot(N, V) * 1.0/sqrt(VdotV);

  // Use saturate(x) instead of max(x,0.f) because that is faster on Kepler
  return clamp(NdotV - NDotVBias,0,1) * clamp(Falloff(VdotV),0,1);
}

//----------------------------------------------------------------------------------
vec2 RotateDirection(vec2 Dir, vec2 CosSin)
{
  return vec2(Dir.x*CosSin.x - Dir.y*CosSin.y,
              Dir.x*CosSin.y + Dir.y*CosSin.x);
}

//----------------------------------------------------------------------------------
vec4 GetJitter()
{
	return textureLod( texRandom, (gl_FragCoord.xy / AO_RANDOMTEX_SIZE), 0);
}

//----------------------------------------------------------------------------------
float ComputeCoarseAO(vec2 FullResUV, float RadiusPixels, vec4 Rand, vec3 ViewPosition, vec3 ViewNormal)
{
#if AO_DEINTERLEAVED
  RadiusPixels /= 4.0;
#endif

  // Divide by NUM_STEPS+1 so that the farthest samples are not fully attenuated
  float StepSizePixels = RadiusPixels / (NUM_STEPS + 1);

  const float Alpha = 2.0 * M_PI / NUM_DIRECTIONS;
  float AO = 0;

  for (float DirectionIndex = 0; DirectionIndex < NUM_DIRECTIONS; ++DirectionIndex)
  {
    float Angle = Alpha * DirectionIndex;

    // Compute normalized 2D direction
    vec2 Direction = RotateDirection(vec2(cos(Angle), sin(Angle)), Rand.xy);

    // Jitter starting sample within the first step
    float RayPixels = (Rand.z * StepSizePixels + 1.0);

    for (float StepIndex = 0; StepIndex < NUM_STEPS; ++StepIndex)
    {
      vec2 SnappedUV = round(RayPixels * Direction) * InvFullResolution + FullResUV;
      vec3 S = FetchViewPos(SnappedUV);

      RayPixels += StepSizePixels;

      AO += ComputeAO(ViewPosition, ViewNormal, S);
    }
  }

  AO *= AOMultiplier / (NUM_DIRECTIONS * NUM_STEPS);
  return clamp(1.0 - AO * 2.0,0,1);
}

// -----------------------------------------------------------------------

void main()
{
	vec2 uv = texCoord;
	
	if (uv.y < upperClip || uv.y > lowerClip)
	{
		vec4 fragColor = texture2D(colorSampler, uv);
		FragColor =  fragColor;
		return;
	}
	
	vec3 ViewPosition = FetchViewPos(uv);

	// Reconstruct view-space normal from nearest neighbors
	vec3 ViewNormal = -ReconstructNormal(uv, ViewPosition);

	if (OnlyAO > 0.0)
	{
		float ViewDepth = textureLod(linearDepthSampler, uv, 0).x;
		FragColor = vec4(vec3(ViewDepth), 1.0);
		return;
	}

	// Compute projection of disk of radius control.R into screen space
	float RadiusPixels = RadiusToScreen / (projOrtho != 0 ? 1.0 : ViewPosition.z);

	// Get jitter vector for the current full-res pixel
	vec4 Rand = GetJitter();
	
	float AO = ComputeCoarseAO(uv, RadiusPixels, Rand, ViewPosition, ViewNormal);
	AO = pow(AO, PowExponent);
	
	if (useMasking > 0.0)
	{
		vec4 mask = texture2D( maskSampler, uv );
		AO = mix(AO, 1.0, mask.r * useMasking);
	}

	vec4 outcolor = vec4(AO);
	
	if (OnlyAO < 1.0)
	{
		outcolor = texture2D( colorSampler, uv );
		outcolor.rgb *= AO;
	}
	
	FragColor = outcolor;		
}