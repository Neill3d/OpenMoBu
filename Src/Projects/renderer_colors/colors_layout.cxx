
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: colors_layout.cxx
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/OpenMoBu
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declarations
#include "colors_layout.h"

//--- Registration define
#define ORColorsRendererCallback__LAYOUT		ORCustomRendererCallbackLayout

//--- FiLMBOX implementation and registration
FBRendererCallbackLayoutImplementation(	ORColorsRendererCallback__LAYOUT		);
FBRegisterRendererCallbackLayout	  (	ORColorsRendererCallback__LAYOUT,
                                       ORColorsRendererCallback__CLASSSTR,
                                       FB_DEFAULT_SDK_ICON			);

/************************************************
*	FiLMBOX constructor.
************************************************/
bool ORCustomRendererCallbackLayout::FBCreate()
{
    UICreate	();
    UIConfigure	();
    UIReset		();

    return true;
}


/************************************************
*	FiLMBOX destructor.
************************************************/
void ORCustomRendererCallbackLayout::FBDestroy()
{
}


/************************************************
*	Create the UI.
************************************************/
void ORCustomRendererCallbackLayout::UICreate()
{
    int lS=4;
    //int lH=18;
    //int lW=100;

    AddRegion( "ButtonTest", "ButtonTest",
        lS, kFBAttachLeft,	"",	1.0,
        lS,	kFBAttachTop,	"",	1.0,
        -lS,kFBAttachRight,	"",	1.0,
        -lS,kFBAttachBottom,"", 1.0 );
    SetControl( "ButtonTest", mButtonTest );
}


/************************************************
*	Configure the UI.
************************************************/
void ORCustomRendererCallbackLayout::UIConfigure()
{
    mButtonTest.OnClick.Add( this, (FBCallback)&ORCustomRendererCallbackLayout::OnButtonTestClick );
}


/************************************************
*	Reset the UI from the device.
************************************************/
void ORCustomRendererCallbackLayout::UIReset()
{
    mButtonTest.Caption = "Test";
}

/************************************************
*	Button clicked.
************************************************/
void ORCustomRendererCallbackLayout::OnButtonTestClick(HISender pSender, HKEvent pEvent)
{
    mButtonTest.Caption = "Clicked";
}


