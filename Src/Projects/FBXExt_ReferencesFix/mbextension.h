#ifndef __MB_EXTENTION_H__
#define __MB_EXTENTION_H__

/**	\file	mbextension.h
*	Interface for the MotionBuilder FBX extension plugin. 
*/

#if defined(_WIN32) || defined(_WIN64)
	#define EXPORT_DLL __declspec(dllexport)
#else
	#define EXPORT_DLL
#endif

#include <fbxsdk.h>

// Open Reality SDK
#include <fbsdk/fbsdk.h>

#include <fbxsdk/fbxsdk_nsbegin.h>

#define FBX_MB_EXTENSION_DECLARE()\
	EXPORT_DLL bool MBExt_IsExtension(){return true;}\
	\
	EXPORT_DLL bool MBExt_ExportHandled( FBComponent* pFBComponent );\
	EXPORT_DLL void MBExt_ExportBegin( FbxScene* pFbxScene );\
	EXPORT_DLL bool MBExt_ExportProcess( FbxObject*& output, FBComponent* inputObject, FbxScene* pFbxScene);\
	EXPORT_DLL void MBExt_ExportTranslated( FbxObject* pFbxObject, FBComponent* pFBComponent );\
	EXPORT_DLL void MBExt_ExportEnd( FbxScene* pFbxScene );\
	\
	EXPORT_DLL bool MBExt_ImportHandled( FbxObject* pFbxObject );\
	EXPORT_DLL void MBExt_ImportBegin( FbxScene* pFbxScene );\
	EXPORT_DLL bool MBExt_ImportProcess( FBComponent*& pOutputObject, FbxObject* pInputFbxObject, bool pIsAnInstance, bool pMerge);\
	EXPORT_DLL void MBExt_ImportTranslated( FbxObject* pFbxObject, FBComponent* pFBComponent );\
	EXPORT_DLL void MBExt_ImportEnd( FbxScene* pFbxScene );\

#include <fbxsdk/fbxsdk_nsend.h>

#endif /* _MB_EXTENTION_H_ */