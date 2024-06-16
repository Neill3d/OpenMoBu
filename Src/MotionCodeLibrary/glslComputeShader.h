
/////////////////////////////////////////////////////////////////////////////////////////
//
// License page - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/OpenMoBu
//
// Author Sergey Solokhin (Neill3d) 2014-2018
//  e-mail to: neill3d@gmail.com
// 
/////////////////////////////////////////////////////////////////////////////////////////


// glsl compute shader for calculating blendshape normals

#pragma once

#include <GL\glew.h>

#include <vector>
#include <map>

#include <iostream>
#include <functional>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////////////////
/// store main compute glsl shader pipeline functionality
class CComputeProgram
{
public:

	//! a constructor
	CComputeProgram(const GLuint shaderid=0, const GLuint programid=0);

	//! a destructor
	~CComputeProgram();

	void Clear();


	bool PrepProgramFromBuffer(const char *bufferData, const char *shaderName);
	bool PrepProgram(const char *filename = "\\GLSL_CS\\computeNormals.cs");

	void CreateGLObjects();
	void ReCreateShaderObject();

	void Bind();
	void UnBind();

	void DispatchPipeline(const int groups_x, const int groups_y, const int groups_z);

	const GLuint		GetProgramId() const
	{
		return mProgram;
	}
	const GLuint GetShaderId() const
	{
		return mShader;
	}

	bool IsOk() const {
		return mStatus;
	}

protected:

	GLuint			mShader;
	GLuint			mProgram;

	bool			mStatus;	//!< compile and link shader status

	bool checkCompileStatus(GLuint shader, const char* shadername);

	bool checkLinkStatus(GLuint program, const char* programName);

	bool loadComputeShaderFromBuffer(const char* buffer, const char* shaderName, const GLuint shaderid, const GLuint programid);

	bool loadComputeShader(const char* computeShaderName, const GLuint shaderid, const GLuint programid);

};