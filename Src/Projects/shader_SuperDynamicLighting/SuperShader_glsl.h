
#pragma once

// SuperShader_glsl.h
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include <GL\glew.h>
#include <glm/glm.hpp>

//////////////////////////////////////

#define	MAX_NUMBER_OF_LIGHTS			16
#define MAX_NUMBER_OF_CASCADED_SPLITS	4

#define DIFFUSE_LAMBERT					0.0f
#define DIFFUSE_OREN_NAYAR				1.0f

#define SPECULAR_PHONG					0.0f
#define SPECULAR_ANISO					1.0f
#define SPECULAR_SKIN					2.0f

#define	LIGHT_TYPE_DIRECTION			0.0f
#define	LIGHT_TYPE_POINT				1.0f
#define LIGHT_TYPE_SPOT					2.0f
#define LIGHT_TYPE_AREA					3.0f	// TODO: add support for that

// light data should be computer in eye space, means for specified camera
//
struct TLight
{
	glm::vec4 	attenuations;

	glm::vec3 	position;
	float		type;

	glm::vec3 	dir;
	float		spotAngle;

	glm::vec3	color;
	float		radius;

	float		pad0;
	float		pad1;
	float		castSpecularOnObject;
	float		shadowMapLayer{ -1.0f };

	glm::mat4		shadowVP;	// view projection matrix of a shadow map

	static void ConstructDefaultLight0(TLight& light, bool inEyeSpace, const glm::mat4& lViewMatrix, const glm::mat4& lViewRotationMatrix);
	static void ConstructDefaultLight1(TLight& light, bool inEyeSpace, const glm::mat4& lViewMatrix, const glm::mat4& lViewRotationMatrix);
};

struct TShadow
{
	float		lightType;
	float		shadowMapLayer{ -1.0f };
	float		shadowMapSize;
	float		shadowPCFKernelSize;
	
	glm::mat4		shadowVP;	// view projection matrix of a shadow map
	glm::vec4	shadowIndex;	// index and count in the shadow matrix array
	glm::vec4	normalizedFarPlanes;	// for cascaded shadows

};

struct TTransform
{
	glm::mat4 	m4_World;
	glm::mat4	m4_View;
	glm::mat4	m4_Proj;
	glm::mat4	m4_Model;

	glm::mat4	normalMatrix;

	glm::vec4	eyePos;
};


struct TMaterial
{
	// textures for material channels
	int			ambientChannel;
	int			diffuseChannel;
	int			specularChannel;
	int			emissiveChannel; //
	int			transparencyChannel;
	int			normalmapChannel;
	int			reflectChannel;

	float		specExp; //

	float 		useDisplacement;
	float		useDiffuse;
	float		useSpecular;
	float		useEmissive; //
	float		useTransparency;
	float		useNormalmap;
	float		useReflect;

	//
	float 		shaderTransparency;

	float		diffuseType; // lambert or Oren-Nayar
	float		specularType; // phong, anisotropic or skin (cook torrance)
	float		roughnessX;
	float		roughnessY;
	
	//
	/// Current material
	//
	glm::vec4		emissiveColor;
	glm::vec4     	diffuseColor;
	glm::vec4     	ambientColor;
	glm::vec4		reflectColor;
	glm::vec4     	transparencyColor;
	glm::vec4     	specularColor;

	//
	glm::mat4		diffuseTransform;
	glm::mat4		transparencyTransform;
	glm::mat4		specularTransform;
	glm::mat4		normalTransform;
	glm::mat4		reflectTransform;
	// 480 in total
};