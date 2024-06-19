
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: curveEditor_Popup.cpp
//
//	Author Sergei Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declarations
#include "curveEditor_popup.h"

#include <Windows.h>
#include <tchar.h>
#include <stdio.h>

#include "StringUtils.h"

#include "OGL_Utils.h"

FBClassImplementation( ORPopup_CurveEditor );
FBClassImplementation( ORPopup_ColorEditor );

/************************************************
 *	Popup constructor.
 ************************************************/
ORPopup_CurveEditor::ORPopup_CurveEditor()
{
	UICreate	();
	UIConfigure	();

	mNode = nullptr;
	mInputNode = nullptr;
}

/************************************************
 *	UI Creation function.
 *	Create UI regions and assign them to UI elements
 ************************************************/
void ORPopup_CurveEditor::UICreate()
{
	int lB = 4;
	int lH = 25;
	int lOperationArea = 120;

	AddRegion(	"CurveEditor",	"CurveEditor",
											lB,		kFBAttachLeft,	"",		1.0,
											lB,		kFBAttachTop,	"",		1.0,
											-lOperationArea,	kFBAttachRight,	"",		1.0,
											-lB,	kFBAttachBottom,"",		1.0);

	AddRegion(	"ButtonReset",	"ButtonReset",
											lB,		kFBAttachRight,	"CurveEditor",		1.0,
											lB,		kFBAttachNone,	"",		1.0,
											-lB,	kFBAttachRight,	"",		1.0,
											lH,		kFBAttachNone,	"",		1.0);

	AddRegion(	"ButtonPreview",	"ButtonPreview",
											lB,		kFBAttachRight,	"CurveEditor",		1.0,
											lB,		kFBAttachBottom,"ButtonReset",		1.0,
											-lB,	kFBAttachRight,	"",		1.0,
											lH,		kFBAttachNone,	"",		1.0);

	AddRegion(	"ButtonClose",	"ButtonClose",
											lB,		kFBAttachRight,	"CurveEditor",		1.0,
											lB,		kFBAttachBottom,"ButtonPreview",		1.0,
											-lB,	kFBAttachRight,	"",		1.0,
											lH,		kFBAttachNone,	"",		1.0);

	SetControl( "CurveEditor", mCurveEditor );
	SetControl( "ButtonReset", mButtonReset );
	SetControl( "ButtonPreview", mButtonPreview );
	SetControl(	"ButtonClose",	mButtonClose	);
}

/************************************************
 *	UI Configuration function.
 *	Configure UI elements
 ************************************************/
void ORPopup_CurveEditor::UIConfigure()
{
	Region.X		= 100;
	Region.Y		= 100;
	Region.Width	= 800;
	Region.Height	= 640;

	mCurveEditor.Clear();

	mButtonReset.Caption = "Reset";
	mButtonReset.OnClick.Add( this, (FBCallback) &ORPopup_CurveEditor::EventButtonResetClick );

	mButtonClose.Caption	= "Close";
	mButtonClose.OnClick.Add( this, (FBCallback) &ORPopup_CurveEditor::EventButtonCloseClick );

	mButtonPreview.Caption	= "Preview";
	mButtonPreview.OnClick.Add( this, (FBCallback) &ORPopup_CurveEditor::EventButtonPreviewClick );
}


/************************************************
 *	
 ************************************************/

void ORPopup_CurveEditor::EventButtonResetClick( HISender pSender, HKEvent pEvent )
{
	
}

void ORPopup_CurveEditor::EventButtonPreviewClick( HISender pSender, HKEvent pEvent )
{
	Preview();
}

/************************************************
 *	Close button callback (on click).
 ************************************************/
void ORPopup_CurveEditor::EventButtonCloseClick( HISender pSender, HKEvent pEvent )
{
	Close();
}

void ORPopup_CurveEditor::Clear()
{
	
}

void ORPopup_CurveEditor::Preview()
{
	FBSystem::TheOne().Scene->Evaluate();

	FBApplication::TheOne().FlushEventQueue();
	FBApplication::TheOne().UpdateAllWidgets();
}


void ORPopup_CurveEditor::SetUp(FBAnimationNode *node, FBPropertyAnimatable *pProp )
{
	mInputNode = node;

	mCurveEditor.Clear();
	mCurveEditor.AddProperty( pProp );
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////

ColorView::ColorView()
	: FBView()
{}

ColorView::~ColorView()
{
}

bool ColorView::SetViewport(int pX,int pY,int pW,int pH)
{
	mWidth = pW;
	mHeight = pH;
	return true;
}

void ColorView::SetUp( FBPopup *pPopup, FBPropertyAnimatableColorAndAlpha *prop )
{
	mPopup = pPopup;
	mTexture.SetUp(prop);
}

//! Exposed view callback function.
void ColorView::ViewExpose()
{
	//checkGlError( "start view draw function" );


	if (mNeedUpdate) {
		mTexture.Generate();
		mNeedUpdate = false;
	}

	GLuint lTexId = mTexture.GetTextureId();
	if (lTexId)
	{
		glEnable(GL_TEXTURE_1D);
		glBindTexture(GL_TEXTURE_1D, lTexId);
	}

	//glViewport(0, 0, mWidth, mHeight);
	//PrepareOrthoView(mWidth, mHeight);
	//DrawQuad2d();

	drawOrthoQuad2d(mWidth, mHeight);

	if (lTexId)
	{
		glBindTexture(GL_TEXTURE_1D, 0);
		glDisable(GL_TEXTURE_1D);
	}

	// draw keyframe markers
	FBPropertyAnimatableColorAndAlpha	*lProp = mTexture.GetProperty();

	glColor3f(1.0, 0.0, 0.0);
	FBAnimationNode *pNode = lProp->GetAnimationNode();
	if (pNode && (pNode->Nodes.GetCount() == 4)) 
	{
		FBAnimationNode *rNode = pNode->Nodes[0];

		if (rNode)
		{
			FBFCurve *pFCurve = rNode->FCurve;

			if (pFCurve)
			{
				for (int i=0; i<pFCurve->Keys.GetCount(); ++i)
				{
					if (mSelectedMarker == i) glColor3f(1.0f, 0.0f, 0.0f);
					else glColor3f(0.0f, 1.0f, 0.0f);

					FBTime time = pFCurve->Keys[i].Time;
					float position = time.GetSecondDouble() * mWidth;

					glPushMatrix();
					glTranslatef(position, 0.0f, 0.0f);

					glBegin(GL_TRIANGLES);

					glVertex3f(-10.0, -50.0, 0.0);
					glVertex3f(0.0, 100.0, 0.0);
					glVertex3f(10.0, -50.0, 0.0);

					glEnd();

					glPopMatrix();
				}
			}
		}
	}

//	checkGlError( "end view draw function" );
}

/** Input callback function.
*	\param pMouseX		Mouse X position.
*	\param pMouseY		Mouse Y position.
*	\param pAction		Mouse action.
*	\param pButtonKey	Keyboard input.
*	\param pModifier	Keyboard input modifier.
*/
void ColorView::ViewInput(int pMouseX,int pMouseY,FBInputType pAction,int pButtonKey,int pModifier)
{
	FBPropertyAnimatableColorAndAlpha *lProp = mTexture.GetProperty();

	if (pAction == kFBButtonPress && pButtonKey == 1)
	{
		mDown = false;
		if (mOperationMode == OperationAdd)
		{
			mSelectedMarker = -1;

			FBAnimationNode *pNode = lProp->GetAnimationNode();
			if (pNode && (pNode->Nodes.GetCount() == 4)) 
			{
				FBTime time;
				time.SetSecondDouble( 1.0 / mWidth * pMouseX);

				FBColorAndAlpha color(1.0, 1.0, 1.0, 1.0);

				pNode->Evaluate( color, time );
				pNode->KeyAdd( time, color );

				if (mPopup) ((ORPopup_ColorEditor*) mPopup)->SetColorForEdit(color);
				mNeedUpdate = true;
			}
		}
		else
		{

			mSelectedMarker = -1;
			FBAnimationNode *pNode = lProp->GetAnimationNode();
			if (pNode && (pNode->Nodes.GetCount() == 4)) 
			{
				FBAnimationNode *rNode = pNode->Nodes[0];

				if (rNode)
				{
					FBFCurve *pFCurve = rNode->FCurve;

					if (pFCurve)
					{
						for (int i=0; i<pFCurve->Keys.GetCount(); ++i)
						{
							FBTime time = pFCurve->Keys[i].Time;
							float position = time.GetSecondDouble() * mWidth;

							if (pMouseX > (position-10.0) && pMouseX < (position+10.0) )
							{
								mSelectedMarker = i;
								if (mPopup) {
									FBColorAndAlpha color;
									pNode->Evaluate( color, time );
									((ORPopup_ColorEditor*) mPopup)->SetColorForEdit(color);
								}
								if (mOperationMode == OperationMove) mDown = true;
								else if (mOperationMode == OperationRemove)
								{
									// TODO: remove this keys
									pNode->KeyRemoveAt(time);
									mSelectedMarker = -1;
									mNeedUpdate = true;
								}
								break;
							}
						}
					}
				}
			}
		}

		Refresh();
	}
	else if (pAction == kFBMotionNotify && mDown && (mSelectedMarker >=0) && (mOperationMode == OperationMove) )
	{
		FBAnimationNode *pNode = lProp->GetAnimationNode();
		if (pNode && (pNode->Nodes.GetCount() == 4)) 
		{
			FBAnimationNode *rNode = pNode->Nodes[0];
			FBAnimationNode *gNode = pNode->Nodes[1];
			FBAnimationNode *bNode = pNode->Nodes[2];
			FBAnimationNode *aNode = pNode->Nodes[3];

			if (rNode && bNode && gNode && aNode)
			{
				FBFCurve *rCurve = rNode->FCurve;
				FBFCurve *gCurve = gNode->FCurve;
				FBFCurve *bCurve = bNode->FCurve;
				FBFCurve *aCurve = aNode->FCurve;

				if (rCurve && gCurve && bCurve && aCurve)
				{
					FBTime time;
					time.SetSecondDouble( 1.0 / mWidth * pMouseX);
					rCurve->Keys[mSelectedMarker].Time = time;
					gCurve->Keys[mSelectedMarker].Time = time;
					bCurve->Keys[mSelectedMarker].Time = time;
					aCurve->Keys[mSelectedMarker].Time = time;

					mNeedUpdate = true;
				}
			}
		}
		Refresh();
	}
	else if (pAction == kFBButtonRelease && pButtonKey == 1)
	{
		mDown = false;
	}
}

void ColorView::SetColor(const FBColorAndAlpha color)
{
	if (mSelectedMarker >=0)
	{
		FBPropertyAnimatableColorAndAlpha *lProp = mTexture.GetProperty();
		FBAnimationNode *pNode = lProp->GetAnimationNode();
		if (pNode && (pNode->Nodes.GetCount() == 4)) 
		{
			FBAnimationNode *rNode = pNode->Nodes[0];
			FBAnimationNode *gNode = pNode->Nodes[1];
			FBAnimationNode *bNode = pNode->Nodes[2];
			FBAnimationNode *aNode = pNode->Nodes[3];

			if (rNode && bNode && gNode && aNode)
			{
				FBFCurve *rCurve = rNode->FCurve;
				FBFCurve *gCurve = gNode->FCurve;
				FBFCurve *bCurve = bNode->FCurve;
				FBFCurve *aCurve = aNode->FCurve;

				if (rCurve && gCurve && bCurve && aCurve)
				{
					rCurve->Keys[mSelectedMarker].Value = color[0];
					gCurve->Keys[mSelectedMarker].Value = color[1];
					bCurve->Keys[mSelectedMarker].Value = color[2];
					aCurve->Keys[mSelectedMarker].Value = color[3];

					mNeedUpdate = true;
				}
			}
		}
		Refresh();
	}
}

void ColorView::SetColor(const FBColor color, const double alpha)
{
	SetColor( FBColorAndAlpha( color[0], color[1], color[2], alpha ) );
}

/************************************************
 *	Popup constructor.
 ************************************************/
ORPopup_ColorEditor::ORPopup_ColorEditor()
{
	UICreate	();
	UIConfigure	();

	mShader = nullptr;
	mColorProperty = nullptr;
}

/************************************************
 *	UI Creation function.
 *	Create UI regions and assign them to UI elements
 ************************************************/
void ORPopup_ColorEditor::UICreate()
{
	int lB = 4;
	int lH = 25;
	int lOperationArea = 32;
	int lButtonWidth = 80;

	AddRegion(	"ColorView",	"ColorView",
											lB,		kFBAttachLeft,	"",		1.0,
											lB,		kFBAttachTop,	"",		1.0,
											-lB,	kFBAttachRight,	"",		1.0,
											-lOperationArea,	kFBAttachBottom,"",		1.0);

	AddRegion(	"EditColor",	"EditColor",
											lB,		kFBAttachLeft,	"",		1.0,
											lB,		kFBAttachBottom,	"ColorView",		1.0,
											120,	kFBAttachNone,	"",		1.0,
											lH,		kFBAttachNone,	"",		1.0);

	AddRegion(	"EditAlpha",	"EditAlpha",
											lB,		kFBAttachRight,	"EditColor",		1.0,
											lB,		kFBAttachBottom,	"ColorView",		1.0,
											120,	kFBAttachNone,	"",		1.0,
											lH,		kFBAttachNone,	"",		1.0);

	AddRegion(	"ButtonSelect",	"ButtonSelect",
											3*lB,		kFBAttachRight,	"EditAlpha",		1.0,
											lB,		kFBAttachBottom,	"ColorView",		1.0,
											lButtonWidth,	kFBAttachNone,	"",		1.0,
											lH,		kFBAttachNone,	"",		1.0);
	AddRegion(	"ButtonMove",	"ButtonMove",
											lB,		kFBAttachRight,	"ButtonSelect",		1.0,
											lB,		kFBAttachBottom,	"ColorView",		1.0,
											lButtonWidth,	kFBAttachNone,	"",		1.0,
											lH,		kFBAttachNone,	"",		1.0);
	AddRegion(	"ButtonAdd",	"ButtonAdd",
											lB,		kFBAttachRight,	"ButtonMove",		1.0,
											lB,		kFBAttachBottom,	"ColorView",		1.0,
											lButtonWidth,	kFBAttachNone,	"",		1.0,
											lH,		kFBAttachNone,	"",		1.0);

	AddRegion(	"ButtonRemove",	"ButtonRemove",
											lB,		kFBAttachRight,	"ButtonAdd",		1.0,
											lB,		kFBAttachBottom,	"ColorView",		1.0,
											lButtonWidth,	kFBAttachNone,	"",		1.0,
											lH,		kFBAttachNone,	"",		1.0);

	AddRegion(	"ButtonClear",	"ButtonClear",
											lB,		kFBAttachRight,	"ButtonRemove",		1.0,
											lB,		kFBAttachBottom,	"ColorView",		1.0,
											lButtonWidth,	kFBAttachNone,	"",		1.0,
											lH,		kFBAttachNone,	"",		1.0);

	AddRegion(	"ButtonPreview",	"ButtonPreview",
											3*lB,		kFBAttachRight,	"ButtonClear",		1.0,
											lB,		kFBAttachBottom,	"ColorView",		1.0,
											lButtonWidth,	kFBAttachNone,	"",		1.0,
											lH,		kFBAttachNone,	"",		1.0);

	AddRegion(	"ButtonClose",	"ButtonClose",
											lB,		kFBAttachRight,	"ButtonPreview",		1.0,
											lB,		kFBAttachBottom,	"ColorView",		1.0,
											lButtonWidth,	kFBAttachNone,	"",		1.0,
											lH,		kFBAttachNone,	"",		1.0);

	SetView( "ColorView", mColorView );
	
	SetControl( "EditColor", mEditColor );
	SetControl( "EditAlpha", mEditAlpha );

	SetControl( "ButtonSelect", mButtonSelect );
	SetControl( "ButtonMove", mButtonMove );
	SetControl( "ButtonAdd", mButtonAdd );
	SetControl( "ButtonRemove", mButtonRemove );
	SetControl( "ButtonClear", mButtonClear );
	SetControl( "ButtonPreview", mButtonPreview );
	SetControl(	"ButtonClose",	mButtonClose	);
}

/************************************************
 *	UI Configuration function.
 *	Configure UI elements
 ************************************************/
void ORPopup_ColorEditor::UIConfigure()
{
	Region.X		= 100;
	Region.Y		= 100;
	Region.Width	= 800;
	Region.Height	= 140;

	mButtonSelect.Caption = "Select";
	mButtonSelect.State = 1;
	mButtonSelect.OnClick.Add( this, (FBCallback) &ORPopup_ColorEditor::EventButtonSelectClick );

	mButtonMove.Caption = "Move";
	mButtonMove.OnClick.Add( this, (FBCallback) &ORPopup_ColorEditor::EventButtonMoveClick );

	mButtonAdd.Caption = "Add";
	mButtonAdd.OnClick.Add( this, (FBCallback) &ORPopup_ColorEditor::EventButtonAddClick );

	mButtonRemove.Caption = "Remove";
	mButtonRemove.OnClick.Add( this, (FBCallback) &ORPopup_ColorEditor::EventButtonRemoveClick );

	mButtonClear.Caption = "Clear";
	mButtonClear.OnClick.Add( this, (FBCallback) &ORPopup_ColorEditor::EventButtonClearClick );

	mButtonClose.Caption	= "Close";
	mButtonClose.OnClick.Add( this, (FBCallback) &ORPopup_ColorEditor::EventButtonCloseClick );

	mButtonPreview.Caption	= "Preview";
	mButtonPreview.OnClick.Add( this, (FBCallback) &ORPopup_ColorEditor::EventButtonPreviewClick );

	mEditAlpha.Value = 255.0;
	mEditAlpha.Min = 0.0;
	mEditAlpha.Max = 255.0;
	mEditAlpha.OnChange.Add( this, (FBCallback) &ORPopup_ColorEditor::EventEditColorChange );

	mEditColor.ColorMode = 3;
	mEditColor.OnChange.Add( this, (FBCallback) &ORPopup_ColorEditor::EventEditColorChange );

	Restructure(true);

	int pX, pY, pW, pH;
	GetRegionPositions( "ColorView", false, &pX, &pY, &pW, &pH);
	if (pW < 0) pW = Region.Width + pW - pX;
	if (pH < 0) pH = Region.Height + pH - pY;
	mColorView.SetViewport(pX, pY, pW, pH);
}


/************************************************
 *	
 ************************************************/

void ORPopup_ColorEditor::SetOperationButtonState()
{
	mButtonSelect.State = 0;
	mButtonMove.State = 0;
	mButtonAdd.State = 0;
	mButtonRemove.State = 0;

	switch(mColorView.GetOperationMode() )
	{
	case OperationSelect:
		mButtonSelect.State = 1;
		break;
	case OperationMove:
		mButtonMove.State = 1;
		break;
	case OperationAdd:
		mButtonAdd.State = 1;
		break;
	case OperationRemove:
		mButtonRemove.State = 1;
		break;
	}
}

void ORPopup_ColorEditor::EventButtonSelectClick( HISender pSender, HKEvent pEvent )
{
	mColorView.SetOperationMode( OperationSelect );
	SetOperationButtonState();
}

void ORPopup_ColorEditor::EventButtonMoveClick( HISender pSender, HKEvent pEvent )
{
	mColorView.SetOperationMode( OperationMove );
	SetOperationButtonState();
}

void ORPopup_ColorEditor::EventButtonAddClick( HISender pSender, HKEvent pEvent )
{
	mColorView.SetOperationMode( OperationAdd );
	SetOperationButtonState();
}

void ORPopup_ColorEditor::EventButtonRemoveClick( HISender pSender, HKEvent pEvent )
{
	mColorView.SetOperationMode( OperationRemove );
	SetOperationButtonState();
}

void ORPopup_ColorEditor::EventButtonClearClick( HISender pSender, HKEvent pEvent )
{
	
}

void ORPopup_ColorEditor::EventButtonPreviewClick( HISender pSender, HKEvent pEvent )
{
	Preview();
}

void ORPopup_ColorEditor::EventEditColorChange( HISender pSender, HKEvent pEvent )
{
	mColorView.SetColor( mEditColor.Value, mEditAlpha.Value / 255.0);
}

void ORPopup_ColorEditor::SetColorForEdit(const FBColorAndAlpha color)
{
	mEditColor.Value = FBColor( color[0], color[1], color[2] );
	mEditAlpha.Value = color[3] * 255.0;
}

/************************************************
 *	Close button callback (on click).
 ************************************************/
void ORPopup_ColorEditor::EventButtonCloseClick( HISender pSender, HKEvent pEvent )
{
	Close();
}

void ORPopup_ColorEditor::Clear()
{
	
}

void ORPopup_ColorEditor::Preview()
{
}


void ORPopup_ColorEditor::SetUp( FBShader *shader, FBPropertyAnimatableColorAndAlpha *pProp )
{
	mShader = shader;
	mColorView.SetUp( this, pProp );
}
