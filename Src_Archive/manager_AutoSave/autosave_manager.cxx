
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

/** \file   autosave_manager.cxx
*/

#include <windows.h>

//--- Class declaration
#include "autosave_manager.h"
#include <vector>

#include "fbsdk/fbmath.h"

#include "helper.h"
#include "shortcut.h"

//--- Registration defines
#define AUTOSAVE__CLASS	AUTOSAVE__CLASSNAME
#define AUTOSAVE__NAME  AUTOSAVE__CLASSSTR

//-- autoback filenames
#define	AUTOSAVE_FILENAME_1		"character1"
#define AUTOSAVE_FILENAME_2		"character2"
#define AUTOSAVE_FILENAME_3		"character3"
#define AUTOSAVE_FILENAME_4		"character4"
#define AUTOSAVE_FILENAME_5		"character5"
#define AUTOSAVE_FILENAME_6		"character6"
#define AUTOSAVE_FILENAME_7		"character7"

//-- menu item captions
#define	MENU_AUTOSAVE_CHECK		"Auto save"
#define MENU_AUTOSAVE_PERIOD	"Save period..."
#define MENU_AUTOSAVE_PATH		"Choose save folder..."

#define MENU_GEOMETRY_REPLACE	"Replace geometry"
#define MENU_ADDITIVE_ANIMATION	"Additive calculate"

#define		menuRePlotItem				0x4000
#define		menuRePlotTakesItem			0x4001
#define		menuPlotRegionItem			0x4002
#define		menuHandleStartItem			0x4003
#define		menuHandleFinishItem		0x4004

#define		menuSaveActiveItem		0x4005
#define		menuSavePerionItem		0x4006
#define		menuSavePathItem		0x4007

#define		menuGeomReplace			0x4008
#define		menuAdditiveCalc		0x4009

//--- FiLMBOX implementation and registration
FBCustomManagerImplementation( AUTOSAVE__CLASS  );  // Manager class name.
FBRegisterCustomManager( AUTOSAVE__CLASS );         // Manager class name.

//-- execute script
HWND		a_gWnd=0;
HMENU		a_gMenu=0;

// fcurve shortcuts
HWND		FCurvesWnd=0;
HWND		BtnLinWnd=0; // make keys linear

//-- global params for main autosave workflow (plug main state)
bool		mActive;			// flag for processing autosave
int			mPeriod;			// time to make up save in ms
FBString	mFilePath;			// dir, where we must make up backup's

// original MB window procedure
WNDPROC wpOrigEditProc; 
WNDPROC wpOrigFCurvesProc;

/************************************************
 *  FiLMBOX Creation
 ************************************************/
bool AutoSaveManager::FBCreate()
{
    return true;
}


/************************************************
 *  FiLMBOX Destruction.
 ************************************************/
void AutoSaveManager::FBDestroy()
{
    // Free any user memory here.
	FBSystem			lSystem;
	lSystem.OnUIIdle.Remove( this, (FBCallback) &AutoSaveManager::EventIdle );

	//
	// save configuration on quit
	//
	FBConfigFile	lConfig( "@autosave.txt" );

	//-- active state
	if (mActive)
		lConfig.Set( "Main", "Active", "true", "state of work condition" );
	else
		lConfig.Set( "Main", "Active", "false", "state of work condition" );

	//-- backup path
	lConfig.Set( "Main", "Path", mFilePath, "folder for backsave files" );

	//-- save period
	char *szTemp = new char[MAX_PATH];
	sprintf( szTemp, "%d", mPeriod );
	lConfig.Set( "Main", "Period", szTemp, "period of the saving operation" );

	if (szTemp)
	{
		delete[] szTemp;
		szTemp = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Subclass procedure 
LRESULT APIENTRY EditSubclassProc(
    HWND hwnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam) 
{ 
    if (uMsg == WM_GETDLGCODE) 
        return DLGC_WANTALLKEYS; 
 /*
	if (uMsg == WM_KEYDOWN)
	{
		printf( "item pressed\n" );
		if (wParam = VK_BACK)
		{
			GetShortCutEmulator().Init();
		}
	}
	*/
	if (uMsg == WM_COMMAND)
	{
		switch ( LOWORD(wParam) )
		{
		// RePlot script
		case menuRePlotItem:
			//-- hold shift key
			keybd_event( VK_SHIFT, MapVirtualKey( VK_SHIFT, 0 ), 0, 0 );

			//-- press F2
			keybd_event( VK_F2, MapVirtualKey( VK_F2, 0), 0, 0 );						// key_down event
			keybd_event( VK_F2, MapVirtualKey( VK_F2, 0), KEYEVENTF_KEYUP | 0, 0 );		// key_up event

			//-- release shift key
			keybd_event( VK_SHIFT, MapVirtualKey( VK_SHIFT, 0 ), KEYEVENTF_KEYUP, 0 );

			//SendMessage( a_gWnd, WM_KEYDOWN, 113, 3932161 );		// F2 key
			break;
		// RePlot all takes
		case menuRePlotTakesItem:
			//-- hold shift key
			keybd_event( VK_SHIFT, MapVirtualKey( VK_SHIFT, 0 ), 0, 0 );

			//-- press F2
			keybd_event( VK_F3, MapVirtualKey( VK_F3, 0), 0, 0 );						// key_down event
			keybd_event( VK_F3, MapVirtualKey( VK_F3, 0), KEYEVENTF_KEYUP | 0, 0 );		// key_up event

			//-- release shift key
			keybd_event( VK_SHIFT, MapVirtualKey( VK_SHIFT, 0 ), KEYEVENTF_KEYUP, 0 );

			//SendMessage( a_gWnd, WM_KEYDOWN, 114, 3932161 );		// F3 key
			break;
		// Plot region
		case menuPlotRegionItem:
			//-- hold shift key
			keybd_event( VK_SHIFT, MapVirtualKey( VK_SHIFT, 0 ), 0, 0 );

			//-- press F2
			keybd_event( VK_F4, MapVirtualKey( VK_F4, 0), 0, 0 );						// key_down event
			keybd_event( VK_F4, MapVirtualKey( VK_F4, 0), KEYEVENTF_KEYUP | 0, 0 );		// key_up event

			//-- release shift key
			keybd_event( VK_SHIFT, MapVirtualKey( VK_SHIFT, 0 ), KEYEVENTF_KEYUP, 0 );

			//SendMessage( a_gWnd, WM_KEYDOWN, 115, 3932161 );		// F4 key
			break;
		// handle start
		case menuHandleStartItem:
			//-- hold shift key
			keybd_event( VK_SHIFT, MapVirtualKey( VK_SHIFT, 0 ), 0, 0 );

			//-- press F2
			keybd_event( VK_F5, MapVirtualKey( VK_F5, 0), 0, 0 );						// key_down event
			keybd_event( VK_F5, MapVirtualKey( VK_F5, 0), KEYEVENTF_KEYUP | 0, 0 );		// key_up event

			//-- release shift key
			keybd_event( VK_SHIFT, MapVirtualKey( VK_SHIFT, 0 ), KEYEVENTF_KEYUP, 0 );

			//SendMessage( a_gWnd, WM_KEYDOWN, 116, 3932161 );		// F5 key
			break;
		// handle finish
		case menuHandleFinishItem:
			//-- hold shift key
			keybd_event( VK_SHIFT, MapVirtualKey( VK_SHIFT, 0 ), 0, 0 );

			//-- press F2
			keybd_event( VK_F6, MapVirtualKey( VK_F6, 0), 0, 0 );						// key_down event
			keybd_event( VK_F6, MapVirtualKey( VK_F6, 0), KEYEVENTF_KEYUP | 0, 0 );		// key_up event

			//-- release shift key
			keybd_event( VK_SHIFT, MapVirtualKey( VK_SHIFT, 0 ), KEYEVENTF_KEYUP, 0 );

			//SendMessage( a_gWnd, WM_KEYDOWN, 117, 3932161 );		// F6 key
			break;

		// save active
		case menuSaveActiveItem:
			if (mActive)
			{
				mActive = !mActive;
				CheckMenuItem( a_gMenu, 8, MF_BYPOSITION|MF_UNCHECKED );
			}
			else
			{
				mActive = !mActive;
				CheckMenuItem( a_gMenu, 8, MF_BYPOSITION|MF_CHECKED );
			}
			break;

		// autosave period
		case menuSavePerionItem:
			{
				int temp = mPeriod;
				if ( FBMessageBoxGetUserValue(	"AutoSave", 
											"Set save period...", 
											&temp,
											kFBPopupInt,
											"Ok",
											"Cancel" ) == 1 )
				{
					mPeriod = temp;
				}
			}
			break;

		case menuSavePathItem:
			{
				FBString szTemp = mFilePath;
				if ( FBMessageBoxGetUserValue(	"AutoSave", 
											"Set save period...", 
											szTemp,
											kFBPopupString,
											"Ok",
											"Cancel" ) == 1 )
				{
					mFilePath = szTemp;
				}
			}
			break;

		case	menuGeomReplace:
			{
				
				GeometryReplace();

			}break;

			case	menuAdditiveCalc:

				AdditiveCalculate();

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

	if (strstr( szTitle, "MotionBuilder " ) != NULL)
	{
		//-- store motion builder window handle
		a_gWnd = hwnd;
		// Subclass the edit control. 
	    wpOrigEditProc = (WNDPROC) SetWindowLongPtr( hwnd, GWLP_WNDPROC, (LONG_PTR) EditSubclassProc); 

		//
		// rename menu
		//

		HMENU	lMenu	= ::GetMenu( hwnd );
		MENUITEMINFO mii;

		int itemsCount = ::GetMenuItemCount( lMenu );
		UINT	Id;
		char	szMenuString[255];

		/*
		mii.cbSize = sizeof( mii );
		mii.fMask = MIIM_STRING;
		mii.dwTypeData = szMenuString;
		mii.cch = 255;

		GetMenuItemInfo( lMenu, i, TRUE, &mii );

		sprintf( szMenuString, "&CREOTEAM" );
		if ( SetMenuItemInfo( lMenu, i, TRUE, &mii ) == 0 )
		{
			printf( "error while renaming menu\n" );
			DisplayErrorText( GetLastError() );
		}
		*/

		//
		// build custom menu
		//
			
		//HMENU cMenu = ::GetSubMenu( lMenu,i );
		HMENU cMenu = ::CreatePopupMenu();
		a_gMenu = cMenu;

		::AppendMenu( cMenu, MF_SEPARATOR, 0, NULL);
		::AppendMenu( cMenu, MF_BYPOSITION, menuRePlotItem,		"&RePlot");
		::AppendMenu( cMenu, MF_BYPOSITION, menuRePlotTakesItem,	"R&ePlot all takes");
		::AppendMenu( cMenu, MF_BYPOSITION, menuPlotRegionItem,	"P&lot region");
		::AppendMenu( cMenu, MF_SEPARATOR, 0, NULL);
		::AppendMenu( cMenu, MF_BYPOSITION, menuHandleStartItem,	"&Handle start");
		::AppendMenu( cMenu, MF_BYPOSITION, menuHandleFinishItem, "Ha&ndle finish");
		::AppendMenu( cMenu, MF_SEPARATOR, 0, NULL);
		::AppendMenu( cMenu, MF_BYPOSITION, menuSaveActiveItem,	MENU_AUTOSAVE_CHECK );
		::AppendMenu( cMenu, MF_BYPOSITION, menuSavePerionItem,	MENU_AUTOSAVE_PERIOD ); 
		::AppendMenu( cMenu, MF_BYPOSITION, menuSavePathItem,		MENU_AUTOSAVE_PATH ); 
		::AppendMenu( cMenu, MF_SEPARATOR, 0, NULL);
		::AppendMenu( cMenu, MF_BYPOSITION, menuGeomReplace,		MENU_GEOMETRY_REPLACE ); 
		::AppendMenu( cMenu, MF_BYPOSITION, menuAdditiveCalc,		MENU_ADDITIVE_ANIMATION ); 

		if (mActive) ::CheckMenuItem( cMenu, 8, MF_BYPOSITION|MF_CHECKED );
			
		::InsertMenu( lMenu, itemsCount, MF_BYPOSITION|MF_POPUP, UINT(cMenu), "Add-Ons" );

		//-- redraw main menu bar
		DrawMenuBar( hwnd );

		//
		//
		//
		GetShortCutEmulator().Init();

		return false;
	}

	return true;
}

bool AutoSaveManager::Init()
{
	mFirstLoad = true;

    return true;
}

bool AutoSaveManager::Open()
{
	FBSystem			lSystem;
	FBString			lPath = lSystem.ApplicationPath;
	FBString			lConfigPath( "\\Config\\" );
	FBString			lFilePath( ".autosave.txt" );

	const char				*lActiveDefault		= "false";
	const char				*lPathDefault		= "\\Autoback\\";
	const char				*lPeriodDefault		= "30000";

	lPath = lPath + lConfigPath;
	lPath = lPath + lSystem.ComputerName;
	lPath = lPath + ".autosave.txt";

	FBConfigFile	lConfig( "@autosave.txt" );
	
	FILE	*f;
	if ( (f = fopen( lPath, "r" )) == NULL )
	{
		lConfig.ClearFile();
	}
	else
	{
		fclose( f );
	}

	//-- active state
	lConfig.GetOrSet( "Main", "Active", lActiveDefault, "state of work condition" );
	if ( strstr(lActiveDefault, "false") != NULL )
	{
		mActive = false;
	}
	else mActive = true;

	//-- backup path
	lConfig.GetOrSet( "Main", "Path", lPathDefault, "folder for backsave files" );
	mFilePath = lPathDefault;
	if ( mFilePath == "\\Autoback\\" )
	{
		mFilePath = lSystem.ApplicationPath;
		mFilePath = mFilePath + lPathDefault;
	} 

	//-- save period
	lConfig.GetOrSet( "Main", "Period", lPeriodDefault, "period of the saving operation" );
	if (lPeriodDefault)
	{
		mPeriod = atoi( lPeriodDefault );
	}


	//-- set event to work
	//if (mActive)
	//{
	lSystem.OnUIIdle.Add( this, (FBCallback) &AutoSaveManager::EventIdle );
	mLastTime	= lSystem.SystemTime;
	mIndex		= 1;
	//}

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// System Idle event
/////////////////////////////////////////////////////////////////////////////////////////////////////
void AutoSaveManager::EventIdle( HISender pSender, HKEvent pEvent )
{
	if (mFirstLoad) {
		EnumWindows( EnumProc, 0 );
		mFirstLoad = false;
	}

	GetShortCutEmulator().Init();

	if (!mActive) return;

	FBSystem			lSystem;
	FBTime				lTime = lSystem.SystemTime;
	long				lThisTime = lTime.GetMilliSeconds();
	long				lLastTime = mLastTime.GetMilliSeconds();
	if ( (lThisTime - lLastTime) > mPeriod )
	{
		FBTrace( "save operation" );
		mLastTime = lTime;

		FBApplication lApplication;
		HFBCharacter lCharacter = lApplication.CurrentCharacter;
		if( lCharacter )
		{
			FBString lFileName( mFilePath );
			switch( mIndex )
			{
			case 1: lFileName = lFileName + AUTOSAVE_FILENAME_1;
				break;
			case 2: lFileName = lFileName + AUTOSAVE_FILENAME_2;
				break;
			case 3: lFileName = lFileName + AUTOSAVE_FILENAME_3;
				break;
			case 4: lFileName = lFileName + AUTOSAVE_FILENAME_4;
				break;
			case 5: lFileName = lFileName + AUTOSAVE_FILENAME_5;
				break;
			case 6: lFileName = lFileName + AUTOSAVE_FILENAME_6;
				break;
			case 7: lFileName = lFileName + AUTOSAVE_FILENAME_7;
				break;
			};
			
			//-- store current manipulator
			HFBManipulator	lCurrManip;
			lCurrManip = FBRenderer(NULL).ManipulatorTransform;

			//-- store character selection
			HFBModel	pModel;

			bool	modelHold[kFBLastNodeId];
			bool	rigmodelHold[kFBLastNodeId];
			bool	effectorHold[kFBLastEffectorId];

			for (int i=0; i< kFBLastNodeId; i++)
			{
				pModel = lCharacter->GetModel( FBBodyNodeId(i) );
				if (pModel) modelHold[i] = pModel->Selected;
				else modelHold[i] = false;
				pModel = lCharacter->GetCtrlRigModel( FBBodyNodeId(i) );
				if (pModel) rigmodelHold[i] = pModel->Selected;
				else rigmodelHold[i] = false;
			}

			for (int i=0; i< kFBLastEffectorId; i++)
			{
				pModel = lCharacter->GetEffectorModel( FBEffectorId(i) );
				if (pModel) effectorHold[i] = pModel->Selected;
				else effectorHold[i] = false;
			}

			//-- save character with rig & extensions
			if (!FBFbxManager().SaveCharacterRigAndAnimation( lFileName, lCharacter, true, true, true ) )
				FBMessageBox( "AutoSave", "saving failed!", "Ok" );
			
			//-- restore character selection
			for (int i=0; i< kFBLastNodeId; i++)
			{
				if (modelHold[i]) {
					pModel = lCharacter->GetModel( FBBodyNodeId(i) );
					if (pModel) pModel->Selected = modelHold[i];
				}
				if (rigmodelHold[i]) {
					pModel = lCharacter->GetCtrlRigModel( FBBodyNodeId(i) );
					if (pModel) pModel->Selected = rigmodelHold[i];
				}
			}

			for (int i=0; i< kFBLastEffectorId; i++)
			{
				if (effectorHold[i]) {
					pModel = lCharacter->GetEffectorModel( FBEffectorId(i) );
					if (pModel) pModel->Selected = effectorHold[i];
				}
			}

			//-- restore current manipulator
			if (lCurrManip) lCurrManip->Active = true;

			mIndex++;
			if (mIndex > 7) 
			{
				mIndex = 1;
			}
		}
	}
}

bool AutoSaveManager::Clear()
{
	//-- reset
	if (mActive)
	{
		FBSystem			lSystem;
		mLastTime = lSystem.SystemTime;
	}

    return true;
}

bool AutoSaveManager::Close()
{
	/*
	if (mActive)
	{
		FBSystem			lSystem;
		lSystem.OnUIIdle.Remove( this, (FBCallback) EventIdle );

		//
		// save configuration on quit
		//
		FBConfigFile	lConfig( "@autosave.txt" );

		//-- active state
		if (mActive)
			lConfig.Set( "Main", "Active", "true", "state of work condition" );
		else
			lConfig.Set( "Main", "Active", "false", "state of work condition" );

		//-- backup path
		lConfig.Set( "Main", "Path", mFilePath, "folder for backsave files" );

		//-- save period
		char *szTemp;
		sprintf( szTemp, "%d", mPeriod );
		lConfig.Set( "Main", "Period", szTemp, "period of the saving operation" );

		if (szTemp)
			delete[] szTemp;
	}
	*/

    return true;
}
