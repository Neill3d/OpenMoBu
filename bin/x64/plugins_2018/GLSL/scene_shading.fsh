//
// Fragment shader - Model Phong Shading 
//
//	Post Processing Toolkit
//	 Created by Sergey <Neill3d> Solokhin (c) 2018
//	Special for Les Androids Associes
//

#version 430 compatibility

#define	PI	3.14159265358979323846264

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
	
	float		shadowMapLayer;
	float		shadowMapSize;
	float		shadowPCFKernelSize;
	float		castSpecularOnObject;
	// 16
	//mat4		shadowVP;	// view projection matrix of a shadow map
	vec4		shadowIndex;	// index and count in the shadow matrix array
	vec4		normalizedFarPlanes;	// for cascaded shadows
	// 64
	
	// total - 144 bytes
};

struct LIGHTINFOS
{
	mat3	tangentMatrix;
	vec3	worldPosition;
	vec3 	position;
	vec3 	viewDistance;
	vec3 	viewDir;	
	vec3 	normal;	
	vec2	uv;
	float 	shininess;
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

/////////////////////////////////////////////////////

uniform int			numberOfDirLights;
uniform int			numberOfPointLights;

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

//////////////////////////////////////////////////////////////////////////////
// lights

void evalDirLighting(in LIGHTINFOS info, in int count, inout LIGHTRES result)
{
	
	for (int i=0; i<count; ++i)
	{
		vec3 dir = -normalize(dirLightsBuffer.lights[i].dir.xyz);
		
		float intensity = dirLightsBuffer.lights[i].attenuations.w;
		float ndotl = max(0.0, dot( info.normal, dir ) );
		
		result.diffContrib += ndotl * intensity * dirLightsBuffer.lights[i].color.rgb;
	}
}
 
void doLight(in LIGHTINFOS info, in TLight light, in float shadow, inout vec3 diffContrib, inout vec3 specContrib)
{
	vec3 lightDir = light.position.xyz - info.position;
	float dist = length(lightDir);
	lightDir = normalize(lightDir);
	float inner = 0.0;
	
	//lightDir = info.tangentMatrix * lightDir;
	
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
	float specular = 0.0;
	
	//if (pLight->castSpecularOnObject > 0.0f)
	{
		vec3 eyeVec = normalize( -info.position); // -info.viewDir
		//eyeVec = info.tangentMatrix * eyeVec;
		vec3 V = reflect(-lightDir, normal);
		//vec3 halfVec = (eyeVec + lightDir)/2.0; // view + light
		//specular = pow( max(0.0, dot( normal, halfVec ) ), 64.0 );
		
		specular = pow( max(0.0, dot( V, eyeVec ) ), info.shininess );
	}
	//specular = 0.5;
	specular = clamp(specular, 0.0, 1.0);
	
	if( light.position.w == LIGHT_TYPE_SPOT )
	{
		vec3 spotLightDir = normalize(light.dir.xyz);
		//spotLightDir = info.tangentMatrix * spotLightDir;
		spotFactor = max(0.0,  (dot( -lightDir, spotLightDir ) - light.dir.w) / (1.0 - light.dir.w) );
	}

	//float lshadow = spotFactor * mix(1.0, shadow, ndotL);

	float factor = light.attenuations.w * att * spotFactor;

	//
	
	diffContrib += ndotL * factor * light.color.xyz; // * lshadow
	specContrib += specular * factor * light.color.xyz;
	//specContrib = vec3(specular);
}

void evalLighting(in LIGHTINFOS info, in int count, inout LIGHTRES result)
{
	vec3 diffContrib = vec3(0.0);
	vec3 specContrib = vec3(0.0);
	
	float shadow = 1.0;
	
	for (int i=0; i<count; ++i)
	{
		// compute and accumulate shading.
		doLight(info, lightsBuffer.lights[i], shadow, diffContrib, specContrib);
	}
	
	result.diffContrib += diffContrib;
	result.specContrib += specContrib;
}

void ApplyRim(in vec3 Nn, in vec3 inPw, in vec4 rimOptions, in vec4 rimColor, inout vec4 difColor)
{
	if( rimOptions.x > 0. ) {
		float f = rimOptions.y * abs( dot( Nn, normalize(inPw) ) );
		f = rimOptions.x * ( 1. - smoothstep( 0.0, 1., f ) );
		
		difColor.rgb = mix(difColor.rgb, rimColor.rgb, f);
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

	layout(location=0) smooth in vec3 inNw;
	layout(location=1) smooth in vec2 inTC;		// input 8 bytes
	layout(location=2) smooth in vec3 inPw;
	layout(location=3) smooth in vec3 inWV;
	
	layout(location=4) smooth in vec3 n_eye; // eyePos
	layout(location=5) smooth in vec3 	inTangent;
	layout(location=6) smooth in vec2 	inTC2;
	
	layout(location=7) smooth in float fogAmount;
	
	layout(location=0) out vec4 	outColor;

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
	
	//
	// calculate lighting indices
	
	LIGHTINFOS lInfo;
	lInfo.viewDistance = Vd;
	lInfo.viewDir = Vn;
	lInfo.normal = Nn;
	lInfo.worldPosition = inWV;
	lInfo.position = inPw;
	lInfo.shininess = materialBuffer.mat.specExp;
	
	LIGHTRES	lResult;
	lResult.ambientContrib = vec3(0.0);
	lResult.diffContrib = vec3(0.0);
	lResult.specContrib = vec3(0.0);
	//lResult.R = vec3(0.0);
	
	if (numberOfDirLights > 0)
	{
		evalDirLighting(lInfo, numberOfDirLights, lResult);
	}
	if (numberOfPointLights > 0)
	{
		evalLighting(lInfo, numberOfPointLights, lResult);
	}
	
	float difFactor = clamp(materialBuffer.mat.diffuseColor.w, 0.0, 1.0);
	vec3 ambientColor = materialBuffer.mat.ambientColor.rgb * globalAmbientLight.rgb * materialBuffer.mat.ambientColor.w;
	//difColor += vec4(ambientColor, theShader->transparency);
	color *= vec4(ambientColor + lResult.diffContrib * difFactor, materialBuffer.mat.shaderTransparency); // ambientColor +
	
	//
	/*theMaterial->emissiveColor.w * theMaterial->emissiveColor.xyz +*/
	
	float specularFactor = materialBuffer.mat.specularColor.w;
	
	if (materialBuffer.mat.useSpecular > 0.0)
	{
		vec4 coords = materialBuffer.mat.specularTransform * vec4(tx.x, tx.y, 0.0, 1.0);
		specularFactor *= texture2D(samplerSpecular, coords.st).r;
	}
	
	color.rgb =  color.rgb + specularFactor * materialBuffer.mat.specularColor.rgb * lResult.specContrib;
	
	//color = vec4(1.0, 0.0, 0.0, 1.0);
	color.rgb = lResult.diffContrib * color.rgb;
	
	ApplyRim(Nn, inPw, rimOptions, rimColor, color);
	
	//
	vec3 reflColor = ApplyReflection( inWV, n_eye );
	color.rgb += lResult.diffContrib * reflColor;
	
	if (switchAlbedoTosRGB > 0.0)
	{
		// linear to sRGB
		outColor.rgb =  pow( outColor.rgb, vec3(1.0 / 2.2));
	}
	
	// apply fog
	color.rgb = mix(color.rgb, fogColor.rgb, fogColor.w * fogAmount);
	
	outColor = color;
}
