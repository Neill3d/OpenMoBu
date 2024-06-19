
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/OpenMoBu
//
// Author Sergei Solokhin (Neill3d) 2014-2024
//  e-mail to: neill3d@gmail.com
// 
/////////////////////////////////////////////////////////////////////////////////////////


/**	\file	solver_normals.cxx
*	Library declarations.
*	Contains the basic routines to declare the DLL as a loadable
*	library.
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#ifdef KARCH_ENV_WIN
	#include <windows.h>
#endif

#include <GL\glew.h>
#include "ResourceUtils.h"

/// <summary>
/// a method to transfer shared library logs into motionbuilder logs output
/// </summary>
void LOGE(const char* pFormatString, ...)
{
	char buffer[256];
	va_list args;
	va_start(args, pFormatString);
	vsnprintf(buffer, 255, pFormatString, args);

	FBTrace(buffer);

	va_end(args);
}

//--- Library declaration.
FBLibraryDeclare( solvercalculatenormals )
{
	FBLibraryRegister(SolverCalculateNormals)
	FBLibraryRegister( KNormalSolverAssociation );
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()		{ return true; }
bool FBLibrary::LibOpen()		{ 
	glewInit();
	InitResourceUtils();
	return true; }
bool FBLibrary::LibReady()		{ return true; }
bool FBLibrary::LibClose()		{ return true; }
bool FBLibrary::LibRelease()	{ return true; }
