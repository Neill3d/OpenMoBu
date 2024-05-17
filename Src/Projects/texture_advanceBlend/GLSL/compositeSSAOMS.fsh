
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// AO fragment shader.
//  Realization based on Hemispherical Screen-Space Ambient Occlusion
// http://blog.evoserv.at/index.php/2012/12/hemispherical-screen-space-ambient-occlusion-ssao-for-deferred-renderers-using-openglglsl/
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#version 120

#extension GL_ARB_texture_multisample : enable
#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_gpu_shader5 : enable
#extension GL_ARB_shader_bit_encoding : enable

uniform	sampler2D	colorSampler;
uniform sampler2DMS	depthSampler;
uniform sampler2DMS	normalSampler;

uniform float 		distanceThreshold;	// = 10.5;
uniform vec2		filterRadius;		// = vec2( 0.02, 0.02 );
uniform int			only_ao;

uniform mat4		g_matInvProjection;

uniform float 		zNear = 10.0;
uniform float 		zFar = 4000.0;

uniform float		gamma = 1.0;
uniform float		contrast = 1.0;

uniform float		logarithmicDepth = 0.0;

uniform int			numberOfSamples = 4;		// AA samples

uniform int			SCREEN_WIDTH = 512;
uniform int			SCREEN_HEIGHT = 512;

const int 	sample_count = 49;
const vec2 oflut[49] = vec2[](
    vec2(-0.864,-0.930),
    vec2(-0.952,-0.608),
    vec2(-0.820,-0.318),
    vec2(-0.960,-0.076),
    vec2(-0.939,0.244),
    vec2(-0.827,0.421),
    vec2(-0.871,0.561),
    vec2(-0.668,-0.810),
    vec2(-0.749,-0.557),
    vec2(-0.734,-0.422),
    vec2(-0.681,-0.043),
    vec2(-0.598,0.132),
    vec2(-0.720,0.323),
    vec2(-0.513,0.723),
    vec2(-0.371,-0.829),
    vec2(-0.366,-0.735),
    vec2(-0.259,-0.318),
    vec2(-0.344,-0.079),
    vec2(-0.386,0.196),
    vec2(-0.405,0.425),
    vec2(-0.310,0.518),
    vec2(-0.154,-0.931),
    vec2(-0.171,-0.572),
    vec2(-0.241,-0.420),
    vec2(-0.130,-0.225),
    vec2(-0.092,0.189),
    vec2(-0.018,0.310),
    vec2(-0.179,0.512),
    vec2(0.133,-0.942),
    vec2(0.235,-0.738),
    vec2(0.229,-0.380),
    vec2(0.027,-0.030),
    vec2(0.052,0.183),
    vec2(0.141,0.415),
    vec2(0.129,0.736),
    vec2(0.340,-0.931),
    vec2(0.254,-0.564),
    vec2(0.388,-0.417),
    vec2(0.363,-0.104),
    vec2(0.413,0.234),
    vec2(0.421,0.321),
    vec2(0.423,0.653),
    vec2(0.631,-0.756),
    vec2(0.665,-0.736),
    vec2(0.551,-0.350),
    vec2(0.526,-0.220),
    vec2(0.519,0.053),
    vec2(0.750,0.321),
    vec2(0.736,0.641) );

vec3 decodeNormal(in ivec2 uv, in int nsample)
{
	// restore normal
	return normalize(texelFetch(normalSampler, uv, nsample).xyz);
}

float LogarithmicDepth(float d)
{
	float C = 1.0;
	float z = (exp(d*log(C*zFar+1.0)) - 1.0)/C;
	return z;
}

float ZBuffer2Linear( float z ,float znear, float zfar)
{
    float z_n = 2.0 * z - 1.0;
    float z_e = 2.0 * znear * zfar / (zfar+znear-z_n*(zfar-znear));
    return z_e / 10.0;
}

vec3 calculatePosition(in vec2 coord, in ivec2 uv, in int nsample)
{
	//float n = zNear; // camera z near
	//float f = zFar; // camera z far
	float z = texelFetch(depthSampler, uv, nsample).x * 2.0 - 1.0;
	//return vec3((2.0 * n) / (f + n - z * (f - n)));	

	//float z_b = texture2D(depthBuffTex, vTexCoord).x;
	float z_b = z;
	float z_n = 2.0 * z_b - 1.0;
	float z_e = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
	//z_e /= 10.0;
	//return z_e;

	// Get x/w and y/w from the viewport position
    float x = coord.x * 2.0 - 1.0;
    float y = coord.y * 2.0 - 1.0;
    vec4 vProjectedPos = vec4(x, y, z, 1.0);
    // Transform by the inverse projection matrix
    vec4 vPositionVS = g_matInvProjection * vProjectedPos;  
    // Divide by w to get the view-space position
    //vPositionVS.z = z_e;
	vPositionVS.xyz = vPositionVS.xyz / vPositionVS.w;
	
	return vPositionVS.xyz;
}

mat3 GetRotationMatrix(in vec3 surfaceNormal, in vec3 randomNormal)
{
 // Gram–Schmidt process to construct an orthogonal basis.
 vec3 tangent = normalize(randomNormal - surfaceNormal * dot(randomNormal, surfaceNormal));
 vec3 bitangent = cross(surfaceNormal, tangent);
 return mat3(tangent, bitangent, surfaceNormal);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// COMMON FUNCTIONS


// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

// Compound versions of the hashing algorithm I whipped together.
uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }


// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}

// Pseudo-random value in half-open range [0:1].
float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
float random( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }

float CalculateAO(in float k1, in float k2, in int nsample, in vec3 viewPos, in vec3 viewNormal)
{
	// perform AO
	float ambientOcclusion = 0.0;
	vec2 screenDimentions = vec2( SCREEN_WIDTH, SCREEN_HEIGHT );
	
    for (int i = 0; i < sample_count; ++i)
    {
		vec3 of = vec3( oflut[i].x*k1 - oflut[i].y*k2,
                        oflut[i].x*k2 + oflut[i].y*k1, 1.0 );
						
		//vec3 randomNormal = vec3( of.x, of.y, gl_TexCoord[0].s + gl_TexCoord[0].t - 2.2 );
		//mat3 rotateMatrix = GetRotationMatrix( viewNormal, randomNormal );
		
		//of = rotateMatrix * of;
	
        // sample at an offset specified by the current Poisson-Disk sample and scale it by a radius (has to be in Texture-Space)
        vec2 sampleTexCoord = gl_TexCoord[0].st + filterRadius * of.st;
		vec2 rasterCoord = sampleTexCoord * screenDimentions;
		
        vec3 samplePos = calculatePosition( sampleTexCoord, ivec2(int(rasterCoord.s), int(rasterCoord.t)), nsample);
        vec3 sampleDir = normalize(samplePos - viewPos);

        // angle between SURFACE-NORMAL and SAMPLE-DIRECTION (vector from SURFACE-POSITION to SAMPLE-POSITION)
        float NdotS = max(dot(viewNormal, sampleDir), 0);
        // distance between SURFACE-POSITION and SAMPLE-POSITION
        float VPdistSP = distance(viewPos, samplePos);

        // a = distance function
        float a = 1.0 - smoothstep(distanceThreshold, distanceThreshold * 2, VPdistSP);
		//a = 1.0;
        // b = dot-Product
		if (abs(NdotS) < 0.15) NdotS = 0.0;
        float b = NdotS;

        ambientOcclusion += (a * b);
    }

    return 1.0 - (ambientOcclusion / sample_count);
}

void main()
{
	//vec2 filterRadius = vec2(0.01, 0.01);
	
	vec2 outUV = gl_TexCoord[0].st;
	vec2 screen = vec2( SCREEN_WIDTH, SCREEN_HEIGHT );
	ivec2 rasterPos = ivec2( int(outUV.s * screen.s), int(outUV.t * screen.t) );
	
	vec4 colorSample = texture2D(colorSampler, outUV);
	
	
	float an = fract(fract(outUV.x*0.36257507)*0.38746515+outUV.y*0.32126721);

    float k1=cos(6.2831*an);
    float k2=sin(6.2831*an);
	
	float ao = 0.0;
	for (int nsample=0; nsample<numberOfSamples; ++nsample)
	{
		// reconstruct position from depth, USE YOUR CODE HERE
		vec3 viewPos = calculatePosition(outUV, rasterPos, nsample);
	
		// get the view space normal, USE YOUR CODE HERE
		vec3 viewNormal = decodeNormal(rasterPos, nsample);
		
		//
		ao += CalculateAO(k1, k2, nsample, viewPos, viewNormal);
		
	}
	ao /= float(numberOfSamples);
	
	//scale occlusion toward white.
    ao = clamp(ao, 0.0, 1.0);
	ao = contrast * pow(ao, gamma);
	
	if (only_ao == 0)
	{
		//add in contrast/gamma to colorsample.
		gl_FragColor = colorSample*ao;
	}
	else
	{
		gl_FragColor = vec4(ao, ao, ao, 1.0);
	}
}