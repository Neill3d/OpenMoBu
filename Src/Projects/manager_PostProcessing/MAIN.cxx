
/** \file   MAIN.cxx

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#ifdef KARCH_ENV_WIN
    #include <windows.h>
#endif

#include "postprocessing_data.h"

// for back compatibility
extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }

/// <summary>
/// a method to transfer shared library logs into motionbuilder logs output
/// </summary>
void LOGE(const char* pFormatString, ...)
{
	char buffer[512];
	va_list args;
	va_start(args, pFormatString);
	vsnprintf(buffer, 255, pFormatString, args);

	FBTrace(buffer);

	va_end(args);
}

//--- Library declaration
FBLibraryDeclare( manager_postprocessing )
{
	FBLibraryRegister(PostProcessingAssociation);
    FBLibraryRegister( PostProcessingManager );

	FBLibraryRegister(PostPersistentData);
	FBLibraryRegisterElement(PostPersistentData);

	FBLibraryRegister(ORManip_Template);
	//FBLibraryRegisterStorable(ORHUDElementCustom);

	FBLibraryRegister(FXMaskingShader);
}
FBLibraryDeclareEnd;

/************************************************
 *  Library functions.
 ************************************************/

extern bool InitializeSockets();
extern void ShutdownSockets();

bool FBLibrary::LibInit()       { return true; }
bool FBLibrary::LibOpen()       { 
	//InitializeSockets();
	return true; }
bool FBLibrary::LibReady()      { 

	PostPersistentData::AddPropertiesToPropertyViewManager();
	return true; 
}
bool FBLibrary::LibClose()      { 
	//ShutdownSockets();
	return true; }
bool FBLibrary::LibRelease()    { return true; }

/**
*   \mainpage   Post Processing Manager
*   \section    intro   Introduction
*   Manager that injects rendering callback and grabs frame information
*		color and depth information is used to apply post processing filters
*/
