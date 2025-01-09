#ifndef GLSLSHADER_H_INCLUDED
#define GLSLSHADER_H_INCLUDED

/*
Sergei <Neill3d> Solokhin 2018
		class is based on http://steps3d.narod.ru library
		Thanks for Alex Boreskov

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//#include <windows.h>
#include <stdio.h>
#include <GL\glew.h>

#include <string_view>
#include <unordered_map>

struct StringViewHash {
	using is_transparent = void;  // Enables heterogeneous lookup
	std::size_t operator()(std::string_view sv) const {
		return std::hash<std::string_view>{}(sv);
	}
};

struct StringViewEqual {
	using is_transparent = void;  // Enables heterogeneous lookup
	bool operator()(std::string_view lhs, std::string_view rhs) const {
		return lhs == rhs;
	}
};

///
/// profile for GLSL, OpenGL 2.0+
///
class GLSLShaderProgram
{
	/// vertex shader handle
	GLhandleARB     vertex{ 0 };

	/// fragment handle
	GLhandleARB     fragment{ 0 };

	/// v & f
	GLhandleARB     programObj{ 0 };

	/// shader header text (defines)
	char          mHeaderText[256]{ 0 };

	static std::unordered_map<std::string, std::string, StringViewHash, StringViewEqual>	g_TextInsertions;

  bool LoadShader( GLhandleARB shader, FILE *file, const char* debugName );
  bool LoadLog( GLhandleARB object, const char* debugName ) const;

public:

	static bool PRINT_WARNINGS;

  GLSLShaderProgram();
  void Free();
  virtual ~GLSLShaderProgram() {
      Free();
  }

  void SetHeaderText( const char *text ) {
	  memset(mHeaderText, 0, sizeof(char) * 256 );
	  strcpy_s(mHeaderText, 256, text);
  }

  /// <summary>
  /// a special keyword that is going to converted into a chunk of defined text
  ///  that could be a way to avoid duplication of similar code for a every shader used in the system
  /// like insert header, insert image cropping, insert masking, etc.
  /// </summary>
  static void AddTextInsertion(const char* insertion_keyword, const char* insertion_data);
  static bool AddTextInsertionFromFile(const char* insertion_keyword, const char* file_name);


  bool LoadShaders( const char* vertex_file, const char* fragment_file );
bool	LoadShaders( GLhandleARB	_vertex,	const char* fragment_file );

	bool ReCompileShaders(const char* vertex_file, const char* fragment_file );


  void Bind() const;
  void UnBind() const;

  bool setUniformUINT ( const char *name, const GLint value );
  bool setUniformFloat( const char *name, const float value );
  bool setUniformVector( const char * name, const float x, const float y, const float z, const float w );
  bool setUniformVector3f(const char* name, const float x, const float y, const float z);
  bool setUniformVector2f( const char *name, const float x, const float y );
  bool setUniformMatrix33( const char * name, const float *m );
  bool setUniformMatrix( const char * name, const float *m );
  
  GLint findLocation( const char * name ) const;

  static void setUniformUINT ( const GLint location, const GLint value );
  static void setUniformFloat( const GLint location, const float value );
  static void setUniformVector( const GLint location, const float x, const float y, const float z, const float w );
  static void setUniformVector2f( const GLint location, const float x, const float y );
  static void setUniformMatrix33( const GLint location, const float *m );
  static void setUniformMatrix( const GLint location, const float *m, bool doTranspose = false );

	void bindTexture(GLenum target, const char *texname, GLuint texid, GLenum texunit)
	{
		glActiveTexture(GL_TEXTURE0 + texunit);
		glBindTexture(target, texid);
		setUniformUINT( texname, (GLint) texunit );
		glActiveTexture(GL_TEXTURE0);
	}
	void bindTexture(GLenum target, const GLint location, GLuint texid, GLenum texunit)
	{
		glActiveTexture(GL_TEXTURE0 + texunit);
		glBindTexture(target, texid);
		setUniformUINT( location, (GLint) texunit );
		glActiveTexture(GL_TEXTURE0);
	}
	void unbindTexture(GLenum target, GLenum texunit)
	{
		glActiveTexture(GL_TEXTURE0 + texunit);
		glBindTexture(target, 0);
		glActiveTexture(GL_TEXTURE0);
	}

	void bindTextureRECT(const char *texname, GLuint texid, GLenum texunit) {
		bindTexture(GL_TEXTURE_RECTANGLE_ARB, texname, texid, texunit);
	}

	GLhandleARB		GetVertexShader() const {
		return vertex;
	}
	GLhandleARB		GetFragmentShader() const {
		return fragment;
	}
	GLhandleARB		GetProgramObj() const {
		return programObj;
	}
};

#endif // GLSLSHADER_H_INCLUDED
