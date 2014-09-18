
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

#pragma once

#include <Windows.h>
#include <vector>

// forward declaration
class ShortCutEmulator;

// function to get access to global variable
ShortCutEmulator	&GetShortCutEmulator();

////////////////////////////////////////////////////////////////////////////////
//
struct ControlPoint
{
	int parent; // 1 - go to sub level
	int	child;  // go to n child

	HWND parentWnd;

	//! a constructor
	ControlPoint(int _parent, int _child, HWND _parentWnd=0)
		: parent(_parent)
		, child(_child)
		, parentWnd(_parentWnd)
	{}
};

///////////////////////////////////////////////////////////////////////////////
//
struct ButtonTarget
{
public:
	std::vector<ControlPoint> points;

	HWND	parent;
	HWND	currNode;
	int		index; // children index

	int checkpoint; // how much points did we past

	HWND	WND;	// we search this value

	//! a constructor
	ButtonTarget();

	void Clear(HWND parentwnd=0);

	bool TryToFind(HWND hwnd, HWND nodeParentWnd);

	void KeyDown();
};

////////////////////////////////////////////////////////////////////////////////
//
class ShortCutEmulator
{
public:
	
	ButtonTarget	ButtonBezier; // switch to bezier mode (5th button)
	ButtonTarget	ButtonCon; // switch to con
	ButtonTarget	ButtonAuto; // switch to auto bezier mode (3rd button)
	ButtonTarget	ButtonFlat; // press flat tangent mode

	ButtonTarget	ButtonDiscLeft; // set tangent discontinues left
	ButtonTarget	ButtonDiscRight; // set right (12th button)

	ButtonTarget	ButtonWt; // weight button (5th button)

	//bool Searching;

	void InitButtonAuto(HWND FCurvesWnd);
	void InitButtonFlat(HWND FCurvesWnd);
	void InitButtonBezier(HWND FCurvesWnd);
	void InitButtonDiscLeft(HWND FCurvesWnd);
	void InitButtonDiscRight(HWND FCurvesWnd);
	void InitButtonWt(HWND FCurvesWnd);

	bool Init();

	void KeyDown(WPARAM wParam, LPARAM lParam);
	void KeyChar(WPARAM wParam, LPARAM lParam);

};

/////////////////////////////////////////////////////////////////////////////////////////////

// Subclass procedure 
LRESULT APIENTRY FCurveSubclassProc(
    HWND hwnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam);

// try to find fcurve editor and buttons
BOOL CALLBACK FCurve_EnumProc( HWND hwnd, LPARAM lParam );