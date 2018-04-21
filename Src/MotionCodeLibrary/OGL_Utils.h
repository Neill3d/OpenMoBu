
#pragma once

/*
	Sergei <Neill3d> Solokhin 2018

	GitHub page - https://github.com/Neill3d/OpenMoBu
	Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//-- 
#include <GL\glew.h>

// Calculates log2 of number.  
//double log2( double n );


/////////////////////////////////////////////////////////


void Push2dViewport(int g_w, int g_h);
void Pop2dViewport();
void Set2dViewport(int g_w, int g_h);

//void drawOrthoQuad2d(const double w, const double h);
void drawOrthoQuad2dOffset(const double x, const double y, const double w, const double h);
//void drawOrthoGradientQuad2d(const int w, const int h);

void PrepareOrthoView(const double w, const double h);

// prepare vertex buffer for faster draw primitives
void PrepareDrawPrimitives();
// will draw quad 1x1 with texcoords
void DrawQuad2d();

// old staff
void drawOrthoQuad2d(const double w, const double h);
void drawOrthoQuad2d(int x, int y, int x2, int y2, int w, int h);
void drawQuad2d(double x, double y, double x2, double y2);

// if using vao, we need to recreate vao each time we change context
void ChangeContextDrawPrimitives();
// cleanup drawing primitives buffers
void FreeDrawPrimitives();

void FitImageWithAspect(const int imgWidth, const int imgHeight, const int regionWidth, const int regionHeight, int &x, int &y, int &width, int &height);

void GenerateFBO(const int Width, const int Height, GLuint &color_buffer_id, GLuint &depth_id, GLuint &fbo_id, GLubyte *data = nullptr);
void CleanupFBO(GLuint &color_buffer_id, GLuint &depth_id, GLuint &fbo_id);

//////////////////

bool BlitFBOToFBO(const GLint FBO, const int width, const int height, const GLint defaultFBO, 
	const int defWidth, const int defHeight, const bool copyColor1, const bool copyColor2, const bool copyColor3);
bool BlitFBOToFBOOffset(const GLint FBO, int x, int y, const int width, const int height, const GLint defaultFBO,
	int defX, int defY, const int defWidth, const int defHeight, bool copyDepth, const bool copyColor1, const bool copyColor2, const bool copyColor3);
bool BlitFBOToScreen(const GLint defaultFBO, const int defWidth, const int defHeight, const int screenWidth, const int screenHeight);


////////////////////////////////////////////////

struct	OGLCullFaceInfo
{
	GLboolean			enabled;	// glIsEnabled
	GLint				mode;		// GL_CULL_FACE_MODE
};

void	StoreCullMode(OGLCullFaceInfo &info);
void	FetchCullMode(const OGLCullFaceInfo &info);

////////////////////////////////////////////////

void TextureObjectGetInfo( const GLenum target, int &width, int &height, int &internalFormat, int &format, bool &isCompressed, int &pixelMemorySize, int &imageSize, int &numberOfLods );
void TextureObjectGetData( const GLenum target, GLubyte *localImageBuffer, int &width, int &height, int &internalFormat, int &format, bool &isCompressed, int &pixelMemorySize, int &imageSize, int &numberOfLods );

//////////////////////////////////////////////////////////////////////////////////////

#define MAX_DRAW_BUFFERS 10

struct FrameBufferInfo
{
	GLint		gViewport[4];
	GLenum		gDrawBuffers[MAX_DRAW_BUFFERS];
	GLint		gMaxDrawBuffers;
	GLint		gLastFramebuffer;
};

void SaveFrameBuffer(FrameBufferInfo *pInfo=nullptr);
void RestoreFrameBuffer(FrameBufferInfo *pInfo=nullptr);

void BindFrameBufferLayers(const GLuint textureId);

// flags for generating a framebuffer
#define		FRAMEBUFFER_MS_TEXTURES				2
#define		FRAMEBUFFER_MS_RENDERBUFFERS		4
#define		FRAMEBUFFER_MS_NORMAL				8

struct CFrameBufferMS
{
	GLuint			fbo;
	GLuint			rbColor;
	GLuint			rbNormal;
	GLuint			rbDepth;
	GLuint			tColor;
	GLuint			tNormal;
	GLuint			tDepth;

	//
	int				width;
	int				height;
	int				samples;

	// creating flags
	int				flags;
};

void	ZeroFrameBufferMS(CFrameBufferMS &framebuffer);
bool	PrepFrameBufferMS(const int width, const int height, const int samples, const int flags, const int normalFormat, CFrameBufferMS &framebuffer);
void	FreeFrameBufferMS(CFrameBufferMS &framebuffer);

//////////////////
/*
bool BlitFBOToFBO(const GLint FBO, const int width, const int height, const GLint defaultFBO, const int defWidth, const int defHeight, const bool copyColor1);
bool BlitFBOToScreen(const GLint defaultFBO, const int defWidth, const int defHeight, const int screenWidth, const int screenHeight);
*/
//////////////////////////////////////////////////////////////////////////////////////
//

//////////////////////////////////////////////////////////////////////////////////////
//
class   VertexArray
{
    GLuint  id;
    
public:
	//! a constructor
    VertexArray ()
    {
        glGenVertexArrays ( 1, &id );
    }
    //! a destructor
    ~VertexArray ()
    {
        glDeleteVertexArrays ( 1, &id );
    }
    
    bool    IsOk () const
    {
        return glIsVertexArray ( id ) == GL_TRUE;
    }
    
    void    Bind ()
    {
        glBindVertexArray ( id );
    }
    
    void    UnBind ()
    {
        glBindVertexArray ( 0 );
    }
};

