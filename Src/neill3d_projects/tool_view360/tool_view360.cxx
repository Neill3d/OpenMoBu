
/**	\file	tool_view360.cxx

// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE

	GitHub repo - https://github.com/Neill3d/MoBu

	Author Sergey Solokhin (Neill3d)
	 e-mail to: s@neill3d.com
	  www.neill3d.com

*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#ifdef KARCH_ENV_WIN
	#include <windows.h>
#endif

#include <GL\glew.h>

//--- Library declaration.
FBLibraryDeclare( ortoolview360 )
{
	FBLibraryRegister( ORToolView360 );
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()		{ return true; }
bool FBLibrary::LibOpen()		
{ 	
	glewInit();
	return true; 
}
bool FBLibrary::LibReady()		{ return true; }
bool FBLibrary::LibClose()		{ return true; }
bool FBLibrary::LibRelease()	{ return true; }
