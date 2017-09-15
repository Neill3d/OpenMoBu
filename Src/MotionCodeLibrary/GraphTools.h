
/**	\file	GraphTools.h
*	Graph Editor manipulators, commands, etc.
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

#include "GraphView.h"

//! plugin identify
#define	MANIPULATOR_GRAPH_SLIDER		"Slider"
#define	MANIPULATOR_GRAPH_PAN			"Pan"
#define	MANIPULATOR_GRAPH_ZOOM			"Zoom"

////////////////////////////////////////////////////////////
//! ManipGraphSlider
/*!
	global manipulator

	* display time slider
	* draw data text and grid
	* moving time slider by mouse dragging input action
*/
class ManipGraphSlider	: BaseGraphManipulator
{
public:
	//! a constructor
	ManipGraphSlider(GraphView	*_pGraph, bool _Modal=false)
		: BaseGraphManipulator(_pGraph, _Modal)
	{}
	//! return manipulator caption for the viewport
	char	*GetCaption()
	{ return MANIPULATOR_GRAPH_SLIDER; }
	//! mouse down processing
	void		MouseDown(int x, int y, int modifier);
	//! mouse move processing
	void		MouseMove(int x, int y, int modifier);
	//! mouse up processing
	void		MouseUp(int x, int y, int modifier);
	//! output 2d drawing
	void		PreDraw();
	void		PostDraw();

private:
	FBTime				mLastTime;
	FBTimeSpan		mLastTimeSpan;
	FBVector2d		mLastValueSpan;

	int			timeDuration;
	double	timeLargeStep;
	double	timeSmallStep;

	double	valueDuration;
	double	valueLargeStep;
	double	valueSmallStep;
};

////////////////////////////////////////////////////////////
//! ManipGraphPan
/*!
	local manipulator
	
	moving along graph
*/
class ManipGraphPan	:	BaseGraphManipulator
{
public:
	//! a constructor
	ManipGraphPan(GraphView	*_pGraph, bool _Modal=false)
			: BaseGraphManipulator(_pGraph, _Modal)
	{}
	//! return manipulator caption for the viewport
	char	*GetCaption()
	{ return MANIPULATOR_GRAPH_PAN; }
	//! mouse down processing
	void		MouseDown(int x, int y, int modifier);
	//! mouse move processing
	void		MouseMove(int x, int y, int modifier);
	//! mouse up processing
	void		MouseUp(int x, int y, int modifier);
	//! output 2d drawing
	void		PreDraw()
	{}
	void		PostDraw()
	{}
};

////////////////////////////////////////////////////////////
//! ManipGraphZoom
/*!
	local manipulator

	Zoom zoom zoom...
*/
class ManipGraphZoom	:	BaseGraphManipulator
{
public:
	//! a constructor
	ManipGraphZoom(GraphView	*_pGraph, bool _Modal=false)
			: BaseGraphManipulator(_pGraph, _Modal)
	{}
	//! return manipulator caption for the viewport
	char	*GetCaption()
	{ return MANIPULATOR_GRAPH_ZOOM; }
	//! mouse down processing
	void		MouseDown(int x, int y, int modifier);
	//! mouse move processing
	void		MouseMove(int x, int y, int modifier);
	//! mouse up processing
	void		MouseUp(int x, int y, int modifier);
	//! output 2d drawing
	void		PreDraw()
	{}
	void		PostDraw()
	{}
private:
	double	wx;
	double	wy;
	int		mZoomCenterX;
	int		mZoomCenterY;
	FBTimeSpan		mTimeSpan;
	FBVector2d		mValueSpan;
};

////////////////////////////////////////////////////////////
//! GraphFitCommand
/*!
	fitting graph view to the current graph

	change timespan, valuespan
*/
class	GraphFitCommand : public BaseGraphCommand
{
public:
	virtual char *GetCaption()
	{
		return "GraphFit";
	}
	virtual bool	CheckInput(int pMouseX, int pMouseY, int Key, int Modifier)
	{
		if ( (Key == 'F') || (Key == 'f' ) )
			return true;
		return false;
	}
	virtual void DoIt(GraphView	*pGraph)
	{
		FBTimeSpan	&lTimeSpan = pGraph->GetTimeSpan();
		FBVector2d	&lValueSpan = pGraph->GetValueSpan();

		lTimeSpan.Set( FBTime(0,0,0, (int)pGraph->mTimeRange[0]), FBTime(0,0,0, (int)pGraph->mTimeRange[1]) );
		lValueSpan = pGraph->mValueRange;
	}
};