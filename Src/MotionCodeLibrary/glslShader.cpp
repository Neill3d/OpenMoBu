
/*
Sergei <Neill3d> Solokhin 2018
		class is based on http://steps3d.narod.ru library
		Thanks for Alex Boreskov

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include <stdio.h>
#include "glslShader.h"
#include "CheckGLError.h"
#include "FileUtils.h"

//
extern void LOGE(const char* pFormatString, ...);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                            GLSLShader

bool GLSLShader::PRINT_WARNINGS = true;

GLSLShader::GLSLShader() 
{}


bool GLSLShader::ReCompileShaders( const char* vertex_file, const char* fragment_file )
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
		if (!LoadShader( fragment, fp ) ) {
			return false;
		}

		// attach shader to program object
		glAttachObjectARB( programObj, fragment );

		GLint linked{ 0 };
		// link the program object and print out the info log
		glLinkProgramARB( programObj );

		glGetObjectParameterivARB( programObj, GL_OBJECT_LINK_STATUS_ARB, &linked );
			
		if (linked == 0 || PRINT_WARNINGS)
		{
			loadlog(programObj);
		}
			
		return (linked != 0);
	}

	return false;
}

bool GLSLShader::LoadShaders( const char* vertex_file, const char* fragment_file )
{
	Free();

	{
		FileReadScope readVertex(vertex_file);

		if (FILE* fp = readVertex.Get())
		{
			vertex = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
			LoadShader(vertex, fp);
		}
	}

	{
		FileReadScope readFragment(fragment_file);

		if (FILE* fp = readFragment.Get())
		{
			fragment = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
			LoadShader(fragment, fp);
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

	  if (linked == 0 || PRINT_WARNINGS)
	  {
		  loadlog(programObj);
	  }
	  
	  return (linked != 0);
	}

	return false;
}

bool GLSLShader::LoadShaders( GLhandleARB	_vertex, const char* fragment_file )
{
	Free();

	vertex = _vertex;

	{
		FileReadScope readFragment(fragment_file);

		if (FILE* fp = readFragment.Get())
		{
			fragment = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
			LoadShader(fragment, fp);
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
	  
	  if (linked == 0 || PRINT_WARNINGS)
	  {
		  loadlog(programObj);
	  }
	  
	  return (linked != 0);
	}

	return false;
}

bool GLSLShader::LoadShader( GLhandleARB shader, FILE *file )
{
	const size_t headerLen = strlen(mHeaderText); // number of bytes in header

	fseek(file, 0, SEEK_END);
	const size_t fileLen = ftell(file);
	fseek(file, 0, SEEK_SET);

	std::vector<char> buffer(headerLen + fileLen + 1);
	const GLcharARB*  bufferARB = buffer.data();

	GLint   len = static_cast<GLint>(fileLen);
	GLint   compileStatus;

	// read shader from file
  
	memset( buffer.data(), 0, sizeof(char) * (headerLen + len + 1));
	if (headerLen)
	{
		memcpy(buffer.data(), &mHeaderText[0], sizeof(char) * headerLen);
	}
	void *buf = (void*)&buffer[headerLen];
  
	const size_t readlen = fread(buf, sizeof(char), fileLen, file);
  
	// trick to zero all outside memory
	memset( &buffer[readlen+headerLen], 0, sizeof(char)*(len + 1 - readlen) );

	if (readlen == 0) //(readlen != len)
	{
		LOGE("glsl shader file size" );
		return false;
	}

	len = len + static_cast<GLint>(headerLen);
	glShaderSourceARB( shader, 1, &bufferARB, &len );
	// compile shader
	glCompileShaderARB( shader );

	//if ( checkopenglerror ) return false;

	glGetObjectParameterivARB ( shader, GL_OBJECT_COMPILE_STATUS_ARB, &compileStatus );

	if (compileStatus == 0 || PRINT_WARNINGS)
	{
		loadlog(shader);
	}
	
	return (compileStatus != 0);
}

void GLSLShader::loadlog( GLhandleARB object )
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
        return;
    
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

	if ( strlen(infoLog) > 0 )
	{
		LOGE( infoLog );
	}

    if ( isAllocOnHeap )
        delete [] infoLog;
}

void GLSLShader::Bind() const
{
  if (programObj)
    glUseProgramObjectARB( programObj );
}

void GLSLShader::UnBind() const
{
  glUseProgramObjectARB( 0 );
}


void GLSLShader::Free()
{
  if (programObj) glDeleteObjectARB( programObj );
  if (vertex)     glDeleteObjectARB( vertex );
  if (fragment)   glDeleteObjectARB( fragment );

  programObj = 0;
  vertex = 0;
  fragment = 0;
}

GLint GLSLShader::findLocation( const char *name ) const
{
	if (!programObj) return -1;
	int loc = glGetUniformLocationARB( programObj, name );
	//loadlog( programObj );
	return loc;
}

bool GLSLShader::setUniformUINT( const char *name, const GLint value )
{
  if (!programObj) return false;
  int loc = glGetUniformLocationARB( programObj, name );
  if (loc < 0)
  {
    //info("uniform location failed");
    return false;
  }

  loadlog( programObj );

  glUniform1iARB( loc, value );

  return true;
}

bool GLSLShader::setUniformFloat( const char *name, const float value )
{
  if (!programObj) return false;
  int loc = glGetUniformLocationARB( programObj, name );
  if (loc < 0)
    return false;

  glUniform1fARB( loc, value );
  return true;
}

bool GLSLShader::setUniformVector( const char *name, const float x, const float y, const float z, const float w )
{
  if (!programObj) return false;
  int loc = glGetUniformLocationARB( programObj, name );
  if (loc < 0)
    return false;

  glUniform4fARB( loc, x, y, z ,w );
  return true;
}

bool GLSLShader::setUniformVector3f(const char* name, const float x, const float y, const float z)
{
	if (!programObj) return false;
	int loc = glGetUniformLocationARB(programObj, name);
	if (loc < 0)
		return false;

	glUniform3fARB(loc, x, y, z);
	return true;
}

bool GLSLShader::setUniformVector2f( const char *name, const float x, const float y )
{
  if (!programObj) return false;
  int loc = glGetUniformLocationARB( programObj, name );
  if (loc < 0)
    return false;

  glUniform2fARB( loc, x, y );
  return true;
}


bool GLSLShader::setUniformMatrix33( const char *name, const float *m )
{
  if (!programObj) return false;
  int loc = glGetUniformLocationARB( programObj, name );
  if (loc < 0)
    return false;

  glUniformMatrix3fvARB( loc, 1, GL_FALSE, m );
  return true;
}

bool GLSLShader::setUniformMatrix( const char *name, const float *m )
{
  if (!programObj) return false;
  int loc = glGetUniformLocationARB( programObj, name );
  if (loc < 0)
    return false;

  glUniformMatrix4fvARB( loc, 1, GL_FALSE, m );
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

void GLSLShader::setUniformUINT( const GLint location, const GLint value )
{
	glUniform1iARB( location, value );
}

void GLSLShader::setUniformFloat( const GLint location, const float value )
{
	glUniform1fARB( location, value );
}

void GLSLShader::setUniformVector( const GLint location, const float x, const float y, const float z, const float w )
{
	glUniform4fARB( location, x, y, z ,w );
}

void GLSLShader::setUniformVector2f( const GLint location, const float x, const float y )
{
	glUniform2fARB( location, x, y );
}

void GLSLShader::setUniformMatrix33( const GLint location, const float *m )
{
	glUniformMatrix3fvARB( location, 1, GL_FALSE, m );
}

void GLSLShader::setUniformMatrix( const GLint location, const float *m )
{
	glUniformMatrix4fvARB( location, 1, GL_FALSE, m );
}