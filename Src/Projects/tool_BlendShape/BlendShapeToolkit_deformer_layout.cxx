
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: BlendShapeToolkit_deformer_layout.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declaration
#include "blendshapetoolkit_deformer_layout.h"

//--- Registration defines
#define BLENDSHAPEDEFORMER__LAYOUT	BlendShapeDeformerLayout

//--- implementation and registration
FBConstraintLayoutImplementation(	BLENDSHAPEDEFORMER__LAYOUT	);
FBRegisterConstraintLayout		(	BLENDSHAPEDEFORMER__LAYOUT,
									BLENDSHAPEDEFORMER__CLASSSTR,
									FB_DEFAULT_SDK_ICON				);	// Icon filename (default=Open Reality icon)



/************************************************
 *	Creation function..
 ************************************************/
bool BlendShapeDeformerLayout::FBCreate()
{
	// Assign handle onto constraint (cast generic pointer).
	mConstraint	= (BlendShapeDeformerConstraint*) (FBConstraint*) Constraint;

	// build & configure layout
	UICreate	();
	UIConfigure	();
	UIReset();
	return true;
}


/************************************************
 *	Destructor for layout.
 ************************************************/
void BlendShapeDeformerLayout::FBDestroy()
{
}


/************************************************
 *	UI Creation function.
 *	Create UI regions and assign them to UI elements
 ************************************************/
void BlendShapeDeformerLayout::UICreate()
{
	int lS, lH, lB, lW;	// space, width, height

	lS = 4;
	lB = 10;
	lH = 18;
	lW = 100;

	AddRegion( "LabelModels",		"LabelModels",
									lB,		kFBAttachLeft,	"",						1.0,
									lB,		kFBAttachNone,	"",						1.0,
									lW*2,		kFBAttachNone,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );

	AddRegion( "SpreadModels",		"SpreadModels",
									lB,		kFBAttachLeft,	"",						1.0,
									lB,		kFBAttachBottom,"LabelModels",			1.0,
									lW*2,		kFBAttachNone,	"",						1.0,
									-lH,	kFBAttachBottom,"",						1.0 );

	AddRegion( "LabelChannels",		"LabelChannels",
									lB,		kFBAttachRight,	"LabelModels",			1.0,
									lB,		kFBAttachNone,	"",						1.0,
									lW*2,		kFBAttachNone,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );

	AddRegion( "ListChannels",		"ListChannels",
									lB,		kFBAttachRight,	"LabelModels",			1.0,
									lB,		kFBAttachBottom,"LabelChannels",		1.0,
									lW*2,		kFBAttachNone,	"",						1.0,
									-lH,	kFBAttachBottom,"",						1.0 );

	AddRegion( "ButtonChannelRename",		"ButtonChannelRename",
									lB,		kFBAttachRight,	"LabelChannels",		1.0,
									lB*3,	kFBAttachNone,	"",						1.0,
									lW,		kFBAttachNone,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );

	AddRegion( "ButtonChannelRemove",		"ButtonChannelRemove",
									lB,		kFBAttachRight,	"LabelChannels",		1.0,
									lB,		kFBAttachBottom,"ButtonChannelRename",	1.0,
									lW,		kFBAttachNone,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );

	AddRegion( "ButtonChannelPlot",		"ButtonChannelPlot",
									lB,		kFBAttachRight,	"LabelChannels",		1.0,
									lB*2,	kFBAttachBottom,"ButtonChannelRemove",	1.0,
									lW,		kFBAttachNone,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );

	AddRegion( "LabelShapes",		"LabelShapes",
									lB,		kFBAttachRight,	"ButtonChannelRename",	1.0,
									lB,		kFBAttachNone,	"",						1.0,
									lW*2,		kFBAttachNone,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );
	AddRegion( "ListShapes",		"ListShapes",
									lB,		kFBAttachRight,	"ButtonChannelRename",	1.0,
									lB,		kFBAttachBottom,"LabelShapes",			1.0,
									lW*2,		kFBAttachNone,	"",						1.0,
									-lH,	kFBAttachBottom,"",						1.0 );
	// shape functions

	AddRegion( "ButtonShapesTimeline",		"ButtonShapesTimeline",
									lB,		kFBAttachRight,	"LabelShapes",			1.0,
									lB*2,	kFBAttachNone,	"",						1.0,
									lW,		kFBAttachNone,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );
	AddRegion( "ButtonShapesUpdate",		"ButtonShapesUpdate",
									lB,		kFBAttachRight,	"LabelShapes",			1.0,
									lB,		kFBAttachBottom,"ButtonShapesTimeline",	1.0,
									lW,		kFBAttachNone,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );

	AddRegion( "ButtonShapesRemoveAll",		"ButtonShapesRemoveAll",
									lB,		kFBAttachRight,	"LabelShapes",			1.0,
									lB*2,	kFBAttachBottom,"ButtonShapesUpdate",	1.0,
									lW,		kFBAttachNone,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );
	AddRegion( "ButtonShapesRemoveSelected",		"ButtonShapesRemoveSelected",
									lB,		kFBAttachRight,	"LabelShapes",			1.0,
									lB,		kFBAttachBottom,"ButtonShapesRemoveAll",1.0,
									lW,		kFBAttachNone,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );
	
	AddRegion( "ButtonShapesProperty",		"ButtonShapesProperty",
									lB,		kFBAttachRight,	"LabelShapes",			1.0,
									lB*2,	kFBAttachBottom,"ButtonShapesRemoveSelected",	1.0,
									lW*2,	kFBAttachNone,	"",						1.0,
									lH,		kFBAttachNone,	"",						1.0 );

	//
	//

	SetControl( "LabelModels", mLabelModels );
	SetControl( "SpreadModels", mSpreadModels );

	SetControl( "LabelChannels", mLabelChannels );
	SetControl( "ListChannels", mListChannels );

	SetControl( "ButtonChannelRename", mButtonChannelRename );
	SetControl( "ButtonChannelRemove", mButtonChannelRemove );
	SetControl( "ButtonChannelPlot", mButtonChannelPlot );

	SetControl( "LabelShapes", mLabelShapes );
	SetControl( "ListShapes", mListShapes );

	SetControl( "ButtonShapesTimeline", mButtonShapesTimeline );
	SetControl( "ButtonShapesUpdate", mButtonShapesUpdate );
	SetControl( "ButtonShapesRemoveAll", mButtonShapesRemoveAll );
	SetControl( "ButtonShapesRemoveSelected", mButtonShapesRemoveSelected );
	SetControl( "ButtonShapesProperty", mButtonShapesProperty );
}


/************************************************
 *	UI Configuration function.
 *	Assign properties & callbacks for UI elements
 ************************************************/
void BlendShapeDeformerLayout::UIConfigure()
{
	mLabelModels.Caption = "Models:";
	mSpreadModels.OnCellChange.Add( this, (FBCallback) &BlendShapeDeformerLayout::EventSpreadChange );

	mLabelChannels.Caption = "Channels:";
	mListChannels.Style.SetPropertyValue(kFBVerticalList);
	mListChannels.OnChange.Add( this, (FBCallback) &BlendShapeDeformerLayout::EventChannelSelect );

	mButtonChannelRename.Caption = "Rename";
	mButtonChannelRemove.Caption = "Remove";
	mButtonChannelPlot.Caption = "Plot";
	mButtonChannelPlot.OnClick.Add( this, (FBCallback) &BlendShapeDeformerLayout::EventButtonPlotClick );

	mLabelShapes.Caption = "Shapes:";
	mListShapes.Style.SetPropertyValue(kFBVerticalList);
	mListShapes.OnChange.Add( this, (FBCallback) &BlendShapeDeformerLayout::EventShapeSelect );

	mButtonShapesTimeline.Caption = "Go to shape";
	mButtonShapesTimeline.Style.SetPropertyValue(kFBCheckbox);
	mButtonShapesTimeline.State = 1;

	mButtonShapesUpdate.Caption = "Update";
	mButtonShapesUpdate.OnClick.Add( this, (FBCallback) &BlendShapeDeformerLayout::EventButtonShapesUpdate );
	mButtonShapesRemoveAll.Caption = "Remove All";
	mButtonShapesRemoveSelected.Caption = "Remove Selected";

	mButtonShapesProperty.Caption = "Shape";
	mButtonShapesProperty.SliderMin = 0.0;
	mButtonShapesProperty.SliderMax = 100.0;
}


void BlendShapeDeformerLayout::UIReset()
{
	// fill channel information

	const int numberOfChannels = mConstraint->GetNumberOfChannels();

	FBString strChannels("");

	char temp[128];
	memset( temp, 0, sizeof(char) * 128 );

	mListChannels.Items.Clear();
	for (int i=0; i<numberOfChannels; ++i)
	{
		const char *channelname = mConstraint->GetChannelName(i);

		sprintf_s( temp, sizeof(char) * 128, "%s - %d", channelname, mConstraint->GetNumberOfChannelVertices(i) );

		mListChannels.Items.Add( temp, (kReference) mConstraint->GetChannelPtr(i) );

		strChannels += channelname;
		if (i < numberOfChannels-1)
			strChannels += "~";
	}

	mListShapes.Items.Clear();

	// fill a spread with model and channel information

	mSpreadModels.Clear();

	mSpreadModels.ColumnAdd( "Model", 0 );
	mSpreadModels.ColumnAdd( "Channel", 1 );

	mSpreadModels.GetColumn(-1).Width = 24;
	mSpreadModels.GetColumn(0).Width = 80;
	mSpreadModels.GetColumn(1).Width = 80;
	
	memset( temp, 0, sizeof(char) * 128 );

	for (int i=0; i<mConstraint->ReferenceGetCount(0); ++i)
	{
		sprintf_s( temp, sizeof(char)*128, "%d", i );
		mSpreadModels.RowAdd( temp, i );

		FBModel *pModel = mConstraint->ReferenceGet(0, i);
		if (pModel)
			mSpreadModels.SetCell(i, 0, pModel->Name);

		mSpreadModels.GetCell(i, 1).Style.SetPropertyValue(kFBCellStyleMenu);
		mSpreadModels.SetCell(i, 1, strChannels);
	}

}

/************************************************
 *	Edit multiplier callback.
 ************************************************/
void BlendShapeDeformerLayout::EventSpreadChange( HISender pSender, HKEvent pEvent)
{

}

void BlendShapeDeformerLayout::EventChannelSelect( HISender pSender, HKEvent pEvent)
{
	mListShapes.Items.Clear();
	mButtonShapesProperty.Property = nullptr;

	const int index = mListChannels.ItemIndex;

	if (index >= 0)
	{
		const int numberOfShapes = mConstraint->GetNumberOfShapes(index);

		for (int i=0; i<numberOfShapes; ++i)
			mListShapes.Items.Add( mConstraint->GetShapeName(index, i), (kReference) mConstraint->GetShapePtr(index, i) );
	}
}

void BlendShapeDeformerLayout::EventShapeSelect( HISender pSender, HKEvent pEvent)
{
	const int index = mListShapes.ItemIndex;

	if (index >= 0)
	{
		CDeformerShape *pShape = (CDeformerShape*) mListShapes.Items.GetReferenceAt(index);
		mButtonShapesProperty.Property = pShape->pProperty;

		if (mButtonShapesTimeline.State > 0)
		{
			FBAnimationNode *pAnimNode = pShape->pProperty->GetAnimationNode();
			if (pAnimNode)
			{
				FBFCurve *pCurve = pAnimNode->FCurve;
				if (pCurve && pCurve->Keys.GetCount())
				{
					FBTime keyTime = pCurve->Keys[0].Time;
					FBPlayerControl::TheOne().Goto(keyTime);
				}
			}
		}
	}
}

void BlendShapeDeformerLayout::EventButtonPlotClick( HISender pSender, HKEvent pEvent )
{
	FBTime startTime = FBPlayerControl::TheOne().ZoomWindowStart;
	FBTime stopTime = FBPlayerControl::TheOne().ZoomWindowStop;
	FBTime stepTime(0,0,0, 1);

	const int index = mListChannels.ItemIndex;

	if (index >= 0)
	{
		CDeformerChannel *pChannel = (CDeformerChannel*) mListChannels.Items.GetReferenceAt(index);
		mConstraint->Plot( pChannel, startTime, stopTime, stepTime );
	}
}

void BlendShapeDeformerLayout::EventButtonShapesUpdate( HISender pSender, HKEvent pEvent )
{
	mListShapes.Items.Clear();
	mButtonShapesProperty.Property = nullptr;

	const int index = mListChannels.ItemIndex;

	char temp[128];
	memset(temp, 0, sizeof(char) * 128);

	if (index >= 0)
	{
		const int numberOfShapes = mConstraint->GetNumberOfShapes(index);
		for (int i=0; i<numberOfShapes; ++i)
		{
			const char *shapename = mConstraint->GetShapeName(index, i); 
			CDeformerShape *pShape = mConstraint->GetShapePtr(index, i);

			double value;
			pShape->pProperty->GetData(&value, sizeof(double) );

			sprintf_s( temp, sizeof(char)*128, "%s [%.2lf]", shapename, value );

			mListShapes.Items.Add( temp, (kReference) pShape );
		}
	}
}