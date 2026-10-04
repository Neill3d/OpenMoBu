
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

		const std::filesystem::path debugName = shaderName
			? std::filesystem::path(AnsiToWide(shaderName))
			: std::filesystem::path();

		if (!loadComputeShaderFromBuffer(bufferData, debugName, mShader, mProgram))
		{
			Clear();
			return false;
		}
		
	}

	return true;
}

bool CComputeProgram::PrepProgram(const char *filename)
{
	if (mProgram != 0 && mShader != 0)
		return false;

	Clear();

	mProgram = glCreateProgram();
	mShader = glCreateShader(GL_COMPUTE_SHADER);
	glAttachShader(mProgram, mShader);

	const auto shaderPath = FindEffectLocation(std::filesystem::path(AnsiToWide(filename)));

	if (!shaderPath)
	{
		Clear();
		return false;
	}

	if (!loadComputeShader(*shaderPath, mShader, mProgram))
	{
		Clear();
		return false;
	}

	return true;
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

bool CComputeProgram::checkCompileStatus(GLuint shader, const std::filesystem::path& shaderName)
{
	GLint  compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		LOGE("%ls failed to compile:", shaderName.c_str());
		GLint  logSize;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
		char* logMsg = new char[logSize+1];
		memset(logMsg, 0, sizeof(char) * (logSize + 1));
		glGetShaderInfoLog(shader, logSize, nullptr, logMsg);
		LOGE("%s", logMsg ? logMsg : "");
		delete[] logMsg;

		return false;
	}
	return true;
}

bool CComputeProgram::checkLinkStatus(GLuint program, const std::filesystem::path& programName)
{
	GLint  linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		LOGE("Shader program %ls failed to link", programName.c_str());
		GLint  logSize;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
		char* logMsg = new char[logSize + 1];
		memset(logMsg, 0, sizeof(char) * (logSize + 1));
		glGetProgramInfoLog(program, logSize, nullptr, logMsg);
		LOGE("%s", logMsg ? logMsg : "");
		delete[] logMsg;
		
		return false;
	}
	return true;
}

bool CComputeProgram::loadComputeShaderFromBuffer(const char* buffer, const std::filesystem::path& shaderName, const GLuint shaderid, const GLuint programid)
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

bool CComputeProgram::loadComputeShader(const std::filesystem::path& computeShaderPath, const GLuint shaderid, const GLuint programid)
{
	FileReadScope FileRead(computeShaderPath);

	if (!FileRead.Get())
		return false;

	const size_t fileLen = FileRead.GetFileSize();
	std::vector<char> buffer(fileLen + 1, 0);
	
	// read shader from file
	const size_t readlen = fread(buffer.data(), sizeof(char), fileLen, FileRead.Get());

	if (readlen == 0)
	{
		LOGE("glsl shader %ls has a zero file size", computeShaderPath.c_str());
		return false;
	}

	// trick to zero all outside memory
	memset(&buffer[readlen], 0, sizeof(char) * (fileLen + 1 - readlen));

	return loadComputeShaderFromBuffer(buffer.data(), computeShaderPath, shaderid, programid);
}
