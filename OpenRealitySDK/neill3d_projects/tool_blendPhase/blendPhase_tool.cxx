/***************************************************************************************
 Autodesk(R) Open Reality(R) Samples
 
 (C) 2006 Autodesk, Inc. and/or its licensors
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

/**	\file	blendPhase_tool.cxx
*/

//--- Class declaration
#include "blendPhase_Tool.h"

//--- Registration defines
#define BLENDPHASE__CLASS	BLENDPHASE__CLASSNAME
#define BLENDPHASE__LABEL	"blendPhase"
#define BLENDPHASE__DESC	"test blend phases between two animations"

//--- FiLMBOX implementation and registration
FBToolImplementation(	BLENDPHASE__CLASS	);
FBRegisterTool		(	BLENDPHASE__CLASS,
						BLENDPHASE__LABEL,
						BLENDPHASE__DESC,
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool BlendPhase::FBCreate()
{
	// Tool options
	StartSize[0] = 400;
	StartSize[1] = 160;
	
	// Configure layout
	int lB = 10;

	// regions for label & edit visual elements 
	AddRegion( "LabelSrc", "LabelSrc",
										lB,	kFBAttachLeft,	"",				1.0	,
										lB,	kFBAttachTop,	"",				1.0,
										120,kFBAttachNone,	"",				1.0,
										30,	kFBAttachNone,	"",				1.0 );
	AddRegion( "EditAnimSrc", "EditAnimSrc",
										lB,	kFBAttachRight,	"LabelSrc",		1.0	,
										lB,	kFBAttachTop,	"",				1.0,
										120,kFBAttachNone,	"",				1.0,
										30,	kFBAttachNone,	"",				1.0 );
	AddRegion( "LabelDst", "LabelDst",
										lB,	kFBAttachLeft,	"",				1.0	,
										lB,	kFBAttachBottom,"LabelSrc",		1.0,
										120,kFBAttachNone,	"",				1.0,
										30,	kFBAttachNone,	"",				1.0 );
	AddRegion( "EditAnimDst", "EditAnimDst",
										lB,	kFBAttachRight,	"LabelDst",		1.0	,
										lB,	kFBAttachBottom,"EditAnimSrc",	1.0,
										120,kFBAttachNone,	"",				1.0,
										30,	kFBAttachNone,	"",				1.0 );
	AddRegion( "LabelBlend", "LabelBlend",
										lB,	kFBAttachLeft,	"",				1.0	,
										lB,	kFBAttachBottom,"LabelDst",		1.0,
										120,kFBAttachNone,	"",				1.0,
										30,	kFBAttachNone,	"",				1.0 );
	AddRegion( "EditBlendSize", "EditBlendSize",
										lB,	kFBAttachRight,	"LabelBlend",	1.0	,
										lB,	kFBAttachBottom,"EditAnimDst",	1.0,
										120,kFBAttachNone,	"",				1.0,
										30,	kFBAttachNone,	"",				1.0 );

	AddRegion( "ButtonBlend", "ButtonBlend",
										lB,	kFBAttachRight,	"EditAnimDst",	1.0	,
										lB,	kFBAttachBottom,"EditAnimSrc",	1.0,
										120,kFBAttachNone,	"",				1.0,
										30,	kFBAttachNone,	"",				1.0 );


	SetControl( "LabelSrc", mLabelSrc );
	SetControl( "LabelDst", mLabelDst );
	SetControl( "LabelBlend", mLabelBlend );

	SetControl( "ButtonBlend", mButtonBlend );

	SetControl( "EditAnimSrc", mEditAnimSrc );
	SetControl( "EditAnimDst", mEditAnimDst );
	SetControl( "EditBlendSize", mEditBlendSize );

	// Configure button's
	mLabelSrc.Caption = "1'st anim out [0;100]";
	mLabelDst.Caption = "2'nd anim in [0;100]";
	mLabelBlend.Caption = "blend len in frames";

	mButtonBlend.Caption = "Blend";
	mButtonBlend.PropertyList.Find( "Hint" )->SetString( "Make blend according current phase" );
	mButtonBlend.OnClick.Add( this, (FBCallback) &BlendPhase::EventButtonBlendClick );

	return true;
}

/************************************************
 *	Button click callback.
 ************************************************/
void BlendPhase::EventButtonBlendClick( HISender pSender, HKEvent pEvent )
{
	FBStory			lStory;
	HFBStoryFolder	lActionFolder	= lStory.RootFolder;	
	HFBStoryTrack	lCharTrack		= lActionFolder->Tracks[0];
	HFBStoryClip	lClipSrc, lClipDst;

	lClipSrc = lCharTrack->Clips[0];
	lClipDst = lCharTrack->Clips[1];

	float blendPos	=	mEditAnimSrc.Value;
	float animDst	=	mEditAnimDst.Value;
	float blendTime	=	mEditBlendSize.Value;

	FBTime	lInTimeSrc = lClipSrc->MarkIn;
	FBTime	lOutTimeSrc = lClipSrc->MarkOut;
	FBTime	lInTimeDst = lClipDst->MarkIn;
	FBTime	lOutTimeDst = lClipDst->MarkOut;

	long lInTime = lInTimeSrc.GetFrame( true );
	long lOutTime = lOutTimeSrc.GetFrame( true );

	blendPos *= 0.01f;
	blendPos = lInTime + (lOutTime - lInTime) * blendPos;

	lInTime = lInTimeDst.GetFrame( true );
	lOutTime = lOutTimeDst.GetFrame( true );

	animDst *= 0.01f;
	animDst = lInTime + (lOutTime - lInTime) * animDst;
	
	FBTimeSpan		lTimeSpanSrc( FBTime( 0, 0, 0, 0  ), 0 );
	FBTimeSpan		lTimeSpanDst( 0, FBTime( 0, 0, 0, 0 ) );


	lClipSrc->PostBlend = lTimeSpanSrc;
	lClipDst->PreBlend = lTimeSpanDst;

	lClipDst->Offset = FBTime( 0, 0, 0, animDst );
	lClipSrc->Stop = FBTime( 0, 0, 0, blendPos + blendTime );
	lClipDst->MoveTo( FBTime( 0, 0, 0, blendPos ) );
}

/************************************************
 *	FiLMBOX Destruction function.
 ************************************************/
void BlendPhase::FBDestroy()
{
	// Free user allocated memory
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool BlendPhase::FbxStore	( HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat )
{
	
	return true;
}

/************************************************
 *	FBX Retrieval.
 ************************************************/
bool BlendPhase::FbxRetrieve( HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat )
{
	
	return true;
}


