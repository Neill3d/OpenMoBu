
/**	\file	GraphView.cpp
*	FCurve editor
*/

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

#include <math.h>

#include "GraphView.h"
#include "GraphTools.h"
#include <fbsdk-opengl.h>

FBClassImplementation( GraphView );

void Push2dViewport(int g_w, int g_h)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();

	glLoadIdentity();
	gluOrtho2D(0, g_w, 0, g_h);


	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();

	glLoadIdentity();

}

void Pop2dViewport()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
}

//////////////////////////////////////////////////////////////////
// GraphManager

//! a constructor
GraphManager::GraphManager(GraphView	*Graph)
	: pGraph(Graph)
{
	mManip = NULL;
}
//! a destructor
GraphManager::~GraphManager()
{
	ClearManipulator();
	ClearGlobalManip();
	ClearCommands();
}
//! add new global manipulator to a graph
void GraphManager::AddGlobal(const char *szManipName)
{
	if (strstr(szManipName, MANIPULATOR_GRAPH_SLIDER) != NULL)
	{
		BaseGraphManipulator	*pNewManip = (BaseGraphManipulator*)new ManipGraphSlider(pGraph);
		mGlobalManip.Add( pNewManip );
	}
}
//! clear all global manips from a graph
void GraphManager::ClearGlobalManip()
{
	for (int i=0; i<mGlobalManip.GetCount(); i++)
		if (mGlobalManip[i])
		{
			delete mGlobalManip[i];
			mGlobalManip[i] = NULL;
		}
	mGlobalManip.Clear();
}
//! set current local manipulator as current
void GraphManager::SetManipulator(const char *szManipName)
{
	ClearManipulator();

	// select new manipulator
	if ( strstr(szManipName, MANIPULATOR_GRAPH_PAN) != NULL )
	{
		mManip = (BaseGraphManipulator*)new ManipGraphPan(pGraph);
	}
	else
	if ( strstr(szManipName, MANIPULATOR_GRAPH_ZOOM) != NULL )
	{
		mManip = (BaseGraphManipulator*)new ManipGraphZoom(pGraph);
	}
}
//! clear local manipulator
void GraphManager::ClearManipulator()
{
	if (mManip)
	{
		delete mManip;
		mManip = NULL;
	}
}

void GraphManager::AddCommand(const char *szCommandName)
{
	BaseGraphCommand	*command = NULL;
	if (strstr(szCommandName, "GraphFit") != NULL)
		command = new GraphFitCommand();

	if (command)
		mCommands.Add(command);
}

void GraphManager::ClearCommands()
{
	for (int i=0; i<mCommands.GetCount(); i++)
		if (mCommands[i])
		{
			delete mCommands[i];
			mCommands[i] = NULL;
		}
	mCommands.Clear();
}

//! mouse down processing
void		GraphManager::MouseDown(int x, int y, int modifier)
{
	bool lModal = false;
	for (int i=0; i<mGlobalManip.GetCount(); i++)
	{
		if (mGlobalManip[i])
		{
			mGlobalManip[i]->MouseDown(x,y, modifier);
			if (mGlobalManip[i]->IsModal())
				lModal = true;
		}
	}
	if (mManip && !lModal)
		mManip->MouseDown(x, y, modifier);
}
//! mouse move processing
void		GraphManager::MouseMove(int x, int y, int modifier)
{
	bool lModal = false;
	for (int i=0; i<mGlobalManip.GetCount(); i++)
	{
		if (mGlobalManip[i])
		{
			mGlobalManip[i]->MouseMove(x,y, modifier);
			if (mGlobalManip[i]->IsModal())
				lModal = true;
		}
	}
	if (mManip && !lModal)
		mManip->MouseMove(x, y, modifier);
}
//! mouse up processing
void		GraphManager::MouseUp(int x, int y, int modifier)
{
	bool lModal = false;
	for (int i=0; i<mGlobalManip.GetCount(); i++)
	{
		if (mGlobalManip[i])
		{
			mGlobalManip[i]->MouseUp(x,y, modifier);
			if (mGlobalManip[i]->IsModal())
				lModal = true;
		}
	}
	if (mManip && !lModal)
		mManip->MouseUp(x, y, modifier);
}

void		GraphManager::UserInput(int x, int y, int key, int modifier)
{
	for (int i=0; i<mCommands.GetCount(); i++)
	{
		if (mCommands[i])
			if (mCommands[i]->CheckInput(x,y,key,modifier) )
				mCommands[i]->DoIt(pGraph);
	}
}

//! output 2d drawing
void		GraphManager::PreDraw()
{
	// process all global manipulators
	for (int i=0; i<mGlobalManip.GetCount(); i++)
	{
		if (mGlobalManip[i])
			mGlobalManip[i]->PreDraw();
	}
	// process current local manipulator
	if (mManip) {
		mManip->PreDraw();
	}
}

void		GraphManager::PostDraw()
{
	// process all global manipulators
	for (int i=0; i<mGlobalManip.GetCount(); i++)
	{
		if (mGlobalManip[i])
			mGlobalManip[i]->PostDraw();
	}
	// process current local manipulator
	if (mManip) {
		mManip->PostDraw();
	}
}


//////////////////////////////////////////////////////////////////////////////////////
// GraphView


GraphView::GraphView()
	: FBView()
	, GraphManager(this)
{
	mDown=false;

	mW = 320;
	mH = 240;

	mClearColor = FBVector4d(0.5, 0.5, 0.5, 1.0);

	// startup time range (X)
	FBPlayerControl		lPlayer;
	mTimeSpan.Set( lPlayer.ZoomWindowStart, lPlayer.ZoomWindowStop );
	// startup value range (Y)
	mValueSpan = FBVector2d( -100.0, 100.0 );

	// min & max
	mTimeRange = FBVector2d( (double)mTimeSpan.GetStart().GetFrame(), (double)mTimeSpan.GetStop().GetFrame() );
	mValueRange = mValueSpan;

	AddGlobal( MANIPULATOR_GRAPH_SLIDER );
	AddCommand( "GraphFit" );
}
//! Refresh callback.
void GraphView::Refresh(bool pNow)
{
	FBView::Refresh(pNow);
}

void GraphView::ViewReSize(int w, int h)
{
	mW = w;
	mH = h;
}

int GraphView::CalcWidth()
{
	return mTimeSpan.GetDuration().GetFrame();
}
int GraphView::CalcHeight()
{
	return (int) (mValueSpan[1] - mValueSpan[0]);
}
/*
double	GraphView::CalcXSens() {
	return mTimeSpan.GetDuration().GetSecondDouble() / mW;
}
double	GraphView::CalcYSens() {
	return (mValueSpan[1] - mValueSpan[0]) / mH;
}

// transfer from cursor pos to world graph pos
void GraphView::ScreenToWorld( int x, int y, double &wx, double &wy )
{
	wx = mTimeSpan.GetStart().GetSecondDouble() + mTimeSpan.GetDuration().GetSecondDouble() * x / mW;
	wy = mValueSpan[0] + (mValueSpan[1] - mValueSpan[0]) * y / mH;
}
*/
//! Expose callback.
void GraphView::BeginViewExpose(bool predraw)
{
	glViewport( 0, 0, mW, mH );

	glDisable(GL_DEPTH_TEST);
	glClearColor( mClearColor[0], mClearColor[1], mClearColor[2], mClearColor[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	glLineWidth(1.0);

	int lWidth = CalcWidth();
	int lHeight = CalcHeight();

	Push2dViewport(lWidth, lHeight);

	// call graph manager proc
	if (predraw)	PreDraw();
}

void GraphView::EndViewExpose()
{
	// call graph manager proc
	PostDraw();

	Pop2dViewport();
}

void GraphView::ViewExpose()
{
	BeginViewExpose();
	/*
	glPushMatrix();
	glTranslatef( -mTimeSpan.GetStart().GetFrame(true), -mValueSpan[0], 0.0 );

	// display channels of the selected nodes
	FBModelList		lList;
	FBGetSelectedModels(lList);
	for (int i=0; i<lList.GetCount(); i++)
	{
		HFBModel	pModel = lList[i];
		HFBAnimationNode	lAnimNode = pModel->AnimationNode;
		HFBAnimationNode	lTransNode = lAnimNode->Nodes.Find("Lcl Translation");

		HFBAnimationNode	lXNode	= lTransNode->Nodes.Find("X");
		if (lXNode->FCurve)
		{
			HFBFCurve	lCurve = lXNode->FCurve;
			glBegin(GL_LINE_STRIP);
			for (int j=0; j<lCurve->Keys.GetCount(); j++)
			{
				FBTime	lTime = lCurve->Keys[j].Time;
				if (mTimeSpan & lTime)
					glVertex2f( lTime.GetFrame(true), lCurve->Keys[j].Value );
			}
			glEnd();
		}
	
		
	}

	glPopMatrix();
	*/
	EndViewExpose();
}
//! input callback.
void GraphView::ViewInput(int pMouseX,int pMouseY,FBInputType pAction,int pButtonKey,int pModifier)
{
	switch(pAction)
	{
	case kFBKeyPress:
		{
			UserInput(pMouseX, pMouseY, pButtonKey, pModifier);
		}
		break;

	case kFBButtonPress:
		{
		// register mouse down in viewport
		mDown = true;
		mLastX = pMouseX;
		mLastY = pMouseY;

		if (pModifier == kFBKeyAlt)
			if (pButtonKey == 2)	// middle button click
				SetManipulator( MANIPULATOR_GRAPH_PAN );
			else if (pButtonKey == 3)	// right button click
				SetManipulator( MANIPULATOR_GRAPH_ZOOM );


		// do some action below
		MouseDown(pMouseX, pMouseY, pModifier);
		} break;
	case kFBButtonRelease:
		// do some action below
		MouseUp(pMouseX, pMouseY, pModifier);
		// release button
		mDown = false;

		ClearManipulator();
		break;
	case kFBMotionNotify:
		// do some action
		MouseMove(pMouseX, pMouseY, pModifier);
		
		// lastx, lasty
		mLastX = pMouseX;
		mLastY = pMouseY;
		break;
	}
}