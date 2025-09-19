
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

#include "mobu_logging.h"
#include "postpersistentdata.h"

#include "gl/glew.h"

// for back compatibility
extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }

/// <summary>
/// a method to transfer shared library logs into motionbuilder logs output
/// </summary>
DEFINE_LOGV; // TODO: have to be enabled only when corresponding log level is set in command line
DEFINE_LOGI;
DEFINE_LOGE;

//--- Library declaration
FBLibraryDeclare( manager_postprocessing )
{
	FBLibraryRegister(PostProcessingAssociation);
    FBLibraryRegister( PostProcessingManager );

	FBLibraryRegister(PostPersistentData);
	FBLibraryRegisterElement(PostPersistentData);

	FBLibraryRegister(PostEffectUserObject);
	FBLibraryRegisterElement(PostEffectUserObject);

	FBLibraryRegister(EffectShaderUserObject);
	FBLibraryRegisterElement(EffectShaderUserObject);

	FBLibraryRegister(EffectShaderBilateralBlurUserObject);
	FBLibraryRegisterElement(EffectShaderBilateralBlurUserObject);

	FBLibraryRegister(Manip_PostProcessing);
	
	FBLibraryRegister(FXMaskingShader);
}
FBLibraryDeclareEnd;

/************************************************
 *  Library functions.
 ************************************************/

extern bool InitializeSockets();
extern void ShutdownSockets();

static bool g_isGlewInitialized = false;

bool FBLibrary::LibInit()       { 
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		// Problem: glewInit failed, something is seriously wrong.
		LOGE("GLEW error: %s\n", glewGetErrorString(err));
		return false;
	}
	
	LOGI("GLEW version: %s\n", glewGetString(GLEW_VERSION));
	g_isGlewInitialized = true;
	return true; 
}
bool FBLibrary::LibOpen()       { 
	//InitializeSockets();

	return g_isGlewInitialized; }
bool FBLibrary::LibReady()      { 

	PostPersistentData::AddPropertiesToPropertyViewManager();
	return g_isGlewInitialized; 
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
