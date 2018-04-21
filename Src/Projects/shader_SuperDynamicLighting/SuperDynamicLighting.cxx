

/**	\file	DynamicLighting.cxx

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include

#include <fbsdk/fbsdk.h>
#include <GL\glew.h>

//--- Library declaration
FBLibraryDeclare( superdynamiclighting )
{
	FBLibraryRegister( SuperDynamicLighting );
    FBLibraryRegister( SuperDynamicLightingLayout );
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()	{ return true; }
bool FBLibrary::LibOpen()	{ return true; }
bool FBLibrary::LibReady()	{ 
	
	
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		// Problem: glewInit failed, something is seriously wrong.
		FBTrace("GLEW error: %s\n", glewGetErrorString(err));
	}
	else
	{
		FBTrace("GLEW version: %s\n", glewGetString(GLEW_VERSION));
	}

	
	return true; }
bool FBLibrary::LibClose()	{ return true; }
bool FBLibrary::LibRelease(){ return true; }
