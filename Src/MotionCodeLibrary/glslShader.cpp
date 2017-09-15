
#include <stdio.h>
#include "glslShader.h"

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////
///                                                                                            GLSLShader



GLSLShader::GLSLShader() {
    vertex = 0;
    fragment = 0;
    programObj = 0;
	memset(mHeaderText, 0, sizeof(char)*256);
}


bool GLSLShader::ReCompileShaders( const char* vertex_file, const char* fragment_file )
{
	FILE *fp = nullptr;
	/*
	fopen_s(&fp, vertex_file, "r");
	if (fp)
	{
		if (vertex) 
			if (!LoadShader( vertex, fp ) ) {
				fclose(fp);
				return false;
			}
		fclose(fp);
	}
	*/
	fopen_s(&fp, fragment_file, "r");
	if (fp)
	{
	
		if (fragment)

			glDetachObjectARB( programObj, fragment );
			glDeleteObjectARB( fragment );


			fragment = glCreateShaderObjectARB( GL_FRAGMENT_SHADER_ARB );
			if (!LoadShader( fragment, fp ) ) {
				fclose(fp);
				return false;
			}
			fclose(fp);

			// attach shader to program object
			glAttachObjectARB( programObj, fragment );

			GLint linked;
			// link the program object and print out the info log
			glLinkProgramARB( programObj );

			glGetObjectParameterivARB( programObj, GL_OBJECT_LINK_STATUS_ARB, &linked );
			loadlog( programObj );

			return (linked != 0);
	}

	return true;
}

bool GLSLShader::LoadShadersSource( const char* vertex_text, const char* fragment_text )
{
	Free();

	if (vertex_text)
	{
		vertex = glCreateShaderObjectARB( GL_VERTEX_SHADER_ARB );
		LoadShader( vertex, vertex_text );
	}
	
	if (fragment_text)
	{
		fragment = glCreateShaderObjectARB( GL_FRAGMENT_SHADER_ARB );
		LoadShader( fragment, fragment_text );
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
	/*
	  if (checkopenglerror() == false)
	{
		printf( "opengl error\n" );
	}
	  //if ( checkopenglerror ) return false;
	  */
	  glGetObjectParameterivARB( programObj, GL_OBJECT_LINK_STATUS_ARB, &linked );
	  loadlog( programObj );

	  return (linked != 0);
	}

	return false;
}

bool GLSLShader::LoadShaders( const char* vertex_file, const char* fragment_file )
{
	Free();

	FILE *fp = nullptr;
	fopen_s(&fp, vertex_file, "r");
	if (fp)
	{
		vertex = glCreateShaderObjectARB( GL_VERTEX_SHADER_ARB );
		LoadShader( vertex, fp );

		fclose(fp);
	}
	
		
	fopen_s(&fp, fragment_file, "r");
	if (fp)
	{
		fragment = glCreateShaderObjectARB( GL_FRAGMENT_SHADER_ARB );
		LoadShader( fragment, fp );

		fclose(fp);
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
	
	  //CHECK_GL_ERROR();
	  //if ( checkopenglerror ) return false;

	  glGetObjectParameterivARB( programObj, GL_OBJECT_LINK_STATUS_ARB, &linked );
	  loadlog( programObj );

	  return (linked != 0);
	}

	return false;
}

bool GLSLShader::LoadShaders( GLhandleARB	_vertex, const char* fragment_file )
{
	Free();

	vertex = _vertex;

	FILE *fp = nullptr;
	fopen_s(&fp, fragment_file, "r");
	if (fp)
	{
		fragment = glCreateShaderObjectARB( GL_FRAGMENT_SHADER_ARB );
		LoadShader( fragment, fp );

		fclose(fp);
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
	  loadlog( programObj );

	  return (linked != 0);
	}

	return false;
}

bool GLSLShader::LoadShader( GLhandleARB shader, FILE *file )
{
  size_t headerLen = strlen(mHeaderText); // number of bytes in header

  fseek(file, 0, SEEK_END);
  size_t fileLen = ftell(file);
  fseek(file, 0, SEEK_SET);

  char  *buffer = new char[ headerLen + fileLen + 1 ];
  const GLcharARB*  bufferARB = buffer;

  GLint   len = (GLint) fileLen;
  GLint   compileStatus;

  // read shader from file
  memset( &buffer[0], 0, sizeof(char)*(headerLen + len + 1) );
  if (headerLen)
    memcpy( &buffer[0], &mHeaderText[0], sizeof(char) * headerLen );
  void *buf = (void*)&buffer[headerLen];
  
  size_t readlen = fread(buf, sizeof(char), fileLen, file);
  
  // trick to zero all outside memory
  memset( &buffer[readlen+headerLen], 0, sizeof(char)*(len + 1 - readlen) );

  if (readlen == 0) //(readlen != len)
  {
    //ERR("glsl shader file size" );
    return false;
  }

  len = len + (GLint) headerLen;
  glShaderSourceARB( shader, 1, &bufferARB, &len );
  // compile shader
  glCompileShaderARB( shader );

  //if ( checkopenglerror ) return false;

  glGetObjectParameterivARB ( shader, GL_OBJECT_COMPILE_STATUS_ARB, &compileStatus );

  loadlog ( shader );

  if (buffer) {
    delete[] buffer;
    buffer = NULL;
  }

  return (compileStatus != 0);
}

bool GLSLShader::LoadShader( GLhandleARB shader, const char *source )
{
  size_t headerLen = strlen(mHeaderText); // number of bytes in header
  
  size_t fileLen = strlen(source);

  char  *buffer = new char[ headerLen + fileLen + 1 ];
  const GLcharARB*  bufferARB = buffer;

  GLint   len = fileLen;
  GLint   compileStatus;

  // read shader from file
  memset( &buffer[0], 0, sizeof(char)*(headerLen + len + 1) );
  if (headerLen)
    memcpy( &buffer[0], &mHeaderText[0], sizeof(char) * headerLen );
  void *buf = (void*)&buffer[headerLen];
  
  strcpy_s( (char*) buf, sizeof(char) * len, source );

  len = len + headerLen;
  glShaderSourceARB( shader, 1, &bufferARB, &len );
  // compile shader
  glCompileShaderARB( shader );

  //if ( checkopenglerror ) return false;

  glGetObjectParameterivARB ( shader, GL_OBJECT_COMPILE_STATUS_ARB, &compileStatus );

  loadlog ( shader );

  if (buffer) {
    delete[] buffer;
    buffer = NULL;
  }

  return (compileStatus != 0);
}


void GLSLShader::loadlog( GLhandleARB object )
{
    GLint       logLength     = 0;
    GLsizei     charsWritten  = 0;
    GLcharARB   buffer [2048];
    GLcharARB * infoLog;

    glGetObjectParameterivARB ( object, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLength );
/*
    if ( checkopenglerror )          // check for OpenGL errors
        return;
*/
    if ( logLength < 1 )
        return;
                                    // try to avoid allocating buffer
    if ( logLength > (int) sizeof ( buffer ) )
    {
        infoLog = (GLcharARB*) malloc ( logLength );

        if ( infoLog == NULL )
        {
            //ERR( "ERROR: Could not allocate log buffer" );
            return;
        }
    }
    else
        infoLog = buffer;

    glGetInfoLogARB ( object, logLength, &charsWritten, infoLog );

	if ( strlen(infoLog) > 0 )
	{
		printf( infoLog );
	}

    if ( infoLog != buffer )
        free ( infoLog );
}

void GLSLShader::Bind()
{
  if (programObj)
    glUseProgramObjectARB( programObj );
}

void GLSLShader::UnBind()
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

const GLint GLSLShader::findLocation( const char *name )
{
	if (!programObj) return -1;
	int loc = glGetUniformLocationARB( programObj, name );
	loadlog( programObj );
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