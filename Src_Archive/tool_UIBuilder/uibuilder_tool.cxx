
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


//--- Class declaration
#include "uibuilder_tool.h"

#include <windows.h>

//--- Registration defines
#define ORTOOLTEMPLATE__CLASS	ORTOOLTEMPLATE__CLASSNAME
#define ORTOOLTEMPLATE__LABEL	"UI Builder v 0.6.7b"
#define ORTOOLTEMPLATE__DESC	"UIBuilder - builder ui for your tool"

#define UIBUILDER_ABOUT "UI Builder v. 0.6.7 (beta)\n Homepage: www.neill3d.com\n Author: Sergey Solohin (Neill3d)\n e-mail to: s@neill3d.com"

//--- FiLMBOX implementation and registration
FBToolImplementation(	ORTOOLTEMPLATE__CLASS	);
FBRegisterTool		(	ORTOOLTEMPLATE__CLASS,
						ORTOOLTEMPLATE__LABEL,
						ORTOOLTEMPLATE__DESC,
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)



/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
HWND g_hwnd = 0;
UIBuilderTool	*pTool = NULL;
bool first = true;

// hints
#define		HINT_SELECT					"Select regions (Shift - add to sel, Alt - remove from sel)"
#define		HINT_MOVEALL				"Move region (Shift - move all selected regions)"
#define		HINT_MOVEX					"Move horizontal (Shift - move all selected regions)"
#define		HINT_MOVEY					"Move vertical (Shift - move all selected regions)"
#define		HINT_SIZEALL				"Size region (Shift - move all selected regions)"
#define		HINT_SIZEX					"Size horizontal (Shift - move all selected regions)"
#define		HINT_SIZEY					"Size vertical (Shift - move all selected regions)"
#define		HINT_REGION_START		"Click to add new region"
#define		HINT_REGION_END			"Click to finish region creation"
#define		HINT_PREVIEW				"You are in visual preview mode (RMB to exit mode)"

//	file menu
#define		menuNewItem				1
#define		menuLoadItem			2
#define		menuSaveItem			3
#define		menuExportORItem		4
#define		menuExportPYTHONItem	5
#define		menuExportXMLItem		6
// edit menu
#define		menuUndoItem			11
#define		menuRedoItem			12
#define		menuDeleteItem		13

#define		menuSelectAll			15
#define		menuSelectNone		16
#define		menuSelectInvert	17

#define		menuSnap					18
#define		menuSnapStep			19

// help menu
#define		menuAboutItem	101

// assign menu
#define		menuLabelItem		201
#define		menuButtonItem	202
#define		menuEditItem		203
#define		menuListItem		204
#define		menuViewItem		205
#define		menuContainerItem	206
#define		menuSpreadItem	207
#define		menuThermometerItem	208
#define		menuSliderItem	209
#define		menuTreeItem		210
#define		menuEditNumberItem	211
#define		menuEditColorItem		212
#define		menuEditVectorItem	213
#define		menuTimeCodeItem		214
#define		menuLayoutRegionItem 215
#define		menuClearItem		216

// context menu
#define		ctxLabelItem		301
#define		ctxButtonItem		302
#define		ctxEditItem			303
#define		ctxListItem			304
#define		ctxViewItem			305
#define		ctxContainerItem	306
#define		ctxSpreadItem		307
#define		ctxThermometerItem	308
#define		ctxSliderItem		309
#define		ctxTreeItem			310
#define		ctxEditNumberItem	311
#define		ctxEditColorItem	312
#define		ctxEditVectorItem	313
#define		ctxTimeCodeItem	314
#define		ctxLayoutRegionItem 315
#define		ctxClearItem		316
#define		ctxPreviewMode	317
#define		ctxDeleteItem		318

// original MB window procedure
WNDPROC wpOrigEditProc;

// Subclass procedure
LRESULT APIENTRY EditSubclassProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    if (uMsg == WM_GETDLGCODE)
        return DLGC_WANTALLKEYS;

	//UIBuilderTool	*pTool = (UIBuilderTool*) ::GetWindowLong(hwnd, GWL_USERDATA);
	if (!pTool)
		return CallWindowProc(wpOrigEditProc, hwnd, uMsg, wParam, lParam);

	if (uMsg == WM_KEYDOWN)
	{
		printf( "item pressed\n" );
	}

	if (uMsg == WM_COMMAND)
	{
		switch ( LOWORD(wParam) )
		{
		// New ui tool
		case menuNewItem:
			{
				int result = FBMessageBox("Clear current ui", "Are you sure?", "Yes", "Cancel" );
				if (result==1)
				{
					pTool->ClosePreviewMode();
					pTool->mRects.New();
					pTool->RebuildAttachList();
					pTool->RebuildNavigatorTree();
					pTool->RebuildProperties();
					pTool->mRects.ClearPropTable(pTool->mPropTable);

					pTool->mToolTable.SetCell(TOOL_CELL_TITLE,0, (char*)pTool->mRects.mTool.name);
					pTool->mToolTable.SetCell(TOOL_CELL_SIZEX,0, pTool->mRects.mTool.sizex);
					pTool->mToolTable.SetCell(TOOL_CELL_SIZEY,0, pTool->mRects.mTool.sizey);
					pTool->SizeRegion( "ViewLayout", pTool->mRects.mTool.sizex, pTool->mRects.mTool.sizey );
				}
			} break;

		case menuLoadItem:
			{
				FBFilePopup	lPopup;

				lPopup.Filter	= "*.ui";
				lPopup.Caption	= "Select file";
				lPopup.Style	= kFBFilePopupOpen;

				FBConfigFile config( "uibuilder.txt" );
				FBString textVal = config.Get( "PATHS", "MAIN" );
				if (textVal != "" ) {
					lPopup.Path = textVal;
				}

				if (lPopup.Execute() )
				{
					config.Set( "PATHS", "MAIN", lPopup.Path );

					// load from file
					pTool->mRects.New();
					pTool->RebuildAttachList();
					pTool->RebuildNavigatorTree();
					pTool->mRects.ClearPropTable(pTool->mPropTable);

					FBString fullFilename = lPopup.FullFilename;
					pTool->mRects.Load(fullFilename, pTool->mFormView, pTool->mPropTable);
					pTool->RebuildAttachList();
					pTool->RebuildNavigatorTree();
				}
			} break;

		// save ui scene
		case menuSaveItem:
			{
				FBFilePopup	lPopup;

				lPopup.Filter	= "*.ui";
				lPopup.Caption	= "Select file";
				lPopup.Style	= kFBFilePopupSave;

				FBConfigFile config( "uibuilder.txt" );
				FBString textVal = config.Get( "PATHS", "MAIN" );
				if (textVal != "" ) {
					lPopup.Path = textVal;
				}

				if (lPopup.Execute() )
				{
					config.Set( "PATHS", "MAIN", lPopup.Path );

					// save to file
					FBString str = lPopup.FullFilename;
					pTool->mRects.Save(str);
				}
			} break;

		// export scene to cpp Open Reality source code
		case menuExportORItem:
			{
				FBFilePopup	lPopup;

				lPopup.Filter	= "*.txt";
				lPopup.Caption	= "Select file";
				lPopup.Style	= kFBFilePopupSave;

				FBConfigFile config( "uibuilder.txt" );
				FBString textVal = config.Get( "PATHS", "EXPORT" );
				if (textVal != "" ) {
					lPopup.Path = textVal;
				}

				if (lPopup.Execute() )
				{
					config.Set( "PATHS", "EXPORT", lPopup.Path );

					// export to file
					FBString str = lPopup.FullFilename;
					pTool->mRects.ExportCPP(str);
				}
			} break;

		// export scene to python source code
		case menuExportPYTHONItem:
			{
				FBFilePopup	lPopup;

				lPopup.Filter	= "*.py";
				lPopup.Caption	= "Select file";
				lPopup.Style	= kFBFilePopupSave;

				FBConfigFile config( "uibuilder.txt" );
				FBString textVal = config.Get( "PATHS", "EXPORT" );
				if (textVal != "" ) {
					lPopup.Path = textVal;
				}

				if (lPopup.Execute() )
				{
					config.Set( "PATHS", "EXPORT", lPopup.Path );

					// export to file
					FBString str = lPopup.FullFilename;
					pTool->mRects.ExportPython(str);
				}
			} break;

		// export scene to python source code
		case menuExportXMLItem:
			{
				FBFilePopup	lPopup;

				lPopup.Filter	= "*.xml";
				lPopup.Caption	= "Select file";
				lPopup.Style	= kFBFilePopupSave;

				FBConfigFile config( "uibuilder.txt" );
				FBString textVal = config.Get( "PATHS", "EXPORT" );
				if (textVal != "" ) {
					lPopup.Path = textVal;
				}

				if (lPopup.Execute() )
				{
					config.Set( "PATHS", "EXPORT", lPopup.Path );

					// export to file
					FBString str = lPopup.FullFilename;
					pTool->mRects.ExportXml(str);
				}
			} break;

		// undo last action
		case menuUndoItem:
			{
				pTool->mRects.Fetch();
				pTool->RebuildNavigatorTree();
				pTool->RebuildAttachList();
				pTool->RebuildProperties();
			} break;

		// redo last
		case menuRedoItem:
			{
			} break;

		// operations with selection
		case menuSelectAll:
			{
				pTool->mRects.SelectAll();
			}break;
		case menuSelectInvert:
			{
				pTool->mRects.SelectInvert();
			}break;
		case menuSelectNone:
			{
				pTool->mRects.SelectNone();
			}break;

		// delete selected regions
		case ctxDeleteItem:
		case menuDeleteItem:
			{
				pTool->mRects.DeleteSel();
				pTool->RebuildNavigatorTree();
				pTool->RebuildAttachList();
				pTool->RebuildProperties();
				//pTool->mRects.FillPropTable(pTool->mPropTable);
			} break;

		// assign new visual components for regions
		#define COMPCLASS(name, objName) case ctx##objName##Item: \
			case menu##objName##Item: \
			pTool->mRects.AssignComponent( name ); \
			pTool->RebuildNavigatorTree(); \
			pTool->RebuildProperties(); break;
		#include "components_list.h"
		#undef COMPCLASS
			
		case ctxClearItem:
		case menuClearItem:
				pTool->mRects.AssignComponent(0);
				pTool->RebuildNavigatorTree();
				pTool->RebuildProperties();
			break;

		case ctxPreviewMode:
			pTool->EnterPreviewMode();
			break;

		// snapping mode
		case menuSnap:
			pTool->ToggleSnapMode();
			break;
		// snapping step
		case menuSnapStep:
			pTool->SetSnapStep();
			break;

		// about
		case menuAboutItem:
			FBMessageBox("About", UIBUILDER_ABOUT, "Ok" );
			break;
		
		}
	}

    return CallWindowProc(wpOrigEditProc, hwnd, uMsg,
        wParam, lParam);
}

BOOL CALLBACK EnumProc( HWND hwnd, LPARAM lParam )
{
	char		szTitle[MAX_PATH];
	GetWindowText( hwnd, szTitle, MAX_PATH );

	if (strstr( szTitle, "UI Builder" ) != NULL)
	{
		g_hwnd = hwnd;

		if (first){
			// Subclass the edit control.
			wpOrigEditProc = (WNDPROC) SetWindowLongPtr( hwnd, GWLP_WNDPROC, (LONG_PTR) EditSubclassProc);

			//first = false;
		}
		return false;
	}

	return true;
}

bool UIBuilderTool::ToggleSnapMode()
{
	if (mSnapStep)
	{
		mOldSnapStep = mSnapStep;
		mSnapStep = 0;
		mRects.mSnapStep = mSnapStep;
		return false;
	}
	else
	{
		mSnapStep = mOldSnapStep;
		mRects.mSnapStep = mSnapStep;
		return true;
	}
	return true;
}

void UIBuilderTool::SetSnapStep()
{
	int value = (mSnapStep > 0) ? mSnapStep : mOldSnapStep;
	if (FBMessageBoxGetUserValue( "Snap", "Enter snapping step:", &value, kFBPopupInt, "Ok", "Cancel" ) == 1)
		(mSnapStep > 0) ? (mSnapStep = value) : (mOldSnapStep = value);
	mRects.mSnapStep = mSnapStep;
}

bool UIBuilderTool::FBCreate()
{

	// snapping
	mSnapStep = 5;
	mRects.mSnapStep = mSnapStep;

	// Tool options
	StartSize[0] = 800;
	StartSize[1] = 600;

    int lB = 10;
		int lT = 4;
	//int lS = 4;
	int lW = 300;
	int lH = 18;

	// Configure layout
	AddRegion( "ViewLayout", "ViewLayout",
										lB,	kFBAttachLeft,	"",	1.0	,
										26,	kFBAttachTop,	"",	1.0,
										mRects.mTool.sizex,kFBAttachNone,	"",	1.0,
										mRects.mTool.sizey,kFBAttachNone,"",	1.0 );

	AddRegion( "TreeNavigator", "TreeNavigator",
										-lW,	kFBAttachRight,	"",	1.0	,
										lB,	kFBAttachTop,	"",	1.0,
										-lB,	kFBAttachRight,	"",	1.0,
										120,	kFBAttachNone,	"",	1.0 );
	AddRegion( "TabProperties", "TabProperties",
										-lW,	kFBAttachRight,	"",	1.0	,
										lB,	kFBAttachBottom,	"TreeNavigator",	1.0,
										-lB,	kFBAttachRight,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	AddRegion( "Properties", "Properties",
										-lW,	kFBAttachRight,	"",	1.0	,
										lB,	kFBAttachBottom,	"TabProperties",	1.0,
										-lB,	kFBAttachRight,	"",	1.0,
										-lB,	kFBAttachBottom,	"",	1.0 );

	AddRegion( "ButtonFile", "ButtonFile",
										lB,	kFBAttachLeft,	"",	1.0	,
										lT,	kFBAttachTop,	"",	1.0,
										35,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	AddRegion( "ButtonEdit", "ButtonEdit",
										lB,	kFBAttachRight,	"ButtonFile",	1.0	,
										lT,	kFBAttachTop,	"",	1.0,
										35,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	AddRegion( "ButtonHelp", "ButtonHelp",
										lB,	kFBAttachRight,	"ButtonEdit",	1.0	,
										lT,	kFBAttachTop,	"",	1.0,
										35,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	AddRegion( "ButtonRun", "ButtonRun",
										lB+10,	kFBAttachRight,	"ButtonHelp",	1.0	,
										lT,	kFBAttachTop,	"",	1.0,
										35,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	AddRegion( "Actions", "Actions",
										20,	kFBAttachRight,	"ButtonRun",	1.0	,
										lT,	kFBAttachTop,	"",	1.0,
										160,kFBAttachNone,	"",	1.0,
										lH+4,	kFBAttachNone,	"",	1.0 );
	AddRegion( "Assign", "Assign",
										20,	kFBAttachRight,	"Actions",	1.0	,
										lT,	kFBAttachTop,	"",	1.0,
										50,kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	AddRegion( "LabelHint", "LabelHint",
										lB,	kFBAttachLeft,	"",	1.0	,
										-20,	kFBAttachBottom,	"",	1.0,
										400,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );



	SetControl( "ViewLayout", mFormView );
	SetControl( "TreeNavigator", mNavigator );
	SetControl( "TabProperties", mPropertiesTab );
	SetControl( "Properties", mProperties );

	SetControl( "ButtonFile", mButtonFile );
	SetControl( "ButtonEdit", mButtonEdit );
	SetControl( "ButtonHelp", mButtonHelp );
	SetControl( "ButtonRun", mButtonRun );
	SetControl( "Actions", mActions );
	SetControl( "Assign", mButtonAssign );
	SetControl( "LabelHint", mHint );

	SetBorder ("ViewLayout", kFBStandardBorder, false,true, 1, 0,90,0);
	SetBorder ("Properties", kFBStandardBorder, false,true, 1, 0,90,0);

	// Configure button

	mHint.Caption = "Welcome to UI Builder!";

	mButtonFile.Caption = "File >";
	mButtonFile.OnClick.Add( this, (FBCallback) &UIBuilderTool::EventButtonFileClick );
	mButtonEdit.Caption = "Edit >";
	mButtonEdit.OnClick.Add( this, (FBCallback) &UIBuilderTool::EventButtonEditClick );
	mButtonHelp.Caption = "Help >";
	mButtonHelp.OnClick.Add( this, (FBCallback) &UIBuilderTool::EventButtonHelpClick );
	mButtonRun.Caption = "Run";
	mButtonRun.OnClick.Add( this, (FBCallback) &UIBuilderTool::EventButtonRunClick );
	mButtonAssign.Caption = "Assign >";
	mButtonAssign.OnClick.Add( this, (FBCallback) &UIBuilderTool::EventButtonAssignClick );

	mActions.Items.Add( "Select" );
	mActions.Items.Add( "Region" );
	mActions.Items.Add( "Preview" );
	mActions.OnChange.Add( this, (FBCallback) &UIBuilderTool::EventActionsChange );

	mLayoutRoot = mNavigator.GetRoot();
	mNavigator.Clear();
    mNavigator.AllowExpansion = true;
    mNavigator.AllowCollapse = true;
    mNavigator.AutoScroll = true;
    mNavigator.MultiSelect = false;
	//mNavigator.CheckBoxes = true;

	mPropertiesTab.Items.SetString("Tool~Region Props~Component Props~Events");
	mPropertiesTab.OnChange.Add( this, (FBCallback) &UIBuilderTool::EventPropertiesTabChange ); 

	mFormView.SetAutoRestructure(true);
	

	// properties table
	mProperties.AddRegion( "Spread", "Spread",
										1,		kFBAttachLeft,		"",	1.0	,
										1,		kFBAttachTop,		"",	1.0,
										-1,	kFBAttachRight,		"",	1.0,
										-1,	kFBAttachBottom,	"",	1.0 );
	mProperties.AddRegion( "EventsList", "EventsList",
										1,	kFBAttachLeft,	"",	1.0	,
										1,	kFBAttachTop,	"",	1.0,
										-1,	kFBAttachRight,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	mProperties.AddRegion( "EventCode", "EventCode",
										1,	kFBAttachLeft,	"",	1.0	,
										lB,	kFBAttachBottom,	"EventsList",	1.0,
										-1,	kFBAttachRight,	"",	1.0,
										-1,	kFBAttachBottom,	"",	1.0 );
	mProperties.SetControl( "Spread", mToolTable );

	// tool properties table
	mToolTable.Caption = "Tool properties";
	mToolTable.MultiSelect = false;
	mToolTable.GetColumn(-1).Width = 90;
	mToolTable.ColumnAdd( "Value", 0 );
	mToolTable.ColumnAdd( "Description", 1 );
	mToolTable.RowAdd( "Title",			TOOL_CELL_TITLE );
	mToolTable.RowAdd( "Size X",		TOOL_CELL_SIZEX );
	mToolTable.RowAdd( "Size Y",		TOOL_CELL_SIZEY );

	mToolTable.GetCell(TOOL_CELL_TITLE, 0).Style = kFBCellStyleString;
	mToolTable.SetCell(TOOL_CELL_TITLE, 0, mRects.mTool.name );
	mToolTable.GetCell(TOOL_CELL_SIZEX, 0).Style = kFBCellStyleInteger;
	mToolTable.SetCell(TOOL_CELL_SIZEX, 0, mRects.mTool.sizex );
	mToolTable.GetCell(TOOL_CELL_SIZEY, 0).Style = kFBCellStyleInteger;
	mToolTable.SetCell(TOOL_CELL_SIZEY, 0, mRects.mTool.sizey );
	mToolTable.GetColumn(0).Width = 80;
	mToolTable.OnCellChange.Add( this, (FBCallback) &UIBuilderTool::EventToolCellChange );

	// config prop table
	mPropTable.Caption		= "Region Properties";
	mPropTable.MultiSelect	= false;
	mPropTable.GetColumn(-1).Width = 100;
	mPropTable.ColumnAdd( "Value", 0 );
	mPropTable.ColumnAdd( "Description", 1 );
	mPropTable.RowAdd( "Title",			PROP_CELL_TITLE );
	mPropTable.RowAdd( "x",				PROP_CELL_X );
	mPropTable.RowAdd( "x attach type", PROP_CELL_X_ATTACH_TYPE );
	mPropTable.RowAdd( "x attach node", PROP_CELL_X_ATTACH_NODE );
	mPropTable.RowAdd( "x multiply",	PROP_CELL_X_MULT );
	mPropTable.RowAdd( "y",				PROP_CELL_Y );
	mPropTable.RowAdd( "y attach type", PROP_CELL_Y_ATTACH_TYPE );
	mPropTable.RowAdd( "y attach node", PROP_CELL_Y_ATTACH_NODE );
	mPropTable.RowAdd( "y multiply",	PROP_CELL_Y_MULT );
	mPropTable.RowAdd( "width",			PROP_CELL_W );
	mPropTable.RowAdd( "w attach type", PROP_CELL_W_ATTACH_TYPE );
	mPropTable.RowAdd( "w attach node", PROP_CELL_W_ATTACH_NODE );
	mPropTable.RowAdd( "w multiply",	PROP_CELL_W_MULT );
	mPropTable.RowAdd( "height",		PROP_CELL_H );
	mPropTable.RowAdd( "h attach type", PROP_CELL_H_ATTACH_TYPE );
	mPropTable.RowAdd( "h attach node", PROP_CELL_H_ATTACH_NODE );
	mPropTable.RowAdd( "h multiply",	PROP_CELL_H_MULT );
	mPropTable.GetColumn(0).Width = 80;


	FBString strAttachType( "AttachLeft~AttachRight~AttachTop~AttachBottom~AttachWidth~AttachHeight~AttachCenter~AttachNone");

	// x menus
	mPropTable.GetCell(PROP_CELL_X, 0).Style = kFBCellStyleInteger;
	mPropTable.GetCell(PROP_CELL_X_ATTACH_TYPE, 0).Style = kFBCellStyleMenu;
	mPropTable.SetCell(PROP_CELL_X_ATTACH_TYPE, 0, strAttachType );
	mPropTable.GetCell(PROP_CELL_X_ATTACH_NODE, 0).Style = kFBCellStyleMenu;
	mPropTable.SetCell(PROP_CELL_X_ATTACH_NODE, 0, "None~Tool" );
	mPropTable.GetCell(PROP_CELL_X_MULT, 0).Style = kFBCellStyleDouble;

	// y menus
	mPropTable.GetCell(PROP_CELL_Y, 0).Style = kFBCellStyleInteger;
	mPropTable.GetCell(PROP_CELL_Y_ATTACH_TYPE, 0).Style = kFBCellStyleMenu;
	mPropTable.SetCell(PROP_CELL_Y_ATTACH_TYPE, 0, strAttachType );
	mPropTable.GetCell(PROP_CELL_Y_ATTACH_NODE, 0).Style = kFBCellStyleMenu;
	mPropTable.SetCell(PROP_CELL_Y_ATTACH_NODE, 0, "None~Tool" );
	mPropTable.GetCell(PROP_CELL_Y_MULT, 0).Style = kFBCellStyleDouble;

	// width menus
	mPropTable.GetCell(PROP_CELL_W, 0).Style = kFBCellStyleInteger;
	mPropTable.GetCell(PROP_CELL_W_ATTACH_TYPE, 0).Style = kFBCellStyleMenu;
	mPropTable.SetCell(PROP_CELL_W_ATTACH_TYPE, 0, strAttachType );
	mPropTable.GetCell(PROP_CELL_W_ATTACH_NODE, 0).Style = kFBCellStyleMenu;
	mPropTable.SetCell(PROP_CELL_W_ATTACH_NODE, 0, "None~Tool" );
	mPropTable.GetCell(PROP_CELL_W_MULT, 0).Style = kFBCellStyleDouble;

	// height menus
	mPropTable.GetCell(PROP_CELL_H, 0).Style = kFBCellStyleInteger;
	mPropTable.GetCell(PROP_CELL_H_ATTACH_TYPE, 0).Style = kFBCellStyleMenu;
	mPropTable.SetCell(PROP_CELL_H_ATTACH_TYPE, 0, strAttachType );
	mPropTable.GetCell(PROP_CELL_H_ATTACH_NODE, 0).Style = kFBCellStyleMenu;
	mPropTable.SetCell(PROP_CELL_H_ATTACH_NODE, 0, "None~Tool" );
	mPropTable.GetCell(PROP_CELL_H_MULT, 0).Style = kFBCellStyleDouble;

	mPropTable.OnCellChange.Add( this, (FBCallback) &UIBuilderTool::EventCellChange );

	// visual component properties
	mCompTable.Caption = "Visual component properties";
	mCompTable.MultiSelect = false;
	mCompTable.GetColumn(-1).Width = 100;
	mCompTable.ColumnAdd( "Value", 0 );
	mCompTable.ColumnAdd( "Description", 1 );
	mCompTable.GetColumn(0).Width = 80;
	mCompTable.OnCellChange.Add( this, (FBCallback) &UIBuilderTool::EventCompCellChange );

	// Add tool callbacks
	OnShow.Add	( this, (FBCallback) &UIBuilderTool::EventToolShow		);
	OnIdle.Add	( this, (FBCallback) &UIBuilderTool::EventToolIdle		);
	OnResize.Add( this, (FBCallback) &UIBuilderTool::EventToolResize	);
	OnPaint.Add	( this, (FBCallback) &UIBuilderTool::EventToolPaint		);
	OnInput.Add	( this, (FBCallback) &UIBuilderTool::EventToolInput		);
	//OnInput.Add ( this, (FBCallback) &UIBuilderTool::EventToolInput );
	

	mNavigator.OnSelect.Add( this, (FBCallback) &UIBuilderTool::EventNavigatorSel );
	mRects.pNavigator = &mNavigator;

	mAction = saSelect;
	mActions.ItemIndex = 0;
	mFormView.OnInput.Add( this, (FBCallback) &UIBuilderTool::EventSelectInput );

	//////////////////////
	// code editor

	mEventsList.Style = kFBDropDownList;
	mEventsList.OnChange.Add( this, (FBCallback) &UIBuilderTool::EventListChange );
	mEventCode.OnChange.Add( this, (FBCallback) &UIBuilderTool::EventMemoChange );

	return true;
}

void UIBuilderTool::RebuildAttachList()
{
	FBString	attachList("None");

	uirect	*iter = mRects.first();
	while (iter != NULL)
	{
		attachList = attachList + "~";
		attachList = attachList + iter->title;
		iter->id = mRects.ndx2;
		//if (iter->id == 0) iter->id = 1;
		iter = mRects.next();
	}
	mPropTable.SetCell(PROP_CELL_X_ATTACH_NODE, 0, attachList);
	mPropTable.SetCell(PROP_CELL_Y_ATTACH_NODE, 0, attachList);
	mPropTable.SetCell(PROP_CELL_W_ATTACH_NODE, 0, attachList);
	mPropTable.SetCell(PROP_CELL_H_ATTACH_NODE, 0, attachList);
}

/************************************************
 *	FiLMBOX Destruction function.
 ************************************************/
void UIBuilderTool::FBDestroy()
{
	// Remove tool callbacks
	OnShow.Remove	( this, (FBCallback) &UIBuilderTool::EventToolShow	);
	OnIdle.Remove	( this, (FBCallback) &UIBuilderTool::EventToolIdle	);
	OnPaint.Remove	( this, (FBCallback) &UIBuilderTool::EventToolPaint );
	OnInput.Remove	( this, (FBCallback) &UIBuilderTool::EventToolInput );
	OnResize.Remove	( this, (FBCallback) &UIBuilderTool::EventToolResize);
	//OnInput.Remove( &mFormView, (FBCallback) &UIBuilderTool::EventToolInput );
	//mFormView.OnInput.Remove( this, (FBCallback) &UIBuilderTool::EventViewInput );
	mNavigator.OnSelect.Remove( this, (FBCallback) &UIBuilderTool::EventNavigatorSel );

	// Free user allocated memory
	mPropTable.OnCellChange.Remove( this, (FBCallback) &UIBuilderTool::EventCellChange );
}


/************************************************
 *	Add Button click callback.
 ************************************************/
void UIBuilderTool::EventButtonAddClick( HISender pSender, HKEvent pEvent )
{
	char	name[256];
	char	title[256];
	mRects.GenerateUniqueName(name, title);

	uirect	&rect = mRects.AddElement();

	rect.Init(mFormView, name, title);
	rect.FillPropTable( mPropTable );

	rect.treeNode = mNavigator.InsertLast( mLayoutRoot, title );
	uirect	*pRect = &rect;
	rect.treeNode->Reference = (kReference) pRect;
	RebuildAttachList();
}

void UIBuilderTool::RebuildProperties()
{
	switch (mPropertiesTab.ItemIndex)
	{
	case 1: mRects.FillPropTable(mPropTable);
		break;
	case 2: mRects.FillCompTable(mCompTable);
		break;
	case 3: {
		mRects.FillEvents(&mEventsList);

		int itemIndex = mEventsList.ItemIndex;
		if ( (itemIndex < mEventsList.Items.GetCount()) && (itemIndex >= 0))
		{
			FBString eventName = mEventsList.Items[itemIndex];
			FBStringList	lines;
			mRects.GetEventCode( eventName, lines );
			mEventCode.SetStrings( &lines );
		}
		else
		{
			FBStringList	lines;
			mEventCode.SetStrings( &lines );
		}

		}break;
	}
}

/************************************************
 *	Delete Button click callback.
 ************************************************/
void UIBuilderTool::EventButtonDelClick( HISender pSender, HKEvent pEvent )
{
	mRects.DeleteSel();
	RebuildNavigatorTree();
	RebuildAttachList();
	RebuildProperties();
}

void UIBuilderTool::PrepareWndProc()
{
	EnumWindows( EnumProc, 0 );

	if (first && (g_hwnd > 0)) {
		//::SetWindowLong( g_hwnd, GWL_USERDATA, (LONG)this );
		pTool = this;
		first = false;
	}
}

/************************************************
 *	File Button click callback.
 ************************************************/
void UIBuilderTool::EventButtonFileClick( HISender pSender, HKEvent pEvent )
{
	PrepareWndProc();

	POINT	pos;
	GetCursorPos(&pos);

	HMENU	hSubMenu;
	hSubMenu = CreatePopupMenu();
	AppendMenu(hSubMenu, MF_STRING, menuNewItem, "New" );
	AppendMenu(hSubMenu, MF_STRING, menuLoadItem, "Load" );
	AppendMenu(hSubMenu, MF_STRING, menuSaveItem, "Save" );
	AppendMenu(hSubMenu, MF_SEPARATOR, 0, " " );
	AppendMenu(hSubMenu, MF_STRING, menuExportORItem, "Export Cpp (Open Reality)" );
	AppendMenu(hSubMenu, MF_STRING, menuExportPYTHONItem, "Export Python" );
	AppendMenu(hSubMenu, MF_STRING, menuExportXMLItem, "Export Xml" );

	BOOL result = TrackPopupMenuEx(   hSubMenu,
                 0,
				 pos.x,
				 pos.y,
                 g_hwnd,
                 NULL);

	if (!result)
	{
//		DWORD err = GetLastError();
		FBMessageBox("Error", "by menu", "OK" );
	}

	DestroyMenu(hSubMenu);
}

/************************************************
 *	Edit Button click callback.
 ************************************************/
void UIBuilderTool::EventButtonEditClick( HISender pSender, HKEvent pEvent )
{
	PrepareWndProc();

	POINT	pos;
	GetCursorPos(&pos);

	HMENU	hSubMenu;
	hSubMenu = CreatePopupMenu();
	AppendMenu(hSubMenu, MF_STRING, menuUndoItem, "Undo" );
	AppendMenu(hSubMenu, MF_STRING, menuRedoItem, "Redo" );
	AppendMenu(hSubMenu, MF_SEPARATOR, 0, " " );
	AppendMenu(hSubMenu, MF_STRING, menuSelectAll, "Select All" );
	AppendMenu(hSubMenu, MF_STRING, menuSelectInvert, "Select Invert" );
	AppendMenu(hSubMenu, MF_STRING, menuSelectNone, "Select None" );
	AppendMenu(hSubMenu, MF_SEPARATOR, 0, " " );
	AppendMenu(hSubMenu, MF_STRING, menuSnap, "Snap" );
	AppendMenu(hSubMenu, MF_STRING, menuSnapStep, "Snap step..." );
	AppendMenu(hSubMenu, MF_SEPARATOR, 0, " " );
	AppendMenu(hSubMenu, MF_STRING, menuDeleteItem, "Delete" );

	if (mSnapStep)
		CheckMenuItem(hSubMenu, menuSnap, MF_CHECKED );

	BOOL result = TrackPopupMenuEx(   hSubMenu,
                 0,
				 pos.x,
				 pos.y,
                 g_hwnd,
                 NULL);

	if (!result)
	{
//		DWORD err = GetLastError();
		FBMessageBox("Error", "by menu", "OK" );
	}

	DestroyMenu(hSubMenu);
}

/************************************************
 *	Visual component assignment context menu
 ************************************************/
void UIBuilderTool::EventButtonAssignClick( HISender pSender, HKEvent pEvent )
{
	PrepareWndProc();

	POINT	pos;
	GetCursorPos(&pos);

	HMENU	hSubMenu;
	hSubMenu = CreatePopupMenu();

	#define COMPCLASS(name, objName) AppendMenu(hSubMenu, MF_STRING, menu##objName##Item, ""#objName );
	#include "components_list.h"
	#undef COMPCLASS

	AppendMenu(hSubMenu, MF_SEPARATOR, 0, " " );
	AppendMenu(hSubMenu, MF_STRING, menuClearItem, "Clear assignment" );

	BOOL result = TrackPopupMenuEx(   hSubMenu,
                 0,
				 pos.x,
				 pos.y,
                 g_hwnd,
                 NULL);

	if (!result)
	{
//		DWORD err = GetLastError();
		FBMessageBox("Error", "by menu", "OK" );
	}

	DestroyMenu(hSubMenu);
}

/************************************************
 *	Help Button click callback.
 ************************************************/
void UIBuilderTool::EventButtonHelpClick( HISender pSender, HKEvent pEvent )
{
	PrepareWndProc();

	POINT	pos;
	GetCursorPos(&pos);

	HMENU	hSubMenu;
	hSubMenu = CreatePopupMenu();
	AppendMenu(hSubMenu, MF_STRING, menuAboutItem, "About" );

	BOOL result = TrackPopupMenuEx(   hSubMenu,
                 0,
				 pos.x,
				 pos.y,
                 g_hwnd,
                 NULL);

	if (!result)
	{
//		DWORD err = GetLastError();
		FBMessageBox("Error", "by menu", "OK" );
	}

	DestroyMenu(hSubMenu);
}

/************************************************
 *	Show context menu
 ************************************************/
void UIBuilderTool::ShowContextMenu()
{
	PrepareWndProc();

	POINT	pos;
	GetCursorPos(&pos);

	HMENU	hSubMenu;
	hSubMenu = CreatePopupMenu();
	AppendMenu(hSubMenu, MF_STRING, ctxPreviewMode, "Enter preview mode" );
	AppendMenu(hSubMenu, MF_SEPARATOR, 0, " " );
	AppendMenu(hSubMenu, MF_STRING, ctxDeleteItem, "Delete" );
	AppendMenu(hSubMenu, MF_SEPARATOR, 0, " " );


	#define COMPCLASS(name, objName) AppendMenu(hSubMenu, MF_STRING, ctx##objName##Item, "Assign "#objName );
	#include "components_list.h"
	#undef COMPCLASS

	AppendMenu(hSubMenu, MF_STRING, ctxClearItem, "Clear Assignment" );
	

	BOOL result = TrackPopupMenuEx(   hSubMenu,
                 0,
				 pos.x,
				 pos.y,
                 g_hwnd,
                 NULL);

	if (!result)
	{
//		DWORD err = GetLastError();
		FBMessageBox("Error", "by menu", "OK" );
	}

	DestroyMenu(hSubMenu);
}

/************************************************
 *	Run Button click callback.
 ************************************************/
void UIBuilderTool::EventButtonRunClick( HISender pSender, HKEvent pEvent )
{
	FBString configPath = FBSystem().UserConfigPath;
	FBString fileName (configPath, "\\testRun.py");

	//FBMessageBox( "UIBuilder", fileName, "Ok" );

	// export script to temp file and execute it
	mRects.ExportPython( fileName );
	FBApplication().ExecuteScript(fileName);
}

/************************************************
 *	Navigator element click callback.
 ************************************************/
void UIBuilderTool::EventNavigatorSel(HISender pSender, HKEvent pEvent )
{
	FBEventTreeSelect	lEvent(pEvent);
	
	FBTreeNode *lNode = lEvent.TreeNode;
	uirect *rect = (uirect*)(kReference)lNode->Reference;
	if (rect)
	{
		mRects.SelectNone();
		rect->Select();
		//rect->FillPropTable(mPropTable);
	}
	RebuildProperties();
}

void UIBuilderTool::RebuildNavigatorTree()
{
	mLayoutRoot = mNavigator.GetRoot();
	mNavigator.Clear();

	uirect	*iter = mRects.first();
	while (iter != NULL)
	{
		iter->treeNode = mNavigator.InsertLast(mLayoutRoot, iter->title);
		iter->treeNode->Reference = (kReference) iter;

		if (iter->visual) {
			switch(iter->visualType)
			{
			#define COMPCLASS(name, objName) case name: \
				iter->treeNode = mNavigator.InsertLast(iter->treeNode, "FB"#objName); break;
			#include "components_list.h"
			#undef COMPCLASS
			}
		}

		iter = mRects.next();
	}
}

/************************************************
 *	UI Idle callback.
 ************************************************/
void UIBuilderTool::EventToolIdle( HISender pSender, HKEvent pEvent )
{
}


/************************************************
 *	Handle tool activation (selection/unselection).
 ************************************************/
void UIBuilderTool::EventToolShow( HISender pSender, HKEvent pEvent )
{
	FBEventShow lEvent( pEvent );

	if( lEvent.Shown )
	{
		// Reset the UI here.
		// update wnd handle
		first = true;
		g_hwnd = 0;
	}
	else
	{
	}
}


/************************************************
 *	Paint callback for tool (on expose).
 ************************************************/
void UIBuilderTool::EventToolPaint( HISender pSender, HKEvent pEvent )
{
}


/************************************************
 *	Tool resize callback.
 ************************************************/
void UIBuilderTool::EventToolResize( HISender pSender, HKEvent pEvent )
{
}


/************************************************
 *	Handle input into the tool.
 ************************************************/
bool componentIn = false;
int lastX, lastY;

void UIBuilderTool::EventToolInput( HISender pSender, HKEvent pEvent )
{
	
}

void UIBuilderTool::EventActionsChange( HISender pSender, HKEvent pEvent )
{
	int ndx = mActions.GetSelection();
	
	if (ndx != (int)mAction)
	{
		switch(mAction)
		{
		case saSelect:
			mFormView.OnInput.Remove( this, (FBCallback) &UIBuilderTool::EventSelectInput );
			break;
		case saRegion:
			mFormView.OnInput.Remove( this, (FBCallback) &UIBuilderTool::EventRegionInput );
			break;
		case saPreview:
			mFormView.OnInput.Remove( this, (FBCallback) &UIBuilderTool::EventPreviewInput );
			mRects.PreviewClose();
			break;
		}

		mAction = (SceneAction) ndx;
		switch(mAction)
		{
		case saSelect:
			mFormView.OnInput.Add( this, (FBCallback) &UIBuilderTool::EventSelectInput );
			break;
		case saRegion:
			mFormView.OnInput.Add( this, (FBCallback) &UIBuilderTool::EventRegionInput );
			mHint.Caption = HINT_REGION_START;
			break;
		case saPreview:
			mFormView.OnInput.Add( this, (FBCallback) &UIBuilderTool::EventPreviewInput );
			mRects.PreviewEnter();
			mHint.Caption = HINT_PREVIEW;
			break;
		}
	}
}

void UIBuilderTool::EnterPreviewMode()
{
	int ndx = saPreview;
	mActions.ItemIndex = saPreview;
	mHint.Caption = HINT_PREVIEW;

	if (ndx != (int)mAction)
	{
		switch(mAction)
		{
		case saSelect:
			mFormView.OnInput.Remove( this, (FBCallback) &UIBuilderTool::EventSelectInput );
			break;
		case saRegion:
			mFormView.OnInput.Remove( this, (FBCallback) &UIBuilderTool::EventRegionInput );
			break;
		case saPreview:
			mFormView.OnInput.Remove( this, (FBCallback) &UIBuilderTool::EventPreviewInput );
			mRects.PreviewClose();
			break;
		}

		mAction = (SceneAction) ndx;
		switch(mAction)
		{
		case saSelect:
			mFormView.OnInput.Add( this, (FBCallback) &UIBuilderTool::EventSelectInput );
			break;
		case saRegion:
			mFormView.OnInput.Add( this, (FBCallback) &UIBuilderTool::EventRegionInput );
			break;
		case saPreview:
			mFormView.OnInput.Add( this, (FBCallback) &UIBuilderTool::EventPreviewInput );
			mRects.PreviewEnter();
			break;
		}
	}
}

void UIBuilderTool::ClosePreviewMode()
{
	int ndx = saSelect;
	mActions.ItemIndex = saSelect;
	mHint.Caption = HINT_SELECT;
	
	if (ndx != (int)mAction)
	{
		switch(mAction)
		{
		case saSelect:
			mFormView.OnInput.Remove( this, (FBCallback) &UIBuilderTool::EventSelectInput );
			break;
		case saRegion:
			mFormView.OnInput.Remove( this, (FBCallback) &UIBuilderTool::EventRegionInput );
			break;
		case saPreview:
			mFormView.OnInput.Remove( this, (FBCallback) &UIBuilderTool::EventPreviewInput );
			mRects.PreviewClose();
			break;
		}

		mAction = (SceneAction) ndx;
		switch(mAction)
		{
		case saSelect:
			mFormView.OnInput.Add( this, (FBCallback) &UIBuilderTool::EventSelectInput );
			break;
		case saRegion:
			mFormView.OnInput.Add( this, (FBCallback) &UIBuilderTool::EventRegionInput );
			break;
		case saPreview:
			mFormView.OnInput.Add( this, (FBCallback) &UIBuilderTool::EventPreviewInput );
			mRects.PreviewEnter();
			break;
		}
	}
}

void UIBuilderTool::EventListChange( HISender pSender, HKEvent pEvent )
{
	int itemIndex = mEventsList.ItemIndex;
	if ( (itemIndex < mEventsList.Items.GetCount()) && (itemIndex >= 0))
	{
		FBString eventName = mEventsList.Items[itemIndex];
		FBStringList	lines;
		mRects.GetEventCode( eventName, lines );
		//if (pLines)
		//{
		//	lines = *pLines;
			mEventCode.SetStrings( &lines );
		//}
	}
	else
	{
		FBStringList lines;
		mEventCode.SetStrings( &lines );
	}
}

void UIBuilderTool::EventMemoChange( HISender pSender, HKEvent pEvent )
{
	int itemIndex = mEventsList.ItemIndex;
	if (itemIndex >=0)
	{
		FBString eventName = mEventsList.Items[itemIndex];
		FBStringList	lines;
		mEventCode.GetStrings( &lines );
		mRects.SetEventCode( eventName, &lines );
	}
}

void UIBuilderTool::EventPropertiesTabChange( HISender pSender, HKEvent pEvent )
{
	mProperties.ClearControl( "EventsList" );
	mProperties.ClearControl( "EventCode" );

	switch(mPropertiesTab.ItemIndex)
	{
	case 0:	mProperties.SetControl( "Spread", mToolTable );
		break;	
	case 1:	mProperties.SetControl( "Spread", mPropTable );
		RebuildProperties();
		break;
	case 2: mProperties.SetControl( "Spread", mCompTable );
		RebuildProperties();
		break;
	case 3: mProperties.ClearControl( "Spread" );
		mProperties.SetControl( "EventsList", mEventsList );
		mProperties.SetControl( "EventCode", mEventCode );
		RebuildProperties();
		break;
	}
}

void UIBuilderTool::EventCompCellChange( HISender pSender, HKEvent pEvent )
{
	FBEventSpread	lEvent(pEvent);
	
	int row, col;
	row = lEvent.Row;
	col = lEvent.Column;

	mRects.ComponentCellChange(mCompTable, col, row);
}

//! tool global properties changed
void UIBuilderTool::EventToolCellChange( HISender pSender, HKEvent pEvent )
{
	FBEventSpread	lEvent(pEvent);
	
	int row, col;
	row = lEvent.Row;
	col = lEvent.Column;

	if ( col == 0 )
	{
		//	x value
		if (row == TOOL_CELL_TITLE)
		{
			const char *lBuffer;
			mToolTable.GetCell(TOOL_CELL_TITLE, 0, lBuffer);
		
			mRects.mTool.name = lBuffer;
		}
		else
		if (row == TOOL_CELL_SIZEX)
		{
			int x;
			mToolTable.GetCell(TOOL_CELL_SIZEX,0, x);
			mRects.mTool.sizex = x;
			SizeRegion( "ViewLayout", mRects.mTool.sizex, mRects.mTool.sizey );
		}
		else
		if (row == TOOL_CELL_SIZEY)
		{
			int y;
			mToolTable.GetCell(TOOL_CELL_SIZEY,0, y);
			mRects.mTool.sizey = y;
			SizeRegion( "ViewLayout", mRects.mTool.sizex, mRects.mTool.sizey );
		}
	}
}

void UIBuilderTool::EventCellChange( HISender pSender, HKEvent pEvent )
{
	FBEventSpread	lEvent(pEvent);
	
	int row, col;
	row = lEvent.Row;
	col = lEvent.Column;

	if ( col == 0 )
	{
		//	x value
		if (row == PROP_CELL_TITLE)
		{
			const char *lBuffer;
			mPropTable.GetCell(PROP_CELL_TITLE, 0, lBuffer);
			//FBString newName(lBuffer);

			mRects.Rename(lBuffer);
			RebuildNavigatorTree();
		}
		else
		if (row == PROP_CELL_X)
		{
			int x;
			mPropTable.GetCell(PROP_CELL_X,0, x);
			mRects.Move( &x );
		}
		else
		if (row == PROP_CELL_Y)
		{
			int y;
			mPropTable.GetCell(PROP_CELL_Y,0, y);
			mRects.Move( 0, &y );
		}
		else
		if (row == PROP_CELL_W)
		{
			int w;
			mPropTable.GetCell(PROP_CELL_W,0, w);
			mRects.Size( &w );
		}
		else
		if (row == PROP_CELL_H)
		{
			int h;
			mPropTable.GetCell(PROP_CELL_H,0, h);
			mRects.Size( 0, &h );
		}
		else
			if ( (row == PROP_CELL_X_ATTACH_TYPE) || (row == PROP_CELL_Y_ATTACH_TYPE) 
				|| (row == PROP_CELL_W_ATTACH_TYPE) || (row == PROP_CELL_H_ATTACH_TYPE) )
		{
			int tX, tY, tW, tH;
			mPropTable.GetCell(PROP_CELL_X_ATTACH_TYPE,0, tX);
			mPropTable.GetCell(PROP_CELL_Y_ATTACH_TYPE,0, tY);
			mPropTable.GetCell(PROP_CELL_W_ATTACH_TYPE,0, tW);
			mPropTable.GetCell(PROP_CELL_H_ATTACH_TYPE,0, tH);

			mRects.ChangeAttachType(&tX, &tY, &tW, &tH);
			RebuildProperties();
		}
		else
			if ( (row == PROP_CELL_X_ATTACH_NODE) || (row == PROP_CELL_Y_ATTACH_NODE) 
				|| (row == PROP_CELL_W_ATTACH_NODE) || (row == PROP_CELL_H_ATTACH_NODE) )
		{
			int tX, tY, tW, tH;
			mPropTable.GetCell(PROP_CELL_X_ATTACH_NODE,0, tX);
			mPropTable.GetCell(PROP_CELL_Y_ATTACH_NODE,0, tY);
			mPropTable.GetCell(PROP_CELL_W_ATTACH_NODE,0, tW);
			mPropTable.GetCell(PROP_CELL_H_ATTACH_NODE,0, tH);

			mRects.ChangeAttachNode(&tX, &tY, &tW, &tH);
			RebuildProperties();
		}
	}
}

//
//
//
RegionAction	regionaction;

void UIBuilderTool::EventSelectInput( HISender pSender, HKEvent pEvent )
{
	
	FBEventInput	lEvent(pEvent);

	int x, y;
	x = lEvent.X;
	y = lEvent.Y;

	switch (lEvent.InputType)
	{
	case  kFBKeyPressRaw:
		{
			int key = lEvent.Key;
			switch(key)
			{
			case VK_RIGHT:{
				int pX = (mSnapStep > 0) ? mSnapStep : 1;
				mRects.Move(&pX, 0, true);
				RebuildProperties();
								 }break;
			case VK_LEFT:{
				int pX = (mSnapStep > 0) ? -mSnapStep : -1;
				mRects.Move(&pX, 0, true);
				RebuildProperties();
									 }break;
			case VK_UP:{
				int pY = (mSnapStep > 0) ? -mSnapStep : -1;
				mRects.Move(0, &pY, true);
				RebuildProperties();
									 }break;
			case VK_DOWN:{
				int pY = (mSnapStep > 0) ? mSnapStep : 1;
				mRects.Move(0, &pY, true);
				RebuildProperties();
										}break;
			case VK_DELETE:
				mRects.DeleteSel();
				RebuildNavigatorTree();
				RebuildAttachList();
				RebuildProperties();
				break;
			}
		} break;
	case kFBButtonPress:
		{
			if (lEvent.MouseButton == 3)
			{
				ShowContextMenu();
			}
			else
			{
				componentIn = false;
				lastX = x;
				lastY = y;
			
				uirect *iter;

				bool drag = false;
				int keystate = lEvent.KeyState;
				if (keystate != kFBKeyShift && keystate != kFBKeyAlt)
				{	
					
					// first - test selection for drag action
					iter = mRects.first();
					while(iter != NULL)
					{
						if (iter->IsSelected() )
						{
							regionaction = iter->testMouseCursor(x,y);
							if (regionaction != raNone)
							{
								drag=true;
								componentIn = true;
								break;
							}
						}
						iter = mRects.next();
					}

					// try to select new object
					if (!drag)
						mRects.SelectNone();
				}
				if (drag) {
					mRects.Hold();
					break;
				}
				int selCount = mRects.GetSelectionCount();

				iter = mRects.first();
				while(iter != NULL)
				{
					regionaction = iter->testMouseCursor(x,y);
					if (regionaction != raNone)
					{
						if (keystate != kFBKeyAlt)
						{
							selCount++;
							iter->Select();
						}
						else
						{
							selCount--;
							iter->Select(false);
						}

						componentIn = (selCount != 0);
						break;
					}
					iter = mRects.next();
				}
				mRects.Hold();
				RebuildProperties();
			}
		}break;
	case kFBButtonRelease:
		{
			mRects.SnapRegions();
			RebuildProperties();

			componentIn = false;
		}break;
	case kFBMotionNotify:
		{
			mHint.Caption = HINT_SELECT;
			if (componentIn)
			{
				switch(regionaction)
				{
				case raMove:
				case raMoveX:
				case raMoveY:
					{
						int pX, pY;

						mHint.Caption = HINT_MOVEALL;
						if (regionaction == raMoveY) {
							pX = 0;
							mHint.Caption = HINT_MOVEY;
						}
						else pX = x - lastX;
						if (regionaction == raMoveX) {
							pY = 0;
							mHint.Caption = HINT_MOVEX;
						}
						else pY = y - lastY;
						mRects.Move(&pX, &pY, true, false);
					}
					break;

				case raSize:
				case raSizeW:
				case raSizeH:
					{
						int pW, pH;
						
						mHint.Caption = HINT_SIZEALL;
						if (regionaction == raSizeH) {
							pW = 0;
							mHint.Caption = HINT_SIZEY;
						}
						else pW = x - lastX;
						if (regionaction == raSizeW) {
							pH = 0;
							mHint.Caption = HINT_SIZEX;
						}
						else pH = y - lastY;
						mRects.Size(&pW, &pH, true, false);
					}
					break;
				}
				//mRects.FillPropTable(mPropTable);
				RebuildProperties();
			}
			else
			{
				HCURSOR	hCursor;
				hCursor=LoadCursor(0, IDC_ARROW);
				SetCursor(hCursor);

				uirect *iter = mRects.first();
				while (iter != NULL)
				{

					regionaction = iter->testMouseCursor(x, y);

					switch (regionaction)
					{
					case raRegion:
						hCursor = LoadCursor(0, IDC_CROSS);
						SetCursor(hCursor);
						mHint.Caption = HINT_SELECT;
						break;
					case raSizeW:
						hCursor = LoadCursor(0, IDC_SIZEWE);
						SetCursor(hCursor);
						mHint.Caption = HINT_SIZEX;
						break;
					case raSizeH:
						hCursor = LoadCursor(0, IDC_SIZENS);
						SetCursor(hCursor);
						mHint.Caption = HINT_SIZEY;
						break;
					case raSize:
						hCursor = LoadCursor(0, IDC_SIZENWSE);
						SetCursor(hCursor);
						mHint.Caption = HINT_SIZEALL;
						break;
					case raMove:
					case raMoveX:
					case raMoveY:
						hCursor = LoadCursor(0, IDC_SIZEALL);
						SetCursor(hCursor);
						mHint.Caption = HINT_MOVEALL;
						break;
					}
					iter = mRects.next();
				}
			}

			lastX = x;
			lastY = y;
		}break;
	}

}

void UIBuilderTool::EventRegionInput( HISender pSender, HKEvent pEvent )
{
	static int creationStep = 0;
	static	uirect	*pRect = NULL;

	FBEventInput	lEvent(pEvent);

	int x, y;
	x = lEvent.X;
	y = lEvent.Y;

	switch (lEvent.InputType)
	{
	case	kFBButtonDoubleClick:
		{
			if (!creationStep)
			{
				componentIn = false;
				lastX = x;
				lastY = y;

				mRects.SelectNone();

				char	name[256];
				char	title[256];
				mRects.GenerateUniqueName(name, title);

				uirect	&rect = mRects.AddElement();
				rect.Clear();

				// add snapping
				if (mSnapStep)
				{
					int step = x / mSnapStep;
					x = step * mSnapStep;

					step = y / mSnapStep;
					y = step * mSnapStep;
				}

				rect.regionx.value = x;
				rect.regiony.value = y;
				rect.Init(mFormView, name, title);
				rect.FillPropTable( mPropTable );

				//uirect *pRect = &rect;
				pRect = &rect;

				pRect->treeNode = mNavigator.InsertLast( mLayoutRoot, pRect->title );
				pRect->treeNode->Reference = (kReference) pRect;
				RebuildAttachList();
			}
		} break;
	case kFBButtonPress:
		{
			if (lEvent.MouseButton == 3)
			{
				
				if (creationStep > 0)
				{
					mRects.DeleteSel();
					creationStep = 0;
					mHint.Caption = HINT_REGION_START;
				}
				else
				{
					// return to select mode
					mActions.ItemIndex = saSelect;
					
					mFormView.OnInput.Remove( this, (FBCallback) &UIBuilderTool::EventRegionInput );
					mFormView.OnInput.Add( this, (FBCallback) &UIBuilderTool::EventSelectInput );

					mAction = saSelect;
				}
				
			}
			else
			if (creationStep == 0)
			{
				// hold rects state to undo
				mRects.Hold();

				componentIn = false;
				lastX = x;
				lastY = y;

				mRects.SelectNone();

				char	name[256];
				char	title[256];
				mRects.GenerateUniqueName(name, title);

				uirect	&rect = mRects.AddElement();

				// add snapping
				if (mSnapStep)
				{
					int step = x / mSnapStep;
					x = step * mSnapStep;

					step = y / mSnapStep;
					y = step * mSnapStep;
				}

				rect.regionx.value = x;
				rect.regiony.value = y;
				rect.Init(mFormView, name, title);
				//rect.FillPropTable( mPropTable );
				RebuildProperties();

				rect.Select();

				creationStep++;
				mHint.Caption = HINT_REGION_END;
			} 
			else
			{
				mRects.SnapRegions();
				pRect = mRects.last();

				pRect->treeNode = mNavigator.InsertLast( mLayoutRoot, pRect->title );
				pRect->treeNode->Reference = (kReference) pRect;
				
				RebuildAttachList();				

				creationStep = 0;
				mHint.Caption = HINT_REGION_START;
			}
		}break;
	case kFBButtonRelease:
		{
			componentIn = false;
		}break;
	case kFBMotionNotify:
		{
			switch(creationStep)
			{
			case 1:
				{
					int pX, pY, pW, pH;

					if (x > lastX)
					{
						pX = lastX;
						pW = x - lastX;
					} 
					else
					{
						pX = x;
						pW = lastX - x;
					}

					if (y > lastY)
					{
						pY = lastY;
						pH = y - lastY;
					} 
					else
					{
						pY = y;
						pH = lastY - y;
					}

					mRects.Move(&pX, &pY, false, false);
					mRects.Size(&pW, &pH, false, false);
				}break;
			}
			/*
			lastX = x;
			lastY = y;
			*/
		}break;
	}

}

void UIBuilderTool::EventPreviewInput( HISender pSender, HKEvent pEvent )
{
	FBEventInput	lEvent(pEvent);

	switch (lEvent.InputType)
	{
	case kFBButtonPress:
		if (lEvent.MouseButton == 3)
			ClosePreviewMode();
		break;
	}
}

/************************************************
 *	FBX Storage.
 ************************************************/
bool UIBuilderTool::FbxStore	( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
{
	pFbxObject->FieldWriteBegin( "ORTool_TemplateSection" );
	{
		//pFbxObject->FieldWriteC( mButton.Caption );
	}
	pFbxObject->FieldWriteEnd();

	return true;
}

/************************************************
 *	FBX Retrieval.
 ************************************************/
bool UIBuilderTool::FbxRetrieve( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat )
{
	pFbxObject->FieldReadBegin( "ORTool_TemplateSection" );
	{
		//mButton.Caption = pFbxObject->FieldReadC();
	}
	pFbxObject->FieldReadEnd();

	return true;
}


