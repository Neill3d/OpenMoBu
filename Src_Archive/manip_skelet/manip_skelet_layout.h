#ifndef __MANIP_SKELET_LAYOUT_H__
#define __MANIP_SKELET_LAYOUT_H__


/**	\file	manip_skelet_layout.h
*	Declaration for the layout class of a simple manipulator.
*	Declaration for the layout class FBSimpleManipulatorLayout which
*	is the layout for a simple manipulator example.
*/

//--- Class declaration
#include "manip_skelet_manip.h"

//! Simple constraint layout.
class Manip_Skelet_Layout : public FBManipulatorLayout
{
	//--- FiLMBOX ceclaration.
	FBManipulatorLayoutDeclare( Manip_Skelet_Layout, FBManipulatorLayout );

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
	void	EventButtonTestClick			( HISender pSender, HKEvent pEvent );
	void	EventButtonModelSpaceClick		( HISender pSender, HKEvent pEvent );
	void	EventButtonDrawSquareClick		( HISender pSender, HKEvent pEvent );
	void	EventButtonDefaultBehaviorClick	( HISender pSender, HKEvent pEvent );
	void	EventButtonAlwaysActiveClick	( HISender pSender, HKEvent pEvent );

private:
	FBButton			mButtonTest;
	FBButton			mButtonModelSpace;
	FBButton			mButtonDrawSquare;
	FBButton			mButtonDefaultBehavior;
	FBButton			mButtonAlwaysActive;

private:
	FBSystem			mSystem;
	Manip_Skelet*	mManipulator;			//!< Handle onto manipulator.
};


#endif	/* __MANIP_SKELET_LAYOUT_H__ */
