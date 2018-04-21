
/**	\file	device_projectTango.cxx

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#ifdef KARCH_ENV_WIN
	#include <windows.h>
#endif

#include "GL/glew.h"

//--- Library declaration
FBLibraryDeclare( device_projecttango )
{
	FBLibraryRegister( Device_ProjectTango		);
	FBLibraryRegister(Device_ProjectTango_Layout);
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/

extern bool InitializeSockets();
extern void ShutdownSockets();

bool FBLibrary::LibInit()	{ return true; }
bool FBLibrary::LibOpen()	{
	InitializeSockets();
	glewInit();
	return true; }
bool FBLibrary::LibReady()	{ return true; }
bool FBLibrary::LibClose()	{
	ShutdownSockets();
	return true; }
bool FBLibrary::LibRelease(){ return true; }

/**
*	\mainpage	Device Template
*	\section	intro	Introduction
*	Template showing what needs to be done
*	in order to create a new device in FiLMBOX.
*/
