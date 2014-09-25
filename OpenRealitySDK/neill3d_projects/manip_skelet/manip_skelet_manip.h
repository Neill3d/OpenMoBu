#ifndef __MANIP_SKELET_MANIP_H__
#define __MANIP_SKELET_MANIP_H__


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


//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration defines
#define MANIPSKELET__CLASSNAME	Manip_Skelet
#define MANIPSKELET__CLASSSTR		"Manip_Skelet"

//! Manipulator template
class Manip_Skelet : public FBManipulator
{
	//--- FiLMBOX declaration.
	FBManipulatorDeclare( Manip_Skelet, FBManipulator );

public:
	//! FiLMBOX Constructor.
	virtual bool FBCreate();

	//!< FiLMBOX Destructor.
	virtual void FBDestroy();

	//!	Deal with manipulator input
	virtual bool ViewInput(int pMouseX, int pMouseY, FBInputType pAction, int pButtonKey,int pModifier);

	//! Manipulator expose function
	virtual void ViewExpose();

public:

	bool			pivotMode;
	FBModel			*pivotModel;
	FBArrayTemplate <FBMatrix>	objsTM;

private:
	FBApplication	mApplication;
	FBSystem		mSystem;
};

#endif /* __MANIP_SKELET_MANIP_H__ */
