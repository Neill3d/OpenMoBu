#ifndef MY_OWN_WRITER_H
#define MY_OWN_WRITER_H

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

/**	\file	MyOwnWriter.h
*/

#include <fbxsdk.h>

/** This class is a custom writer.
*   The writer provide you the ability to write out node hierarchy to a custom file format.
*/
class MyOwnWriter : public FbxWriter
{
public:

    MyOwnWriter(FbxManager &pFbxManager, int pID);

    virtual ~MyOwnWriter();

    /**
    * \name Override functions from base class FbxWriter
    */
    //@{

	/** Creates a new file via a stream.
      * \param pStream The stream to write to.
      * \param pStreamData the user-defined stream data to be written.
      */
    virtual bool FileCreate(char* pFileName);

    /** Closes the file.
	*/
    virtual bool FileClose() ;

    /**  Test if the file is open.
	*/
    virtual bool IsFileOpen();

    /**  Setup write options.
	*/
    virtual void GetWriteOptions();

	/** Writes content to the specified file with given stream options
      *	\param pDocument        FbxDocument to write file data to.
	  */
    virtual bool Write(FbxDocument* pDocument);

	/** Pre-processes the scene.
      * \param pScene The scene needs to be pre-processed.
      */
    virtual bool PreprocessScene(FbxScene &pScene);

	/** Post-processes the scene.
      * \param pScene The scene needs to be post-processed.
      */
    virtual bool PostprocessScene(FbxScene &pScene);
    //@}

    /** Write out Node Hierarchy recursively
    */
    virtual void PrintHierarchy(FbxNode* pStartNode);

private:
    FILE *mFilePointer;
    FbxManager *mManager;
    FbxStatus mStatus;
};

#endif
