#ifndef __RENDERADVANCE_TOOL_H__
#define __RENDERADVANCE_TOOL_H__


/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://code.google.com/p/motioncodelibrary/wiki/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>

#include "renderadvance_view.h"

//--- Registration define
#define RENDERADVANCE__CLASSNAME	ToolRenderAdvance
#define RENDERADVANCE__CLASSSTR		"ToolRenderAdvance"

/**	Tool template.
*/
class ToolRenderAdvance : public FBTool
{
	//--- Tool declaration.
	FBToolDeclare( ToolRenderAdvance, FBTool );

public:
	//--- Construction/Destruction,
	virtual bool FBCreate();		//!< Creation function.
	virtual void FBDestroy();		//!< Destruction function.

	// FBX store/retrieve
	virtual bool FbxStore		( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );
	virtual bool FbxRetrieve	( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );

private:
	// UI Management
	void	UICreate	();
	void	UICreate1();
	void	UICreate2();
	void	UICreate3();
	void	UIConfigure	();
	void	UIConfigure1();
	void	UIConfigure2();
	void	UIConfigure3();
	void	UIReset		();

	// UI callbacks
	void	EventShowChange( HISender pSender, HKEvent pEvent );
	void	EventPictureChange( HISender pSender, HKEvent pEvent );
	void	EventLayoutChange( HISender pSender, HKEvent pEvent );
	void	EventCameraChange1( HISender pSender, HKEvent pEvent );
	void	EventCameraChange2( HISender pSender, HKEvent pEvent );
	void	EventCameraChange3( HISender pSender, HKEvent pEvent );
	void	EventCameraChange4( HISender pSender, HKEvent pEvent );
	void	EventTabChange( HISender pSender, HKEvent pEvent );
	void	EventButtonAbout( HISender pSender, HKEvent pEvent );
	void	EventButtonRender( HISender pSender, HKEvent pEvent );
	void	EventButtonFilePopup( HISender pSender, HKEvent pEvent );
	void	EventToolIdle       ( HISender pSender, HKEvent pEvent );
	void	EventToolShow       ( HISender pSender, HKEvent pEvent );
	void	EventToolPaint      ( HISender pSender, HKEvent pEvent );
	void	EventToolResize     ( HISender pSender, HKEvent pEvent );
	void	EventToolInput      ( HISender pSender, HKEvent pEvent );

	// Refresh the view
	void	RefreshView			();

private:
	FBSystem			mSystem;

    FBButton    mButtonFilePopup; // Dialog to select the path of the file.
    FBFilePopup mFilePopup;       // Actual browser.

	FBLayout			mLayout			[2 ];		// render && view settings
	FBTabPanel			mTabPanel;
	FBLayout			mLayoutSettings;
	FBButton			mButtonRender;
	FBButton			mButtonAbout;

	ORView3D	mView;			  // 3D view.

	// all variables for layout1
	FBLabel	region1;
	FBEdit	EditFile;
	FBButton	ButtonBrowse;
	FBLabel	region4;
	FBEditNumber	EditStart;
	FBLabel	region6;
	FBEditNumber	EditStop;
	FBLabel	region8;
	FBEditNumber	EditStep;
	FBLabel	region10;
	FBList	ListPicture;
	FBLabel	region12;
	FBList	ListField;
	FBLabel	region14;
	FBList	ListPixel;
	FBButton	ButtonAudio;

	// all variables for layout2
	FBLabel	LabelLayout;
	FBList	ListLayout;
	FBLabel	LabelCamera1;
	FBList	ListCamera1;
	FBLabel	LabelCamera2;
	FBList	ListCamera2;
	FBLabel	LabelCamera3;
	FBList	ListCamera3;
	FBLabel	LabelCamera4;
	FBList	ListCamera4;
	FBLabel	LabelDisplay;
	FBList	ListDisplay;
	FBButton	ButtonShowTime;
	FBButton	ButtonShowSafe;
	FBButton	ButtonShowLabel;
	FBButton	ButtonAntialiasing;
};

#endif /* __RENDERADVANCE_TOOL_H__ */
