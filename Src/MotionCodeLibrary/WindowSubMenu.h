
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

#pragma once

#include <windows.h>
//--- OR SDK include
#include <fbsdk/fbsdk.h>
#include <array>

// Subclass procedure
LRESULT APIENTRY EditSubclassProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);
// iterate searching window class
BOOL CALLBACK EnumProc( HWND hwnd, LPARAM lParam );

//! menu item type
struct	MenuItem
{
	int		type;			//!> string / separator
	int		id;				//!> menu item id
	std::array<char,80>	text;		//!> menu item caption
	bool	state;			//!> is item checked ?

	//! a constructor
	MenuItem()
	{
		type = MF_STRING;
		id = 0;
		state = false;
		text.fill(0);
	}
	MenuItem(int _id, const char *_caption="item", int _type=MF_STRING, bool _state=false)
		: id(_id)
		, type(_type)
		, state(_state)
	{
		text.fill(0);
		strcpy_s( text.data(), text.size(), _caption );
	}
};

////////////////////////////////////////////////////////////////////////////////
// MenuHandle
/*!
	*	 abstract class for menu handler (add new items and listen for notifiers)
*/
class		MenuHandle
{
public:
	//! item press notify
	virtual		int		ItemPress(int item) = 0;
	//! add items to the menu
	virtual		MenuItem		*ItemCollect(int *count) = 0;
};

////////////////////////////////////////////////////////////////////////////////
// ToolMenu
/*!
	*	 class for operating with a button and popup win32 menu with items
*/
class ToolMenu
{
public:
	// original MB window procedure
	WNDPROC mOrigEditProc;

	//! a constructor
	ToolMenu();

	//! set menu title
	void SetTitle(const char *title);
	//! set menu items handler
	void SetHandle(MenuHandle		*handle);
	//! item press notify
	void ItemPress(int item);
	//! popup menu
	void	Show();

	//! use update when your tool is shown
	void Update();

private:
	//!	window title 
	FBString		mTitle;

	bool			mFirst;	// flag for updating window id

	//! items notify handler
	MenuHandle		*mHandle;

	//! prepare global variables
	void PrepareWndProc();
};

class FBToolMenu : public FBComponent, public ToolMenu
{
public:

	//! constructor
	FBToolMenu();

	//! menu button click
	void	EventButtonClick( HISender pSender, HKEvent pEvent );

	//! return reference to the fbbutton control
	FBButton	&GetControl();

private:
	
	//! menu popup button
	FBButton		mButton;
};


