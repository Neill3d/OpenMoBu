
/**	\file	graphview_tool.cxx
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


//--- Class declaration
#include "graphview_tool.h"
#include <fbsdk-opengl.h>

//--- Registration defines
#define ORTOOLGRAPHVIEW__CLASS	ORTOOLGRAPHVIEW__CLASSNAME
#define ORTOOLGRAPHVIEW__LABEL	"GraphView"
#define ORTOOLGRAPHVIEW__DESC	"GraphView - neill3d"

//--- FiLMBOX implementation and registration
FBToolImplementation(	ORTOOLGRAPHVIEW__CLASS	);
FBRegisterTool		(	ORTOOLGRAPHVIEW__CLASS,
						ORTOOLGRAPHVIEW__LABEL,
						ORTOOLGRAPHVIEW__DESC,
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)


void MyGraphView::ViewExpose()
{
	// graphview predraw
	BeginViewExpose();
	
	FBTimeSpan		&lTimeSpan = GetTimeSpan();
	FBVector2d		&lValueSpan = GetValueSpan();

	glPushMatrix();
	glTranslatef( -lTimeSpan.GetStart().GetFrame(), -lValueSpan[0], 0.0 );

	// display channels of the selected nodes
	if (mGraphModel)
	{
		FBAnimationNode	*lAnimNode = mGraphModel->AnimationNode;
		FBAnimationNode	*lTransNode = lAnimNode->Nodes.Find("Lcl Translation");

		// output x translation curve
		glColor3f( 1.0f, 0.0f, 0.0f );
		FBAnimationNode	*lXNode	= lTransNode->Nodes.Find("X");
		if (lXNode->FCurve)
		{
			FBFCurve	*lCurve = lXNode->FCurve;
			glBegin(GL_LINE_STRIP);
			for (int j=0; j<lCurve->Keys.GetCount(); j++)
			{
				FBTime	lTime = lCurve->Keys[j].Time;
				if (lTimeSpan & lTime)
					glVertex2f( lTime.GetFrame(), lCurve->Keys[j].Value );
			}
			glEnd();
		}
	
		// output y translation curve
		glColor3f( 0.0f, 1.0f, 0.0f );
		lXNode	= lTransNode->Nodes.Find("Y");
		if (lXNode->FCurve)
		{
			FBFCurve	*lCurve = lXNode->FCurve;
			glBegin(GL_LINE_STRIP);
			for (int j=0; j<lCurve->Keys.GetCount(); j++)
			{
				FBTime	lTime = lCurve->Keys[j].Time;
				if (lTimeSpan & lTime)
					glVertex2f( lTime.GetFrame(), lCurve->Keys[j].Value );
			}
			glEnd();
		}

		// output z translation curve
		glColor3f( 0.0f, 0.0f, 1.0f );
		lXNode	= lTransNode->Nodes.Find("Z");
		if (lXNode->FCurve)
		{
			FBFCurve	*lCurve = lXNode->FCurve;
			glBegin(GL_LINE_STRIP);
			for (int j=0; j<lCurve->Keys.GetCount(); j++)
			{
				FBTime	lTime = lCurve->Keys[j].Time;
				if (lTimeSpan & lTime)
					glVertex2f( lTime.GetFrame(), lCurve->Keys[j].Value );
			}
			glEnd();
		}
	}

	glPopMatrix();
	
	// graphview postdraw
	EndViewExpose();
}

void MyGraphView::SetModel(FBModel *pModel)
{
	mGraphModel = pModel;
}


/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ORTool_GraphView::FBCreate()
{
	// Tool options
	StartSize[0] = 600;
	StartSize[1] = 300;

    int lB = 10;
	//int lS = 4;
	int lW = 100;
	int lH = 18;

	// Configure layout
	AddRegion( "ButtonTest", "ButtonTest",
										lB,	kFBAttachLeft,	"",	1.0	,
										lB,	kFBAttachTop,	"",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	AddRegion( "GraphView", "GraphView",
										lB,	kFBAttachRight,	"ButtonTest",	1.0	,
										lB,	kFBAttachTop,	"",	1.0,
										-lB,	kFBAttachRight,	"",	1.0,
										-lB,	kFBAttachBottom,	"",	1.0 );
	SetControl( "ButtonTest", mButtonTest );
	SetView( "GraphView", mGraph );

	// Configure button
	mButtonTest.OnClick.Add( this, (FBCallback) &ORTool_GraphView::EventButtonTestClick );
	mButtonTest.Caption = "Create Model";

	// Add tool callbacks
	OnShow.Add	( this, (FBCallback) &ORTool_GraphView::EventToolShow		);
	OnIdle.Add	( this, (FBCallback) &ORTool_GraphView::EventToolIdle		);
	OnResize.Add( this, (FBCallback) &ORTool_GraphView::EventToolResize	);
	OnPaint.Add	( this, (FBCallback) &ORTool_GraphView::EventToolPaint		);
	OnInput.Add	( this, (FBCallback) &ORTool_GraphView::EventToolInput		);

	return true;
}


/************************************************
 *	FiLMBOX Destruction function.
 ************************************************/
void ORTool_GraphView::FBDestroy()
{
	// Remove tool callbacks
	OnShow.Remove	( this, (FBCallback) &ORTool_GraphView::EventToolShow	);
	OnIdle.Remove	( this, (FBCallback) &ORTool_GraphView::EventToolIdle	);
	OnPaint.Remove	( this, (FBCallback) &ORTool_GraphView::EventToolPaint );
	OnInput.Remove	( this, (FBCallback) &ORTool_GraphView::EventToolInput );
	OnResize.Remove	( this, (FBCallback) &ORTool_GraphView::EventToolResize);

	// Free user allocated memory
}


/************************************************
 *	Button click callback.
 ************************************************/
void ORTool_GraphView::EventButtonTestClick( HISender pSender, HKEvent pEvent )
{
	FBModel	*lModel = nullptr;

	// Create a new marker
	lModel = new FBModelPlane("Tool_Template Model");
	lModel->Show = true;
			
	FBAnimationNode	*pNode;
	FBTime	t1, t2;
	t1 = FBTime(0,0,0,0);
	t2 = FBTime(0,0,0,30);

	// create animation nodes and fcurves for a translation
	lModel->Translation.SetAnimated(true);

	// animate x
	pNode = lModel->Translation.GetAnimationNode()->Nodes[0];
	if (!pNode->FCurve)
	{
		pNode->FCurve = new FBFCurve();
	}
	pNode->FCurve->KeyAdd( t1, 0.0 );
	pNode->FCurve->KeyAdd( t2, 5.0 );

	// animate y
	pNode = lModel->Translation.GetAnimationNode()->Nodes[1];
	if (!pNode->FCurve)
	{
		pNode->FCurve = new FBFCurve();
	}
	pNode->FCurve->KeyAdd( t1, 2.0 );
	pNode->FCurve->KeyAdd( t2, 7.0 );

	// animate z
	pNode = lModel->Translation.GetAnimationNode()->Nodes[2];
	if (!pNode->FCurve)
	{
		pNode->FCurve = new FBFCurve();
	}
	pNode->FCurve->KeyAdd( t1, 7.0 );
	pNode->FCurve->KeyAdd( t2, 1.0 );

	// assign model to a graphview
	mGraph.SetModel( lModel );
}


/************************************************
 *	UI Idle callback.
 ************************************************/
void ORTool_GraphView::EventToolIdle( HISender pSender, HKEvent pEvent )
{
	mGraph.Refresh(true);
}


/************************************************
 *	Handle tool activation (selection/unselection).
 ************************************************/
void ORTool_GraphView::EventToolShow( HISender pSender, HKEvent pEvent )
{
	FBEventShow lEvent( pEvent );

	if( lEvent.Shown )
	{
		// Reset the UI here.
	}
	else
	{
	}
}


/************************************************
 *	Paint callback for tool (on expose).
 ************************************************/
void ORTool_GraphView::EventToolPaint( HISender pSender, HKEvent pEvent )
{
}


/************************************************
 *	Tool resize callback.
 ************************************************/
void ORTool_GraphView::EventToolResize( HISender pSender, HKEvent pEvent )
{
	int x,y, w,h;
	GetRegionPositions("GraphView", true, &x, &y, &w, &h);
	mGraph.ViewReSize(w,h);
}


/************************************************
 *	Handle input into the tool.
 ************************************************/
void ORTool_GraphView::EventToolInput( HISender pSender, HKEvent pEvent )
{
}


