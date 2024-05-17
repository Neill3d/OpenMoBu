#ifndef __ORSHADERTEXMAT_LAYOUT_H__
#define __ORSHADERTEXMAT_LAYOUT_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: colors_layout.h
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/OpenMoBu
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "colors_renderer.h"

//! Renderer Colors device layout.
class ORCustomRendererCallbackLayout : public FBRendererCallbackLayout
{
	//--- FiLMBOX declaration.
	FBRendererCallbackLayoutDeclare( ORCustomRendererCallbackLayout, FBRendererCallbackLayout );

public:
	//--- FiLMBOX Creation/Destruction.
	virtual bool FBCreate();			//!< FiLMBOX constructor.
	virtual void FBDestroy();			//!< FiLMBOX destructor.

private:
	// UI Management
	void	UICreate	();
	void	UIConfigure	();
	void	UIReset		();

	// UI Callbacks
	void	OnButtonTestClick(HISender pSender, HKEvent pEvent);

private:
	// UI Elements
	FBButton mButtonTest;

};

#endif /* __ORSHADERTEXMAT_LAYOUT_H__ */
