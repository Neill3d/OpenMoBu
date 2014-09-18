
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://code.google.com/p/motioncodelibrary/wiki/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////

/** \file   autosave.cxx
*   Library declarations.
*   Contains the basic routines to declare the DLL as a loadable
*   library.
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#ifdef KARCH_ENV_WIN
    #include <windows.h>
#endif

//--- Library declaration
FBLibraryDeclare( autosavemanager )
{
    FBLibraryRegister( AutoSaveManager );
}
FBLibraryDeclareEnd;

/************************************************
 *  Library functions.
 ************************************************/
bool FBLibrary::LibInit()       { return true; }
bool FBLibrary::LibOpen()       { return true; }
bool FBLibrary::LibReady()      { return true; }
bool FBLibrary::LibClose()      { return true; }
bool FBLibrary::LibRelease()    { return true; }

/**
*   \mainpage   Custom Manager Template
*   \section    intro   Introduction
*   Template showing what needs to be done
*   in order to create a custom manager that is started at the beginning of
*   the lifetime of the application and and last until the application is
*   closed.
*/
