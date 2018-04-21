//
// Return floating-point texture format with given precision
//

#ifndef	__FP_TEXTURE__
#define	__FP_TEXTURE__

#include	<GL\glew.h>				// get all OpenGL stuff from it

GLenum	fpRgbaFormatWithPrecision  ( int bits = 32 );
GLenum	fpAlphaFormatWithPrecision ( int bits = 32 );

#endif
