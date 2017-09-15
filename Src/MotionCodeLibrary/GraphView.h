
/**	\file	GraphView.h
*	FCurve editor
*/

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


//--- OR SDK include
#include <fbsdk/fbsdk.h>
//-- MCL include
#include "Types.h"

//! declare classes
class GraphView;

//////////////////////////////////////////////////////
//! BaseGraphManipulator
/*
	base class for a graph manipulator
*/
class BaseGraphManipulator
{
public:
	BaseGraphManipulator(GraphView *_pGraph, bool _Modal=false)
		: pGraph(_pGraph)
		, Modal(_Modal)
	{}

	//! return manipulator caption for the viewport
	virtual char	*GetCaption() = 0;
	//! mouse down processing
	virtual void		MouseDown(int x, int y, int modifier) = 0;
	//! mouse move processing
	virtual void		MouseMove(int x, int y, int modifier) = 0;
	//! mouse up processing
	virtual void		MouseUp(int x, int y, int modifier) = 0;
	//! output 2d drawing
	virtual void		PreDraw() = 0;
	virtual void		PostDraw() = 0;

	GraphView		*GetGraphPtr()	{	return pGraph;	}
	bool		IsModal()	{	return Modal;		}
	void		SetModal(bool modal)	{	Modal = modal; }
private:
	//! graph view pointer
	GraphView		*pGraph;
	//! is graph modal (global manip overlaps locals)
	bool				Modal;
};


//////////////////////////////////////////////////////
//! BaseGraphCommand
/*!
	base class for a graph command
*/
class	BaseGraphCommand
{
public:
	virtual char *GetCaption() = 0;
	virtual bool	CheckInput(int pMouseX, int pMouseY, int Key, int Modifier) = 0;
	virtual void DoIt(GraphView		*pGraph) = 0;
};

//////////////////////////////////////////////////////
//! GraphManager
/*!
	 class for operate with my custom graph manipulators
*/
class GraphManager
{
public:
	//! a constructor
	GraphManager(GraphView	*Graph);
	//! a destructor
	~GraphManager();
	//! add new global manipulator to a graph
	void AddGlobal(const char *szManipName);
	//! clear all global manips from a graph
	void ClearGlobalManip();
	//! set current local manipulator as current
	void SetManipulator(const char *szManipName);
	//! clear local manipulator
	void ClearManipulator();
	//! add new graph command
	void AddCommand(const char *szCommandName);
	void ClearCommands();

	//! mouse down processing
	void		MouseDown(int x, int y, int modifier);
	//! mouse move processing
	void		MouseMove(int x, int y, int modifier);
	//! mouse up processing
	void		MouseUp(int x, int y, int modifier);
	//! user input
	void		UserInput(int x, int y, int key, int modifier);
	//! output 2d drawing
	void		PreDraw();		//	drawing before scene
	void		PostDraw();		//	drawing after

private:
	//! pointer to a graph class
	GraphView					*pGraph;
	//! current local manipulator
	BaseGraphManipulator		*mManip;
	//! list of global manipulators
	FBArrayTemplate<BaseGraphManipulator*>	mGlobalManip;
	//! list of graph commands
	FBArrayTemplate<BaseGraphCommand*>	mCommands;
};

/////////////////////////////////////////////////////////
//! GraphView
/**	
	Graph View class.
*/
class GraphView : public FBView, public	GraphManager
{
	FBClassDeclare( GraphView, FBView );

protected:
	void BeginViewExpose(bool predraw = true);
	void EndViewExpose();

public:
	//! Constructor.
	GraphView();
	
	//! Refresh callback.
	void Refresh(bool pNow=false);
	//! Resize callback.
	void ViewReSize(int w, int h);

	int CalcWidth();
	int CalcHeight();

	double	CalcXSens() {
		return mTimeSpan.GetDuration().GetSecondDouble() / mW;
	}
	double	CalcYSens() {
		return (mValueSpan[1] - mValueSpan[0]) / mH;
	}

	// transfer from cursor pos to world graph pos
	void GraphView::ScreenToWorld( int x, int y, double &wx, double &wy )
	{
		wx = mTimeSpan.GetStart().GetSecondDouble() + mTimeSpan.GetDuration().GetSecondDouble() * x / mW;
		wy = mValueSpan[0] + (mValueSpan[1] - mValueSpan[0]) * y / mH;
	}

	//! Expose callback.
	void ViewExpose();
	//! input callback.
	void ViewInput(int pMouseX,int pMouseY,FBInputType pAction,int pButtonKey,int pModifier);

	//! X axes - time range
	FBTimeSpan	&GetTimeSpan() {	return mTimeSpan;	}
	//! Y axes - value range
	FBVector2d	&GetValueSpan() {	return mValueSpan;	}
	bool	IsMouseDown() {	return mDown;	}
	int GetLastX() {	return mLastX;	}
	int GetLastY() {	return mLastY;	}
	int GetWidth() {	return mW;	}
	int GetHeight() {	return mH;	}

	void	SetClearColor(FBVector4d	clearcolor)
	{
		mClearColor = clearcolor;
	}

	FBVector2d			mTimeRange;		// min & max of the X axes
	FBVector2d			mValueRange;		// min & max of the Y axes

private:
	int mW, mH;
	FBTimeSpan			mTimeSpan;		//!> x axes - time range
	FBVector2d			mValueSpan;		//!> y axes - value range

	FBVector4d			mClearColor;

	bool				mDown;
	int					mLastX, mLastY;
};

