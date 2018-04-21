
// desc.h
//
// Helper class
//
// Sergei <Neill3d> Solokhin 2018

#include "fbxsdk.h"

#include <Windows.h>

#include <vector>
#include <set>

///////////////////////////////////////////////////////
// declarations

int FileDate(const char* name, __int64 &fileDate);
BOOL FileSizeAndDate(const char* name, __int64 &fileSize, LPTSTR lpszString, DWORD dwSize);
bool FileExists(LPCTSTR szPath);

void XmlBeginExport(const char *fbxFilename);
bool XmlWriteShader(FbxObject *pShader);
bool XmlWriteModel(FbxNode *pModel);
void XmlEndExport(const char *filename = "D:\\shadersGraph.xml");

//
// baking

//bool ReadXmlShader(FbxScene *pScene, FbxObject *pFbxShader, TiXmlElement *pFirstElem);
bool ReadXml(const char *fname, FbxManager *pManager, FbxScene *pScene, std::set<FbxObject*> &fbxShaders, std::vector<FbxObject*> &fbxShadersLibrary);