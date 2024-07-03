
#pragma once

// SuperShader_glsl.h
/*
Sergei <Neill3d> Solokhin 2018

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
	nv::vec4 		attenuations;

	nv::vec3 		position;
	float		type;

	nv::vec3 		dir;
	float		spotAngle;

	nv::vec3 		color;
	float		radius;

	float		shadowMapLayer;
	float		shadowMapSize;
	float		shadowPCFKernelSize;
	float		castSpecularOnObject;

	//mat4		shadowVP;	// view projection matrix of a shadow map
	nv::vec4		shadowIndex;	// index and count in the shadow matrix array
	nv::vec4		normalizedFarPlanes;	// for cascaded shadows
};

struct TTransform
{
	nv::mat4 	m4_World;
	nv::mat4	m4_View;
	nv::mat4	m4_Proj;
	nv::mat4	m4_Model;

	nv::mat4	normalMatrix;

	nv::vec4	eyePos;
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
	nv::vec4		emissiveColor;
	nv::vec4     	diffuseColor;
	nv::vec4     	ambientColor;
	nv::vec4		reflectColor;
	nv::vec4     	transparencyColor;
	nv::vec4     	specularColor;

	//
	nv::mat4		diffuseTransform;
	nv::mat4		transparencyTransform;
	nv::mat4		specularTransform;
	nv::mat4		normalTransform;
	nv::mat4		reflectTransform;
	// 160 in total
};