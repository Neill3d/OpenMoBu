#ifndef __ORTEXTURE_ADVANCEBLEND_LAYOUT_H__
#define __ORTEXTURE_ADVANCEBLEND_LAYOUT_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ortexture_advanceBlend_layout.h
//
//	Author Sergei Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/OpenMoBu
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "ortexture_advanceBlend_texture.h"

//! Texture Advance Blend device layout.
class ORTextureAdvanceBlendLayout : public FBTextureLayout
{
	//--- FiLMBOX declaration.
	FBTextureLayoutDeclare( ORTextureAdvanceBlendLayout, FBTextureLayout );

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
	ORTextureAdvanceBlend		*mTextureCustom;

private:
	// UI Elements
	FBEditPropertyModern		mEditAuxLayer; 
};

#endif /* __ORTEXTURE_ADVANCEBLEND_LAYOUT_H__ */
