#ifndef GLSLSHADER_H_INCLUDED
#define GLSLSHADER_H_INCLUDED

#include <windows.h>
#include <stdio.h>
#include <GL\glew.h>

//
// base class for gpu shaders
//
class CGShader
{
public:
  virtual bool LoadShaders( const char *vertex_file, const char *fragment_file ) = 0;
  virtual void Free() = 0;

  virtual void Bind() = 0;
  virtual void UnBind() = 0;

  virtual bool setUniformUINT ( const char *name, const GLint value )=0;
  virtual bool setUniformFloat( const char *name, const float value )=0;
  virtual bool setUniformVector( const char * name, const float x, const float y, const float z, const float w )=0;
  virtual bool setUniformVector2f( const char *name, const float x, const float y ) = 0;
  virtual bool setUniformMatrix33( const char * name, const float *m )=0;
  virtual bool setUniformMatrix( const char * name, const float *m )=0;

  virtual void SetHeaderText( const char *text ) = 0;
};

//
// profile for GLSL, OpenGL 2.0
//
class GLSLShader : public CGShader
{
  // vertex shader handle
  GLhandleARB     vertex;

  // fragment handle
  GLhandleARB     fragment;

  // v & f
  GLhandleARB     programObj;

  // shader header text (defines)
  char          mHeaderText[256];

  bool LoadShader( GLhandleARB shader, FILE *file );
  void loadlog( GLhandleARB object );

public:
  GLSLShader();
  void Free();
  virtual ~GLSLShader() {
      Free();
  }

  void SetHeaderText( const char *text ) {
	  memset(mHeaderText, 0, sizeof(char) * 256 );
	  strcpy_s(mHeaderText, 256, text);
  }

  bool LoadShadersSource( const char *vertex_text, const char *fragment_text );
  bool LoadShaders( const char* vertex_file, const char* fragment_file );
bool	LoadShaders( GLhandleARB	_vertex,	const char* fragment_file );
	bool LoadShader( GLhandleARB shader, const char *source );

	bool ReCompileShaders(const char* vertex_file, const char* fragment_file );


  void Bind();
  void UnBind();

  bool setUniformUINT ( const char *name, const GLint value );
  bool setUniformFloat( const char *name, const float value );
  bool setUniformVector( const char * name, const float x, const float y, const float z, const float w );
  bool setUniformVector2f( const char *name, const float x, const float y );
  bool setUniformMatrix33( const char * name, const float *m );
  bool setUniformMatrix( const char * name, const float *m );
  
  const GLint findLocation( const char * name );

  void setUniformUINT ( const GLint location, const GLint value );
  void setUniformFloat( const GLint location, const float value );
  void setUniformVector( const GLint location, const float x, const float y, const float z, const float w );
  void setUniformVector2f( const GLint location, const float x, const float y );
  void setUniformMatrix33( const GLint location, const float *m );
  void setUniformMatrix( const GLint location, const float *m );

	void bindTexture(GLenum target, const char *texname, GLuint texid, int texunit)
	{
		glActiveTexture(GL_TEXTURE0 + texunit);
		glBindTexture(target, texid);
		setUniformUINT( texname, (GLint) texunit );
		glActiveTexture(GL_TEXTURE0);
	}
	void bindTexture(GLenum target, const GLint location, GLuint texid, int texunit)
	{
		glActiveTexture(GL_TEXTURE0 + texunit);
		glBindTexture(target, texid);
		setUniformUINT( location, (GLint) texunit );
		glActiveTexture(GL_TEXTURE0);
	}
	void unbindTexture(GLenum target, int texunit)
	{
		glActiveTexture(GL_TEXTURE0 + texunit);
		glBindTexture(target, 0);
		glActiveTexture(GL_TEXTURE0);
	}

	void bindTextureRECT(const char *texname, GLuint texid, int texunit) {
		bindTexture(GL_TEXTURE_RECTANGLE_ARB, texname, texid, texunit);
	}

	GLhandleARB		GetVertexShader() {
		return vertex;
	}
	GLhandleARB		GetFragmentShader() {
		return fragment;
	}
	GLhandleARB		GetProgramObj() {
		return programObj;
	}
};

#endif // GLSLSHADER_H_INCLUDED
