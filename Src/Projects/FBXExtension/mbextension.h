#ifndef __MB_EXTENTION_H__
#define __MB_EXTENTION_H__

/***************************************************************************************
 Autodesk(R) Open Reality(R) Samples
 
 (C) 2009 Autodesk, Inc. and/or its licensors
 All rights reserved.
 
 AUTODESK SOFTWARE LICENSE AGREEMENT
 Autodesk, Inc. licenses this Software to you only upon the condition that 
 you accept all of the terms contained in the Software License Agreement ("Agreement") 
 that is embedded in or that is delivered with this Software. By selecting 
 the "I ACCEPT" button at the end of the Agreement or by copying, installing, 
 uploading, accessing or using all or any portion of the Software you agree 
 to enter into the Agreement. A contract is then formed between Autodesk and 
 either you personally, if you acquire the Software for yourself, or the company 
 or other legal entity for which you are acquiring the software.
 
 AUTODESK, INC., MAKES NO WARRANTY, EITHER EXPRESS OR IMPLIED, INCLUDING BUT 
 NOT LIMITED TO ANY IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR 
 PURPOSE REGARDING THESE MATERIALS, AND MAKES SUCH MATERIALS AVAILABLE SOLELY ON AN 
 "AS-IS" BASIS.
 
 IN NO EVENT SHALL AUTODESK, INC., BE LIABLE TO ANYONE FOR SPECIAL, COLLATERAL, 
 INCIDENTAL, OR CONSEQUENTIAL DAMAGES IN CONNECTION WITH OR ARISING OUT OF PURCHASE 
 OR USE OF THESE MATERIALS. THE SOLE AND EXCLUSIVE LIABILITY TO AUTODESK, INC., 
 REGARDLESS OF THE FORM OF ACTION, SHALL NOT EXCEED THE PURCHASE PRICE OF THE 
 MATERIALS DESCRIBED HEREIN.
 
 Autodesk, Inc., reserves the right to revise and improve its products as it sees fit.
 
 Autodesk and Open Reality are registered trademarks or trademarks of Autodesk, Inc., 
 in the U.S.A. and/or other countries. All other brand names, product names, or 
 trademarks belong to their respective holders. 
 
 GOVERNMENT USE
 Use, duplication, or disclosure by the U.S. Government is subject to restrictions as 
 set forth in FAR 12.212 (Commercial Computer Software-Restricted Rights) and 
 DFAR 227.7202 (Rights in Technical Data and Computer Software), as applicable. 
 Manufacturer is Autodesk, Inc., 10 Duke Street, Montreal, Quebec, Canada, H3C 2L7.
***************************************************************************************/

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