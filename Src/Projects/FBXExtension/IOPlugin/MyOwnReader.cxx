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

/**	\file	MyOwnReader.cxx
*/

#include "MyOwnReader.h"

MyOwnReader::MyOwnReader(FbxManager &pFbxManager, int pID):
    FbxReader(pFbxManager, pID, mStatus),
    mFilePointer(NULL),
    mManager(&pFbxManager)
{
}

MyOwnReader::~MyOwnReader()
{
    FbxArrayDelete(mTakeInfo);
    FileClose();
}

void MyOwnReader::GetVersion(int& pMajor, int& pMinor, int& pRevision)
{
    pMajor = 1;
    pMinor = 0;
    pRevision=0;
}

bool MyOwnReader::FileOpen(char* pFileName)
{
    if(mFilePointer != NULL)
        FileClose();
    mFilePointer = fopen(pFileName, "r");
    if(mFilePointer == NULL)
        return false;


    FbxTakeInfo * lTakeInfo = FbxNew<FbxTakeInfo>();
    lTakeInfo->mName = "First Take";
    mTakeInfo.Add(lTakeInfo);

    lTakeInfo = FbxNew<FbxTakeInfo>();
    lTakeInfo->mName = "Second Take";
    mTakeInfo.Add(lTakeInfo);

    return true;
}

bool MyOwnReader::FileClose()
{
    if(mFilePointer!=NULL)
        fclose(mFilePointer);
    return true;

}

bool MyOwnReader::IsFileOpen()
{
    if(mFilePointer != NULL)
        return true;
    return false;
}

bool MyOwnReader::GetReadOptions(bool /*pParseFileAsNeeded*/)
{
    return true;
}

FbxArray<FbxTakeInfo*>* MyOwnReader::GetTakeInfo()
{
    return &mTakeInfo;
}

bool MyOwnReader::Read(FbxDocument* pDocument)
{
    //
    //Read the custom file and reconstruct node hierarchy.
    //

    if (!pDocument)
    {
        GetStatus().SetCode(FbxStatus::eInvalidFile);
        return false;
    }

    FbxScene*       lScene = FbxCast<FbxScene>(pDocument);
    bool            lIsAScene = (lScene != NULL);
    bool            lResult = false;

    if(lIsAScene)
    {
        FbxNode* lRootNode = lScene->GetRootNode();
        FbxNodeAttribute * lRootNodeAttribute = FbxNull::Create(lScene,"");
        lRootNode->SetNodeAttribute(lRootNodeAttribute);

        int lSize;
        char* lBuffer = NULL;    
        if(mFilePointer != NULL)
        {
            //To obtain file size
            fseek (mFilePointer , 0 , SEEK_END);
            lSize = ftell (mFilePointer);
            rewind (mFilePointer);

            //Read file content to a string.
            lBuffer = (char*) malloc (sizeof(char)*lSize + 1);
            size_t lRead = fread(lBuffer, 1, lSize, mFilePointer);
            lBuffer[lRead]='\0';
            FbxString lString(lBuffer);

            //Parse the string to get name and relation of Nodes. 
            FbxString lSubString, lChildName, lParentName;
            FbxNode* lChildNode;
            FbxNode* lParentNode;
            FbxNodeAttribute* lChildAttribute;
            int lEndTokenCount = lString.GetTokenCount("\n");

            for (int i = 0; i < lEndTokenCount; i++)
            {
                lSubString = lString.GetToken(i, "\n");
                FbxString lNodeString;
                lChildName = lSubString.GetToken(0, "\"");
                lParentName = lSubString.GetToken(2, "\"");

                //Build node hierarchy.
                if(lParentName == "RootNode")
                {
                    lChildNode = FbxNode::Create(lScene,lChildName.Buffer());
                    lChildAttribute = FbxNull::Create(mManager,"");
                    lChildNode->SetNodeAttribute(lChildAttribute);

                    lRootNode->AddChild(lChildNode);
                }
                else
                {
                    lChildNode = FbxNode::Create(lScene,lChildName.Buffer());
                    lChildAttribute = FbxNull::Create(lScene,"");
                    lChildNode->SetNodeAttribute(lChildAttribute);

                    lParentNode = lRootNode->FindChild(lParentName.Buffer());
                    if(lParentNode)
                    {
                        lParentNode->AddChild(lChildNode);
                    } else {
                        // Bad format parent should already be created
                    }
                }
            }
            free(lBuffer);
        }
        lResult = true;
    }    
    return lResult;
}
