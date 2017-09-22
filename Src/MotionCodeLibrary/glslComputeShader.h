
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solokhin (Neill3d) 2014-2017
//  e-mail to: s@neill3d.com
//		www.neill3d.com
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
// store main shader pipeline functionality
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

	bool			mStatus;	// compile and link shader status


	
	bool checkCompileStatus(GLuint shader, const char *shadername)
	{
		GLint  compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
			std::cerr << shadername << " failed to compile:" << std::endl;
			GLint  logSize;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
			char* logMsg = new char[logSize];
			glGetShaderInfoLog(shader, logSize, NULL, logMsg);
			std::cerr << logMsg << std::endl;
			delete[] logMsg;

			return false;
		}
		return true;
	}

	bool checkLinkStatus(GLuint program, const char * programName)
	{
		GLint  linked;
		glGetProgramiv(program, GL_LINK_STATUS, &linked);
		if (!linked)
		{
			std::cerr << "Shader program " << programName << " failed to link" << std::endl;
			GLint  logSize;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
			char* logMsg = new char[logSize];
			glGetProgramInfoLog(program, logSize, NULL, logMsg);
			std::cerr << logMsg << std::endl;
			delete[] logMsg;
			system("Pause");
			// exit(0);
			return false;
		}
		return true;
	}

	bool loadComputeShaderFromBuffer(const char* buffer, const char *shaderName, const GLuint shaderid, const GLuint programid)
	{
		const GLcharARB*  bufferARB = buffer;

		//GLuint shaderCompute = glCreateShader(GL_COMPUTE_SHADER);
		GLuint shaderCompute = shaderid;
		glShaderSource(shaderCompute, 1, &bufferARB, NULL);
	
		if (GLEW_ARB_shading_language_include)
		{
			std::string rootPath = "/";
			const char *SourceString = rootPath.c_str();
			glCompileShaderIncludeARB(shaderCompute, 1, &SourceString, NULL);
		}
		else
		{
			glCompileShader(shaderCompute);
		}

		if (false == checkCompileStatus(shaderCompute, shaderName) )
			return false;
		
		//GLuint programCompute = glCreateProgram();
		//glAttachShader(programCompute, shaderCompute);
		GLuint programCompute = programid;
		glLinkProgram(programCompute);
		if (false == checkLinkStatus(programCompute, shaderName) )
			return false;
		/*
		shaderid = shaderCompute;
		programid = programCompute;
		*/
		return true;
	}

	bool loadComputeShader(const char* computeShaderName, const GLuint shaderid, const GLuint programid)
	{
		//shaderid = 0;
		//programid = 0;
		bool lSuccess = false;

		FILE *fp = nullptr;
		fopen_s(&fp, computeShaderName, "r");
		if (fp)
		{
			fseek(fp, 0, SEEK_END);
			size_t fileLen = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			char  *buffer = new char[ fileLen + 1 ];
		
			GLint   len = (GLint) fileLen;

			// read shader from file
			memset( &buffer[0], 0, sizeof(char)*(len + 1) );
		
			size_t readlen = fread(buffer, sizeof(char), fileLen, fp);
  
			// trick to zero all outside memory
			memset( &buffer[readlen], 0, sizeof(char)*(len + 1 - readlen) );

			if (readlen == 0) //(readlen != len)
			{
				//ERR("glsl shader file size" );
				fclose(fp);
				return 0;
			}

			if (true == loadComputeShaderFromBuffer(buffer, computeShaderName, shaderid, programid) )
			{
				lSuccess = true;
			}

			if (buffer) {
				delete[] buffer;
				buffer = NULL;
			}

			fclose(fp);
		}

		return lSuccess;
	}


};