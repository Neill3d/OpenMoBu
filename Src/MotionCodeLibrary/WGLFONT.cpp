
/**	\file	wglfont.cpp
*
*	output windows text into opengl drawing context.

*	Author Sergey Solokhin, 2010
*		e-mail to: s@neill3d.com
*/

/**
*	\page	WGL Font
*	\section	Usage example
*
	WGLFont::instance().InitFont( mApplication->GetGLInfo().dc );

*	CEOGL::WGLFont::instance().PositionText( mRegion.Position.X, mRegion.Position.Y+mRegion.Position.Height - fontHeight );
	CEOGL::WGLFont::instance().glDrawText( mLastComponent->Hint.c_str(), mLastComponent->Hint.size() );
*
*	\section	History
*	22.05.10 - поправил вывод русских символов
*
*/

#include <math.h>
#include "WGLFONT.h"

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>



// This define is for the amount of lists we want to create.  There will need to be 1
// for every character.  Since there are 256 ascii characters, we will use 256.
// If we only wanted certain characters used, like alpha numerics, we would give less.
#define MAX_CHARS	256

// This defines how high we want our font to be
#define FONT_HEIGHT	14

//using	namespace	CEOGL;

//////////////////////////////////////////////
WGLFont* a_pDefaultFont{ nullptr };

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

WGLFont::WGLFont()
	: g_FontListID(0)
{
}

WGLFont::WGLFont( HDC hDC, LPSTR strFontName, int height ) 
	: g_hDC(hDC)
	, m_height(height)
{
	g_FontListID = CreateOpenGLFont( strFontName, height );
}

WGLFont::~WGLFont()
{

}

void WGLFont::InitFont(HDC hDC, LPSTR strFontName, int height)
{
	g_hDC = hDC;
	m_height = height;
	g_FontListID = CreateOpenGLFont(strFontName, height);
}

void WGLFont::FreeFont()
{
	if (g_FontListID)
		glDeleteLists(g_FontListID, MAX_CHARS);				// Free the display list
}

///////////////////////////////// CREATE OPENGL FONT \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function creates a windows font like "Arial" and returns a display list ID
/////
///////////////////////////////// CREATE OPENGL FONT \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

UINT WGLFont::CreateOpenGLFont(LPSTR strFontName, int height)	// Build Our Bitmap Font
{
	UINT	fontListID = 0;								// This will hold the base ID for our display list
	HFONT	hFont;										// This will store the handle to our font
	HFONT	oldfont;									// Used For Good House Keeping


	fontListID = glGenLists(MAX_CHARS);								// Storage For 256 Characters

	hFont = CreateFont(	-height,							// Height Of Font
						0,								// Width Of Font
						0,								// Angle Of Escapement
						0,								// Orientation Angle
						FW_NORMAL,						// Font Weight
						FALSE,							// Italic
						FALSE,							// Underline
						FALSE,							// Strikeout
						RUSSIAN_CHARSET,					// Character Set Identifier
						OUT_TT_PRECIS,					// Output Precision
						CLIP_DEFAULT_PRECIS,			// Clipping Precision
						ANTIALIASED_QUALITY,			// Output Quality
						FF_DONTCARE|DEFAULT_PITCH,		// Family And Pitch
						TEXT("Courier New"));					// Font Name
	assert( hFont != NULL );
	oldfont = (HFONT)SelectObject(g_hDC, hFont);           // Selects The Font We Want
	BOOL err = wglUseFontBitmaps(g_hDC, 0, MAX_CHARS, fontListID);

	//assert( err != 0 );				// Builds 96 Characters Starting At Character 32
	if (!err) {
		glDeleteLists(fontListID, MAX_CHARS);
		fontListID = 0;
	}

	SelectObject(g_hDC, oldfont);							// Selects The Font We Want
	DeleteObject(hFont);									// Delete The Font

	return fontListID;									// Return the ID to the display list to use later
}

///////////////////////////////// POSITION TEXT \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This function sets the drawing position for the text
/////
///////////////////////////////// POSITION TEXT \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

void WGLFont::PositionText( float x, float y )
{
	mX = x;
	mY = y;
	glRasterPos2f( x, y );
}

///////////////////////////////// GL DRAW TEXT \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This draws 2D text onto the screen using OpenGL, given an X and Y position
/////
///////////////////////////////// GL DRAW TEXT \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

GLvoid WGLFont::glDrawText(const char *str, const int len)					// Custom GL "Print" Routine
{
	if (!str) return;
	if (!g_FontListID) {
		// try to create font, if it's not ready yet
		InitFont(GetDC(NULL));
	}
	if (!g_FontListID) return;	// failed to prepare font

	int pos = 0;
	for (int i=0; i<len; i++, pos++)
	{
		if (str[i] == '\n') break;
	}

	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	glListBase(g_FontListID);								// Sets The Base Character to 32
	glCallLists( pos, GL_UNSIGNED_BYTE, str);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits

	// next line
	if (pos < len) {
		mY -= m_height;
		glRasterPos2f( mX, mY );
		glDrawText( &str[pos+1], len-pos-1 );
	}
}

GLvoid WGLFont::glDrawTextAligned(const char *str, const int len, float width, float height, CEAlign	align)					// Custom GL "Print" Routine
{
	float rasterPos[4];
	if (!str) return;
	if (!g_FontListID) {
		// try to create font, if it's not ready yet
		InitFont( GetDC(NULL) );
	}
	if (!g_FontListID) return;	// failed to prepare font

	int pos = 0;
	for (int i=0; i<len; i++, pos++)
	{
		if (str[i] == '\n') break;
	}

	glColorMask(false, false, false, false);
  glRasterPos2i(0,0);
  glPushAttrib(GL_LIST_BIT);
  glListBase(g_FontListID);
  glCallLists( pos, GL_UNSIGNED_BYTE, str);
  glPopAttrib();
	glGetFloatv(GL_CURRENT_RASTER_POSITION, &rasterPos[0]);
  glColorMask(true, true, true, true);

	float newX = mX;
	switch( align )
	{
	case kCEAlignCenter: 
		newX += (width - rasterPos[0]) / 2;
		break;
	case kCEAlignRight:
		newX += width - rasterPos[0];
		break;
	}

	glRasterPos2f(newX, mY);
	glPushAttrib(GL_LIST_BIT);							// Pushes The Display List Bits
	glListBase(g_FontListID);								// Sets The Base Character to 32
	glCallLists( pos, GL_UNSIGNED_BYTE, str);	// Draws The Display List Text
	glPopAttrib();										// Pops The Display List Bits

	// next line
	if (pos < len) {
		mY -= m_height;
		//glRasterPos2f( mX, mY );
		glDrawTextAligned( &str[pos+1], len-pos-1, width, height, align );
	}
}

void WGLFont::ComputeDimensions( const char* str, const int len, float &width, float &height)
{
	if (!g_FontListID) {
		// try to create font, if it's not ready yet
		InitFont( GetDC(NULL) );
	}
	if (!g_FontListID) return;	// failed to prepare font

  float rasterPos[4];

	int pos = 0;
	for (int i=0; i<len; i++, pos++)
	{
		if (str[i] == '\n') break;
	}

  glColorMask(false, false, false, false);
  glRasterPos2i(0,0);
  glPushAttrib(GL_LIST_BIT);
  glListBase(g_FontListID);
  glCallLists( pos, GL_UNSIGNED_BYTE, str);
  glPopAttrib();
  glGetFloatv(GL_CURRENT_RASTER_POSITION, &rasterPos[0]);
  glColorMask(true, true, true, true);

  //width = std::max(width, rasterPos[0]);
  if (rasterPos[0] > width)
	  width = rasterPos[0];

  height += m_height;

	// next line
	if (pos < len) {
		ComputeDimensions( &str[pos+1], len-pos-1, width, height );
	}
}
