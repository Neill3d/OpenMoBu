
#pragma once

// SuperShader_glsl.h
/*
Sergei <Neill3d> Solokhin 2018-2020

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https ://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include <GL\glew.h>
#include "nv_math.h"

//////////////////////////////////////

#define	MAX_NUMBER_OF_LIGHTS			16
#define MAX_NUMBER_OF_CASCADED_SPLITS	4

#define	LIGHT_TYPE_DIRECTION			0.0f
#define	LIGHT_TYPE_POINT				1.0f
#define LIGHT_TYPE_SPOT					2.0f
#define LIGHT_TYPE_AREA					3.0f	// TODO: add support for that

// light data should be computer in eye space, means for specified camera
//
struct TLight
{
	vec4 		attenuations;

	vec3 		position;
	float		type;

	vec3 		dir;
	float		spotAngle;

	vec3 		color;
	float		radius;

	float		shadowMapLayer;
	float		shadowMapSize;
	float		shadowPCFKernelSize;
	float		castSpecularOnObject;

	//mat4		shadowVP;	// view projection matrix of a shadow map
	vec4		shadowIndex;	// index and count in the shadow matrix array
	vec4		normalizedFarPlanes;	// for cascaded shadows
};

struct TTransform
{
	mat4 	m4_World;
	mat4	m4_View;
	mat4	m4_Proj;
	mat4	m4_Model;

	mat4	normalMatrix;

	vec4	eyePos;
};


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

	float 		useDisplacement;
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