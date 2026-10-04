#ifndef __ORMANIP_TEMPLATE_MANIP_H__
#define __ORMANIP_TEMPLATE_MANIP_H__

/**	\file	ormanip_template_manip.h

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//#include "WGLFONT.h"

//--- Registration defines
#define ORMANIPTEMPLATE__CLASSNAME	Manip_PostProcessing
#define ORMANIPTEMPLATE__CLASSSTR	"Manip_PostProcessing"

///////////////////////////////////////////////////////////////////////////////
//! PostProcessing Manipulator

class Manip_PostProcessing : public FBManipulator
{
	//--- FiLMBOX declaration.
	FBManipulatorDeclare( Manip_PostProcessing, FBManipulator );

public:
	//! FiLMBOX Constructor.
	virtual bool FBCreate();

	//!< FiLMBOX Destructor.
	virtual void FBDestroy();

	//!	Deal with manipulator input
	virtual bool ViewInput(int pMouseX, int pMouseY, FBInputType pAction, int pButtonKey,int pModifier);

	//! Manipulator expose function
	virtual void ViewExpose();

protected:
	
	bool				m_firstRun{ true };
	bool				m_showGraph{ false };
	FBSystem			mSystem;
	
	void ShowGraph();
	void HideGraph();

};

#endif /* __ORMANIP_TEMPLATE_MANIP_H__ */
