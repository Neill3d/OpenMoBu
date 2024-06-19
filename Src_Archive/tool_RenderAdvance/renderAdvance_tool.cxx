
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://code.google.com/p/motioncodelibrary/wiki/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////

//--- Class declaration
#include "renderadvance_tool.h"
#include "resource.h"
#include "resourceUtils.h"

//--- Registration defines
#define RENDERADVANCE__CLASS	RENDERADVANCE__CLASSNAME
#define RENDERADVANCE__LABEL	"Render Advance"
#define RENDERADVANCE__DESC	"MultiView render"

//--- Implementation and registration
FBToolImplementation(	RENDERADVANCE__CLASS	);
FBRegisterTool		(	RENDERADVANCE__CLASS,
						RENDERADVANCE__LABEL,
						RENDERADVANCE__DESC,
						FB_DEFAULT_SDK_ICON	);	// Icon filename (default=Open Reality icon)

/************************************************
 *	Constructor.
 ************************************************/
bool ToolRenderAdvance::FBCreate()
{
	StartSize[0] = 800;
	StartSize[1] = 800;

    // Manage UI
	UICreate	();
	UIConfigure	();
	UIReset		();

	// Add tool callbacks
	OnShow.Add	( this, (FBCallback) &ToolRenderAdvance::EventToolShow		);
	OnIdle.Add	( this, (FBCallback) &ToolRenderAdvance::EventToolIdle		);
	OnResize.Add( this, (FBCallback) &ToolRenderAdvance::EventToolResize		);
	OnPaint.Add	( this, (FBCallback) &ToolRenderAdvance::EventToolPaint		);
	OnInput.Add	( this, (FBCallback) &ToolRenderAdvance::EventToolInput		);

	return true;
}

/************************************************
 *	Create, configure & reset UI.
 ************************************************/
void ToolRenderAdvance::UICreate()
{
	// add regions
	AddRegion( "TabPanel",	"TabPanel",
			10,		kFBAttachLeft,		"",		1.0,
			5,		kFBAttachNone,		"",		1.0,
			-5,		kFBAttachLeft,		"Settings",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	AddRegion( "Layout",	"Layout",
			10,		kFBAttachLeft,		"",		1.0,
			10,		kFBAttachBottom,		"TabPanel",		1.0,
			-5,		kFBAttachLeft,		"Settings",		1.0,
			200,	kFBAttachNone,		"",		1.0 );
	AddRegion( "Settings",	"Settings",
			-200,		kFBAttachRight,		"",		1.0,
			5,		kFBAttachNone,		"",		1.0,
			200,		kFBAttachNone,		"",		1.0,
			150,		kFBAttachNone,		"",		1.0 );
	AddRegion( "ButtonRender",	"ButtonRender",
			0,		kFBAttachLeft,		"Settings",		1.0,
			5,		kFBAttachBottom,	"Settings",		1.0,
			80,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	AddRegion( "ButtonAbout",	"ButtonAbout",
			10,		kFBAttachRight,		"ButtonRender",		1.0,
			5,		kFBAttachBottom,	"Settings",		1.0,
			80,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	AddRegion( "View3d",	"View3d",
			5,		kFBAttachLeft,		"",		1.0,
			5,		kFBAttachBottom,		"Layout",		1.0,
			640,		kFBAttachNone,		"",		1.0,
			480,		kFBAttachNone,		"",		1.0 );

	// Assign regions
	SetControl( "TabPanel", mTabPanel );
	SetControl( "Layout", mLayout[0] );
	SetControl( "Settings", mLayoutSettings );
	SetControl( "ButtonRender", mButtonRender );
	SetControl( "ButtonAbout", mButtonAbout );
	SetView		( "View3d",          mView            );

	UICreate1();
	UICreate2();
	UICreate3();
}

/************************************************
 *	regions for render settings
 ************************************************/
void ToolRenderAdvance::UICreate1()
{
	FBLayout &lLayout = mLayout[0];

	// add regions
	lLayout.AddRegion( "region1",	"region1",
			5,		kFBAttachNone,		"",		1.0,
			5,		kFBAttachNone,		"",		1.0,
			80,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "EditFile",	"EditFile",
			100,		kFBAttachNone,		"",		1.0,
			5,		kFBAttachNone,		"",		1.0,
			215,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "ButtonBrowse",	"ButtonBrowse",
			320,		kFBAttachNone,		"",		1.0,
			5,		kFBAttachNone,		"",		1.0,
			70,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "region4",	"region4",
			5,		kFBAttachNone,		"",		1.0,
			40,		kFBAttachNone,		"",		1.0,
			30,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "EditStart",	"EditStart",
			40,		kFBAttachNone,		"",		1.0,
			40,		kFBAttachNone,		"",		1.0,
			70,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "region6",	"region6",
			145,		kFBAttachNone,		"",		1.0,
			40,		kFBAttachNone,		"",		1.0,
			30,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "EditStop",	"EditStop",
			180,		kFBAttachNone,		"",		1.0,
			40,		kFBAttachNone,		"",		1.0,
			70,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "region8",	"region8",
			280,		kFBAttachNone,		"",		1.0,
			40,		kFBAttachNone,		"",		1.0,
			30,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "EditStep",	"EditStep",
			315,		kFBAttachNone,		"",		1.0,
			40,		kFBAttachNone,		"",		1.0,
			70,		kFBAttachNone,		"",		1.0,
			25,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "region10",	"region10",
			15,		kFBAttachNone,		"",		1.0,
			80,		kFBAttachNone,		"",		1.0,
			100,		kFBAttachNone,		"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "ListPicture",	"ListPicture",
			125,		kFBAttachNone,		"",		1.0,
			80,		kFBAttachNone,		"",		1.0,
			110,		kFBAttachNone,		"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "region12",	"region12",
			15,		kFBAttachNone,		"",		1.0,
			105,		kFBAttachNone,		"",		1.0,
			100,		kFBAttachNone,		"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "ListField",	"ListField",
			125,		kFBAttachNone,		"",		1.0,
			105,		kFBAttachNone,		"",		1.0,
			110,		kFBAttachNone,		"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "region14",	"region14",
			15,		kFBAttachNone,		"",		1.0,
			130,		kFBAttachNone,		"",		1.0,
			100,		kFBAttachNone,		"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "ListPixel",	"ListPixel",
			125,		kFBAttachNone,		"",		1.0,
			130,		kFBAttachNone,		"",		1.0,
			110,		kFBAttachNone,		"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "ButtonAudio",	"ButtonAudio",
			15,		kFBAttachNone,		"",		1.0,
			165,		kFBAttachNone,		"",		1.0,
			120,		kFBAttachNone,		"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
}

/************************************************
 *	regions  for view settings
 ************************************************/
void ToolRenderAdvance::UICreate2()
{
	FBLayout	&lLayout = mLayout[1];

	// add regions
	lLayout.AddRegion( "LabelLayout",	"LabelLayout",
			5,		kFBAttachNone,		"",		1.0,
			10,		kFBAttachNone,		"",		1.0,
			65,		kFBAttachNone,		"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "ListLayout",	"ListLayout",
			75,		kFBAttachNone,		"",		1.0,
			10,		kFBAttachNone,		"",		1.0,
			110,		kFBAttachNone,		"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "LabelCamera1",	"LabelCamera1",
			5,		kFBAttachNone,		"",		1.0,
			50,		kFBAttachNone,		"",		1.0,
			60,		kFBAttachNone,		"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "ListCamera1",	"ListCamera1",
			70,		kFBAttachNone,		"",		1.0,
			50,		kFBAttachNone,		"",		1.0,
			130,		kFBAttachNone,		"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "LabelCamera2",	"LabelCamera2",
			5,		kFBAttachNone,		"",		1.0,
			75,		kFBAttachNone,		"",		1.0,
			60,		kFBAttachNone,		"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "ListCamera2",	"ListCamera2",
			70,		kFBAttachNone,		"",		1.0,
			75,		kFBAttachNone,		"",		1.0,
			130,		kFBAttachNone,		"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "LabelCamera3",	"LabelCamera3",
			5,		kFBAttachNone,		"",		1.0,
			100,		kFBAttachNone,		"",		1.0,
			60,		kFBAttachNone,		"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "ListCamera3",	"ListCamera3",
			70,		kFBAttachNone,		"",		1.0,
			100,		kFBAttachNone,		"",		1.0,
			130,		kFBAttachNone,		"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "LabelCamera4",	"LabelCamera4",
			5,		kFBAttachNone,		"",		1.0,
			125,		kFBAttachNone,		"",		1.0,
			60,		kFBAttachNone,		"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "ListCamera4",	"ListCamera4",
			70,		kFBAttachNone,		"",		1.0,
			125,		kFBAttachNone,		"",		1.0,
			130,		kFBAttachNone,		"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "LabelDisplay",	"LabelDisplay",
			195,		kFBAttachNone,		"",		1.0,
			15,		kFBAttachNone,		"",		1.0,
			125,		kFBAttachNone,		"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "ListDisplay",	"ListDisplay",
			215,		kFBAttachNone,		"",		1.0,
			40,		kFBAttachNone,		"",		1.0,
			135,		kFBAttachNone,		"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "ButtonShowTime",	"ButtonShowTime",
			215,		kFBAttachNone,		"",		1.0,
			70,		kFBAttachNone,		"",		1.0,
			120,		kFBAttachNone,		"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "ButtonShowSafe",	"ButtonShowSafe",
			215,		kFBAttachNone,		"",		1.0,
			95,		kFBAttachNone,		"",		1.0,
			120,		kFBAttachNone,		"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "ButtonShowLabel",	"ButtonShowLabel",
			215,		kFBAttachNone,		"",		1.0,
			120,		kFBAttachNone,		"",		1.0,
			120,		kFBAttachNone,		"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	lLayout.AddRegion( "ButtonAntialiasing",	"ButtonAntialiasing",
			215,		kFBAttachNone,		"",		1.0,
			145,		kFBAttachNone,		"",		1.0,
			120,		kFBAttachNone,		"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
}

/************************************************
 *	regions for grab output info
 ************************************************/
void ToolRenderAdvance::UICreate3()
{
	mLayoutSettings.AddRegion( "TotalFrameCount",	"TotalFrameCount",
			5,		kFBAttachNone,		"",		1.0,
			5,		kFBAttachNone,		"",		1.0,
			180,		kFBAttachNone,	"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	mLayoutSettings.AddRegion( "RemainingFrameCount",	"RemainingFrameCount",
			5,		kFBAttachNone,		"",		1.0,
			5,		kFBAttachBottom,		"TotalFrameCount",		1.0,
			180,		kFBAttachNone,	"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	mLayoutSettings.AddRegion( "TotalTimeElapsed",	"TotalTimeElapsed",
			5,		kFBAttachNone,		"",		1.0,
			5,		kFBAttachBottom,		"RemainingFrameCount",		1.0,
			180,		kFBAttachNone,	"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	mLayoutSettings.AddRegion( "EstimatedTime",	"EstimatedTime",
			5,		kFBAttachNone,		"",		1.0,
			5,		kFBAttachBottom,		"TotalTimeElapsed",		1.0,
			180,		kFBAttachNone,	"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	mLayoutSettings.AddRegion( "EstimatedTimeRemaining",	"EstimatedTimeRemaining",
			5,		kFBAttachNone,		"",		1.0,
			5,		kFBAttachBottom,		"EstimatedTime",		1.0,
			180,		kFBAttachNone,	"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
	mLayoutSettings.AddRegion( "TimePerFrame",	"TimePerFrame",
			5,		kFBAttachNone,		"",		1.0,
			5,		kFBAttachBottom,		"EstimatedTimeRemaining",		1.0,
			180,		kFBAttachNone,	"",		1.0,
			20,		kFBAttachNone,		"",		1.0 );
}

/************************************************
 *	config UI (params, events)
 ************************************************/
void ToolRenderAdvance::UIConfigure()
{
    mFilePopup.Caption  = "Where to save the rendering...";
    mFilePopup.Style    = kFBFilePopupSave;
	mFilePopup.Filter	= "*.*";
    mFilePopup.Path     = mView.GetFilePath();
    mFilePopup.FileName = mView.GetFileName();

    // We have to explicitely set the caption this way because the property 'FBFilePopup::FullFilename'
    // Is not set until e 'FBFilePopup::Execute()' has been done... Unfortunate.
    mButtonFilePopup.Caption = mView.GetFilePath() + FBString( "\\" ) + mView.GetFileName();
	mButtonFilePopup.OnClick.Add( this, (FBCallback) &ToolRenderAdvance::EventButtonFilePopup );

	mTabPanel.Items.SetString( "Render settings~View settings" );
	mTabPanel.OnChange.Add( this, (FBCallback) &ToolRenderAdvance::EventTabChange );

	mButtonRender.Caption = "Render";
	mButtonRender.OnClick.Add( this, (FBCallback) &ToolRenderAdvance::EventButtonRender );

	mButtonAbout.Caption = "About";
	mButtonAbout.OnClick.Add(this, (FBCallback) &ToolRenderAdvance::EventButtonAbout );

	UIConfigure1();
	UIConfigure2();
	UIConfigure3();
}

/************************************************
 *	config mLayoutSettigns - grab settings output labels
 ************************************************/
void ToolRenderAdvance::UIConfigure3()
{
	mLayoutSettings.SetControl( "TotalFrameCount", mView.mTotalFrameCount );
	mLayoutSettings.SetControl( "RemainingFrameCount", mView.mRemainingFrameCount );
	mLayoutSettings.SetControl( "TotalTimeElapsed", mView.mTotalTimeElapsed );
	mLayoutSettings.SetControl( "EstimatedTime", mView.mEstimatedTime );
	mLayoutSettings.SetControl( "EstimatedTimeRemaining", mView.mEstimatedTimeRemaining );
	mLayoutSettings.SetControl( "TimePerFrame", mView.mTimePerFrame );
}

/************************************************
 *	config mLayout[0] - render settings
 ************************************************/
void ToolRenderAdvance::UIConfigure1()
{
	FBTime		lTime;
	int				frame;
	FBLayout	&lLayout	= mLayout[0];

	// add controls
	lLayout.SetControl("region1", region1);
	region1.Visible = true;
	region1.ReadOnly = false;
	region1.Enabled = true;
	region1.Hint = "";
	region1.Caption = "Output filename";
	region1.Style = kFBTextStyleNone;
	region1.Justify = kFBTextJustifyLeft;
	region1.WordWrap = false;

	lLayout.SetControl("EditFile", EditFile);
	EditFile.Visible = true;
	EditFile.ReadOnly = false;
	EditFile.Enabled = true;
	EditFile.Hint = "";
	EditFile.Text = "C:\\Output.avi";
	EditFile.PasswordMode = false;

	lLayout.SetControl("ButtonBrowse", ButtonBrowse);
	ButtonBrowse.Visible = true;
	ButtonBrowse.ReadOnly = false;
	ButtonBrowse.Enabled = true;
	ButtonBrowse.Hint = "";
	ButtonBrowse.Caption = "Browse...";
	ButtonBrowse.State = 0;
	ButtonBrowse.Style = kFBPushButton;
	ButtonBrowse.Justify = kFBTextJustifyCenter;
	ButtonBrowse.Look = kFBLookNormal;
	ButtonBrowse.OnClick.Add( this, (FBCallback) &ToolRenderAdvance::EventButtonFilePopup );

	lLayout.SetControl("region4", region4);
	region4.Visible = true;
	region4.ReadOnly = false;
	region4.Enabled = true;
	region4.Hint = "";
	region4.Caption = "Start";
	region4.Style = kFBTextStyleNone;
	region4.Justify = kFBTextJustifyLeft;
	region4.WordWrap = false;

	lTime = FBPlayerControl().ZoomWindowStart;
#ifdef OLD_FBTIME_GETFRAME
	frame = lTime.GetFrame(true);
#else
	frame = lTime.GetFrame();
#endif
	lLayout.SetControl("EditStart", EditStart);
	EditStart.Visible = true;
	EditStart.ReadOnly = false;
	EditStart.Enabled = true;
	EditStart.Hint = "";
	EditStart.Value = (double)frame;
	EditStart.Min = -9999.000000;
	EditStart.Max = 99999.000000;
	EditStart.Precision = 1.000000;
	EditStart.LargeStep = 5.000000;
	EditStart.SmallStep = 1.000000;

	lLayout.SetControl("region6", region6);
	region6.Visible = true;
	region6.ReadOnly = false;
	region6.Enabled = true;
	region6.Hint = "";
	region6.Caption = "Stop:";
	region6.Style = kFBTextStyleNone;
	region6.Justify = kFBTextJustifyLeft;
	region6.WordWrap = false;

	lTime = FBPlayerControl().ZoomWindowStop;
#ifdef OLD_FBTIME_GETFRAME
	frame = lTime.GetFrame(true);
#else
	frame = lTime.GetFrame();
#endif
	lLayout.SetControl("EditStop", EditStop);
	EditStop.Visible = true;
	EditStop.ReadOnly = false;
	EditStop.Enabled = true;
	EditStop.Hint = "";
	EditStop.Value = (double)frame;
	EditStop.Min = -9999.000000;
	EditStop.Max = 99999.000000;
	EditStop.Precision = 1.000000;
	EditStop.LargeStep = 5.000000;
	EditStop.SmallStep = 1.000000;

	lLayout.SetControl("region8", region8);
	region8.Visible = true;
	region8.ReadOnly = false;
	region8.Enabled = true;
	region8.Hint = "";
	region8.Caption = "Step:";
	region8.Style = kFBTextStyleNone;
	region8.Justify = kFBTextJustifyLeft;
	region8.WordWrap = false;

	lLayout.SetControl("EditStep", EditStep);
	EditStep.Visible = true;
	EditStep.ReadOnly = false;
	EditStep.Enabled = true;
	EditStep.Hint = "";
	EditStep.Value = 1.000000;
	EditStep.Min = -9999.000000;
	EditStep.Max = 99999.000000;
	EditStep.Precision = 1.000000;
	EditStep.LargeStep = 0.000000;
	EditStep.SmallStep = 0.000000;

	lLayout.SetControl("region10", region10);
	region10.Visible = true;
	region10.ReadOnly = false;
	region10.Enabled = true;
	region10.Hint = "";
	region10.Caption = "Picture Format:";
	region10.Style = kFBTextStyleNone;
	region10.Justify = kFBTextJustifyLeft;
	region10.WordWrap = true;

	lLayout.SetControl("ListPicture", ListPicture);
	ListPicture.Visible = true;
	ListPicture.ReadOnly = false;
	ListPicture.Enabled = true;
	ListPicture.Hint = "";
	ListPicture.ItemIndex = -1;
	ListPicture.MultiSelect = false;
	ListPicture.ExtendedSelect = false;
	ListPicture.Style = kFBDropDownList;
	ListPicture.Items.SetString( "From Camera~D1 NTSC~NTSC~PAL~D1 PAL~HD 1920x1080~640x480~320x200~320x240~128x128~FullScreen" );
	ListPicture.ItemIndex = 6;
	ListPicture.OnChange.Add( this, (FBCallback) &ToolRenderAdvance::EventPictureChange );

	lLayout.SetControl("region12", region12);
	region12.Visible = true;
	region12.ReadOnly = false;
	region12.Enabled = true;
	region12.Hint = "";
	region12.Caption = "Field Mode:";
	region12.Style = kFBTextStyleNone;
	region12.Justify = kFBTextJustifyLeft;
	region12.WordWrap = true;

	lLayout.SetControl("ListField", ListField);
	ListField.Visible = true;
	ListField.ReadOnly = false;
	ListField.Enabled = true;
	ListField.Hint = "";
	ListField.ItemIndex = -1;
	ListField.MultiSelect = false;
	ListField.ExtendedSelect = false;
	ListField.Style = kFBDropDownList;
	ListField.Items.SetString( "No Field~Field 0~Field 1~Half Field 0~Half Field 1" );

	lLayout.SetControl("region14", region14);
	region14.Visible = true;
	region14.ReadOnly = false;
	region14.Enabled = true;
	region14.Hint = "";
	region14.Caption = "Pixel Format:";
	region14.Style = kFBTextStyleNone;
	region14.Justify = kFBTextJustifyLeft;
	region14.WordWrap = true;

	lLayout.SetControl("ListPixel", ListPixel);
	ListPixel.Visible = true;
	ListPixel.ReadOnly = false;
	ListPixel.Enabled = true;
	ListPixel.Hint = "";
	ListPixel.ItemIndex = -1;
	ListPixel.MultiSelect = false;
	ListPixel.ExtendedSelect = false;
	ListPixel.Style = kFBDropDownList;
	ListPixel.Items.SetString( "24 bits~32 bits" );

	lLayout.SetControl("ButtonAudio", ButtonAudio);
	ButtonAudio.Visible = true;
	ButtonAudio.ReadOnly = false;
	ButtonAudio.Enabled = true;
	ButtonAudio.Hint = "";
	ButtonAudio.Caption = "Audio";
	ButtonAudio.State = 0;
	ButtonAudio.Style = kFBCheckbox;
	ButtonAudio.Justify = kFBTextJustifyLeft;
	ButtonAudio.Look = kFBLookNormal;
}

/************************************************
 *	config mLayout[1]
 ************************************************/
void ToolRenderAdvance::UIConfigure2()
{
	
	FBLayout	&lLayout = mLayout[1];
		
	// string of cameras names
	FBString	cameraNames;
	int count = mSystem.Scene->Cameras.GetCount();
	for (int i=0; i<count; i++)
	{
		FBCamera *lCamera = mSystem.Scene->Cameras[i];
		cameraNames = cameraNames + lCamera->Name;
		if (i < (count-1) )
			cameraNames = cameraNames + "~";
	}

	// add controls
	lLayout.SetControl("LabelLayout", LabelLayout);
	LabelLayout.Visible = true;
	LabelLayout.ReadOnly = false;
	LabelLayout.Enabled = true;
	LabelLayout.Hint = "";
	LabelLayout.Caption = "View layout";
	LabelLayout.Style = kFBTextStyleNone;
	LabelLayout.Justify = kFBTextJustifyLeft;
	LabelLayout.WordWrap = false;

	lLayout.SetControl("ListLayout", ListLayout);
	ListLayout.Visible = true;
	ListLayout.ReadOnly = false;
	ListLayout.Enabled = true;
	ListLayout.Hint = "";
	ListLayout.ItemIndex = -1;
	ListLayout.MultiSelect = false;
	ListLayout.ExtendedSelect = false;
	ListLayout.Style = kFBDropDownList;
	ListLayout.Items.SetString( "1 Sub-views~2 Sub-views~3 Sub-views~4 Sub-views" );
	ListLayout.OnChange.Add( this, (FBCallback) &ToolRenderAdvance::EventLayoutChange );

	lLayout.SetControl("LabelCamera1", LabelCamera1);
	LabelCamera1.Visible = true;
	LabelCamera1.ReadOnly = false;
	LabelCamera1.Enabled = true;
	LabelCamera1.Hint = "";
	LabelCamera1.Caption = "Camera 1:";
	LabelCamera1.Style = kFBTextStyleNone;
	LabelCamera1.Justify = kFBTextJustifyLeft;
	LabelCamera1.WordWrap = true;

	lLayout.SetControl("ListCamera1", ListCamera1);
	ListCamera1.Visible = true;
	ListCamera1.ReadOnly = false;
	ListCamera1.Enabled = true;
	ListCamera1.Hint = "";
	ListCamera1.ItemIndex = -1;
	ListCamera1.MultiSelect = false;
	ListCamera1.ExtendedSelect = false;
	ListCamera1.Style = kFBDropDownList;
	ListCamera1.Items.SetString( cameraNames );
	ListCamera1.OnChange.Add( this, (FBCallback) &ToolRenderAdvance::EventCameraChange1 );

	lLayout.SetControl("LabelCamera2", LabelCamera2);
	LabelCamera2.Visible = true;
	LabelCamera2.ReadOnly = false;
	LabelCamera2.Enabled = true;
	LabelCamera2.Hint = "";
	LabelCamera2.Caption = "Camera 2:";
	LabelCamera2.Style = kFBTextStyleNone;
	LabelCamera2.Justify = kFBTextJustifyLeft;
	LabelCamera2.WordWrap = false;

	lLayout.SetControl("ListCamera2", ListCamera2);
	ListCamera2.Visible = true;
	ListCamera2.ReadOnly = false;
	ListCamera2.Enabled = true;
	ListCamera2.Hint = "";
	ListCamera2.ItemIndex = -1;
	ListCamera2.MultiSelect = false;
	ListCamera2.ExtendedSelect = false;
	ListCamera2.Style = kFBDropDownList;
	ListCamera2.Items.SetString( cameraNames );
	ListCamera2.OnChange.Add( this, (FBCallback) &ToolRenderAdvance::EventCameraChange2 );

	lLayout.SetControl("LabelCamera3", LabelCamera3);
	LabelCamera3.Visible = true;
	LabelCamera3.ReadOnly = false;
	LabelCamera3.Enabled = true;
	LabelCamera3.Hint = "";
	LabelCamera3.Caption = "Camera 3:";
	LabelCamera3.Style = kFBTextStyleNone;
	LabelCamera3.Justify = kFBTextJustifyLeft;
	LabelCamera3.WordWrap = false;

	lLayout.SetControl("ListCamera3", ListCamera3);
	ListCamera3.Visible = true;
	ListCamera3.ReadOnly = false;
	ListCamera3.Enabled = true;
	ListCamera3.Hint = "";
	ListCamera3.ItemIndex = -1;
	ListCamera3.MultiSelect = false;
	ListCamera3.ExtendedSelect = false;
	ListCamera3.Style = kFBDropDownList;
	ListCamera3.Items.SetString( cameraNames );
	ListCamera3.OnChange.Add( this, (FBCallback) &ToolRenderAdvance::EventCameraChange3 );

	lLayout.SetControl("LabelCamera4", LabelCamera4);
	LabelCamera4.Visible = true;
	LabelCamera4.ReadOnly = false;
	LabelCamera4.Enabled = true;
	LabelCamera4.Hint = "";
	LabelCamera4.Caption = "Camera 4:";
	LabelCamera4.Style = kFBTextStyleNone;
	LabelCamera4.Justify = kFBTextJustifyLeft;
	LabelCamera4.WordWrap = false;

	lLayout.SetControl("ListCamera4", ListCamera4);
	ListCamera4.Visible = true;
	ListCamera4.ReadOnly = false;
	ListCamera4.Enabled = true;
	ListCamera4.Hint = "";
	ListCamera4.ItemIndex = -1;
	ListCamera4.MultiSelect = false;
	ListCamera4.ExtendedSelect = false;
	ListCamera4.Style = kFBDropDownList;
	ListCamera4.Items.SetString( cameraNames );
	ListCamera4.OnChange.Add( this, (FBCallback) &ToolRenderAdvance::EventCameraChange4 );

	lLayout.SetControl("LabelDisplay", LabelDisplay);
	LabelDisplay.Visible = true;
	LabelDisplay.ReadOnly = false;
	LabelDisplay.Enabled = true;
	LabelDisplay.Hint = "";
	LabelDisplay.Caption = "Display Options";
	LabelDisplay.Style = kFBTextStyleNone;
	LabelDisplay.Justify = kFBTextJustifyLeft;
	LabelDisplay.WordWrap = true;

	lLayout.SetControl("ListDisplay", ListDisplay);
	ListDisplay.Visible = true;
	ListDisplay.ReadOnly = false;
	ListDisplay.Enabled = true;
	ListDisplay.Hint = "";
	ListDisplay.ItemIndex = -1;
	ListDisplay.MultiSelect = false;
	ListDisplay.ExtendedSelect = false;
	ListDisplay.Style = kFBDropDownList;
	ListDisplay.Items.SetString( "Standard~Model Only~X-Ray~Current" );
	ListDisplay.ItemIndex = 1;

	lLayout.SetControl("ButtonShowTime", ButtonShowTime);
	ButtonShowTime.Visible = true;
	ButtonShowTime.ReadOnly = false;
	ButtonShowTime.Enabled = true;
	ButtonShowTime.Hint = "";
	ButtonShowTime.Caption = "Show Time Code";
	ButtonShowTime.State = 0;
	ButtonShowTime.Style = kFBCheckbox;
	ButtonShowTime.Justify = kFBTextJustifyLeft;
	ButtonShowTime.Look = kFBLookNormal;
	ButtonShowTime.OnClick.Add( this, (FBCallback) &ToolRenderAdvance::EventShowChange );

	lLayout.SetControl("ButtonShowSafe", ButtonShowSafe);
	ButtonShowSafe.Visible = true;
	ButtonShowSafe.ReadOnly = false;
	ButtonShowSafe.Enabled = true;
	ButtonShowSafe.Hint = "";
	ButtonShowSafe.Caption = "Show Safe Area";
	ButtonShowSafe.State = 0;
	ButtonShowSafe.Style = kFBCheckbox;
	ButtonShowSafe.Justify = kFBTextJustifyLeft;
	ButtonShowSafe.Look = kFBLookNormal;
	ButtonShowSafe.OnClick.Add( this, (FBCallback) &ToolRenderAdvance::EventShowChange );

	lLayout.SetControl("ButtonShowLabel", ButtonShowLabel);
	ButtonShowLabel.Visible = true;
	ButtonShowLabel.ReadOnly = false;
	ButtonShowLabel.Enabled = true;
	ButtonShowLabel.Hint = "";
	ButtonShowLabel.Caption = "Show Camera Label";
	ButtonShowLabel.State = 0;
	ButtonShowLabel.Style = kFBCheckbox;
	ButtonShowLabel.Justify = kFBTextJustifyLeft;
	ButtonShowLabel.Look = kFBLookNormal;
	ButtonShowLabel.OnClick.Add( this, (FBCallback) &ToolRenderAdvance::EventShowChange );

	lLayout.SetControl("ButtonAntialiasing", ButtonAntialiasing);
	ButtonAntialiasing.Visible = true;
	ButtonAntialiasing.ReadOnly = false;
	ButtonAntialiasing.Enabled = true;
	ButtonAntialiasing.Hint = "";
	ButtonAntialiasing.Caption = "Enable Antialiasing";
	ButtonAntialiasing.State = 0;
	ButtonAntialiasing.Style = kFBCheckbox;
	ButtonAntialiasing.Justify = kFBTextJustifyLeft;
	ButtonAntialiasing.Look = kFBLookNormal;
}


void ToolRenderAdvance::UIReset()
{
}

/************************************************
 *	show special viewport widgets (time code, camera label, safe area).
 ************************************************/
void ToolRenderAdvance::EventShowChange( HISender pSender, HKEvent pEvent )
{
	FBRenderer	*pRender = mSystem.Scene->Renderer;
	FBViewingOptions* lVO = pRender->GetViewingOptions();
	lVO->ShowCameraLabel() = (ButtonShowLabel.State == 1);
	lVO->ShowTimeCode() = (ButtonShowTime.State == 1);
	lVO->ShowSafeArea() = (ButtonShowSafe.State == 1);
}

/************************************************
 *	Destruction function.
 ************************************************/
void ToolRenderAdvance::FBDestroy()
{
	// Remove tool callbacks
	OnShow.Remove	( this, (FBCallback) &ToolRenderAdvance::EventToolShow	);
	OnIdle.Remove	( this, (FBCallback) &ToolRenderAdvance::EventToolIdle	);
	OnPaint.Remove	( this, (FBCallback) &ToolRenderAdvance::EventToolPaint	);
	OnInput.Remove	( this, (FBCallback) &ToolRenderAdvance::EventToolInput	);
	OnResize.Remove	( this, (FBCallback) &ToolRenderAdvance::EventToolResize	);

	// Free user allocated memory
}

void ToolRenderAdvance::EventPictureChange( HISender pSender, HKEvent pEvent )
{
	int w=640,h=480;
	int ndx = ListPicture.ItemIndex;
	FBCameraResolutionMode	lResMode = (FBCameraResolutionMode) ndx;
	switch( lResMode )
	{
	case kFBResolutionCustom:
		{
			FBCamera	*lCamera = mSystem.Renderer->CurrentCamera;
			w = lCamera->ResolutionWidth;
			h =	lCamera->ResolutionHeight;
		} break;
	case kFBResolution128x128:
		w = 128;
		h = 128;
		break;
	case kFBResolution320x200:
		w = 320;
		h = 200;
		break;
	case kFBResolution320x240:
		w = 320;
		h = 240;
		break;
	case kFBResolution640x480:
		w = 640;
		h = 480;
		break;
	case kFBResolutionHD:
		w = 1920;
		h = 1080;
		break;
	case kFBResolutionPAL:
		w = 570;
		h = 486;
		break;
	case kFBResolutionD1PAL:
		w = 720;
		h = 576;
		break;
	case kFBResolutionD1NTSC:
		w = 720;
		h = 486;
		break;
	case kFBResolutionNTSC:
		w = 640;
		h = 480;
		break;
	}
	SizeRegion( "View3d", w, h );
}

/************************************************
 *	Number of panes in the view change.
 ************************************************/
void ToolRenderAdvance::EventLayoutChange( HISender pSender, HKEvent pEvent )
{
	mView.SetPaneLayout( ListLayout.ItemIndex );
}

/************************************************
 *	index of first camera
 ************************************************/
void ToolRenderAdvance::EventCameraChange1( HISender pSender, HKEvent pEvent )
{
	mView.SetPaneCamera( 0, ListCamera1.ItemIndex );
}

/************************************************
 *	index of second camera
 ************************************************/
void ToolRenderAdvance::EventCameraChange2( HISender pSender, HKEvent pEvent )
{
	mView.SetPaneCamera( 1, ListCamera2.ItemIndex );
}

/************************************************
 *	index of third camera
 ************************************************/
void ToolRenderAdvance::EventCameraChange3( HISender pSender, HKEvent pEvent )
{
	mView.SetPaneCamera( 2, ListCamera3.ItemIndex );
}

/************************************************
 *	index of fourth camera
 ************************************************/
void ToolRenderAdvance::EventCameraChange4( HISender pSender, HKEvent pEvent )
{
	mView.SetPaneCamera( 3, ListCamera4.ItemIndex );
}

/************************************************
 *	Tab panel change.
 ************************************************/
void ToolRenderAdvance::EventTabChange( HISender pSender, HKEvent pEvent )
{
	switch( mTabPanel.ItemIndex )
	{
	case 0:	SetControl( "Layout", mLayout[0] );	break;
	case 1: SetControl( "Layout", mLayout[1] );	break;
	}
}

/************************************************
 *	show about info
 ************************************************/
void ToolRenderAdvance::EventButtonAbout( HISender pSender, HKEvent pEvent )
{
	const char *szVersion = LoadVersionFromResource();

	FBMessageBox( FBString("RenderAdvance ", szVersion) , "Author: Sergey Solohin (Neill3d)\n e-mail to: s@neill3d.com\n web-site: http:\\\\neill3d.com", "Ok" );
}

/************************************************
 *	define grab options and grab all time range
 ************************************************/
void ToolRenderAdvance::EventButtonRender( HISender pSender, HKEvent pEvent )
{
	FBVideoGrabber	&VideoGrabber = mView.VideoGrabber;

	int ndx;
	FBVideoGrabOptions	GrabOptions = VideoGrabber.GetOptions();

	FBTime lStart, lStop, lStep;
	lStart.SetTime(0, 0, 0, (int)EditStart.Value, 0);	//Frame 0
	lStop.SetTime(0, 0, 0, (int)EditStop.Value, 0);	//Frame 150
	lStep.SetTime(0, 0, 0, (int)EditStep.Value, 0);	//1 Frame

	GrabOptions.mTimeSpan.Set(lStart, lStop);
	GrabOptions.mTimeSteps.SetTime(0, 0, 0, (int)EditStep.Value, 0);
	ndx = ListPicture.ItemIndex;
	GrabOptions.mCameraResolution = (FBCameraResolutionMode) ndx;
	ndx = ListPixel.ItemIndex;
	GrabOptions.mBitsPerPixel = (FBVideoRenderDepth) ndx;
	ndx = ListField.ItemIndex;
	GrabOptions.mFieldMode = (FBVideoRenderFieldMode) ndx;
	ndx = ListDisplay.ItemIndex;
	GrabOptions.mViewingMode = (FBVideoRenderViewingMode)ndx;
	GrabOptions.mOutputFileName = EditFile.Text;
	GrabOptions.mAntiAliasing = (ButtonAntialiasing.State == 1);
	GrabOptions.mRenderAudio = (ButtonAudio.State == 1);
	GrabOptions.mAudioRenderFormat = FBAudioFmt_GetDefaultFormat();	//44100Hz 16bits stereo

	VideoGrabber.SetOptions(&GrabOptions);
	VideoGrabber.SetRefreshViewFunc(&mView, mView.RefreshViewCallback);

	//mStats = VideoGrabber.GetStatistics();
	mView.RefreshStats();

	//Start rendering while grabbing each frame
	mView.Render(GrabOptions);
/*
	if( VideoGrabber.BeginGrab() )	//Check if we can grab
	{VideoGrabber.SetOptions(&GrabOptions);
		mStats = VideoGrabber.GetStatistics();
		RefreshStats();

		VideoGrabber.Grab();		//Grab all frame
		VideoGrabber.EndGrab();		//End grabbing session
	}
	*/
}

/************************************************
 *	choose output filename
 ************************************************/
void ToolRenderAdvance::EventButtonFilePopup( HISender pSender, HKEvent pEvent )
{
    //
    // Show the folder popup to select the folder
    //
	mFilePopup.Execute();

    //
    // Set the path in the edit box
    //	
		EditFile.Text = mFilePopup.FullFilename;
    mView.SetFilePath( (FBString)mFilePopup.Path );
    mView.SetFileName( (FBString)mFilePopup.FileName );

}

/************************************************
 *	UI Idle callback.
 ************************************************/
void ToolRenderAdvance::EventToolIdle( HISender pSender, HKEvent pEvent )
{
	RefreshView();
}

void ToolRenderAdvance::RefreshView()
{
	mView.Refresh(true);
}

/************************************************
 *	Handle tool activation (selection/unselection).
 ************************************************/
void ToolRenderAdvance::EventToolShow( HISender pSender, HKEvent pEvent )
{
	FBEventShow lEvent( pEvent );

	if( lEvent.Shown )
	{
	}
}

/************************************************
 *	Paint callback for tool (on expose).
 ************************************************/
void ToolRenderAdvance::EventToolPaint( HISender pSender, HKEvent pEvent )
{
}

/************************************************
 *	Tool resize callback.
 ************************************************/
void ToolRenderAdvance::EventToolResize( HISender pSender, HKEvent pEvent )
{
}

/************************************************
 *	Handle input into the tool.
 ************************************************/
void ToolRenderAdvance::EventToolInput( HISender pSender, HKEvent pEvent )
{
}

/************************************************
 *	FBX Storage.
 ************************************************/
bool ToolRenderAdvance::FbxStore	( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	pFbxObject->FieldWriteBegin( "ToolRenderAdvanceSection" );
	{
	}
	pFbxObject->FieldWriteEnd();
	*/
	return true;
}

/************************************************
 *	FBX Retrieval.
 ************************************************/
bool ToolRenderAdvance::FbxRetrieve( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	pFbxObject->FieldReadBegin( "ToolRenderAdvanceSection" );
	{
	}
	pFbxObject->FieldReadEnd();
	*/
	return true;
}

