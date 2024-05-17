
#ifndef WGLFONT_H_
#define WGLFONT_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**	\file	wglfont.h
*
*	output windows text into opengl drawing context.

*	Author Sergey Solokhin, 2010
*		e-mail to: s@neill3d.com
*/

/**
*	\page	OpenGL Font for Windows
*	\section	Usage example
*
*	WGLFont*  pFont;
    pFont = (WGLFont*) lua_newuserdata(L, sizeof(WGLFont) );
    HDC dc = clApp()->GetGLInfo()->dc;
    pFont->InitFont( dc, (char*) fontName.c_str(), fontHeight );
*
*	\section	History
*	22.08.10 - add GetDC(NULL), check if font is created
*	16.05.10 - reformated code
*
*	\section	TODO
*	- в создание шрифта добавить настройки - bold, italic
*
*/

//#include "Types.h"

#include <windows.h>
#include <GL/glew.h>


	enum CEAlign {	
		kCEAlignLeft,					//!< text to the left side.						
		kCEAlignCenter,				//!< text in the center of an object.				
		kCEAlignRight,				//!< text to the right side.							
	};

	/*!	WGLFont
	 *
	 */
	class WGLFont  // : public	Singleton<WGLFont>
	{
		UINT g_FontListID;

		// This will save our old font and select it back in at the end of our program.
		// We need this because we use SelectObject() to select in the new font.
		// We don't want any memory leaks :)
		HFONT hOldFont;

		int m_height;

		float mX;
		float mY;

		// global HDC
		HDC		g_hDC;

		UINT CreateOpenGLFont(LPSTR strFontName, int height);

	public:
		//! a default constructor
		WGLFont();
		/*! a constructor
		 *	\param	hDC	- handle to opengl device
		 *	\param	strFontName	- windows font name
		 *	\param	height	- font height
		 */
		WGLFont( HDC hDC, LPSTR strFontName = LPSTR("Arial"), int height = 16 );
		//! a destructor
		virtual ~WGLFont();

		/*!	InitFont
		 * create display lists and so on
		 *	\param	hDC	- handle to opengl device
		 *	\param	strFontName	- windows font name
		 *	\param	height	- font height
		 */
		void InitFont(HDC hDC, LPSTR strFontName = LPSTR("Arial"), int height = 16 );

		void	FreeFont();

		/*!	PositionText
		 *	set drawing text position
		 *	\param	x - x value
		 *	\param	y - y value
		 */
		void PositionText( float x, float y );

		/*!	glDrawText
		 *	output text
		 *	\param	str - text
		 */
		void glDrawText(const char *str, const int len);
		void glDrawTextAligned(const char *str, const int len, float width, float height, CEAlign	align=kCEAlignCenter);

		// 
		/*!	ComputeDimensions
		 * return width & height with drawing str text of te current font
		 * \param	str - text to compute
		 *	\param width - return text width
		 *	\param	height - return text height
		 */
		void ComputeDimensions( const char* str, const int len, float &width, float &height);

		float	GetHeight() { return (float)m_height; }
	};


#endif
