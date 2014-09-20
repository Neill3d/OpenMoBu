#ifndef __ORTOOL_GRAPHVIEW_TOOL_H__
#define __ORTOOL_GRAPHVIEW_TOOL_H__

/**	\file	graphview_tool.h
*	Simple tool to demo a graphview visual component.
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


//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Motion Code Library include
#include "GraphView.h"

//--- Registration define
#define ORTOOLGRAPHVIEW__CLASSNAME	ORTool_GraphView
#define ORTOOLGRAPHVIEW__CLASSSTR	"ORTool_GraphView"

// class of a demo graph
class MyGraphView	: public GraphView
{
public:
	//! a constructor
	MyGraphView()
		: GraphView()
	{
		mGraphModel = NULL;
		SetClearColor( FBVector4d(0.2, 0.2, 0.2, 1.0) );
	}

	//! override graph drawing method
	void	ViewExpose();

	//! set model to analize in graph view
	void	SetModel(FBModel	*pModel=nullptr);

private:
	FBModel		*mGraphModel;	//!> model to analyze in graph view
};


/**	Sample Tool.
*/
class ORTool_GraphView : public FBTool
{
	//--- FiLMBOX Tool declaration.
	FBToolDeclare( ORTool_GraphView, FBTool );

public:
	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

private:
	void		EventButtonTestClick( HISender pSender, HKEvent pEvent );
	void		EventToolIdle		( HISender pSender, HKEvent pEvent );
	void		EventToolShow		( HISender pSender, HKEvent pEvent );
	void		EventToolPaint		( HISender pSender, HKEvent pEvent );
	void		EventToolResize		( HISender pSender, HKEvent pEvent );
	void		EventToolInput		( HISender pSender, HKEvent pEvent );

private:
	FBButton	mButtonTest;
	MyGraphView	mGraph;
};

#endif /* __ORTOOL_GRAPHVIEW_TOOL_H__ */
