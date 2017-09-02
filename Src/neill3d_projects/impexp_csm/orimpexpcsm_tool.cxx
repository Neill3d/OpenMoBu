

/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////

/**	\file	orimpexptool_tool.cxx
*	Function defintions for an import/export tool class.
*	Contains the definitions for the functions of an import/export
*	tool class ORToolImportExport.
*/

//--- Class declaration
#include "orimpexpcsm_tool.h"


//--- Registration defines
#define	ORTOOLIMPORTEXPORT__CLASS	ORTOOLIMPORTEXPORT__CLASSNAME
#define	ORTOOLIMPORTEXPORT__LABEL	"Import/Export CSM"
#define	ORTOOLIMPORTEXPORT__DESC	"CSM - Import/Export Description"


//--- FiLMBOX implementation and registration
FBToolImplementation(	ORTOOLIMPORTEXPORT__CLASS	);
FBRegisterTool		(	ORTOOLIMPORTEXPORT__CLASS,
						ORTOOLIMPORTEXPORT__LABEL,
						ORTOOLIMPORTEXPORT__DESC,
						FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)


/************************************************
 *	FiLMBOX Creation Function.
 ************************************************/
bool ORToolImportExport::FBCreate()
{
	// UI Create & Configure
	UICreate	();
	UIConfigure	();

	return true;
}


/************************************************
 *	FiLMBOX Destruction function.
 ************************************************/
void ORToolImportExport::FBDestroy()
{
}


/************************************************
 *	Create the UI.
 ************************************************/
void ORToolImportExport::UICreate()
{
	int lW = 200;
	int lS = 5;
	int lH = 25;

	// Add regions
    AddRegion ( "LabelImport",			"LabelImport",
											lS,		kFBAttachLeft,		"",						1.0,
											lS,		kFBAttachTop,		"",						1.0,
											lW,		kFBAttachNone,		NULL,					1.0,
											lH,		kFBAttachNone,		NULL,					1.0 );
    AddRegion ("ButtonBrowse",			"ButtonBrowse",
											0,		kFBAttachLeft,		"LabelImport",			1.0,
											2*lS,	kFBAttachBottom,	"LabelImport",			1.0,
											0,		kFBAttachWidth,		"LabelImport",			1.0,
											0,		kFBAttachHeight,	"LabelImport",			1.0 );
    AddRegion ("ButtonImportScene",		"ButtonImportScene",
											0,		kFBAttachLeft,		"ButtonBrowse",			1.0,
											lS,		kFBAttachBottom,	"ButtonBrowse",			1.0,
											0,		kFBAttachWidth,		"ButtonBrowse",			1.0,
											0,		kFBAttachHeight,	"ButtonBrowse",			1.0 );
    AddRegion ("ButtonImportOptical",	"ButtonImportOptical",
											0,		kFBAttachLeft,		"ButtonImportScene",	1.0,
											lS,		kFBAttachBottom,	"ButtonImportScene",	1.0,
											0,		kFBAttachWidth,		"ButtonImportScene",	1.0,
											0,		kFBAttachHeight,	"ButtonImportScene",	1.0 );
    AddRegion ("ButtonImportAnimation",	"ButtonImportAnimation",
											0,		kFBAttachLeft,		"ButtonImportOptical",	1.0,
											lS,		kFBAttachBottom,	"ButtonImportOptical",	1.0,
											0,		kFBAttachWidth,		"ButtonImportOptical",	1.0,
											0,		kFBAttachHeight,	"ButtonImportOptical",	1.0 );
	AddRegion ("LabelExport",			"LabelExport",
											lW,		kFBAttachRight,		"LabelImport",			1.0,
											0,		kFBAttachTop,		"LabelImport",			1.0,
											0,		kFBAttachWidth,		"LabelImport",			1.0,
											0,		kFBAttachHeight,	"LabelImport",			1.0 );
	AddRegion ("ButtonExportSelected",	"ButtonExportSelected",
											0,		kFBAttachLeft,		"LabelExport",			1.0,
											2*lS,	kFBAttachBottom,	"LabelExport",			1.0,
											0,		kFBAttachWidth,		"LabelExport",			1.0,
											0,		kFBAttachHeight,	"LabelExport",			1.0 );
	AddRegion ("ButtonExportVoice",		"ButtonExportVoice",
											0,		kFBAttachLeft,		"ButtonExportSelected",	1.0,
											lS,		kFBAttachBottom,	"ButtonExportSelected",	1.0,
											0,		kFBAttachWidth,		"ButtonExportSelected",	1.0,
											0,		kFBAttachHeight,	"ButtonExportSelected",	1.0 );

	// Assign regions
	SetControl("LabelImport",			mLabelImport			);
	SetControl("LabelExport",			mLabelExport			);
	SetControl("ButtonBrowse",			mButtonBrowse			);
	SetControl("ButtonImportScene",		mButtonImportScene		);
	SetControl("ButtonImportOptical",	mButtonImportOptical	);
	SetControl("ButtonImportAnimation",	mButtonImportAnimation	);
	SetControl("ButtonExportSelected",	mButtonExportSelected	);
	SetControl("ButtonExportVoice",		mButtonExportVoice		);
}


/************************************************
 *	Configure the UI.
 ************************************************/
void ORToolImportExport::UIConfigure()
{
	// Configure UI elements
	mLabelImport.Caption			= "Importing";
	mLabelExport.Caption			= "Exporting";
	mLabelImport.Justify			= kFBTextJustifyCenter;
	mLabelExport.Justify			= kFBTextJustifyCenter;
	mButtonBrowse.Caption			= "Browse File...";
	mButtonImportScene.Caption		= "Import Scene";
	mButtonImportOptical.Caption	= "Import Optical";
	mButtonImportAnimation.Caption	= "Import Animation on Selected";
	mButtonExportSelected.Caption   = "Export Selected Model and Animation...";
	mButtonExportVoice.Caption		= "Export Voice...";

	// Add callbacks
	mButtonBrowse.OnClick.Add			( this,(FBCallback)&ORToolImportExport::EventButtonBrowseClick			);
	mButtonImportScene.OnClick.Add		( this,(FBCallback)&ORToolImportExport::EventButtonImportSceneClick		);
	mButtonImportOptical.OnClick.Add	( this,(FBCallback)&ORToolImportExport::EventButtonImportOpticalClick	);
	mButtonImportAnimation.OnClick.Add	( this,(FBCallback)&ORToolImportExport::EventButtonImportAnimationClick );
	mButtonExportSelected.OnClick.Add	( this,(FBCallback)&ORToolImportExport::EventButtonExportSelectedClick	);
	mButtonExportVoice.OnClick.Add		( this,(FBCallback)&ORToolImportExport::EventButtonExportVoiceClick		);
}


/************************************************
 *	Browse button callback.
 ************************************************/
void ORToolImportExport::EventButtonBrowseClick(HIRegister /*pSender*/, HKEvent /*pEvent*/)
{
	FBFilePopup 	lFilePopup;

	lFilePopup.Style	= kFBFilePopupOpen;
    lFilePopup.Path		= "C:\\";
	lFilePopup.Filter	= "*.txt";

	if( lFilePopup.Execute() )
	{
		mEngine.ImportFile( lFilePopup.FullFilename.AsString() );
	}
}


/************************************************
 *	Import scene button callback.
 ************************************************/
void ORToolImportExport::EventButtonImportSceneClick(HIRegister /*pSender*/, HKEvent /*pEvent*/)
{
//	mEngine.ImportFile();
}


/************************************************
 *	Import animation button callback.
 ************************************************/
void ORToolImportExport::EventButtonImportAnimationClick(HIRegister /*pSender*/, HKEvent /*pEvent*/)
{
    //mEngine.ImportAnimation();
}


/************************************************
 *	Import optical button callback.
 ************************************************/
void ORToolImportExport::EventButtonImportOpticalClick(HIRegister /*pSender*/, HKEvent /*pEvent*/)
{
    //mEngine.ImportOptical();
}


/************************************************
 *	Export selected button callback.
 ************************************************/
void ORToolImportExport::EventButtonExportSelectedClick(HIRegister /*pSender*/, HKEvent /*pEvent*/)
{
	FBFilePopup 	FilePopup;

	// Configure popup
	FilePopup.Style		= kFBFilePopupSave;
	FilePopup.Path		= "C:\\";
	FilePopup.Filter	= "*.txt";

	// Show popup. Get filename & export if ok was clicked.
	if( FilePopup.Execute() )
	{
		//mEngine.ExportSelected( FilePopup.FullFilename.AsString() );
	}
}


/************************************************
 *	Export voice reality callback.
 ************************************************/
void ORToolImportExport::EventButtonExportVoiceClick(HIRegister /*pSender*/, HKEvent /*pEvent*/)
{
	FBFilePopup FilePopup;

	FilePopup.Style		= kFBFilePopupSave;
	FilePopup.Path		= "C:\\";
	FilePopup.Filter	= "voice_export.txt";

	if( FilePopup.Execute() )
	{
		//mEngine.ExportVoice( FilePopup.FullFilename.AsString() );
	}
}

