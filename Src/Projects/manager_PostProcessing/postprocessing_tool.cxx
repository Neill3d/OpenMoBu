
/** \file postprocessing_tool.cxx

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

// Class declaration
#include "postprocessing_tool.h"
#include "postprocessing_data.h"

//--- Registration defines
#define TOOL_POSTPROCESSING__CLASS     TOOL_POSTPROCESSING__CLASSNAME
#define TOOL_POSTPROCESSING__LABEL     "Post Processing Tool"
#define TOOL_POSTPROCESSING__DESC      "Post Processing Tool"

//--- FiLMBOX Registration & Implementation.
FBToolImplementation(TOOL_POSTPROCESSING__CLASS);
FBRegisterTool(	TOOL_POSTPROCESSING__CLASS,
				TOOL_POSTPROCESSING__LABEL,
				TOOL_POSTPROCESSING__DESC,
                        FB_DEFAULT_SDK_ICON     );  // Icon filename (default=Open Reality icon)


/************************************************
 *  FiLMBOX Constructor.
 ************************************************/
bool ToolPostProcessing::FBCreate()
{

    StartSize[0] = 175;
    StartSize[1] = 95;

    UICreate    ();
    UIConfigure ();
    UIReset     ();

	OnShow.Add(this, (FBCallback)&ToolPostProcessing::EventToolShow);
	OnIdle.Add(this, (FBCallback)&ToolPostProcessing::EventToolIdle);

	mApplication.OnFileExit.Add(this, (FBCallback)&ToolPostProcessing::EventOnFileExit);
	mApplication.OnFileNew.Add(this, (FBCallback)&ToolPostProcessing::EventOnFileNew);
	mApplication.OnFileNewCompleted.Add(this, (FBCallback)&ToolPostProcessing::EventOnFileNewCompleted);
	mApplication.OnFileOpen.Add(this, (FBCallback)&ToolPostProcessing::EventOnFileOpen);
	mApplication.OnFileOpenCompleted.Add(this, (FBCallback)&ToolPostProcessing::EventOnFileOpenCompleted);
	mApplication.OnFileSave.Add(this, (FBCallback)&ToolPostProcessing::EventOnFileSave);
	mApplication.OnFileSaveCompleted.Add(this, (FBCallback)&ToolPostProcessing::EventOnFileSaveCompleted);

    return true;
}


/************************************************
 *  FiLMBOX Destructor.
 ************************************************/
void ToolPostProcessing::FBDestroy()
{
	OnShow.Remove(this, (FBCallback)&ToolPostProcessing::EventToolShow);
	OnIdle.Remove(this, (FBCallback)&ToolPostProcessing::EventToolIdle);

	mApplication.OnFileExit.Remove(this, (FBCallback)&ToolPostProcessing::EventOnFileExit);
	mApplication.OnFileNew.Remove(this, (FBCallback)&ToolPostProcessing::EventOnFileNew);
	mApplication.OnFileNewCompleted.Remove(this, (FBCallback)&ToolPostProcessing::EventOnFileNewCompleted);
	mApplication.OnFileOpen.Remove(this, (FBCallback)&ToolPostProcessing::EventOnFileOpen);
	mApplication.OnFileOpenCompleted.Remove(this, (FBCallback)&ToolPostProcessing::EventOnFileOpenCompleted);
	mApplication.OnFileSave.Remove(this, (FBCallback)&ToolPostProcessing::EventOnFileSave);
	mApplication.OnFileSaveCompleted.Remove(this, (FBCallback)&ToolPostProcessing::EventOnFileSaveCompleted);
}


/************************************************
 *  Create the UI.
 ************************************************/
void ToolPostProcessing::UICreate()
{
    int lS = 4;
    int lW = 100;
    int lH = 18;

    // Create regions
    
	AddRegion( "LabelDirections",           "LabelDirections",
                                            lS, kFBAttachLeft,  "",     1.0,
                                            lS, kFBAttachTop,   "",     1.0,
                                            -lS,kFBAttachRight, "",     1.0,
                                            30, kFBAttachNone,  NULL,   1.0 );
    AddRegion( "PersistentText",            "PersistentText",
                                            lS, kFBAttachLeft,  "",     1.0,
                                            lS, kFBAttachBottom,"LabelDirections",1.0,
                                            lW, kFBAttachNone,  NULL,   1.0,
                                            lH, kFBAttachNone,  NULL,   1.0);
    // Assign regions
    SetControl( "LabelDirections",          mLabelDirections            );
    SetControl( "PersistentText",           mPersistentText             );
}


/************************************************
 *  .
 ************************************************/
void ToolPostProcessing::UIConfigure()
{
    // Configuration
    mLabelDirections.Caption        = "Create persistent data for the\n"
                                      "tool by entering some text:";
    mPersistentText.Text            = GetPersistentData();

    // Callbacks
	mPersistentText.OnChange.Add(this, (FBCallback)&ToolPostProcessing::EventTextChanged);
}


/************************************************
 *  .
 ************************************************/
void ToolPostProcessing::UIReset()
{

}


/************************************************
 *  .
 ************************************************/
void ToolPostProcessing::UIRefresh()
{

}


/************************************************
 *  Create actor button callback.
 ************************************************/
void ToolPostProcessing::EventTextChanged(HISender pSender, HKEvent pEvent)
{
    SetPersistentData( (FBString)mPersistentText.Text );
}

/************************************************
 *  FBApplication callbacks.
 ************************************************/

void ToolPostProcessing::EventOnFileExit(HISender pSender, HKEvent pEvent)
{
    // nothing to do...
}

void ToolPostProcessing::EventOnFileNew(HISender pSender, HKEvent pEvent)
{
    // nothing to do...
}

void ToolPostProcessing::EventOnFileNewCompleted(HISender pSender, HKEvent pEvent)
{
    // nothing to do...
}

void ToolPostProcessing::EventOnFileOpen(HISender pSender, HKEvent pEvent)
{
    // nothing to do...
}

void ToolPostProcessing::EventOnFileOpenCompleted(HISender pSender, HKEvent pEvent)
{
    // Now that the file open, we want to see if there is already some
    // persistent data for us in the scene. If not, we use the current value.
}

void ToolPostProcessing::EventOnFileSave(HISender pSender, HKEvent pEvent)
{
    // nothing to do...
}

void ToolPostProcessing::EventOnFileSaveCompleted(HISender pSender, HKEvent pEvent)
{
    // nothing to do...
}


/************************************************
 *  Show event.
 ************************************************/
void ToolPostProcessing::EventToolShow(HISender pSender, HKEvent pEvent)
{
    FBEventShow lEvent( pEvent );

    if( lEvent.Shown )
    {
        UIReset();
    }
    else
    {
    }
}

/************************************************
 *  UI Idle event.
 ************************************************/
void ToolPostProcessing::EventToolIdle(HISender pSender, HKEvent pEvent)
{
    UIRefresh();
}


/************************************************
 *  
 ************************************************/
PostPersistentData* ToolPostProcessing::FindPersistentData(bool pCreate)
{
    // First we want to see if there is any infos for this tool present in the scene
    FBScene* lScene = mSystem.Scene;
    int lIdx = 0;
    
    for( lIdx = 0; lIdx < lScene->UserObjects.GetCount(); ++lIdx ) {
        FBUserObject* lObject = lScene->UserObjects[lIdx];
		if (lObject->Is(PostPersistentData::TypeInfo)) {
			return (PostPersistentData*)lObject;
        }
    }

	PostPersistentData* lData = 0;

    if( pCreate ) {
		lData = new PostPersistentData("ORToolPersistentData");
        lData->mText = "Default";
    }
    
    return lData;
}

void ToolPostProcessing::DeletePersistentData()
{
	PostPersistentData* lData = FindPersistentData(false);
    if( lData ) {
        lData->FBDelete();
    }
}

FBString ToolPostProcessing::GetPersistentData()
{
    return FindPersistentData( true )->mText;
}

void ToolPostProcessing::SetPersistentData(FBString pText)
{
    FindPersistentData( true )->mText = pText;
}
