
/////////////////////////////////////////////////////////////////////////////////////////
//
// License page - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/OpenMoBu
//
// Author Sergei Solokhin (Neill3d) 2014-2017
//  e-mail to: neill3d@gmail.com
// 
/////////////////////////////////////////////////////////////////////////////////////////


#include "glslComputeShader.h"
#include "FileUtils.h"
#include "Logger.h"

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

		if (!loadComputeShaderFromBuffer(bufferData, shaderName, mShader, mProgram) )
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
		if (FindEffectLocation( filename, effectPath, effectFullName ) )
		{
			if (!loadComputeShader(effectFullName, mShader, mProgram) )
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

bool CComputeProgram::checkCompileStatus(GLuint shader, const char* shadername)
{
	GLint  compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		LOGE("%s failed to compile:", shadername);
		GLint  logSize;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
		char* logMsg = new char[logSize];
		glGetShaderInfoLog(shader, logSize, nullptr, logMsg);
		LOGE(logMsg);
		delete[] logMsg;

		return false;
	}
	return true;
}

bool CComputeProgram::checkLinkStatus(GLuint program, const char* programName)
{
	GLint  linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		LOGE("Shader program %s failed to link", programName);
		GLint  logSize;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
		char* logMsg = new char[logSize];
		glGetProgramInfoLog(program, logSize, nullptr, logMsg);
		LOGE(logMsg);
		delete[] logMsg;
		
		return false;
	}
	return true;
}

bool CComputeProgram::loadComputeShaderFromBuffer(const char* buffer, const char* shaderName, const GLuint shaderid, const GLuint programid)
{
	const GLcharARB* bufferARB = buffer;

	GLuint shaderCompute = shaderid;
	glShaderSource(shaderCompute, 1, &bufferARB, NULL);

	if (GLEW_ARB_shading_language_include)
	{
		std::string rootPath = "/";
		const char* SourceString = rootPath.c_str();
		glCompileShaderIncludeARB(shaderCompute, 1, &SourceString, NULL);
	}
	else
	{
		glCompileShader(shaderCompute);
	}

	if (!checkCompileStatus(shaderCompute, shaderName))
		return false;

	GLuint programCompute = programid;
	glLinkProgram(programCompute);
	if (!checkLinkStatus(programCompute, shaderName))
		return false;
	
	return true;
}

bool CComputeProgram::loadComputeShader(const char* computeShaderName, const GLuint shaderid, const GLuint programid)
{
	bool lSuccess = false;

	FILE* fp = nullptr;
	fopen_s(&fp, computeShaderName, "r");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		size_t fileLen = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		char* buffer = new char[fileLen + 1];
		if (!buffer)
		{
			fclose(fp);
			return false;
		}

		GLint   len = (GLint)fileLen;

		// read shader from file
		memset(&buffer[0], 0, sizeof(char) * (len + 1));

		size_t readlen = fread(buffer, sizeof(char), fileLen, fp);

		// trick to zero all outside memory
		memset(&buffer[readlen], 0, sizeof(char) * (len + 1 - readlen));

		if (readlen == 0)
		{
			LOGE("glsl shader %s has a zero file size", computeShaderName );
			fclose(fp);
			delete[] buffer;

			return 0;
		}

		if (loadComputeShaderFromBuffer(buffer, computeShaderName, shaderid, programid))
		{
			lSuccess = true;
		}

		delete[] buffer;
		fclose(fp);
	}

	return lSuccess;
}