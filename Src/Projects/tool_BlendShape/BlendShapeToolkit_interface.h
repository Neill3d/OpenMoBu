
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BlendShapeToolkit_interface.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef EXPORT_DLL
	/** \def ORSDK_DLL
	*	Be sure that ORSDK_DLL is defined only once...
	*/
	#define ORSDK_DLL __declspec(dllexport)
#else
	#define ORSDK_DLL __declspec(dllimport)
#endif

////////////////// MESH EDIT TOOLS ////////////////////



//////////////////////////////////////////
extern "C"
{
	void ORSDK_DLL	AddManualBlendShape();
}
