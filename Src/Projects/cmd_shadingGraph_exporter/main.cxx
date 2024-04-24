
// main.cxx
//
// Shading Graph Exporter
//
// Trace fbx document and export a xml shaders graph
//
// Sergei <Neill3d> Solokhin 2018

#include "fbxsdk.h"

#include "../Common_FBXSDK/Common.h"

#include "desc.h"

#include <set>
#include <vector>

//
// run in two possible modes - extract xml and bake xml

///////////////////////////////////////////////////////////////////////////////////////////////////
// mainExtract

bool mainExtract(const FbxString &lFilePath, const FbxString &strOutput)
{
	if (lFilePath.IsEmpty())
	{
		return false;
	}

	FbxManager* lSdkManager = NULL;
	FbxScene* lScene = NULL;

	// DONE: before we continue, let's check that xml not exist or xml is out of date !

	FbxString lFileOutput(strOutput);

	if (true == lFileOutput.IsEmpty())
	{
		int pointNdx = lFilePath.ReverseFind('.');
		if (pointNdx >= 0 && pointNdx == lFilePath.GetLen() - 4)
		{
			lFileOutput = lFilePath.Left(lFilePath.GetLen() - 4);
			lFileOutput = lFileOutput + ".xml";
		}
	}
	

	if (!FileExists(lFilePath.Buffer()))
	{
		printf("ERROR: fbx file is not found!\n");
		return false;
	}

	bool outOfDate = false;

	if (!FileExists(lFileOutput.Buffer()))
	{
		outOfDate = true;
	}
	else
	{
		__int64 fbxDate, xmlDate;
		FileDate(lFilePath.Buffer(), fbxDate);
		FileDate(lFileOutput.Buffer(), xmlDate);

		outOfDate = (fbxDate > xmlDate);
	}
	//outOfDate = true;
	if (false == outOfDate)
	{
		printf("No need to update a fbx file\n");
		return false;
	}

	// Prepare the FBX SDK.
	InitializeSdkObjects(lSdkManager, lScene);

	bool lResult = LoadScene(lSdkManager, lScene, lFilePath.Buffer());
	if (lResult)
	{
		XmlBeginExport(lFilePath);

		//
		// Print the nodes of the scene and their attributes recursively.
		// Note that we are not printing the root node because it should
		// not contain any attributes.

		std::set<FbxObject*>	lShaders;
		std::vector<FbxNode*>	lModels;

		printf("check  nodes - %d\n", lScene->GetNodeCount());
		for (int i = 0, genCount = lScene->GetNodeCount(); i < genCount; ++i)
		{
			FbxNode *pNode = lScene->GetNode(i);
			const char *name = pNode->GetName();

			// skip system nodes
			if (pNode->GetObjectFlags(FbxObject::eSystem))
				continue;

			const int srcCount = pNode->GetSrcObjectCount();
			for (int j = 0; j < srcCount; ++j)
			{
				FbxObject *pObject = pNode->GetSrcObject(j);
				FbxProperty typeProp = pObject->FindProperty("MoBuTypeName");

				if (typeProp.IsValid())
				{
					FbxString typeStr(typeProp.Get<FbxString>());

					if (0 == strcmp("Shader", typeStr.Buffer()))
					{
						lShaders.insert(pObject);
					}
				}


			}

			// check if that is a scene model node
			FbxGeometry *pGeom = pNode->GetGeometry();

			if (nullptr != pGeom)
			{
				lModels.push_back(pNode);
			}

			printf("node name - %s\n", name);
		}

		//
		// DONE: write models show and vis data 

		printf("[Shading Graph Exporter] Writing %zd shaders and %zd models\n", lShaders.size(), lModels.size());

		for (auto iter = begin(lShaders); iter != end(lShaders); ++iter)
		{
			XmlWriteShader(*iter);
		}
		for (auto iter = begin(lModels); iter != end(lModels); ++iter)
		{
			XmlWriteModel(*iter);
		}

		printf("[Shading Graph Exporter] Output xml - %s\n", lFileOutput.Buffer());

		XmlEndExport(lFileOutput.Buffer());

	}
	else
	{
		FBXSDK_printf("Call to LoadScene() failed.\n");
	}

	// Delete the FBX SDK manager. All the objects that have been allocated 
	// using the FBX SDK manager and that haven't been explicitly destroyed 
	// are automatically destroyed at the same time.
	DestroySdkObjects(lSdkManager, lResult);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// mainBake

bool mainBake(const FbxString &lFilePath, const FbxString &lXmlPath, const FbxString &strOutput)
{
	if (lFilePath.IsEmpty() || lXmlPath.IsEmpty())
	{
		return false;
	}

	FbxManager *lSdkManager = nullptr;
	FbxScene *lScene = nullptr;
	FbxScene *lSceneShaders = nullptr;

	FbxString lFileOutput(strOutput);

	if (true == lFileOutput.IsEmpty())
	{
		// override source file with a new update
		lFileOutput = lFilePath;
	}
	
	if (!FileExists(lFilePath.Buffer()))
	{
		printf("ERROR: fbx file is not found!\n");
		return 0;
	}

	// Prepare the FBX SDK.
	InitializeSdkObjects(lSdkManager, lScene);

	// init shader classes special scene
	//Create an FBX scene. This object holds most objects imported/exported from/to files.
	lSceneShaders = FbxScene::Create(lSdkManager, "Scene Shader Classes");
	if (!lSceneShaders)
	{
		FBXSDK_printf("Error: Unable to create a shader classes scene!\n");
		return false;
	}

	std::vector<FbxObject*>		lShadersLibrary;

	bool lResult2 = LoadScene(lSdkManager, lSceneShaders, "D:\\Temp\\FbxShadersClasses.fbx");
	if (lResult2)
	{

		int srcCount = lSceneShaders->GetSrcObjectCount();
		for (int i = 0; i < srcCount; ++i)
		{
			FbxObject *pObject = lSceneShaders->GetSrcObject(i);
			FbxProperty typeProp = pObject->FindProperty("MoBuTypeName");

			if (typeProp.IsValid())
			{
				FbxString typeStr(typeProp.Get<FbxString>());

				if (0 == strcmp("Shader", typeStr.Buffer()))
				{
					lShadersLibrary.push_back(pObject);
				}
			}
		}
	}

	printf("> have founded %zd library shaders\n", lShadersLibrary.size());

	bool lResult = LoadScene(lSdkManager, lScene, lFilePath.Buffer());
	if (lResult)
	{
		
		//
		// Update a scene 

		std::set<FbxObject*>	lShaders;
		std::vector<FbxNode*>	lModels;

		printf("check  nodes - %d\n", lScene->GetNodeCount());
		for (int i = 0, genCount = lScene->GetNodeCount(); i < genCount; ++i)
		{
			FbxNode *pNode = lScene->GetNode(i);
			const char *name = pNode->GetName();

			// skip system nodes
			if (pNode->GetObjectFlags(FbxObject::eSystem))
				continue;

			const int srcCount = pNode->GetSrcObjectCount();
			for (int j = 0; j < srcCount; ++j)
			{
				FbxObject *pObject = pNode->GetSrcObject(j);
				FbxProperty typeProp = pObject->FindProperty("MoBuTypeName");

				if (typeProp.IsValid())
				{
					FbxString typeStr(typeProp.Get<FbxString>());

					if (0 == strcmp("Shader", typeStr.Buffer()))
					{
						lShaders.insert(pObject);
					}
				}
			}

			// check if that is a scene model node
			FbxGeometry *pGeom = pNode->GetGeometry();

			if (nullptr != pGeom)
			{
				lModels.push_back(pNode);
			}

			printf("node name - %s\n", name);
		}

		//
		ReadXml(lXmlPath.Buffer(), lSdkManager, lScene, lShaders, lShadersLibrary);
		
		//
		/*
		if (nullptr != pShaderBase)
		{
			if (pShaderBase->IsRuntimePlug())
			{
				FbxClassId classid = pShaderBase->GetRuntimeClassId();

				// adding two new clones
				pShaderBase->SetName("My Bump Map\0");
				FbxObject *newObj = classid.Create(*lSdkManager, "new shader\0", pShaderBase);
				newObj->SetName("My Bump Map\0"); 
				lScene->ConnectSrcObject(newObj);
				
				newObj = classid.Create(*lSdkManager, "new shader", pShaderBase);
				lScene->ConnectSrcObject(newObj);
				newObj->SetName("My Bump Map");

				for (FbxProperty prop = newObj->GetFirstProperty(); prop.IsValid(); prop = newObj->GetNextProperty(prop))
				{
					if (FbxBoolDT == prop.GetPropertyDataType() )
					{
						prop.Set<bool>(true);
					}
				}
			}
		}
		*/

		//
		/*
		printf("check  nodes - %d\n", lScene->GetNodeCount());
		for (int i = 0, genCount = lScene->GetNodeCount(); i < genCount; ++i)
		{
			FbxNode *pNode = lScene->GetNode(i);
			const char *name = pNode->GetName();

			const int srcCount = pNode->GetSrcObjectCount();
			for (int j = 0; j < srcCount; ++j)
			{
				FbxObject *pObject = pNode->GetSrcObject(j);
				const char *objName = pObject->GetName();

				const char *classIdName = pObject->GetClassId().GetName();
				const char *fbxType = pObject->GetClassId().GetFbxFileTypeName(true);
				const char *fbxSubType = pObject->GetClassId().GetFbxFileSubTypeName();

				bool hasImpl = pObject->HasDefaultImplementation();
				printf("  src obj name - %s, type - %s, fbxtype - %s, fbxsubtype - %s, defImpl - %d\n", objName, classIdName, fbxType, fbxSubType, (hasImpl) ? 1 : 0);

				FbxProperty subTypeProp = pObject->FindProperty("MoBuSubTypeName");

				if (subTypeProp.IsValid())
				{
					FbxString subTypeStr(subTypeProp.Get<FbxString>());

					if (0 == strcmp("FlatCartoonShader", subTypeStr.Buffer()))
					{
						FbxProperty prop = pObject->GetFirstProperty();
						while (prop.IsValid())
						{
							printf("    prop name - %s\n", prop.GetName());

							const char *propName = prop.GetNameAsCStr();
							FbxDataType dataType = prop.GetPropertyDataType();
							const char *dataTypeName = dataType.GetName();

							if (0 == strcmp("MoBuTypeName", propName))
							{
								FbxString str(prop.Get<FbxString>());
								printf("MoBuTypeName Value - %s\n", str.Buffer());
							}
							else if (0 == strcmp("MoBuSubTypeName", propName))
							{
								FbxString str(prop.Get<FbxString>());
								printf("MoBuSubTypeName Value - %s\n", str.Buffer());
							}
							else if (0 == strcmp("AllEdges", propName))
							{
								bool value = prop.Get<bool>();
								value = true;
								bool lSuccess = prop.Set<bool>(value); // , eFbxDouble);
								printf("try to change reflexion factor - %d\n", (lSuccess) ? 1 : 0);

								//pObject->Clone();
							}

							//
							prop = pObject->GetNextProperty(prop);
						}
					}
				}


			}

			printf("generic node name - %s\n", name);
		}
		*/
		
		
		//SaveScene(lSdkManager, lScene, lFileOutput.Buffer());
		
		// Create an exporter.
		FbxExporter* lExporter = FbxExporter::Create(lSdkManager, "");

		// Initialize the exporter.

		#ifdef IOS_REF
		#undef  IOS_REF
		#define IOS_REF (*(lSdkManager->GetIOSettings()))
		#endif

		IOS_REF.SetBoolProp(EXP_FBX_MATERIAL, true);
		IOS_REF.SetBoolProp(EXP_FBX_TEXTURE, true);
		IOS_REF.SetBoolProp(EXP_FBX_EMBEDDED, false);
		IOS_REF.SetBoolProp(EXP_FBX_SHAPE, true);
		IOS_REF.SetBoolProp(EXP_FBX_GOBO, true);
		IOS_REF.SetBoolProp(EXP_FBX_ANIMATION, true);
		IOS_REF.SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

		// Get the appropriate file format.
		int lFileFormat = lSdkManager->GetIOPluginRegistry()->GetNativeWriterFormat();
		
		lResult = lExporter->Initialize(lFileOutput.Buffer(), lFileFormat, lSdkManager->GetIOSettings());
		if (!lResult)
		{
			//FBXSDK_printf("%s:\tCall to FbxExporter::Initialize() failed.\n", lFileTypes[i * 2 + 1]);
			FBXSDK_printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
		}
		else
		{
			// Export the scene.
			lResult = lExporter->Export(lScene);
			if (!lResult)
			{
				FBXSDK_printf("Call to FbxExporter::Export() failed.\n");
			}
		}
		
		// Destroy the exporter.
		lExporter->Destroy();
	
		//
		// Print the nodes of the scene and their attributes recursively.
		// Note that we are not printing the root node because it should
		// not contain any attributes.

	}
	else
	{
		FBXSDK_printf("Call to LoadScene() failed.\n");
	}

	// Delete the FBX SDK manager. All the objects that have been allocated 
	// using the FBX SDK manager and that haven't been explicitly destroyed 
	// are automatically destroyed at the same time.
	DestroySdkObjects(lSdkManager, lResult);

	return true;
}

////////////////////////////////////////////////////////////////////
// main

int main(int argc, char** argv)
{
	enum
	{
		MODE_OPEN_NONE,
		MODE_OPEN_EXTRACT,
		MODE_OPEN_BAKE
	};

	int currMode = MODE_OPEN_NONE;

	FbxString lFilePath("");
	FbxString lXmlPath("");
	FbxString lOutPath("");

	for( int i = 1, c = argc; i < c; ++i )
	{
		FbxString arg(argv[i]);
		const char *buf = arg.Buffer();

		if (0 == strcmp("-e", buf) || 0 == strcmp("-extract", arg) || 0 == strcmp("-f", arg))
		{
			currMode = MODE_OPEN_EXTRACT;
			lFilePath.Clear();
			lXmlPath.Clear();
		}
		else if (0 == strcmp("-b", buf) || 0 == strcmp("-bake", buf))
		{
			currMode = MODE_OPEN_BAKE;
			lFilePath.Clear();
			lXmlPath.Clear();
			lOutPath.Clear();
		}
		else if (lFilePath.IsEmpty())
		{
			lFilePath = arg;
		}
		else if (lXmlPath.IsEmpty())
		{
			lXmlPath = arg;
		}
		else if (lOutPath.IsEmpty())
		{
			lOutPath = arg;
		}
	}
	
	//

	switch (currMode)
	{
	case MODE_OPEN_EXTRACT:
		mainExtract(lFilePath, lXmlPath);
		break;
	case MODE_OPEN_BAKE:
		mainBake(lFilePath, lXmlPath, lOutPath);
		break;
	}
	
    return 0;
}

