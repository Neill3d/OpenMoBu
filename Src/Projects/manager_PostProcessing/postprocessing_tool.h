#ifndef __POST_PROCESSING_TOOL_H__
#define __POST_PROCESSING_TOOL_H__

/** \file postprocessing_tool.h

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include
#include <fbsdk/fbsdk.h>

class PostPersistentData;

//--- Registration define
#define TOOL_POSTPROCESSING__CLASSNAME	ToolPostProcessing
#define TOOL_POSTPROCESSING__CLASSSTR	"ToolPostProcessing"

////////////////////////////////////////////////////////////////////////////
//! Simple tool that saves persistent data into FBX files.
class ToolPostProcessing : public FBTool
{
    //--- FiLMBOX declaration.
	FBToolDeclare(ToolPostProcessing, FBTool);

public:
    virtual bool FBCreate();        //!< FiLMBOX Constructor.
    virtual void FBDestroy();       //!< FiLMBOX Destructor.

private:
    //! Create the UI.
    void    UICreate    ();
    void    UIConfigure ();
    void    UIReset     ();
    void    UIRefresh   ();

    //--- UI Callbacks
    void    EventTextChanged                    ( HISender pSender, HKEvent pEvent  );      //!< \b UI: Text updated callback.

    //--- Tool Callbacks
    void    EventToolShow   ( HISender pSender, HKEvent pEvent  );          //!< \b Show event in tool.
    void    EventToolIdle   ( HISender pSender, HKEvent pEvent  );          //!< \b Idle event in tool.

    void    EventOnFileExit( HISender pSender, HKEvent pEvent   );          //!< \b Callback for File->Exit.
    void    EventOnFileNew( HISender pSender, HKEvent pEvent    );          //!< \b Callback for File->New (before anything is deleted).
    void    EventOnFileNewCompleted( HISender pSender, HKEvent pEvent   );  //!< \b Callback for File->New (When the new scene is there).
    void    EventOnFileOpen( HISender pSender, HKEvent pEvent    );         //!< \b Callback for File->Open.
    void    EventOnFileOpenCompleted( HISender pSender, HKEvent pEvent   ); //!< \b Callback for File->Open.
    void    EventOnFileSave( HISender pSender, HKEvent pEvent    );         //!< \b Callback for File->Save.
    void    EventOnFileSaveCompleted( HISender pSender, HKEvent pEvent   ); //!< \b Callback for File->Save.

private:

    FBSystem                mSystem;
    FBApplication           mApplication;

	PostPersistentData*   FindPersistentData(bool pCreate);
    void                    DeletePersistentData();
    FBString                GetPersistentData();
    void                    SetPersistentData( FBString pText );

	FBButton				mButtonFishEyeEffect;

    FBLabel                 mLabelDirections;
    FBEdit                  mPersistentText;
};

#endif /* __POST_PROCESSING_TOOL_H__ */
