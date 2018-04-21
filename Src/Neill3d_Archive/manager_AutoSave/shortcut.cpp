
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

#include "shortcut.h"


ShortCutEmulator	gShortCutEmulator;

ShortCutEmulator	&GetShortCutEmulator()
{
	return gShortCutEmulator;
}

/////////////////////////////////////////////////////////////////////////////////////

//! a constructor
ButtonTarget::ButtonTarget()
{
	parent = 0;
	currNode = 0;
	index = 0;
	checkpoint = 0;
	WND = 0;
}

void ButtonTarget::Clear(HWND parentwnd)
{
	parent = parentwnd;
	currNode = 0;
	index = 0;
	checkpoint = 0;
	WND = 0;
}

bool ButtonTarget::TryToFind(HWND hwnd, HWND nodeParentWnd)
{
	if (!points.size() ) return false;
	if (WND) return true;

	ControlPoint &control = points[checkpoint];
	if (nodeParentWnd == control.parentWnd) {
		if (control.child == index) {
			// we pass this controlpoint, go to next
			checkpoint++;

			if (checkpoint == points.size() )
			{
				// we already find our node!!
				WND = hwnd;
			}
			else
			{
				points[checkpoint].parentWnd = hwnd; // this node is parent
				index = 0;
			}
		}
		else
		{
			index++;
		}
	}

	return (WND != 0);
}

void ButtonTarget::KeyDown()
{
	if (WND) {
		RECT lRect;
		if (TRUE == GetWindowRect(WND, &lRect) )
		{
				
			POINT lPoint, tempPoint;
			GetCursorPos(&tempPoint);
				
			lPoint.x = lRect.left+13;
			lPoint.y = lRect.bottom-3;

			int ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
			int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);

			INPUT lInput;
			memset( &lInput, 0, sizeof(INPUT) );
			lInput.type = INPUT_MOUSE;
			lInput.mi.dx = lPoint.x * (65535/ScreenWidth);
			lInput.mi.dy = lPoint.y * (65535/ScreenHeight);
			lInput.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP;

			SendInput( 1, &lInput, sizeof(INPUT) );

			//SetCursorPos(tempPoint.x, tempPoint.y);
			/*
			lInput.mi.dx = lPoint.x * (65535/ScreenWidth);
			lInput.mi.dy = lPoint.y * (65535/ScreenHeight);
			lInput.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
			SendInput( 1, &lInput, sizeof(INPUT) );
			*/
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//

void ShortCutEmulator::InitButtonAuto(HWND FCurvesWnd)
{
	std::vector<ControlPoint> &points = ButtonAuto.points;
	points.clear();
	points.push_back( ControlPoint(1, 0, FCurvesWnd) );
		points.push_back( ControlPoint(1, 0) );
			points.push_back( ControlPoint(1, 0) );
					points.push_back( ControlPoint(1, 0) );
					points.push_back( ControlPoint(1, 3) ); // auto button

	ButtonAuto.Clear(FCurvesWnd);
}

void ShortCutEmulator::InitButtonFlat(HWND FCurvesWnd)
{
	std::vector<ControlPoint> &points = ButtonFlat.points;
	points.clear();
	points.push_back( ControlPoint(1, 0, FCurvesWnd) );
		points.push_back( ControlPoint(1, 0) );
			points.push_back( ControlPoint(1, 0) );
					points.push_back( ControlPoint(1, 1) );
					points.push_back( ControlPoint(1, 0) );
					points.push_back( ControlPoint(1, 3) );
						points.push_back( ControlPoint(1, 15) ); // flat keys button

	ButtonFlat.Clear(FCurvesWnd);
}

void ShortCutEmulator::InitButtonBezier(HWND FCurvesWnd)
{
	std::vector<ControlPoint> &points = ButtonBezier.points;
	points.clear();
	points.push_back( ControlPoint(1, 0, FCurvesWnd) );
		points.push_back( ControlPoint(1, 0) );
			points.push_back( ControlPoint(1, 0) );
					points.push_back( ControlPoint(1, 0) );
					points.push_back( ControlPoint(1, 5) ); // bezier button

	ButtonBezier.Clear(FCurvesWnd);
}

void ShortCutEmulator::InitButtonDiscLeft(HWND FCurvesWnd)
{
	std::vector<ControlPoint> &points = ButtonDiscLeft.points;
	points.clear();
	points.push_back( ControlPoint(1, 0, FCurvesWnd) );
		points.push_back( ControlPoint(1, 0) );
			points.push_back( ControlPoint(1, 0) );
					points.push_back( ControlPoint(1, 1) );
					points.push_back( ControlPoint(1, 0) );
					points.push_back( ControlPoint(1, 3) );
						points.push_back( ControlPoint(1, 16) ); // flat keys button

	ButtonDiscLeft.Clear(FCurvesWnd);
}

void ShortCutEmulator::InitButtonDiscRight(HWND FCurvesWnd)
{
	std::vector<ControlPoint> &points = ButtonDiscRight.points;
	points.clear();
	points.push_back( ControlPoint(1, 0, FCurvesWnd) );
		points.push_back( ControlPoint(1, 0) );
			points.push_back( ControlPoint(1, 0) );
					points.push_back( ControlPoint(1, 1) );
					points.push_back( ControlPoint(1, 0) );
					points.push_back( ControlPoint(1, 3) );
						points.push_back( ControlPoint(1, 12) ); // flat keys button

	ButtonDiscRight.Clear(FCurvesWnd);
}

void ShortCutEmulator::InitButtonWt(HWND FCurvesWnd)
{
	std::vector<ControlPoint> &points = ButtonWt.points;
	points.clear();
	points.push_back( ControlPoint(1, 0, FCurvesWnd) );
		points.push_back( ControlPoint(1, 0) );
			points.push_back( ControlPoint(1, 0) );
					points.push_back( ControlPoint(1, 1) );
					points.push_back( ControlPoint(1, 0) );
					points.push_back( ControlPoint(1, 3) );
						points.push_back( ControlPoint(1, 5) ); // flat keys button

	ButtonWt.Clear(FCurvesWnd);
}

bool ShortCutEmulator::Init()
{
	static bool Searching = false;

	if (Searching == true) return Searching;

	// bind fcurve shortcuts
	HWND FCurvesWnd = FindWindow("KWindow", "FCurves");
	if (FCurvesWnd) {
			
		// flat tangents button path
		InitButtonAuto(FCurvesWnd);
		InitButtonFlat(FCurvesWnd);
		
		InitButtonBezier(FCurvesWnd);

		// discontinues left
		InitButtonDiscLeft(FCurvesWnd);
		InitButtonDiscRight(FCurvesWnd);
		InitButtonWt(FCurvesWnd);

		// bezier key type button path


		Searching = true;

		EnumChildWindows(FCurvesWnd, FCurve_EnumProc, (LPARAM) this );
	}

	return Searching;
}

void ShortCutEmulator::KeyDown(WPARAM wParam, LPARAM lParam)
{
		
}


void ShortCutEmulator::KeyChar(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case 's':
	case 'S':
		ButtonAuto.KeyDown();
		break;
	case 'b':
	case 'B':
		ButtonBezier.KeyDown();
		break;
	case 'w':
	case 'W':
		ButtonWt.KeyDown();
		break;
	case 'q':
	case 'Q':
		ButtonDiscLeft.KeyDown();
		break;
	case 'e':
	case 'E':
		ButtonDiscRight.KeyDown();
		break;
	case 't':
	case 'T':
		ButtonFlat.KeyDown();
		break;
	}
		
}


//////////////////////////////////////////////////////////////////////////////////////

// original MB window procedure
extern WNDPROC wpOrigEditProc; 
extern WNDPROC wpOrigFCurvesProc;

// Subclass procedure 
LRESULT APIENTRY FCurveSubclassProc(
    HWND hwnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam) 
{ 
    if (uMsg == WM_GETDLGCODE) 
        return DLGC_WANTALLKEYS; 
 
	if (uMsg == WM_KEYDOWN)
	{
		printf( "fcurve key pressed\n" );
		gShortCutEmulator.KeyDown(wParam, lParam);
		
	}
	else if (uMsg == WM_CHAR)
	{
		printf( "fcurve char msg\n" );
		gShortCutEmulator.KeyChar(wParam, lParam);
	}

    return CallWindowProc(wpOrigFCurvesProc, hwnd, uMsg, 
        wParam, lParam); 
}

// try to find fcurve editor and buttons
BOOL CALLBACK FCurve_EnumProc( HWND hwnd, LPARAM lParam )
{
	ShortCutEmulator *emulator = (ShortCutEmulator*) lParam;
	
	char		szTitle[MAX_PATH];
	GetWindowText( hwnd, szTitle, MAX_PATH );
	
	if (strstr(szTitle, "KtOpenGL") != NULL)
	{
		//if (!FCurvesWnd)
		//{
			//FCurvesWnd = hwnd;
			// Subclass the edit control. 
			wpOrigFCurvesProc = (WNDPROC) SetWindowLongPtr( hwnd, GWLP_WNDPROC, (LONG_PTR) FCurveSubclassProc); 
		//}
	}
	else 
	{
		HWND nodeParentWnd = GetParent(hwnd);

		emulator->ButtonAuto.TryToFind(hwnd, nodeParentWnd);
		emulator->ButtonFlat.TryToFind(hwnd, nodeParentWnd);
		emulator->ButtonBezier.TryToFind(hwnd, nodeParentWnd);
		emulator->ButtonDiscLeft.TryToFind(hwnd, nodeParentWnd);
		emulator->ButtonDiscRight.TryToFind(hwnd, nodeParentWnd);
		emulator->ButtonWt.TryToFind(hwnd, nodeParentWnd);
	}
	
	return true;
}