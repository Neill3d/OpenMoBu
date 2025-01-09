
/*
Sergei <Neill3d> Solokhin 2018
		class is based on http://steps3d.narod.ru library
		Thanks for Alex Boreskov

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include <stdio.h>
#include "glslShaderProgram.h"
#include "CheckGLError.h"
#include "FileUtils.h"

#include <string>
#include <string_view>
#include <sstream>
#include <iostream>

//
extern void LOGI(const char* pFormatString, ...);
extern void LOGE(const char* pFormatString, ...);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                            GLSLShader

bool GLSLShaderProgram::PRINT_WARNINGS = true;

std::unordered_map<std::string, std::string, StringViewHash, StringViewEqual> GLSLShaderProgram::g_TextInsertions;


GLSLShaderProgram::GLSLShaderProgram() 
{}

void GLSLShaderProgram::AddTextInsertion(const char* insertion_keyword, const char* insertion_data)
{
	g_TextInsertions.emplace(insertion_keyword, insertion_data);
}

bool GLSLShaderProgram::AddTextInsertionFromFile(const char* insertion_keyword, const char* file_name)
{
	FileReadScope insertionFile(file_name);

	std::string insertionText;
	insertionFile.ReadString(insertionText);

	bool status = !insertionText.empty();
	g_TextInsertions.emplace(insertion_keyword, std::move(insertionText));
	return status;
}

bool GLSLShaderProgram::ReCompileShaders( const char* vertex_file, const char* fragment_file )
{	
	FileReadScope readFragment(fragment_file);

	if (FILE* fp = readFragment.Get())
	{
		if (fragment)
		{
			glDetachObjectARB(programObj, fragment);
			glDeleteObjectARB(fragment);
		}
		
		fragment = glCreateShaderObjectARB( GL_FRAGMENT_SHADER_ARB );
		if (!LoadShader( fragment, fp, fragment_file ) ) {
			return false;
		}

		// attach shader to program object
		glAttachObjectARB( programObj, fragment );

		GLint linked{ 0 };
		// link the program object and print out the info log
		glLinkProgramARB( programObj );

		glGetObjectParameterivARB( programObj, GL_OBJECT_LINK_STATUS_ARB, &linked );
		
		bool doPrint = PRINT_WARNINGS;
		if (doPrint && linked == GL_TRUE)
		{
			GLint       logLength = 0;
			glGetObjectParameterivARB(programObj, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLength);
			doPrint = logLength > 0;
		}

		if (linked == GL_FALSE || doPrint)
		{
			LOGI("[GLSLShader] Recompile status for vertex - %s, fragment - %s\n", vertex_file, fragment_file);
			LoadLog(programObj, nullptr);
		}
			
		return (linked != 0);
	}

	return false;
}

bool GLSLShaderProgram::LoadShaders( const char* vertex_file, const char* fragment_file )
{
	Free();

	{
		FileReadScope readVertex(vertex_file);

		if (FILE* fp = readVertex.Get())
		{
			vertex = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
			LoadShader(vertex, fp, vertex_file);
		}
	}

	{
		FileReadScope readFragment(fragment_file);

		if (FILE* fp = readFragment.Get())
		{
			fragment = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
			LoadShader(fragment, fp, fragment_file);
		}
	}

	if (vertex > 0 && fragment > 0)
	{
	  programObj = glCreateProgramObjectARB();
	  // check for errors
	  //if ( checkopenglerror ) return false;
	  // attach shader to program object
	  glAttachObjectARB( programObj, vertex );
	  // attach shader to program object
	  glAttachObjectARB( programObj, fragment );

	  GLint linked;
	  // link the program object and print out the info log
	  glLinkProgramARB( programObj );
	
	  CHECK_GL_ERROR();
	  //if ( checkopenglerror ) return false;

	  glGetObjectParameterivARB( programObj, GL_OBJECT_LINK_STATUS_ARB, &linked );

	  bool doPrint = PRINT_WARNINGS;
	  if (doPrint && linked == GL_TRUE)
	  {
		  GLint       logLength = 0;
		  glGetObjectParameterivARB(programObj, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLength);
		  doPrint = logLength > 0;
	  }

	  if (linked == GL_FALSE || doPrint)
	  {
		  LOGI("[GLSLShader ] link status for vertex - %s, fragment - %s\n", vertex_file, fragment_file);
		  LoadLog(programObj, nullptr);
	  }
	  
	  return (linked != 0);
	}

	return false;
}

bool GLSLShaderProgram::LoadShaders( GLhandleARB	_vertex, const char* fragment_file )
{
	Free();

	vertex = _vertex;

	{
		FileReadScope readFragment(fragment_file);

		if (FILE* fp = readFragment.Get())
		{
			fragment = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
			LoadShader(fragment, fp, fragment_file);
		}
	}

	if (vertex > 0 && fragment > 0)
	{

	  programObj = glCreateProgramObjectARB();
	  // check for errors
	  //if ( checkopenglerror ) return false;
	  // attach shader to program object
	  glAttachObjectARB( programObj, vertex );
	  // attach shader to program object
	  glAttachObjectARB( programObj, fragment );

	  GLint linked;
	  // link the program object and print out the info log
	  glLinkProgramARB( programObj );
	//checkopenglerror;
	  //if ( checkopenglerror ) return false;

	  glGetObjectParameterivARB( programObj, GL_OBJECT_LINK_STATUS_ARB, &linked );
	  
	  bool doPrint = PRINT_WARNINGS;
	  if (doPrint && linked == GL_TRUE)
	  {
		  GLint       logLength = 0;
		  glGetObjectParameterivARB(programObj, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLength);
		  doPrint = logLength > 0;
	  }

	  if (linked == GL_FALSE || doPrint)
	  {
		  LOGI("[GLSLShader ] link status for fragment - %s\n", fragment_file);
		  LoadLog(programObj, nullptr);
	  }
	  
	  return (linked != 0);
	}

	return false;
}



bool HasAnyInsertions(const std::string& inputText, const std::unordered_map<std::string, std::string, StringViewHash, StringViewEqual>& insertions)
{
	const std::string insertPrefix = "INSERT: ";

	if (inputText.find(insertPrefix) == std::string::npos)
		return false;

	for (const auto& key : insertions)
	{
		if (inputText.find(key.first) != std::string::npos)
			return true;
	}

	return false;
}

void ProcessTextWithInsertions(std::string& inputText, const std::unordered_map<std::string, std::string, StringViewHash, StringViewEqual>& insertions)
{
	std::istringstream inputStream(inputText);
	std::ostringstream outputStream;
	std::string line;

	const std::string insertPrefix = "INSERT: ";

	while (std::getline(inputStream, line))
	{
		size_t pos = line.rfind(insertPrefix);
		if (pos != std::string::npos) {

			size_t endPos = pos + insertPrefix.size() + 1;
			const size_t lineLen = line.size();

			while (endPos < lineLen && !std::isspace(line[endPos] && line[endPos] != '\n'))
			{
				++endPos;
			}
			
			std::string_view keyword(line.c_str() + pos, endPos - pos);
			auto it = insertions.find(keyword);
			if (it != end(insertions)) {
				outputStream << it->second << '\n';
			} else {
				outputStream << line << '\n';
			}
		}
		else {
			outputStream << line << '\n';
		}
	}
	inputText = outputStream.str();
}

bool GLSLShaderProgram::LoadShader( GLhandleARB shader, FILE *file, const char* debugName )
{
	if (!file)
	{
		LOGE("[GLSLShaderProgram::LoadShader] failed to read a file");
		return false;
	}
	
	const size_t headerLen = strnlen(mHeaderText, 256); // number of bytes in header

	fseek(file, 0, SEEK_END);
	const size_t fileLen = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (fileLen == 0) //(readlen != len)
	{
		LOGE("[GLSLShaderProgram::LoadShader] glsl shader %s has empty file size", debugName);
		return false;
	}

	// read shader from file
  
	const size_t expectedSize = (headerLen + fileLen + 1);

	std::string textBuffer;
	textBuffer.resize(expectedSize, 0);

	if (headerLen > 0)
	{
		strcpy_s(&textBuffer[0], headerLen, mHeaderText);
	}

	const size_t readlen = fread(&textBuffer[headerLen], sizeof(char), fileLen, file);

	if (readlen == 0) //(readlen != len)
	{
		LOGE("[GLSLShaderProgram::LoadShader] glsl shader %s has empty read size", debugName);
		return false;
	}

	// trick to zero all outside memory
	memset( &textBuffer[readlen+headerLen], 0, sizeof(char)*(fileLen + 1 - readlen) );

	if (HasAnyInsertions(textBuffer, g_TextInsertions))
	{

		ProcessTextWithInsertions(textBuffer, g_TextInsertions);
	}

	//
	// pass the code as the shader source

	GLint   len = static_cast<GLint>(textBuffer.size());
	GLint   compileStatus;

	const GLcharARB* bufferARB = textBuffer.data();

	glShaderSourceARB( shader, 1, &bufferARB, &len);

	//
	// compile shader

	glCompileShaderARB( shader );

	glGetObjectParameterivARB ( shader, GL_OBJECT_COMPILE_STATUS_ARB, &compileStatus );

	if (compileStatus == GL_FALSE || PRINT_WARNINGS)
	{
		LoadLog(shader, debugName);
	}
	
	return (compileStatus != GL_FALSE);
}

bool GLSLShaderProgram::LoadLog( GLhandleARB object, const char* debugName ) const
{
	constexpr int STACK_BUFFER_SIZE{ 2048 };

    GLint       logLength     = 0;
    GLsizei     charsWritten  = 0;
	GLcharARB   buffer[STACK_BUFFER_SIZE]{ 0 };
    GLcharARB*  infoLog;

    glGetObjectParameterivARB ( object, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLength );
/*
    if ( checkopenglerror )          // check for OpenGL errors
        return;
*/
    if ( logLength < 1 )
        return false;
    
	// try to avoid allocating buffer
	bool isAllocOnHeap = false;
    if (logLength >= STACK_BUFFER_SIZE)
    {
        infoLog = new GLcharARB[logLength + 1];
		memset(infoLog, 0, sizeof(GLcharARB) * (logLength + 1));
		isAllocOnHeap = true;
    }
    else
        infoLog = buffer;

    glGetInfoLogARB ( object, logLength, &charsWritten, infoLog );

	bool status = false;
	if ( strlen(infoLog) > 0 )
	{
		status = true;
		if (debugName)
		{
			LOGE("[GLSLShader] print info for %s\n", debugName);
		}
		LOGE( infoLog );
	}

    if ( isAllocOnHeap )
        delete [] infoLog;

	return status;
}

void GLSLShaderProgram::Bind() const
{
  if (programObj)
    glUseProgramObjectARB( programObj );
}

void GLSLShaderProgram::UnBind() const
{
  glUseProgramObjectARB( 0 );
}


void GLSLShaderProgram::Free()
{
  if (programObj) glDeleteObjectARB( programObj );
  if (vertex)     glDeleteObjectARB( vertex );
  if (fragment)   glDeleteObjectARB( fragment );

  programObj = 0;
  vertex = 0;
  fragment = 0;
}

GLint GLSLShaderProgram::findLocation( const char *name ) const
{
	if (!programObj) return -1;
	int loc = glGetUniformLocationARB( programObj, name );
	return loc;
}

bool GLSLShaderProgram::setUniformUINT( const char *name, const GLint value )
{
  if (!programObj) return false;
  int loc = glGetUniformLocationARB( programObj, name );
  if (loc < 0)
    return false;
  
  glUniform1iARB( loc, value );
  return true;
}

bool GLSLShaderProgram::setUniformFloat( const char *name, const float value )
{
  if (!programObj) return false;
  int loc = glGetUniformLocationARB( programObj, name );
  if (loc < 0)
    return false;

  glUniform1fARB( loc, value );
  return true;
}

bool GLSLShaderProgram::setUniformVector( const char *name, const float x, const float y, const float z, const float w )
{
  if (!programObj) return false;
  int loc = glGetUniformLocationARB( programObj, name );
  if (loc < 0)
    return false;

  glUniform4fARB( loc, x, y, z ,w );
  return true;
}

bool GLSLShaderProgram::setUniformVector3f(const char* name, const float x, const float y, const float z)
{
	if (!programObj) return false;
	int loc = glGetUniformLocationARB(programObj, name);
	if (loc < 0)
		return false;

	glUniform3fARB(loc, x, y, z);
	return true;
}

bool GLSLShaderProgram::setUniformVector2f( const char *name, const float x, const float y )
{
  if (!programObj) return false;
  int loc = glGetUniformLocationARB( programObj, name );
  if (loc < 0)
    return false;

  glUniform2fARB( loc, x, y );
  return true;
}


bool GLSLShaderProgram::setUniformMatrix33( const char *name, const float *m )
{
  if (!programObj) return false;
  int loc = glGetUniformLocationARB( programObj, name );
  if (loc < 0)
    return false;

  glUniformMatrix3fvARB( loc, 1, GL_FALSE, m );
  return true;
}

bool GLSLShaderProgram::setUniformMatrix( const char *name, const float *m )
{
  if (!programObj) return false;
  int loc = glGetUniformLocationARB( programObj, name );
  if (loc < 0)
    return false;

  glUniformMatrix4fvARB( loc, 1, GL_FALSE, m );
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

void GLSLShaderProgram::setUniformUINT( const GLint location, const GLint value )
{
	glUniform1iARB( location, value );
}

void GLSLShaderProgram::setUniformFloat( const GLint location, const float value )
{
	glUniform1fARB( location, value );
}

void GLSLShaderProgram::setUniformVector( const GLint location, const float x, const float y, const float z, const float w )
{
	glUniform4fARB( location, x, y, z ,w );
}

void GLSLShaderProgram::setUniformVector2f( const GLint location, const float x, const float y )
{
	glUniform2fARB( location, x, y );
}

void GLSLShaderProgram::setUniformMatrix33( const GLint location, const float *m )
{
	glUniformMatrix3fvARB( location, 1, GL_FALSE, m );
}

void GLSLShaderProgram::setUniformMatrix( const GLint location, const float *m, bool doTranspose )
{
	glUniformMatrix4fvARB( location, 1, (doTranspose) ? GL_TRUE : GL_FALSE, m );
}