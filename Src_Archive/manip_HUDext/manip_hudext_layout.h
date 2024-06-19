#ifndef __MANIP_HUDEXT_LAYOUT_H__
#define __MANIP_HUDEXT_LAYOUT_H__

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


//--- Class declaration
#include "manip_hudext_manip.h"

//! Simple constraint layout.
class Manip_HUDext_Layout : public FBManipulatorLayout
{
	//--- FiLMBOX ceclaration.
	FBManipulatorLayoutDeclare( Manip_HUDext_Layout, FBManipulatorLayout );

public:
	//--- FiLMBOX Constructor/Destructor
	virtual bool FBCreate();				//! FiLMBOX constructor.
	virtual void FBDestroy();				//! FiLMBOX destructor.

	// UI Management
	void	UICreate	();
	void	UIConfigure	();
	void	UIReset		();

	// UI Callbacks
	void	EventUIIdle						( HISender pSender, HKEvent pEvent );

private:
	FBLabel			mLabelAbout;

private:
	Manip_HUDext*		mManipulator;			//!< Handle onto manipulator.
};


#endif	/* __MANIP_HUDEXT_LAYOUT_H__ */
