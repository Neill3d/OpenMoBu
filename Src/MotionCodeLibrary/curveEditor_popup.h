#ifndef __CURVE_EDITOR_POPUP_H__
#define __CURVE_EDITOR_POPUP_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: curveEditor_popup.h
//
//	Author Sergei Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK Include
#include <fbsdk/fbsdk.h>

#include "Delegate.h"
#include "ParticlesDrawHelper.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! Popup with Curve Editor.
class ORPopup_CurveEditor : public FBPopup
{
	FBClassDeclare( ORPopup_CurveEditor, FBPopup );
public:
	ORPopup_CurveEditor();
public:
	// UI Management
	void	UICreate	();
	void	UIConfigure	();

	// UI Callbacks
	void	EventButtonResetClick			( HISender pSender, HKEvent pEvent );
	void	EventButtonPreviewClick			( HISender pSender, HKEvent pEvent );
	void	EventButtonCloseClick			( HISender pSender, HKEvent pEvent );

public:
	
	void SetUp( FBAnimationNode *node, FBPropertyAnimatable *pProp );

	void Clear();
	void Preview();

	Gallant::Delegate1<double>		OnPreviewChange;

private:

	FBFCurveEditor		mCurveEditor;		// edit variable value in time here

	FBButton	mButtonReset;
	FBButton	mButtonPreview;		// update missing files according to the missing path list
	FBButton	mButtonClose;		// button to close popup

	FBAnimationNode		*mInputNode;	// node source
	FBAnimationNode		*mNode;		// use this node for editing
};


///////////////////////////////////////////////////////////////////////////////////////////////
//

enum OperationMode
{
	OperationSelect,
	OperationMove,
	OperationAdd,
	OperationRemove
};

////////////////////////////////////////////////////////////////////
//

class ColorView : public FBView
{
public:
	//! a constructor
	ColorView();
	//! a destructor
	~ColorView();

	/**	Set view's viewport
    *	\param	pX	Viewport X value.
    *	\param	pY	Viewport Y value.
    *	\param	pW	Viewport W (width) value.
    *	\param	pH	Viewport H (height) value.
    *	\return	Operation was successful (\b true or \b false).
    */
    virtual bool SetViewport(int pX,int pY,int pW,int pH) override;

	//! Exposed view callback function.
    virtual void ViewExpose() override;

    /** Input callback function.
    *	\param pMouseX		Mouse X position.
    *	\param pMouseY		Mouse Y position.
    *	\param pAction		Mouse action.
    *	\param pButtonKey	Keyboard input.
    *	\param pModifier	Keyboard input modifier.
    */
    virtual void ViewInput(int pMouseX,int pMouseY,FBInputType pAction,int pButtonKey,int pModifier) override;

	void		SetUp( FBPopup *pPopup, FBPropertyAnimatableColorAndAlpha *prop );

	void SetOperationMode (const OperationMode mode)
	{
		mOperationMode = mode;
	}
	const OperationMode GetOperationMode()
	{
		return mOperationMode;
	}

	void SetColor(const FBColorAndAlpha color);
	void SetColor(const FBColor color, const double alpha);

private:
	
	int						mSelectedMarker{ -1 };

	ColorPropertyTexture	mTexture;
	bool					mNeedUpdate{ true };

	bool					mDown{ false };
	OperationMode			mOperationMode{ OperationSelect };

	FBPopup* mPopup{ nullptr };

	int						mWidth{ 0 };
	int						mHeight{ 0 };
};


///////////////////////////////////////////////////////////////////////////////////
// ORPopup_ColorEditor

class ORPopup_ColorEditor : public FBPopup
{
	FBClassDeclare( ORPopup_ColorEditor, FBPopup );
public:
	ORPopup_ColorEditor();
public:
	// UI Management
	void	UICreate	();
	void	UIConfigure	();

	// UI Callbacks
	void	EventButtonSelectClick			( HISender pSender, HKEvent pEvent );
	void	EventButtonMoveClick			( HISender pSender, HKEvent pEvent );
	void	EventButtonAddClick			( HISender pSender, HKEvent pEvent );
	void	EventButtonRemoveClick			( HISender pSender, HKEvent pEvent );
	void	EventEditColorChange			( HISender pSender, HKEvent pEvent );
	void	EventButtonClearClick			( HISender pSender, HKEvent pEvent );

	void	EventButtonPreviewClick			( HISender pSender, HKEvent pEvent );
	void	EventButtonCloseClick			( HISender pSender, HKEvent pEvent );

public:
	
	void SetUp( FBShader *shader, FBPropertyAnimatableColorAndAlpha *pProp );

	void Clear();
	void Preview();

	void SetColorForEdit(const FBColorAndAlpha color);

private:

	ColorView	mColorView;		// edit variable value in time here

	FBButton		mButtonSelect;
	FBButton		mButtonMove;
	FBButton		mButtonAdd;		// add color mode
	FBButton		mButtonRemove;	// remove color mode
	FBButton		mButtonClear;
	FBEditColor		mEditColor;
	FBEditNumber	mEditAlpha;

	FBButton	mButtonPreview;		// update missing files according to the missing path list
	FBButton	mButtonClose;		// button to close popup

	FBShader							*mShader;		// connected shader (for making a preview operation)
	FBPropertyAnimatableColorAndAlpha	*mColorProperty;

	void SetOperationButtonState();
};

#endif	/* __CURVE_EDITOR_POPUP_H__ */
