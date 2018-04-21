// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently


#if !defined(AFX_STDAFX_H__AE78B9E2_A5B8_11D4_A1FB_00500C0076C8__INCLUDED_)
#define AFX_STDAFX_H__AE78B9E2_A5B8_11D4_A1FB_00500C0076C8__INCLUDED_

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers


typedef char dInt8;
typedef unsigned char dUnsigned8;

typedef short dInt16;
typedef unsigned short dUnsigned16;

typedef int dInt32;
typedef unsigned dUnsigned32;
typedef unsigned int dUnsigned32;

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <ctype.h>


#ifdef _MSC_VER
	#include <windows.h>
	#include <crtdbg.h>
	//#include <gl/glut.h>
	#include <malloc.h>
#else
	#ifdef _MIPS_ARCH
		#include <Glut/glut.h>
	#else
		#include <glut.h>
	#endif
	#include <unistd.h>
#endif


// transcendental functions
#define	dAbs(x)		dFloat (fabs (dFloat(x))) 
#define	dSqrt(x)	dFloat (sqrt (dFloat(x))) 
#define	dFloor(x)	dFloat (floor (dFloat(x))) 
#define	dMod(x,y)	dFloat (fmod (dFloat(x), dFloat(y))) 

#define dSin(x)		dFloat (sin (dFloat(x)))
#define dCos(x)		dFloat (cos (dFloat(x)))
#define dAsin(x)	dFloat (asin (dFloat(x)))
#define dAcos(x)	dFloat (acos (dFloat(x)))
#define	dAtan2(x,y) dFloat (atan2 (dFloat(x), dFloat(y)))


#ifdef __USE_DOUBLE_PRECISION__
	#define glMultMatrix(x) glMultMatrixd(x)
	#define glGetFloat(x,y) glGetDoublev(x,(GLdouble *)y) 
#else
	#define glMultMatrix(x) glMultMatrixf(x)
	#define glGetFloat(x,y) glGetFloatv(x,(GLfloat  *)y) 
#endif


// SDK include
#include "dList.h"
#include "dVector.h"
#include "dMatrix.h"
//#include <dQuaternion.h>
#include "Newton.h"


#ifdef _MSC_VER
	#pragma warning (disable: 4100) //unreferenced formal parameter
	#pragma warning (disable: 4505) //unreferenced local function has been removed
	#pragma warning (disable: 4201) //nonstandard extension used : nameless struct/union
	#pragma warning (disable: 4127) //conditional expression is constant

	#if (_MSC_VER >= 1400)
		#pragma warning (disable: 4996) // for 2005 users declared deprecated
	#endif

#else
	//#define _ASSERTE(x) assert(x)
	#define _ASSERTE(x) 
	#define min(a,b) (a < b ? a : b)
	#define max(a,b) (a > b ? a : b)

	#ifndef dAsin
		#define dAsin(x) ((dFloat) dAsin(x))
		#define dAcos(x) ((dFloat) dAcos(x))
	#endif
#endif


// for some reason specifying a relative does not seem to work in Linus
// and i have to specify a absolute path
// #define ASSETS_PATH "."


#ifdef _MSC_VER
		// Windows user assets path
		#define ASSETS_PATH "."
		inline void GetWorkingFileName (const char* name, char* outPathName)
		{
			sprintf (outPathName, "%s/%s", ASSETS_PATH, name);
		}

#else

	#ifdef _MIPS_ARCH
		// Mac user assets path
		#define ASSETS_PATH "../../.."
		inline void GetWorkingFileName (const char* name, char* outPathName)
		{
			sprintf (outPathName, "%s/%s", ASSETS_PATH, name);
		}
	#else
		// Linux user assets path
		#define ASSETS_PATH "newtonSDK/samples/bin"
		inline void GetWorkingFileName (const char* name, char* outPathName)
		{
			char *env;
			env = getenv("HOME");
			sprintf (outPathName, "%s/%s/%s", env, ASSETS_PATH, name);
		}
	#endif
#endif


// little Indian/big Indian conversion
#ifdef _MIPS_ARCH
	#define SWAP_INT16(x) (((x >> 8) & 0xff) + ((x & 0xff) << 8))
	#define SWAP_INT32(x) ((SWAP_INT16 ( x >> 16)) + (SWAP_INT16 (x) << 16))

	inline void SWAP_FLOAT32_ARRAY (dFloat *array, dInt32 count)
	{
		dInt32 i;
		dInt32 x;

		for (i = 0; i < count; i ++) {
			x = SWAP_INT32 (*((dInt32*) &array[i]));
			array[i] = *((dFloat*)&x);
		}
	}

#else

	#define SWAP_INT16(x) x
	#define SWAP_INT32(x) x
	inline void SWAP_FLOAT32_ARRAY (float *array, dInt32 count)
	{
	}

#endif


#endif 
