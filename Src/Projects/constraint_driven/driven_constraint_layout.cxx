
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: neill3d@gmail.com
// 
/////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "driven_constraint_layout.h"

//--- Registration define
#define ORCONSTRAINT__LAYOUT	Driven_Constraint_Layout

//--- implementation and registration
FBConstraintLayoutImplementation(	ORCONSTRAINT__LAYOUT		);		// Layout class name
FBRegisterConstraintLayout		(	ORCONSTRAINT__LAYOUT,				// Layout class name
									ORCONSTRAINT__CLASSSTR,				// Constraint class name (for association)
									FB_DEFAULT_SDK_ICON			);		// Icon filename (default=Open Reality icon)

static constexpr const char* INFO_TEXT{"Transfer value of an non animatable property into an obj's animatable property\n"
								"Documentation page - https://github.com/Neill3d/OpenMoBu/wiki/Driven-property-constraint \n" };
static constexpr const char* ABOUT_TEXT{"Created by Sergei <Neill3d> Solokhin\n"
                                "- OpenMoBu project, github - https://github.com/Neill3d/OpenMoBu" };

/************************************************
 *	Creation function..
 ************************************************/
bool Driven_Constraint_Layout::FBCreate()
{
	// Assign handle onto constraint (cast generic pointer).
	mConstraint	= (Driven_Constraint*) (FBConstraint*) Constraint;

	FBSystem lSystem;
    mScene = lSystem.Scene;

	// Create & configure the UI
	UICreate	();		// create the UI
	UIConfigure	();		// configure the UI
	UIReset		();		// set the UI values from the constraint

	mScene->OnChange.Add( this, (FBCallback) &Driven_Constraint_Layout::EventSceneChange );

	return true;
}


/************************************************
 *	Destructor for layout.
 ************************************************/
void Driven_Constraint_Layout::FBDestroy()
{
	mScene->OnChange.Remove( this, (FBCallback) &Driven_Constraint_Layout::EventSceneChange );
}


/************************************************
 *	UI Creation function.
 *	Create UI regions and assign them to UI elements
 ************************************************/
void Driven_Constraint_Layout::UICreate()
{
	//int lS = 4;
	int lS = 6;
	int lH = 18;
	int lW = 250;

	// Create UI
	AddRegion( "LabelDirections", "LabelDirections",
											lS,	kFBAttachTop,	"",		1.0,
											lS,	kFBAttachLeft,	"",		1.0,
											-lS,kFBAttachRight,	"",		1.0,
											65,	kFBAttachNone,	NULL,	1.0 );

	AddRegion( "SrcContainer", "SrcContainer",	lS,	kFBAttachLeft,	"",	1.0,
											lS,	kFBAttachBottom,"LabelDirections",	1.0,
											lW,	kFBAttachNone,	NULL,	1.0,
											lH,	kFBAttachNone,	NULL,	1.0 );
	AddRegion( "SrcPropertyList", "SrcPropertyList",	lS,	kFBAttachLeft,	"",	1.0,
											lS,	kFBAttachBottom,"SrcContainer",	1.0,
											lW,	kFBAttachNone,	NULL,	1.0,
											lH,	kFBAttachNone,	NULL,	1.0 );
	AddRegion( "SrcProperty", "SrcProperty",	lS,	kFBAttachLeft,	"",	1.0,
											lS,	kFBAttachBottom,"SrcPropertyList",	1.0,
											lW,	kFBAttachNone,	NULL,	1.0,
											lH,	kFBAttachNone,	NULL,	1.0 );

	AddRegion( "DstContainer", "DstContainer",	lS,	kFBAttachLeft,	"",	1.0,
											lS+8,	kFBAttachBottom,"SrcProperty",	1.0,
											lW,	kFBAttachNone,	NULL,	1.0,
											lH,	kFBAttachNone,	NULL,	1.0 );
	AddRegion( "DstPropertyList", "DstPropertyList",	lS,	kFBAttachLeft,	"",	1.0,
											lS,	kFBAttachBottom,"DstContainer",	1.0,
											lW,	kFBAttachNone,	NULL,	1.0,
											lH,	kFBAttachNone,	NULL,	1.0 );
	AddRegion( "DstProperty", "DstProperty",	lS,	kFBAttachLeft,	"",	1.0,
											lS,	kFBAttachBottom,"DstPropertyList",	1.0,
											lW,	kFBAttachNone,	NULL,	1.0,
											lH,	kFBAttachNone,	NULL,	1.0 );

	AddRegion( "Info", "Info",	lS,	kFBAttachLeft,	"",	1.0,
											lS,	kFBAttachBottom,"DstProperty",	1.0,
											lW*2,	kFBAttachNone,	NULL,	1.0,
											lH*2,	kFBAttachNone,	NULL,	1.0 );

	// Assign regions
	SetControl( "LabelDirections", mLabelDirections	);
	SetControl( "SrcContainer",			mSrcContainerModel	);
	SetControl( "SrcPropertyList",		mSrcListProperties	);
	SetControl( "SrcProperty",			mSrcProperty		);
	SetControl( "DstContainer",			mDstContainerModel	);
	SetControl( "DstPropertyList",		mDstListProperties	);
	SetControl( "DstProperty",			mDstProperty		);
	SetControl( "Info", mInfo );
}


/************************************************
 *	Reset the UI from the constraint values.
 ************************************************/
void Driven_Constraint_Layout::UIReset()
{
}


/************************************************
 *	UI Configuration function.
 *	Assign properties & callbacks for UI elements
 ************************************************/
void Driven_Constraint_Layout::UIConfigure()
{
	// Configure UI
	mLabelDirections.Caption =	INFO_TEXT;

	//mButtonTest.Caption = "Test";
	//mButtonTest.OnClick.Add( this, (FBCallback) &Driven_Constraint_Layout::EventButtonTestClick );

	mSrcContainerModel.IconPosition        = kFBIconLeft; 
	mDstContainerModel.IconPosition        = kFBIconLeft;

	// Callbacks
    mSrcContainerModel.OnDragAndDrop.Add   ( this, (FBCallback) &Driven_Constraint_Layout::EventSrcContainerDragAndDrop   );
    mSrcContainerModel.OnDblClick.Add      ( this, (FBCallback) &Driven_Constraint_Layout::EventSrcContainerDblClick      );
    mSrcListProperties.OnChange.Add        ( this, (FBCallback) &Driven_Constraint_Layout::EventSrcListPropertyChange     );

	mDstContainerModel.OnDragAndDrop.Add   ( this, (FBCallback) &Driven_Constraint_Layout::EventDstContainerDragAndDrop   );
    mDstContainerModel.OnDblClick.Add      ( this, (FBCallback) &Driven_Constraint_Layout::EventDstContainerDblClick      );
    mDstListProperties.OnChange.Add        ( this, (FBCallback) &Driven_Constraint_Layout::EventDstListPropertyChange     );


    // Initial setup of the list.
	SetupSrcPropertyList( mConstraint->mSrcModel, mConstraint->mSrcProp );
	SetupDstPropertyList( mConstraint->mDstModel, mConstraint->mDstProp );

	mInfo.Caption = ABOUT_TEXT;
}


/************************************************
 *	Button click callback.
 ************************************************/
void Driven_Constraint_Layout::EventButtonTestClick( HISender pSender, HKEvent pEvent )
{
}


void Driven_Constraint_Layout::EventSrcContainerDragAndDrop( HISender pSender, HKEvent pEvent )
{
    FBEventDragAndDrop lDragAndDrop( pEvent );

    switch( lDragAndDrop.State )
    {
        case kFBDragAndDropDrag:
        {
            lDragAndDrop.Accept();
        }
        break;
        case kFBDragAndDropDrop:
        {
			mConstraint->mSrcModel = lDragAndDrop.Get(0);
            SetupSrcPropertyList( lDragAndDrop.Get(0), nullptr );
        }
        break;
    }
}

void Driven_Constraint_Layout::SetupSrcPropertyList( FBComponent *lModel, FBProperty *pProperty )
{
    mSrcContainerModel.Items.Clear();
    mSrcListProperties.Items.Clear();
/*
	if (mSrcProperty.Property)
	{
		HFBProperty		lProp = mSrcProperty.Property;

		if ( lProp->IsAnimatable() ) {
		FBPropertyAnimatable *lPropAnimatable = (FBPropertyAnimatable*) lProp;
				
		//-- must be setUP for XYZ (three nodes)
		FBAnimationNode *lAnimationNode = lPropAnimatable->GetAnimationNode()->Nodes[0];
		if ( lAnimationNode ) {
			lAnimationNode->OnChange.Remove( this, (FBCallback) &Driven_Constraint_Layout::EventSrcValueChange );
		}
	}

	}
	*/

	mSrcProperty.Property			= pProperty;
	mConstraint->mSrcProp			= pProperty;

    if( lModel )
    {
        mSrcContainerModel.Items.Add( lModel->Name, (kReference)lModel );
        int lIdx = 0;
        int lNbProps = lModel->PropertyList.GetCount();

        mSrcListProperties.Items.Add( "<Select Property>", (kReference)0 );
        mSrcListProperties.ItemIndex = 0;
        for( lIdx = 0; lIdx < lNbProps; ++lIdx )
        {
            FBProperty *lProp = lModel->PropertyList[lIdx];
            if( lProp->IsInternal() && !lProp->GetPropertyFlag( kFBPropertyFlagHideProperty ))
            {
                mSrcListProperties.Items.Add( lProp->GetName(), (kReference)lProp );
            }

			if (pProperty)
				if ( strcmp(lProp->GetName(), pProperty->GetName()) == 0)
					mSrcListProperties.ItemIndex = lIdx;
        }
    }
}

void Driven_Constraint_Layout::EventSrcContainerDblClick( HISender pSender, HKEvent pEvent )
{
	mConstraint->mSrcModel = nullptr;
	mConstraint->mSrcProp = nullptr;
    SetupSrcPropertyList( nullptr, nullptr );
}

void Driven_Constraint_Layout::EventSrcListPropertyChange( HISender pSender, HKEvent pEvent )
{
    FBProperty *lProp = (FBProperty*)mSrcListProperties.Items.GetReferenceAt( mSrcListProperties.ItemIndex );
    mSrcProperty.Property = lProp;
	mConstraint->mSrcProp = lProp;
/*
	if ( lProp->IsAnimatable() ) {
		FBPropertyAnimatable *lPropAnimatable = (FBPropertyAnimatable*) lProp;
				
		//-- must be setUP for XYZ (three nodes)
		FBAnimationNode *lAnimationNode = lPropAnimatable->GetAnimationNode()->Nodes[0];
		if ( lAnimationNode ) {
			lAnimationNode->OnChange.Add( this, (FBCallback) &Driven_Constraint_Layout::EventSrcValueChange ); 
		}
	}
	*/
}

//
// dst property
//

void Driven_Constraint_Layout::EventDstContainerDragAndDrop( HISender pSender, HKEvent pEvent )
{
    FBEventDragAndDrop lDragAndDrop( pEvent );

    switch( lDragAndDrop.State )
    {
        case kFBDragAndDropDrag:
        {
            lDragAndDrop.Accept();
        }
        break;
        case kFBDragAndDropDrop:
        {
			mConstraint->mDstModel = lDragAndDrop.Get(0);
            SetupDstPropertyList( lDragAndDrop.Get(0), nullptr );
        }
        break;
    }
}

void Driven_Constraint_Layout::SetupDstPropertyList( FBComponent *lModel, FBProperty *pProperty )
{
    mDstContainerModel.Items.Clear();
    mDstListProperties.Items.Clear();

    mDstProperty.Property = pProperty;
	mConstraint->mDstProp = pProperty;

    if( lModel )
    {
        mDstContainerModel.Items.Add( lModel->Name, (kReference)lModel );
        int lIdx = 0;
        int lNbProps = lModel->PropertyList.GetCount();

        mDstListProperties.Items.Add( "<Select Property>", (kReference)0 );
        mDstListProperties.ItemIndex = 0;
        for( lIdx = 0; lIdx < lNbProps; ++lIdx )
        {
            FBProperty *lProp = lModel->PropertyList[lIdx];
            if( lProp->IsInternal() && !lProp->GetPropertyFlag( kFBPropertyFlagHideProperty ))
            {
                mDstListProperties.Items.Add( lProp->GetName(), (kReference)lProp );
            }

			if (pProperty)
				if ( strstr(lProp->GetName(), pProperty->GetName()) != NULL )
					mDstListProperties.ItemIndex = lIdx;
        }
    }
}

void Driven_Constraint_Layout::EventDstContainerDblClick( HISender pSender, HKEvent pEvent )
{
	mConstraint->mDstModel = nullptr;
	mConstraint->mDstProp = nullptr;
    SetupDstPropertyList( nullptr, nullptr );
}

void Driven_Constraint_Layout::EventDstListPropertyChange( HISender pSender, HKEvent pEvent )
{
    FBProperty *lProp = (FBProperty*)mDstListProperties.Items.GetReferenceAt( mDstListProperties.ItemIndex );
    mDstProperty.Property = lProp;
	mConstraint->mDstProp = lProp;
}

void Driven_Constraint_Layout::EventSrcValueChange( HISender pSender, HKEvent pEvent )
{
	if (mConstraint->Active )
	{
		/*
		if (mSrcProp != NULL && mDstProp != NULL)
		{
			if (mSrcProp->GetPropertyType() == kFBPT_double)
			{
				if (mDstProp->GetPropertyType() == kFBPT_double)
				{
					// get value from the dst prop
					double temp;
					mDstProp->GetData( &temp, sizeof(double) );
					// and set up for src prop
					mSrcProp->SetData( &temp );
				}
				else if (mDstProp->GetPropertyType() == kFBPT_Vector3D)
				{
					// get vector from the dst prop
					FBVector3d	temp( 100.0, 100.0, 100.0 );
					mDstProp->GetData( &temp, sizeof(FBVector3d) );

					// set only x for src prop
					mSrcProp->SetData( &temp );
				}
			}
			else if (mSrcProp->GetPropertyType() == kFBPT_Vector3D)
			{
				//
			}
		}
		*/
	}
}

void Driven_Constraint_Layout::EventSceneChange( HISender pSender, HKEvent pEvent )
{
    if( mSrcContainerModel.Items.GetCount() != 0 )
    {
        FBEventSceneChange lEvent( pEvent );
		FBComponent *lComponent = mConstraint->mSrcModel;

        if( lEvent.Type == kFBSceneChangeDetach && (FBComponent*)lEvent.ChildComponent == lComponent )
        {
			mConstraint->mSrcModel = nullptr;
			mConstraint->mSrcProp = nullptr;
            SetupSrcPropertyList( nullptr, nullptr );
        }

		lComponent = mConstraint->mDstModel;

        if( lEvent.Type == kFBSceneChangeDetach && (FBComponent*)lEvent.ChildComponent == lComponent )
        {
			mConstraint->mDstModel = nullptr;
			mConstraint->mDstProp = nullptr;
            SetupDstPropertyList( nullptr, nullptr );
        }

/*
		lModel = (HFBModel)mDstContainerModel.Items.GetReferenceAt( 0 );

        if( lEvent.Type == kFBSceneChangeDetach && (HFBComponent)lEvent.ChildComponent == lModel )
        {
            SetupDstPropertyList( NULL );
        }
		*/
    }
}

