//
// Fragment shader - Model Phong Shading 
//
//	Post Processing Toolkit
//
// Sergei <Neill3d> Solokhin 2018-2024
//
// GitHub page - https://github.com/Neill3d/OpenMoBu
// Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
//	Special for Les Androids Associes
//

#version 430 compatibility

#define	PI	3.14159265358979323846264

#define	SPECULAR_PHONG			0.0
#define	SPECULAR_ANISO			1.0
#define SPECULAR_SKIN			2.0

struct TMaterial
{
	// textures for material channels
	int			ambientChannel;
	int			diffuseChannel;
	int			specularChannel;
	int			emissiveChannel;
	int			transparencyChannel;
	int			normalmapChannel;
	int			reflectChannel;
	
	float		specExp;
	
	float 		useAmbient;
	float		useDiffuse;
	float		useSpecular;
	float		useEmissive;
	float		useTransparency;
	float		useNormalmap;
	float		useReflect;
	
	//
	float 		shaderTransparency;
	
	float		specularType; // phong, anisotropic or skin (cook torrance)
	float		roughnessX;
	float		roughnessY;
	float		pad;

	//
	/// Current material
	//
	vec4		emissiveColor;
	vec4     	diffuseColor;
	vec4     	ambientColor;
	vec4		reflectColor;
	vec4     	transparencyColor;
	vec4     	specularColor;
	
	//
	mat4		diffuseTransform;
	mat4		transparencyTransform;
	mat4		specularTransform;
	mat4		normalTransform;
	mat4		reflectTransform;
	// 160 in total
};


#define	LIGHT_TYPE_DIRECTION			0.0
#define	LIGHT_TYPE_POINT				1.0
#define LIGHT_TYPE_SPOT					2.0

// light data should be computer in eye space, means for specified camera
//
struct TLight
{
	vec4 		attenuations;
	// 16
	vec4 		position; 		// w - type
	// 16
	vec4 		dir; 			// w - spotAngle
	// 16
	vec4 		color;			// w - radius	
	// 16
	
	vec4		pad_castSpecularOnObject_shadowMapLayer; // where xy - pad, z - castSpecularOnObject, w - shadow map layer

	mat4		shadowVP; // view & projection matrix of a shadow map

	// total - 144 bytes
};

struct TShadow
{
	float		lightType;
	float		shadowMapLayer;
	float		shadowMapSize;
	float		shadowPCFKernelSize;
	
	// 16
	mat4		shadowVP;	// view projection matrix of a shadow map
	vec4		shadowIndex;	// index and count in the shadow matrix array
	vec4		normalizedFarPlanes;	// for cascaded shadows

	// total - 112 bytes
};

struct LIGHTINFOS
{
	vec3	worldPosition;
	vec3 	position;
	vec3 	viewDistance;
	vec3 	viewDir;	
	vec3 	normal;	
	vec3	tangent;
	vec3	bitangent;
	vec2	uv;
	float 	shininess;
	vec4	specularInput; // xyz - color and w - factor
};

struct LIGHTRES
{
	vec3 ambientContrib;
	vec3 diffContrib;
	vec3 specContrib;
	//vec3 R;	// reflection vector : returned by the reflection computation
	//vec4 reflectionHi; // result from a high res env.map
	//vec4 reflectionLow; // result from a Low res env.map
};


layout (std430, binding = 1) buffer MaterialBuffer
{
	TMaterial mat;
	
} materialBuffer;

layout (std430, binding = 2) buffer DirLightsBuffer
{
	TLight lights[];
	
} dirLightsBuffer;

layout (std430, binding = 3) buffer LightsBuffer
{
	TLight lights[];

} lightsBuffer;

layout (std430, binding = 4) buffer ShadowsBuffer
{
	TShadow shadows[];
} shadowsBuffer;

/////////////////////////////////////////////////////

uniform int			numberOfDirLights;
uniform int			numberOfPointLights;
uniform int			numberOfShadows;

uniform float		switchAlbedoTosRGB;
uniform float		useMatCap;
uniform float		useLightmap;
uniform vec4		globalAmbientLight;

uniform vec4		fogColor;
//uniform vec4		fogOptions;

uniform vec4		rimOptions;
uniform vec4		rimColor;

uniform sampler2D	samplerDiffuse;
uniform sampler2D	samplerDetail;	// second map and possible second UV set
uniform sampler2D	samplerTransparency;
uniform sampler2D	samplerSpecular;
uniform sampler2D	samplerNormal;
uniform sampler2D	samplerReflect;
uniform sampler2D	samplerMatCap;

// Sampler for the shadow map
uniform sampler2DArray samplerShadowMaps;

	layout(location=0) smooth in vec3 inNw;
	layout(location=1) smooth in vec2 inTC;		// input 8 bytes
	layout(location=2) smooth in vec3 inPw;
	layout(location=3) smooth in vec3 inWV;
	
	layout(location=4) smooth in vec3 n_eye; // eyePos
	layout(location=5) smooth in vec3 	inTangent;
	layout(location=6) smooth in vec2 	inTC2;
	
	layout(location=7) smooth in float fogAmount;
	layout(location=8) smooth in vec3 inViewPos;

	layout(location=0) out vec4 	outColor;

//////////////////////////////////////////////////////////////////////////////
// lights

float calculateShadow(vec4 shadowCoord, float shadowMapLayer) 
{
	// perform perspective divide
    vec3 projCoords = shadowCoord.xyz / shadowCoord.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(samplerShadowMaps, vec3(projCoords.xy, shadowMapLayer)).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}

float calculateSoftShadow(vec4 shadowCoord, float shadowMapLayer, int samples, float radius) {
    // Perform perspective divide
    vec3 projCoords = shadowCoord.xyz / shadowCoord.w;
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Ensure the fragment is within the shadow map's range
    if (projCoords.z > 1.0 || projCoords.z < 0.0) {
        return 0.0; // Outside shadow map bounds
    }

    // Initialize shadow variable
    float shadow = 0.0;
    float currentDepth = projCoords.z;

    // Define a sampling kernel for PCF
    for (int x = -samples; x <= samples; ++x) {
        for (int y = -samples; y <= samples; ++y) {
            // Compute offset in texture space
            vec2 offset = vec2(x, y) * radius / float(samples);
            // Sample shadow map at the offset
            float closestDepth = texture(samplerShadowMaps, vec3(projCoords.xy + offset, shadowMapLayer)).r;
            // Accumulate shadow contribution
            shadow += currentDepth > closestDepth ? 1.0 : 0.0;
        }
    }

    // Average shadow contribution
    float totalSamples = float((samples * 2 + 1) * (samples * 2 + 1));
    shadow /= totalSamples;

    return shadow;
}


float evalShadows(in LIGHTINFOS info)
{
	float shadowContrib = 1.0;

	for (int i=0; i<numberOfShadows; ++i)
	{
		if (shadowsBuffer.shadows[i].shadowMapLayer >= 0.0)
		{
			vec4 shadowCoord = shadowsBuffer.shadows[i].shadowVP * vec4(info.worldPosition, 1.0);
			float shadowFactor = calculateShadow(shadowCoord, shadowsBuffer.shadows[i].shadowMapLayer);

			shadowContrib *= shadowFactor;
		}
	}
	return shadowContrib;
}

// GLSL Function for Standard Phong Specular Reflection
float phongSpecular(
    vec3 normal,       // Surface normal at the shading point (N), must be normalized
    vec3 viewDir,      // Direction from the surface point to the camera (V), must be normalized
    vec3 lightDir,     // Direction from the surface point to the light source (L), must be normalized
    float shininess    // Shininess exponent controlling the size of the specular highlight
) {
    // Compute the reflection vector (R) of the light direction about the normal
    vec3 reflection = reflect(-lightDir, normal);

    // Compute the dot product between the reflection vector and the view direction
    // Clamp to avoid negative values
    float specAngle = max(dot(reflection, viewDir), 0.0);

    // Compute the specular term using the shininess exponent
    return pow(specAngle, shininess);
}

// GLSL Function for Physically-Based Anisotropic Specular Reflection
float anisotropicSpecular(
    vec3 normal,          // Surface normal (N), must be normalized
    vec3 viewDir,         // Direction from the surface to the camera (V), must be normalized
    vec3 lightDir,        // Direction from the surface to the light source (L), must be normalized
    float roughnessX,     // Roughness along the tangent direction
    float roughnessY,     // Roughness along the bitangent direction
    vec3 tangent,         // Tangent vector (aligned with brushing direction)
    vec3 bitangent        // Bitangent vector (perpendicular to tangent and normal)
) {
    // Compute the half-vector (H) between view and light directions
    vec3 halfVector = normalize(viewDir + lightDir);

    // Transform the half-vector into tangent space
    vec3 halfVectorTS = vec3(
        dot(halfVector, tangent),
        dot(halfVector, bitangent),
        dot(halfVector, normal)
    );

    // Transform the normal into tangent space
    vec3 normalTS = vec3(0.0, 0.0, 1.0);

    // Compute the anisotropic GGX distribution
    float D = 0.0; // Default value for the distribution
    {
        float aX = roughnessX;
        float aY = roughnessY;

        float hX = halfVectorTS.x / aX;
        float hY = halfVectorTS.y / aY;
        float hZ = halfVectorTS.z;

        float hZ2 = hZ * hZ;
        float normFactor = (hX * hX + hY * hY) / hZ2 + 1.0;
        float denom = 3.14159265359 * aX * aY * hZ2 * hZ2;

        D = 1.0 / (denom * normFactor * normFactor);
    }

    // Fresnel term using Schlick's approximation
    vec3 F0 = vec3(1.0);
    float dotVH = max(dot(viewDir, halfVector), 0.0);
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - dotVH, 5.0);

    // Geometry term using Smith's GGX model
    float G = 0.0; // Default value for the geometry term
    {
        float k = 0.5; // Approximated k factor for anisotropic surfaces
        float geomV = dot(normal, viewDir) / (dot(normal, viewDir) * (1.0 - k) + k);
        float geomL = dot(normal, lightDir) / (dot(normal, lightDir) * (1.0 - k) + k);
        G = geomV * geomL;
    }

    // Combine the terms into the specular reflection
    float NdotL = max(dot(normal, lightDir), 0.0);
    float NdotV = max(dot(normal, viewDir), 0.0);

    float specular = (D * F.x * G) / (4.0 * NdotL * NdotV + 0.0001);
    return specular * NdotL; // Multiply by NdotL for energy conservation
}

// following functions are copies of UE4
// for computing cook-torrance specular lighting terms

float D_blinn(in float roughness, in float NdH)
{
	float m = roughness * roughness;
	float m2 = m * m;
	float n = 2.0 / m2 - 2.0;
	return (n + 2.0) / (2.0 * PI) * pow(NdH, n);
}

float D_beckmann(in float roughness, in float NdH)
{
	float m = roughness * roughness;
	float m2 = m * m;
	float NdH2 = NdH * NdH;
	return exp((NdH2 - 1.0) / (m2 * NdH2)) / (PI * m2 * NdH2 * NdH2);
}

float D_GGX(in float roughness, in float NdH)
{
	float m = roughness * roughness;
	float m2 = m * m;
	float d = (NdH * m2 - NdH) * NdH + 1.0;
	return m2 / (PI * d * d);
}

float G_schlick(in float roughness, in float NdV, in float NdL)
{
	float k = roughness * roughness * 0.5;
	float V = NdV * (1.0 - k) + k;
	float L = NdL * (1.0 - k) + k;
	return 0.25 / (V * L);
}

#define COOK_BECKMANN	1

// cook-torrance specular calculation                      
vec3 cooktorrance_specular(in float NdL, in float NdV, in float NdH, in vec3 specular, in float roughness)
{
#ifdef COOK_BLINN
	float D = D_blinn(roughness, NdH);
#endif

#ifdef COOK_BECKMANN
	float D = D_beckmann(roughness, NdH);
#endif

#ifdef COOK_GGX
	float D = D_GGX(roughness, NdH);
#endif

	float G = G_schlick(roughness, NdV, NdL);
	float rim = mix(1.0 - roughness * 1.0 * 0.9, 1.0, NdV); // material.w

	return (1.0 / rim) * specular * G * D;
}

vec3 skinSpecular(vec3 lightDir, vec3 viewDir, vec3 normal, float roughness1) {
    
    vec3 eyeVec = viewDir;
    // Compute half-vector
    vec3 halfVector = normalize(lightDir + eyeVec);
    
    // Cosine of angles
    float NdotV = max(dot(normal, viewDir), 0.0);
    float NdotL = max(dot(normal, lightDir), 0.0);
    float NdotH = max(dot(normal, halfVector), 0.0);
    float VdotH = max(dot(eyeVec, halfVector), 0.0);
    
    if (NdotL <= 0.0 || NdotV <= 0.0) {
        return vec3(0.0); // No light contribution if light or view are behind the surface
    }

    float f = 0.5 * abs( dot( normal, normalize(viewDir) ) );
    vec3 specfresnel = vec3(f);
    vec3 ct = cooktorrance_specular(NdotL, NdotV, NdotH, specfresnel, roughness1);
    return max(ct, vec3(0.0));
}

void evalDirLighting(in LIGHTINFOS info, inout LIGHTRES result)
{
	for (int i=0; i<numberOfDirLights; ++i)
	{
		vec3 lightDir = -normalize(dirLightsBuffer.lights[i].dir.xyz);
		
		float intensity = dirLightsBuffer.lights[i].attenuations.w;
		float ndotl = max(0.0, dot( info.normal, lightDir ) );
		
		float shadow = 1.0;
		float shadowLayer = dirLightsBuffer.lights[i].pad_castSpecularOnObject_shadowMapLayer.w;
		if (shadowLayer >= 0.0)
		{
			vec4 shadowCoord = dirLightsBuffer.lights[i].shadowVP * vec4(info.worldPosition, 1.0);
			shadow = 1.0 - calculateSoftShadow(shadowCoord, shadowLayer, 9, 0.001);
		}

		result.diffContrib += ndotl * shadow * intensity * dirLightsBuffer.lights[i].color.rgb;
		
		if (dirLightsBuffer.lights[i].pad_castSpecularOnObject_shadowMapLayer.z < 0.0f)
			continue;
		
		if (materialBuffer.mat.specularType == SPECULAR_SKIN)
		{
			result.specContrib += ndotl * shadow * intensity * skinSpecular(lightDir, -info.viewDir, info.normal, info.specularInput.x);
		}
		else if (materialBuffer.mat.specularType == SPECULAR_ANISO)
		{
			float roughnessX = materialBuffer.mat.roughnessX;
			float roughnessY = materialBuffer.mat.roughnessY;
			float specular = anisotropicSpecular(info.normal, -info.viewDir, lightDir, roughnessX, roughnessY, normalize(info.tangent), normalize(info.bitangent));
			specular = clamp(specular, 0.0, 1.0);
			result.specContrib += specular * shadow * intensity * dirLightsBuffer.lights[i].color.xyz;
		}
		else
		{
			float specular = phongSpecular(info.normal, -info.viewDir, lightDir, info.shininess);
			specular = clamp(specular, 0.0, 1.0);
			result.specContrib += specular * shadow * intensity * dirLightsBuffer.lights[i].color.xyz;
		}
	}
}

void doLight(in LIGHTINFOS info, in TLight light, inout vec3 diffContrib, inout vec3 specContrib)
{
	vec3 lightDir = light.position.xyz - info.position;
	float dist = length(lightDir);
	lightDir = normalize(lightDir);
	float inner = 0.0;
	
	vec3 normal = info.normal;
	// double sided lighting
	//if (normal.z <= 0.0) normal = -normal;
	
	float ndotL = max(dot(normal, lightDir),0.0);
	
	float radius = light.color.w;
	float att = max(1.0 - max(0.0, (dist - inner) / (radius - inner)), 0.0);
	if (dist > radius)
		att = 0.0f;
	
	// And last but not least, figure out the spot factor ...
	float spotFactor = 1.0;
	
	if( light.position.w == LIGHT_TYPE_SPOT )
	{
		vec3 spotLightDir = normalize(light.dir.xyz);
		spotFactor = max(0.0,  (dot( -lightDir, spotLightDir ) - light.dir.w) / (1.0 - light.dir.w) );
	}

	float shadow = 1.0;
	float shadowLayer = light.pad_castSpecularOnObject_shadowMapLayer.w;
	if (shadowLayer >= 0.0)
	{
		vec4 shadowCoord = light.shadowVP * vec4(info.worldPosition, 1.0);
		shadow = 1.0 - calculateShadow(shadowCoord, shadowLayer);
	}

	float factor = light.attenuations.w * att * spotFactor * shadow;

	//
	
	diffContrib += ndotL * factor * light.color.xyz;
	
	if (light.pad_castSpecularOnObject_shadowMapLayer.z < 0.0f)
		return;
	
	if (materialBuffer.mat.specularType == SPECULAR_SKIN)
	{
		specContrib += ndotL * factor * skinSpecular(lightDir, -info.viewDir, info.normal, info.specularInput.x);
	}
	else if (materialBuffer.mat.specularType == SPECULAR_ANISO)
	{
		float roughnessX = materialBuffer.mat.roughnessX;
		float roughnessY = materialBuffer.mat.roughnessY;
		float specular = anisotropicSpecular(normal, -info.viewDir, lightDir, roughnessX, roughnessY, normalize(info.tangent), normalize(info.bitangent));
		specular = clamp(specular, 0.0, 1.0);
		specContrib += specular * factor * light.color.xyz;
	}
	else
	{
		float specular = phongSpecular(normal, -info.viewDir, lightDir, info.shininess);
		specular = clamp(specular, 0.0, 1.0);
		specContrib += specular * factor * light.color.xyz;
	}
}

void evalLighting(in LIGHTINFOS info, in int count, inout LIGHTRES result)
{
	vec3 diffContrib = vec3(0.0);
	vec3 specContrib = vec3(0.0);
	
	for (int i=0; i<count; ++i)
	{
		// compute and accumulate shading.
		doLight(info, lightsBuffer.lights[i], diffContrib, specContrib);
	}
	
	result.diffContrib += diffContrib;
	result.specContrib += specContrib;
}

void ApplyRim(in vec3 Nn, in vec3 inPw, in vec4 rimOptions, in vec4 rimColor, inout vec4 difColor)
{
	if( rimOptions.x > 0. ) {
		float f = rimOptions.y * abs( dot( Nn, normalize(inPw) ) );
		f = rimOptions.x * ( 1. - smoothstep( 0.0, 1., f ) );
		
		difColor.rgb = mix(difColor.rgb, rimColor.rgb, f * difColor.a);
	}
}

vec3 ApplyReflection( in vec3 inWV, in vec3 n_eye )
{
	vec3 reflColor = materialBuffer.mat.reflectColor.rgb;
	
	if (materialBuffer.mat.useReflect > 0.0f)
	{
		
		vec3 PositionWS = inWV;
		vec3 NormalWS = normalize(n_eye);
		
		// calculations are done in eye space... so the eyePos is in vec3(0.0)
		vec3 dirWS = normalize(PositionWS); // PositionWS - eyePos
		
		vec3 reflection = normalize(reflect(dirWS, NormalWS)); // inWV - eyePos
		
		float yaw = .5 - atan( reflection.x, -reflection.z ) / ( 2.0 * PI );
		float pitch = .5 - asin( reflection.y ) / PI;
		
		reflColor.rgb = texture2D(samplerReflect, vec2(1.0-pitch, 1.0-yaw)).rgb;
	}

	return reflColor * materialBuffer.mat.reflectColor.w;
}

//////////////////////////////////////////////////////////////////
//

	

void main (void)
{
	//vec3 Nn = normalize(inNw);
	vec3 Vd = inWV; // n_eye - inWV;
	vec3 Vn = normalize(Vd);
	
	vec2 tx = inTC.st;
	vec4 color = materialBuffer.mat.diffuseColor;
	
	if (materialBuffer.mat.useDiffuse > 0.0)
	{
		vec4 coords = materialBuffer.mat.diffuseTransform * vec4(tx.x, tx.y, 0.0, 1.0);
		color = texture2D(samplerDiffuse, coords.st);
		
		if (switchAlbedoTosRGB > 0.0)
		{
			color.rgb = pow(color.rgb, vec3(2.2) );
		}
	}
	
	if (materialBuffer.mat.useTransparency > 0.0)
	{
		vec4 coords = materialBuffer.mat.transparencyTransform * vec4(tx.x, tx.y, 0.0, 1.0);
		color.a = materialBuffer.mat.transparencyColor.a * texture2D(samplerTransparency, coords.st).a;
	}
	
	// Compute normal.
	vec3 Nn = normalize(inNw);
	vec3 binormal = cross(normalize(inTangent), Nn);
	
	if (materialBuffer.mat.useNormalmap > 0.0f)
	{
		vec4 coords = materialBuffer.mat.normalTransform * vec4(tx.x, tx.y, 0.0, 1.0);
		vec4 nmColor = texture2D(samplerNormal, coords.st);
		
		nmColor.rgb = (nmColor.rgb - vec3(0.5f,0.5f,0.5f)) * vec3(2.0f,2.0f,2.0f);
		vec3 newNn = normalize(nmColor.x * inTangent + nmColor.y * binormal + nmColor.z * Nn);
		Nn = mix(Nn, newNn, materialBuffer.mat.useNormalmap);
	}
	
	// MatCap
	if (useMatCap > 0.0)
	{
		vec3 r = reflect( normalize(inPw.xyz), Nn );
		float m = 2.0 * sqrt( r.x * r.x + r.y * r.y + ( r.z + 1.0 ) * ( r.z+1.0 ) );
		vec2 vN = vec2( r.x / m + 0.5,  r.y / m + 0.5 );
			
		vec4 matColor = texture( samplerMatCap, vN );
		color.rgb *= matColor.rgb;
	}
	
	// Lightmap
	if (useLightmap > 0.0)
	{
		vec3 lightmap = color.rgb * texture2D(samplerDetail, inTC2.st).rgb;
		color.rgb = mix(color.rgb, lightmap, useLightmap);
	}
	
	// specular channel map
	float specularFactor = materialBuffer.mat.specularColor.w;
	vec3 specularColor = materialBuffer.mat.specularColor.rgb;

	if (materialBuffer.mat.useSpecular > 0.0)
	{
		vec4 coords = materialBuffer.mat.specularTransform * vec4(tx.x, tx.y, 0.0, 1.0);
		specularColor = texture2D(samplerSpecular, coords.st).rgb;
	}

	//
	// calculate lighting indices
	
	LIGHTINFOS lInfo;
	lInfo.viewDistance = Vd;
	lInfo.viewDir = normalize(inWV - inViewPos);
	lInfo.normal = Nn;
	lInfo.tangent = normalize(inTangent);
	lInfo.bitangent = binormal;
	lInfo.worldPosition = inWV;
	lInfo.position = inPw;
	lInfo.shininess = materialBuffer.mat.specExp;
	lInfo.specularInput = vec4(specularColor, specularFactor);

	LIGHTRES	lResult;
	lResult.ambientContrib = vec3(0.0);
	lResult.diffContrib = vec3(0.0);
	lResult.specContrib = vec3(0.0);
	
	if (numberOfDirLights > 0)
	{
		evalDirLighting(lInfo, lResult);
	}
	if (numberOfPointLights > 0)
	{
		evalLighting(lInfo, numberOfPointLights, lResult);
	}

	float difFactor = clamp(materialBuffer.mat.diffuseColor.w, 0.0, 1.0);
	vec3 ambientColor = materialBuffer.mat.ambientColor.rgb * globalAmbientLight.rgb * materialBuffer.mat.ambientColor.w;
	vec3 emissiveColor = materialBuffer.mat.emissiveColor.a * materialBuffer.mat.emissiveColor.rgb;
	vec3 difColor = lResult.diffContrib * difFactor;
	
	color *= vec4(ambientColor + difColor, materialBuffer.mat.shaderTransparency); // ambientColor +
	color.rgb = color.rgb + materialBuffer.mat.emissiveColor.rgb * materialBuffer.mat.emissiveColor.w;
	color.rgb += color.a * lResult.specContrib;

	ApplyRim(Nn, inPw, rimOptions, rimColor, color);
	
	//
	vec3 reflColor = ApplyReflection( inWV, n_eye );
	color.rgb += lResult.diffContrib * reflColor;
	
	color = clamp(color, 0.0, 1.0);

	if (switchAlbedoTosRGB > 0.0)
	{
		// linear to sRGB
		color.rgb =  pow( color.rgb, vec3(1.0 / 2.2));
	}
	
	// apply fog
	color.rgb = mix(color.rgb, fogColor.rgb, fogColor.w * fogAmount);
	
	if (color.a < 0.5)
	{
		discard;
		return;
	}

	outColor = color;
}
