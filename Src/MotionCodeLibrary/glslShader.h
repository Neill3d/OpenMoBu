#ifndef GLSLSHADER_H_INCLUDED
#define GLSLSHADER_H_INCLUDED

/*
Sergei <Neill3d> Solokhin 2018
		class is based on http://steps3d.narod.ru library
		Thanks for Alex Boreskov

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include <windows.h>
#include <stdio.h>
#include <GL\glew.h>

//
// profile for GLSL, OpenGL 2.0+
//
class GLSLShader
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

  bool LoadShaders( const char* vertex_file, const char* fragment_file );
bool	LoadShaders( GLhandleARB	_vertex,	const char* fragment_file );

	bool ReCompileShaders(const char* vertex_file, const char* fragment_file );


  void Bind() const;
  void UnBind() const;

  bool setUniformUINT ( const char *name, const GLint value );
  bool setUniformFloat( const char *name, const float value );
  bool setUniformVector( const char * name, const float x, const float y, const float z, const float w );
  bool setUniformVector2f( const char *name, const float x, const float y );
  bool setUniformMatrix33( const char * name, const float *m );
  bool setUniformMatrix( const char * name, const float *m );
  
  const GLint findLocation( const char * name ) const;

  static void setUniformUINT ( const GLint location, const GLint value );
  static void setUniformFloat( const GLint location, const float value );
  static void setUniformVector( const GLint location, const float x, const float y, const float z, const float w );
  static void setUniformVector2f( const GLint location, const float x, const float y );
  static void setUniformMatrix33( const GLint location, const float *m );
  static void setUniformMatrix( const GLint location, const float *m );

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

	const GLhandleARB		GetVertexShader() const {
		return vertex;
	}
	const GLhandleARB		GetFragmentShader() const {
		return fragment;
	}
	const GLhandleARB		GetProgramObj() const {
		return programObj;
	}
};

#endif // GLSLSHADER_H_INCLUDED
