
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

#include "tinyxml.h"

#include <vector>
#include <filesystem>

#include <fbxsdk/fbxsdk_nsbegin.h>

static FbxManager* gFbxManager = nullptr;
static bool        gProcessedOnce = false;


class MyPlugin : public FbxPlugin
{
	FBXSDK_PLUGIN_DECLARE(MyPlugin); //This macro is mandatory for any plug-in definition
	
protected:
	explicit MyPlugin(const FbxPluginDef& pDefinition, FbxModule pModuleHandle) : FbxPlugin(pDefinition, pModuleHandle)
	{	
	}
	
	//Abstract functions that *must* be implemented
	virtual bool SpecificInitialize()
	{
		//For example, here we could register as many new I/O readers/writers as we would like, or classes, etc.
		return true;
	}
	
	virtual bool SpecificTerminate()
	{
		//Here we would have to unregister whatever we registered to the FBX SDK
		return true;
	}
};

FBXSDK_PLUGIN_IMPLEMENT(MyPlugin); //This macro is mandatory for any plug-in implementation

/** FBX Interface
*/
extern "C"
{
	static MyPlugin* sMyPluginInstance = nullptr; //The module is owner of the plug-in

    // This function will be called when an application will request the plug-in
    EXPORT_DLL void FBXPluginRegistration(FbxPluginContainer& pContainer, FbxModule pFbxModule)
    {
		if (sMyPluginInstance == nullptr)
		{
            //Create the plug-in definition which contains the information about the plug-in
			FbxPluginDef sPluginDef;
			sPluginDef.mName = "ReferencesFix";
			sPluginDef.mVersion = "1.0";
			
			//Create an instance of the plug-in
			sMyPluginInstance = MyPlugin::Create(sPluginDef, pFbxModule);
			
			//Register the plug-in with the FBX SDK
			pContainer.Register(*sMyPluginInstance);
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
    // update IsLocked and IsLoaded flags for reference file object
	
	if (FBIS(pFBComponent, FBFileReference) && nullptr != pFbxObject)
	{
		FBFileReference *pRef = FBCast<FBFileReference>(pFBComponent);

		FBProperty *prop = pRef->PropertyList.Find("TempIsLoaded");
		if (nullptr == prop)
			prop = pRef->PropertyCreate("TempIsLoaded", kFBPT_bool, "bool", false, true);

		if (nullptr != prop)
			prop->SetInt(pRef->IsLoaded.AsInt());

		//pRef->ReferenceFilePath.SetPropertyValue("D:\\Temp\\temp3.fbx");
		pRef->IsLoaded.SetPropertyValue(false);
		
		
		if (FbxSceneReference* pref = FbxCast<FbxSceneReference>(pFbxObject))
		{
			const FbxString path = pref->ReferenceFilePath.Get();
			const bool isLoaded = pref->IsLoaded.Get();

			if (nullptr != prop)
			{
				int value = 0;
				if (isLoaded)
				{
					if (std::filesystem::exists(path.Buffer()))
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
