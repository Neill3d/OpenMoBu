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

/**	\file	MyOwnWriter.cxx
*/

#include "MyOwnWriter.h"

MyOwnWriter::MyOwnWriter(FbxManager &pFbxManager, int pID):
    FbxWriter(pFbxManager, pID, mStatus),
    mFilePointer(NULL),
    mManager(&pFbxManager)
{

}

MyOwnWriter::~MyOwnWriter()
{
    FileClose();
}

bool MyOwnWriter::FileCreate(char* pFileName)
{
    if(mFilePointer != NULL)
    {
        FileClose();
    }

    // 
    // Create a file stream with pFileName
    //
    mFilePointer = fopen(pFileName,"w");
    if(mFilePointer == NULL)
    {
        return false;
    }
    return true;
}

bool MyOwnWriter::FileClose()
{
    //
    // Close the file stream
    //
    if(mFilePointer != NULL)
    {
        fclose(mFilePointer);
        return true;
    }
    return false;
}

bool MyOwnWriter::IsFileOpen()
{
    //
    // Check whether the file stream is open.
    //
    if(mFilePointer != NULL)
        return true;
    return false;
}

void MyOwnWriter::GetWriteOptions()
{
    //
    // Setup the file write options
    //
}

bool MyOwnWriter::Write(FbxDocument* pDocument)
{
    //
    // Write file with stream options
    //
    if (!pDocument)
    {
        GetStatus().SetCode(FbxStatus::eInvalidFile);
        return false;
    }

    FbxScene* lScene = FbxCast<FbxScene>(pDocument);
    bool lIsAScene = (lScene != NULL);
    bool lResult = false;

    if(lIsAScene)
    {
        PreprocessScene(*lScene);
        printf("I'm in my own writer\n");

        FbxNode* lRootNode = lScene->GetRootNode();
        PrintHierarchy(lRootNode);

        PostprocessScene(*lScene);
        lResult = true;        
    }
    return lResult;
}

void MyOwnWriter::PrintHierarchy(FbxNode* pStartNode)
{
    //
    // Write out Node Hierarchy recursively
    //
    FbxNode* lChildNode;
    char const* lParentName = pStartNode->GetName();
    for(int i = 0; i<pStartNode->GetChildCount(); i++)
    {
        lChildNode = pStartNode->GetChild(i);
        char const* lChildName = lChildNode->GetName();
        fprintf(mFilePointer,"%s%s%s%s%s%s%s","\"",lChildName,"\"",", parent is ","\"",lParentName,"\"\n");
    }

    int lNodeChildCount = pStartNode->GetChildCount ();
    while (lNodeChildCount > 0)
    {
        lNodeChildCount--;
        lChildNode = pStartNode->GetChild (lNodeChildCount);
        PrintHierarchy(lChildNode);        
    }
}

bool MyOwnWriter::PreprocessScene(FbxScene& /*pScene*/)
{
    //
    // Pre-process the scene before write it out 
    //
    printf("I'm in pre-process\n");
    return true;
}

bool MyOwnWriter::PostprocessScene(FbxScene& /*pScene*/)
{
    //
    // Post-process the scene after write it out 
    //
    printf("I'm in post process\n");
    return true;
}
