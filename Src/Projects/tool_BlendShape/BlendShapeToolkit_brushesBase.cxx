
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BlendShapeToolkit_brushesBase.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "BlendShapeToolkit_brushes.h"
#include "math3d.h"

FBColor		gWhiteColor(1.0, 1.0, 1.0);

void BaseBrush::UICreate()
{
	int lB = 10;
//	int	lS = 4;
	int lH = 18;
	int lW = 160;
	
	// Add regions
	mLayout.AddRegion( "Override",		"Override",
									lB,		kFBAttachLeft,		"",						1.0,
									lB,		kFBAttachTop,		"",						1.0,
									-lB,	kFBAttachRight,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );
	mLayout.AddRegion( "Color",		"Color",
									lB,		kFBAttachLeft,		"",						1.0,
									lB,		kFBAttachBottom,	"Override",				1.0,
									lW,		kFBAttachNone,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );
	mLayout.AddRegion( "Radius",		"Radius",
									lB,		kFBAttachLeft,		"",						1.0,
									lB,		kFBAttachBottom,	"Color",				1.0,
									lW,		kFBAttachNone,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );
	mLayout.AddRegion( "Strength",		"Strength",
									lB,		kFBAttachLeft,		"",						1.0,
									lB,		kFBAttachBottom,	"Radius",				1.0,
									lW,		kFBAttachNone,		"",						1.0,
									lH,		kFBAttachNone,		"",						1.0 );

	// Set controls

	mLayout.SetControl( "Override", mButtonOverride );
	mLayout.SetControl( "Color", mEditColor );
	mLayout.SetControl( "Radius", mEditRadius );
	mLayout.SetControl( "Strength", mEditStrength );
}

void BaseBrush::UIDestroy()
{
	mLayout.ClearControl( "Override" );
	mLayout.ClearControl( "Color" );
	mLayout.ClearControl( "Radius" );
	mLayout.ClearControl( "Strength" );
}

void BaseBrush::UIConfig()
{
	mButtonOverride.Caption = "Override Common";
	mButtonOverride.Style = kFBCheckbox;

	mEditColor.Caption = "Color";
	mEditRadius.Caption = "Radius";
	mEditStrength.Caption = "Strength";
}

void BaseBrush::UIReset()
{
	mButtonOverride.State = (Override) ? 1 : 0;
	mEditColor.Value = Color;
	mEditRadius.Value = Radius;
	mEditStrength.Value = Strength;
}

void BaseBrush::UpdateData()
{
	Override = mButtonOverride.State > 0;
	Color = mEditColor.Value;
	Radius = mEditRadius.Value;
	Strength = mEditStrength.Value;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//

int BrushManager::AddBrush( BaseBrush *brush )
{
	return mBrushes.Add(brush);
}
int BrushManager::AddFalloff( BaseFalloff *falloff )
{
	return mFalloffs.Add(falloff);
}


void BrushManager::SetCurrentBrush(const int index)
{
	mCurrentBrush = index;
}

void BrushManager::SetCurrentFalloff(const int index)
{
	mCurrentFalloff = index;
}

BaseBrush *BrushManager::GetCurrentBrushPtr() 
{ 
	int lCurrentBrush = (mSmoothBrush >=0) ? mSmoothBrush : mCurrentBrush;
	return (lCurrentBrush>=0) ? mBrushes[lCurrentBrush] : nullptr; 
}

int BrushManager::GetCurrentBrush()
{
	int lCurrentBrush = (mSmoothBrush >=0) ? mSmoothBrush : mCurrentBrush;
	return lCurrentBrush;
}

bool BrushManager::IsOverride()
{
	int lCurrentBrush = (mSmoothBrush >=0) ? mSmoothBrush : mCurrentBrush;
	return (lCurrentBrush>=0) ? mBrushes[lCurrentBrush]->IsOverride() : false;
}

FBColor &BrushManager::GetColor()
{
	int lCurrentBrush = (mSmoothBrush >=0) ? mSmoothBrush : mCurrentBrush;
	return (lCurrentBrush>=0) ? mBrushes[lCurrentBrush]->GetColor() : gWhiteColor;
}

double BrushManager::GetRadius()
{
	int lCurrentBrush = (mSmoothBrush >=0) ? mSmoothBrush : mCurrentBrush;
	return (lCurrentBrush>=0) ? mBrushes[lCurrentBrush]->GetRadius() : false;
}

double BrushManager::GetStrength()
{
	int lCurrentBrush = (mSmoothBrush >=0) ? mSmoothBrush : mCurrentBrush;
	return (lCurrentBrush>=0) ? mBrushes[lCurrentBrush]->GetStrength() : false;
}

void BrushManager::SetCurrentBrushRadius(const double value)
{
	int lCurrentBrush = (mSmoothBrush >=0) ? mSmoothBrush : mCurrentBrush;
	if (lCurrentBrush>=0) mBrushes[lCurrentBrush]->SetRadius(value);
}

void BrushManager::SetColor(const FBColor value)
{
	for (int i=0; i<mBrushes.GetCount(); ++i)
		if (mBrushes[i]->IsOverride() == false)
		{
			mBrushes[i]->SetColor(value);
		}
}

void BrushManager::SetRadius(const double value)
{
	for (int i=0; i<mBrushes.GetCount(); ++i)
		if (mBrushes[i]->IsOverride() == false)
		{
			mBrushes[i]->SetRadius(value);
		}
}

void BrushManager::SetCurrentBrushStrength(const double value)
{
	int lCurrentBrush = (mSmoothBrush >=0) ? mSmoothBrush : mCurrentBrush;
	if (lCurrentBrush>=0) mBrushes[lCurrentBrush]->SetStrength(value);
}

void BrushManager::SetStrength(const double value)
{
	for (int i=0; i<mBrushes.GetCount(); ++i)
		if (mBrushes[i]->IsOverride() == false)
		{
			mBrushes[i]->SetStrength(value);
		}
}

void BrushManager::ToggleSmooth(const bool active, const int index)
{
	if (active) 
	{
		mSmoothBrush = index;
	}
	else mSmoothBrush = -1;
}

void BrushManager::Draw( const FBDrawState state )
{
	
}

bool BrushManager::Input(int pMouseX, int pMouseY, FBInputType pAction, int pButtonKey,int pModifier)
{
	/*
	mInput.ctrl = (pModifier == 2);
	mInput.shift = (pModifier == 1);
	mInput.alt = false;

	mInput.mouseX = pMouseX;
	mInput.mouseY = pMouseY;

	if (pAction >= kFBButtonPress && pAction <= kFBDropping )
	{
		switch (pButtonKey)
		{
		case 1: mInput.buttonAction = ViewInput::MouseButton::left;
			break;
		case 2:	mInput.buttonAction = ViewInput::MouseButton::middle;
			break;
		case 3:	mInput.buttonAction = ViewInput::MouseButton::right;
			break;
		default:
			mInput.buttonAction = ViewInput::MouseButton::none;
		}
	}

	switch(pAction)
	{
	case kFBButtonPress:
		mInput.buttonState |= mInput.buttonAction;
		break;
	case kFBButtonRelease:
		if (mInput.buttonState & mInput.buttonAction)
			mInput.buttonState &= mInput.buttonAction;
		break;
	}
	*/
	//
	///
	//
	

	// update last values
	//mInput.lastMouseX = pMouseX;
	//mInput.lastMouseY = pMouseY;

	return true;
}

void BrushManager::FreeMem()
{
	for (int i=0; i<mBrushes.GetCount(); ++i)
	{
		BaseBrush *pBrush = mBrushes[i];
		if (pBrush)
		{
			delete pBrush;
		}
	}

	for (int i=0; i<mFalloffs.GetCount(); ++i)
	{
		BaseFalloff *pFalloff = mFalloffs[i];
		if (pFalloff)
		{
			delete pFalloff;
		}
	}
}

bool BrushManager::WantToReacalcWeights()
{
	int lCurrentBrush = (mSmoothBrush >=0) ? mSmoothBrush : mCurrentBrush;

	if (lCurrentBrush < 0) return false;

	return mBrushes[lCurrentBrush]->WantToReacalcWeights();
}

bool BrushManager::Process( const BrushData &brushData, BrushCameraData *pCameraData, const OperationBuffer &bufferZero, OperationBuffer &buffer )
{
	int lCurrentBrush = (mSmoothBrush >=0) ? mSmoothBrush : mCurrentBrush;

	if (lCurrentBrush < 0) return false;

	mBrushes[lCurrentBrush]->Process(brushData, pCameraData, bufferZero, buffer);

	return true;
}