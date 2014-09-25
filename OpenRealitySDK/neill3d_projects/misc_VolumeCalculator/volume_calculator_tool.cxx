/***************************************************************************************
 Autodesk(R) Open Reality(R) Samples
 
 (C) 2009 Autodesk, Inc. and/or its licensors
 All rights reserved.
 
 AUTODESK SOFTWARE LICENSE AGREEMENT
 Autodesk, Inc. licenses this Software to you only upon the condition that 
 you accept all of the terms contained in the Software License Agreement ("Agreement") 
 that is embedded in or that is delivered with this Software. By selecting 
 the "I ACCEPT" button at the end of the Agreement or by copying, installing, 
 uploading, accessing or using all or any portion of the Software you agree 
 to enter into the Agreement. A contract is then formed between Autodesk and 
 either you personally, if you acquire the Software for yourself, or the company 
 or other legal entity for which you are acquiring the software.
 
 AUTODESK, INC., MAKES NO WARRANTY, EITHER EXPRESS OR IMPLIED, INCLUDING BUT 
 NOT LIMITED TO ANY IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR 
 PURPOSE REGARDING THESE MATERIALS, AND MAKES SUCH MATERIALS AVAILABLE SOLELY ON AN 
 "AS-IS" BASIS.
 
 IN NO EVENT SHALL AUTODESK, INC., BE LIABLE TO ANYONE FOR SPECIAL, COLLATERAL, 
 INCIDENTAL, OR CONSEQUENTIAL DAMAGES IN CONNECTION WITH OR ARISING OUT OF PURCHASE 
 OR USE OF THESE MATERIALS. THE SOLE AND EXCLUSIVE LIABILITY TO AUTODESK, INC., 
 REGARDLESS OF THE FORM OF ACTION, SHALL NOT EXCEED THE PURCHASE PRICE OF THE 
 MATERIALS DESCRIBED HEREIN.
 
 Autodesk, Inc., reserves the right to revise and improve its products as it sees fit.
 
 Autodesk and Open Reality are registered trademarks or trademarks of Autodesk, Inc., 
 in the U.S.A. and/or other countries. All other brand names, product names, or 
 trademarks belong to their respective holders. 
 
 GOVERNMENT USE
 Use, duplication, or disclosure by the U.S. Government is subject to restrictions as 
 set forth in FAR 12.212 (Commercial Computer Software-Restricted Rights) and 
 DFAR 227.7202 (Rights in Technical Data and Computer Software), as applicable. 
 Manufacturer is Autodesk, Inc., 10 Duke Street, Montreal, Quebec, Canada, H3C 2L7.
***************************************************************************************/

/**	\file	ormodel_template_tool.cxx
*/

//--- Class declaration
#include "ormodel_template_tool.h"
#include "ormodel_template_model.h"
#include "ormodel_template_model_display.h"

//--- Registration defines
#define ORTOOLCUSTOMMODEL__CLASS	ORTOOLCUSTOMMODEL__CLASSNAME
#define ORTOOLCUSTOMMODEL__LABEL	"Custom Model"
#define ORTOOLCUSTOMMODEL__DESC		"OR - Custom Model Tool Description"

//--- FiLMBOX implementation and registration
FBToolImplementation(	ORTOOLCUSTOMMODEL__CLASS	);
FBRegisterTool		(	ORTOOLCUSTOMMODEL__CLASS,
						ORTOOLCUSTOMMODEL__LABEL,
						ORTOOLCUSTOMMODEL__DESC,
						FB_DEFAULT_SDK_ICON			);	// Icon filename (default=Open Reality icon)

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ORToolCustomModel::FBCreate()
{
	// Create/reset/manage UI
	UICreate	();
	UIConfigure	();
	UIReset		();

	// Add tool callbacks
	OnShow.Add(this, (FBCallback)&ORToolCustomModel::EventToolShow);
	OnIdle.Add(this, (FBCallback)&ORToolCustomModel::EventToolIdle);

	mSystem.OnConnectionNotify.Add(this, (FBCallback)&ORToolCustomModel::ConnectionNotify);
	mSystem.OnConnectionStateNotify.Add(this, (FBCallback)&ORToolCustomModel::ConnectionStateNotify);

	return true;
}


/************************************************
 *	FiLMBOX Destruction function.
 ************************************************/
void ORToolCustomModel::FBDestroy()
{
	// Remove tool callbacks
	OnShow.Remove(this, (FBCallback) &ORToolCustomModel::EventToolShow);
	OnIdle.Remove(this, (FBCallback) &ORToolCustomModel::EventToolIdle);

	mSystem.OnConnectionNotify.Remove(this, (FBCallback)&ORToolCustomModel::ConnectionNotify);
	mSystem.OnConnectionStateNotify.Remove(this, (FBCallback)&ORToolCustomModel::ConnectionStateNotify);

	// Free user allocated memory
}

/************************************************
 *	UI Management
 ************************************************/
void ORToolCustomModel::UICreate()
{
	int lS = 4;
	int lW = 100;
	int lH = 18;

	// Add regions
	AddRegion( "ButtonCreateModel", "ButtonCreateModel",
										lS,		kFBAttachLeft,		"",	1.0	,
										lS,		kFBAttachTop,		"",	1.0,
										lW,		kFBAttachNone,		"",	1.0,
										lH,		kFBAttachNone,		"",	1.0 );
	AddRegion( "ButtonCreateModelWithCustomDisplay", "ButtonCreateModel",
										lS,		kFBAttachRight,		"ButtonCreateModel",	1.0	,
										lS,		kFBAttachTop,		"",	1.0,
										lW*3,	kFBAttachNone,		"",	1.0,
										lH,		kFBAttachNone,		"",	1.0 );
	AddRegion( "ListModels",	"ListModels",
										0,		kFBAttachLeft,		"ButtonCreateModel",	1.0,
										lS,		kFBAttachBottom,	"ButtonCreateModel",	1.0,
										0,		kFBAttachWidth,		"ButtonCreateModel",	1.0,
										0,		kFBAttachHeight,	"ButtonCreateModel",	1.0);

	// Assign regions
	SetControl( "ButtonCreateModel",	mButtonCreateModel	);
	SetControl( "ButtonCreateModelWithCustomDisplay",	mButtonCreateModelWithCustomDisplay	);
	SetControl( "ListModels",			mListModels			);
}
void ORToolCustomModel::UIConfigure()
{
	mButtonCreateModel.Caption = "Create Model";
	mButtonCreateModel.OnClick.Add( this, (FBCallback) &ORToolCustomModel::EventButtonCreateModelClick );
	mButtonCreateModelWithCustomDisplay.Caption = "Create Model with custom display";
	mButtonCreateModelWithCustomDisplay.OnClick.Add( this, (FBCallback) &ORToolCustomModel::EventButtonCreateModelWithCustomDisplayClick );
}
void ORToolCustomModel::UIReset()
{
	FBModelList	lList;
	FBFindModelsOfType( lList, ORModelCustom::TypeInfo );

	mListModels.Items.Clear();
	int i;
	for( i=0; i<lList.GetCount(); i++)
	{
		if( lList[i]->Show )
		{
			mListModels.Items.Add( lList[i]->Name, (kReference) lList[i] );
		}
	}
}

void ORToolCustomModel::UIRefresh()
{
}

/************************************************
 *	Create model callback.
 ************************************************/
void ORToolCustomModel::EventButtonCreateModelClick( HISender pSender, HKEvent pEvent )
{
	ORModelCustom*	lModel = new ORModelCustom( "Custom 1" );
	lModel->FBCreate();
	lModel->Show = true;

	UIReset();
}

void ORToolCustomModel::EventButtonCreateModelWithCustomDisplayClick( HISender pSender, HKEvent pEvent )
{
	ORModelCustomDisplay*	lModel = new ORModelCustomDisplay( "Custom Display 1" );
	lModel->FBCreate();
	lModel->Show = true;

	UIReset();
}

/************************************************
 *	UI Idle callback.
 ************************************************/
void ORToolCustomModel::EventToolIdle( HISender pSender, HKEvent pEvent )
{
	UIRefresh();
}

/************************************************
 *	Handle tool activation (selection/unselection).
 ************************************************/
void ORToolCustomModel::EventToolShow( HISender pSender, HKEvent pEvent )
{
	FBEventShow lEvent( pEvent );

	if( lEvent.Shown )
	{
		UIReset();
	}
	else
	{
	}
}

/************************************************
 *	Model selection callback.
 ************************************************/
void ORToolCustomModel::ConnectionStateNotify( HISender pSender, HKEvent pEvent )
{
	FBEventConnectionStateNotify lEvent(pEvent);
	switch( lEvent.Action )
	{
		case kFBDestroy:
		case kFBRename:
			UIReset();
			break;
	}
}

void ORToolCustomModel::ConnectionNotify( HISender pSender, HKEvent pEvent )
{
	FBEventConnectionNotify lEvent(pEvent);
	switch( lEvent.Action )
	{
		case kFBConnect:
		case kFBDisconnect:
			UIReset();
			break;
	}
}

/************************************************
 *	FBX Storage.
 ************************************************/
bool ORToolCustomModel::FbxStore	( HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat )
{
	pFbxObject->FieldWriteBegin( "ORToolCustomModelSection" );
	{
	}
	pFbxObject->FieldWriteEnd();

	return true;
}

/************************************************
 *	FBX Retrieval.
 ************************************************/
bool ORToolCustomModel::FbxRetrieve( HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat )
{
	pFbxObject->FieldReadBegin( "ORToolCustomModelSection" );
	{
	}
	pFbxObject->FieldReadEnd();

	return true;
}


