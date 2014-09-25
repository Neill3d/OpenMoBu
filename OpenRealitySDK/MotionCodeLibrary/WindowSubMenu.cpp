
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

#include "WindowSubMenu.h"
#include "Types.h"

//! pointer to each menu
struct	ToolMenuId
{
	HWND		id;
	ToolMenu		*ptr;
};

////////////////////////////////////////////////////////////////////////////////
// ToolMenuHolder
/*!
	*	 singleton for registering all menues
*/
class	ToolMenuHolder	: public Singleton<ToolMenuHolder>
{
public:
	//! a constructor
	ToolMenuHolder()
	{
		sprintf( g_szTitle, "MoCap Advance" );
		g_hwnd = 0;
		first = true;
		mToolMenu.ptr = NULL;
	}
	//! add new menu to the list
	void		AddMenu(HWND	hWnd,	ToolMenu	*ptr)
	{
		//ToolMenuId	item;
		mToolMenu.id = hWnd;
		mToolMenu.ptr = ptr;

		//mToolMenus.Add(item);
	}
	void		FreeMenu()
	{
		//mToolMenu.id = 0;
		//mToolMenu.ptr = NULL;
	}
	//! redirect notify to each menu
	void Message(UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
	{
		if ( (uMsg == WM_COMMAND) )
			if (mToolMenu.ptr)
				mToolMenu.ptr->ItemPress( LOWORD(wParam) );
		/*
			for (int i=0; i<mToolMenus.GetCount(); i++)
			{
				mToolMenu.ptr->ItemPress( LOWORD(wParam) );
			}
			*/
	}

public:
	// original MB window procedure
	WNDPROC wpOrigEditProc;
	//	window handle
	HWND	g_hwnd;
	// is we searching window at the first time
	bool	first;
	// title of the searching window
	char	g_szTitle[MAX_PATH];

private:
	//! menues list
	//FBArrayTemplate<ToolMenuId>		mToolMenus;
	ToolMenuId		mToolMenu;
};

//! replacable window class proc
LRESULT APIENTRY EditSubclassProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
  if (uMsg == WM_GETDLGCODE)
      return DLGC_WANTALLKEYS;

	ToolMenuHolder::instance().Message(uMsg, wParam, lParam);
	return CallWindowProc(ToolMenuHolder::instance().wpOrigEditProc, hwnd, uMsg,
      wParam, lParam);
}

//! enumerate all windows to find actually needed
BOOL CALLBACK EnumProc( HWND hwnd, LPARAM lParam )
{
	char		szTitle[MAX_PATH];
	GetWindowText( hwnd, szTitle, MAX_PATH );

	if (strstr( szTitle, ToolMenuHolder::instance().g_szTitle ) != NULL)
	{
		ToolMenuHolder::instance().g_hwnd = hwnd;

		if (ToolMenuHolder::instance().first){
			// Subclass the edit control.
			ToolMenuHolder::instance().wpOrigEditProc = (WNDPROC) SetWindowLongPtr( hwnd, GWLP_WNDPROC, (LONG_PTR) EditSubclassProc);
		}
		return false;
	}

	return true;
}

ToolMenu::ToolMenu()
{
	mTitle = "MotionBuilder";
	//mWnd = 0;
	mFirst = true;
	mHandle = NULL;
}

FBToolMenu::FBToolMenu()
		: FBComponent(NULL)
		, ToolMenu()
{
	mButton.OnClick.Add(this, (FBCallback) &FBToolMenu::EventButtonClick );
}

//! set menu title
void ToolMenu::SetTitle(const char *title)
{
	mTitle = title;
}
//! set menu items handler
void ToolMenu::SetHandle(MenuHandle		*handle)
{
	mHandle = handle;
}
//! item press notify
void ToolMenu::ItemPress(int item)
{
	if (mHandle)
		mHandle->ItemPress(item);
}
//! menu button click
void FBToolMenu::EventButtonClick( HISender pSender, HKEvent pEvent )
{
	Show();
}

//! return reference to the fbbutton control
FBButton	&FBToolMenu::GetControl()
{
	return mButton;
}
void ToolMenu::Update() {
	mFirst = true;
}

//! find window handle
void ToolMenu::PrepareWndProc()
{
	if (mFirst) {
		ToolMenuHolder::instance().first = true;
		mFirst = false;
	}
	if (ToolMenuHolder::instance().first)
	{
		EnumWindows( EnumProc, 0 );
		ToolMenuHolder::instance().first = false;
	}
	ToolMenuHolder::instance().AddMenu( ToolMenuHolder::instance().g_hwnd, this );
}

//! popup menu
void	ToolMenu::Show()
{
	strcpy_s( ToolMenuHolder::instance().g_szTitle, mTitle );
	PrepareWndProc();

	POINT	pos;
	GetCursorPos(&pos);

	HMENU	hSubMenu;
	hSubMenu = CreatePopupMenu();

	if (mHandle)
	{
		int count=0;
		MenuItem		*items = NULL;
		if ( (items = mHandle->ItemCollect(&count)) != NULL )
		{
			for (int i=0; i<count; i++)
			{
				AppendMenu( hSubMenu, items[i].type, items[i].id, items[i].text.data() );
				if (items[i].state)
					CheckMenuItem(hSubMenu, items[i].id, MF_CHECKED );
			}
		}
	}

	BOOL result = TrackPopupMenuEx(   hSubMenu,
								 0,
				 pos.x,
				 pos.y,
				 ToolMenuHolder::instance().g_hwnd,
								 NULL);

	if (!result)
	{
//		DWORD err = GetLastError();
		FBMessageBox("Error", "by menu", "OK" );
	}
	ToolMenuHolder::instance().FreeMenu();
	DestroyMenu(hSubMenu);
}