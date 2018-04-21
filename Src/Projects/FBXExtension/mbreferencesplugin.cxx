
//
// mbreferencesplugin.cxx
//
// Plugin makes a fix for MoBu reference bug with locked reference model on missing reference file
//
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "mbextension.h"

//#include "References_Exchange.h"
#include "tinyxml.h"

#include <vector>
#include <Windows.h>

#include <fbxsdk/fbxsdk_nsbegin.h>

static FbxManager* gFbxManager = NULL;
static bool        gProcessedOnce = false;

bool FileExists(LPCTSTR szPath);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Custom functions

bool FileExists(LPCTSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Custom Material declaration

#define MY_CUSTOM_SHADER_CLASS_NAME		"MyCustomShaderClass"
#define MY_CUSTOM_SHADER_CLASS_SUBTYPE	"MyCustomShaderClass" // need to match the SURFACE_CLASS_NAME so the class can be recognized

// by the FBX reader.

/** Define a custom material object. 
*   For the purpose of this sample, this class will simply contains a Diffuse color property.
*/

//
class MyCustomShader : public FbxGenericNode
{
	FBXSDK_OBJECT_DECLARE(MyCustomShader, FbxGenericNode);

protected:

	void ConstructProperties(bool pForceSet)
	{
		ParentClass::ConstructProperties(pForceSet);
		Intensity.StaticInit(this, "Intensity", FbxDoubleDT, 100.0, pForceSet);

		FbxStringList	list;
		Links.StaticInit(this, "Links", FbxStringListDT, list, pForceSet, fbxsdk::FbxPropertyFlags::eUserDefined);
	}

public:
	FbxPropertyT<FbxDouble>			Intensity;
	FbxPropertyT<FbxStringList>		Links;
};

FBXSDK_OBJECT_IMPLEMENT(MyCustomShader);

/** Plugin declaration and initialization methods
*/
class MBCustomSurfacePlugin : public FbxPlugin
{
    FBXSDK_PLUGIN_DECLARE(MBCustomSurfacePlugin);

protected:
    explicit MBCustomSurfacePlugin(const FbxPluginDef& pDefinition, FbxModule pFbxModule) : FbxPlugin(pDefinition, pFbxModule)
    {
    }

    // Implement kfbxmodules::FbxPlugin
    virtual bool SpecificInitialize()
    {
        gFbxManager = GetData().mSDKManager;

        // Register MyCustomSurfacer class with the plug-in's manager
        
		//gFbxManager->RegisterFbxClass(MY_CUSTOM_SHADER_CLASS_NAME, FBX_TYPE(MyCustomShader), FBX_TYPE(FbxGenericNode), FIELD_OBJECT_DEFINITION_OBJECT_TYPE_GENERIC_NODE, MY_CUSTOM_SHADER_CLASS_SUBTYPE);
        return true;
    }

    virtual bool SpecificTerminate()
    {
        // Unregister MyCustomSurface class with the plug-in's manager
        
		//gFbxManager->UnregisterFbxClass(FBX_TYPE(MyCustomShader));

        return true;
    }
};

FBXSDK_PLUGIN_IMPLEMENT(MBCustomSurfacePlugin);

/** FBX Interface
*/
extern "C"
{
    // The DLL is owner of the plug-in
    static MBCustomSurfacePlugin* sPlugin = NULL;

    // This function will be called when an application will request the plug-in
    EXPORT_DLL void FBXPluginRegistration(FbxPluginContainer& pContainer, FbxModule pFbxModule)
    {
        if( sPlugin == NULL )
        {
            // Create the plug-in definition which contains the information about the plug-in
            FbxPluginDef sPluginDef;
            sPluginDef.mName = "ReferencesShadersPlugin";
            sPluginDef.mVersion = "1.0";

            // Create an instance of the plug-in.  The DLL has the ownership of the plug-in
            sPlugin = MBCustomSurfacePlugin::Create(sPluginDef, pFbxModule);

            // Register the plug-in
            pContainer.Register(*sPlugin);
        }
    }

    FBX_MB_EXTENSION_DECLARE();
}

///////////////////////////////////////////////////////////////////////////////////////////
//

/** MB Extension Export Callbacks
*/
bool MBExt_ExportHandled( FBComponent* pFBComponent )
{
    // In this example, we aren't replacing any objects in the fbx scene, so we don't have to
    // prevent the MB plugin from translating MB objects into fbx objects.

    return false;
}

void MBExt_ExportBegin(FbxScene* pFbxScene)
{
    // In this example, we will create our custom data during the export process.
    // So there is nothing to do here.

	//XmlBeginExport();
}

void MBExt_ExportTranslated(FbxObject* pFbxObject, FBComponent* pFBComponent)
{
    // Since we aren't replacing objects in the scene hierarchy in this example, we don't have
    // anything to do with this information.

}

bool MBExt_ExportProcess( FbxObject*& pOutputFbxObject, FBComponent* pInputObject, FbxScene* pFbxScene)
{
    // In this example, we replace the Maya phong shader with our custom material.
    // Because this function is called several times during the export, we need to figure out what 
    // we are processing and take action only on the objects of interest.

    // validate the inputs
    if (pOutputFbxObject == NULL || pInputObject == NULL || pFbxScene == NULL)
        return false;

    // let the process continue to export as usual even if the object was replaced
    return false;
}

void MBExt_ExportEnd(FbxScene* pFbxScene)
{
    //Since we process all our data in the MBExt_ExportProcess callback, we have nothing to do here.
	//XmlEndExport("D:\\export_test.xml");
}


/** MB Extension Import Callbacks
*/
bool MBExt_ImportHandled( FbxObject* pFbxObject )
{
    // In this example, we want to intercept the processing pass only. If we return true here, we tell the 
    // caller that we are responsible for creating the geometry associated with pFbxObject which we don't.
    // So we return false and let the caller process the geometry as usual.
    return false;
}



void MBExt_ImportBegin(FbxScene* pFbxScene)
{
    // In this example we have nothing to do before we translate the FBX scene into
    // MB data.

	//XmlBeginExport();

}

bool MBExt_ImportProcess( FBComponent*& pOutputObject, FbxObject* pInputFbxObject, bool pIsAnInstance, bool pMerge)
{
    // Validate the input
    if (pInputFbxObject == NULL)
        return false;

    return false;
}

void MBExt_ImportTranslated( FbxObject* pFbxObject, FBComponent* pFBComponent )
{
    // In this example, this doesn't interest us. A typical usage would be to
    // record this information to able to re-make connections and such.

	// store temproary imported comps
	
	if (FBIS(pFBComponent, FBFileReference) && nullptr != pFbxObject)
	{
		FBFileReference *pRef = (FBFileReference*)pFBComponent;

		FBProperty *prop = pRef->PropertyList.Find("TempIsLoaded");
		if (nullptr == prop)
			prop = pRef->PropertyCreate("TempIsLoaded", kFBPT_bool, "bool", false, true);

		if (nullptr != prop)
			prop->SetInt(pRef->IsLoaded.AsInt());

		//pRef->ReferenceFilePath.SetPropertyValue("D:\\Temp\\temp3.fbx");
		pRef->IsLoaded.SetPropertyValue(false);
		
		//
		FbxSceneReference* pref = FbxCast<FbxSceneReference>(pFbxObject);
		if (nullptr != pref)
		{

			FbxString path = pref->ReferenceFilePath.Get();

			bool isLoaded = pref->IsLoaded.Get();
			if (nullptr != prop)
			{
				int value = 0;
				if (isLoaded)
				{
					if (true == FileExists(path.Buffer()))
					{
						value = 1;
					}
				}

				prop->SetInt(value);
			}
			
			// always turn off
			//if (false == FileExists(path.Buffer()))
			{
				pref->IsLoaded.Set(false);
				pref->IsLocked.Set(false);
			}
		}
	}
	
}


void MBExt_ImportEnd(FbxScene* pFbxScene)
{
    // In this example, this doesn't interest us either. Every thing has been processed in
    // the MBExt_ImportProcess callback.
	
}


#include <fbxsdk/fbxsdk_nsend.h>
