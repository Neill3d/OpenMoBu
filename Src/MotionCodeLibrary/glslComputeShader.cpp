
/////////////////////////////////////////////////////////////////////////////////////////
//
// License page - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/OpenMoBu
//
// Author Sergey Solokhin (Neill3d) 2014-2017
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////


#include "glslComputeShader.h"
#include "FileUtils.h"
//#include "graphics\CheckGLError_MOBU.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

CComputeProgram::CComputeProgram(const GLuint shaderid, const GLuint programid)
	: mShader(shaderid)
	, mProgram(programid)
{
	mStatus = false;
}

CComputeProgram::~CComputeProgram()
{
	Clear();
}

void CComputeProgram::Clear()
{
	if (mShader > 0)
	{
		if (mProgram > 0)
			glDetachShader(mProgram, mShader);
		glDeleteShader(mShader);
		mShader = 0;
	}
	if (mProgram > 0)
	{
		glDeleteProgram(mProgram);
		mProgram = 0;
	}
}

void CComputeProgram::CreateGLObjects()
{
	Clear();
	// TODO:
}

void CComputeProgram::ReCreateShaderObject()
{
	if (mProgram > 0 && mShader > 0)
	{
		glDetachShader(mProgram, mShader);
		glDeleteShader(mShader);

		mShader = glCreateShader(GL_COMPUTE_SHADER);
		glAttachShader(mProgram, mShader);
	}
}

bool CComputeProgram::PrepProgramFromBuffer(const char *bufferData, const char *shaderName)
{
	if (0 == mProgram || 0 == mShader)
	{
		Clear();

		mProgram = glCreateProgram();
		mShader = glCreateShader(GL_COMPUTE_SHADER);
		glAttachShader(mProgram, mShader);


		//
		
		if (false == loadComputeShaderFromBuffer(bufferData, shaderName, mShader, mProgram) )
		{
			Clear();
			return false;
		}
		
	}

	return true;
}

bool CComputeProgram::PrepProgram(const char *filename)
{
	if (0 == mProgram || 0 == mShader)
	{
		Clear();

		mProgram = glCreateProgram();
		mShader = glCreateShader(GL_COMPUTE_SHADER);
		glAttachShader(mProgram, mShader);


		//
		FBString effectPath, effectFullName;
		if (true == FindEffectLocation( filename, effectPath, effectFullName ) )
		{
			if (false == loadComputeShader(effectFullName, mShader, mProgram) )
			{
				Clear();
				return false;
			}

			return true;
		}
	}

	return false;
}

void CComputeProgram::Bind()
{
	if (mProgram > 0)
		glUseProgram(mProgram);
}

void CComputeProgram::UnBind()
{
	glUseProgram(0);
}

void CComputeProgram::DispatchPipeline(const int groups_x, const int groups_y, const int groups_z)
{
	glDispatchCompute(groups_x, groups_y, groups_z);
}
