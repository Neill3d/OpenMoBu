
#ifndef __RIGID_BODY_TOOL_H__
#define __RIGID_BODY_TOOL_H__

/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>

//--- Registration define
#define RIGIDBODY_TOOL__CLASSNAME	RigidBodyTool
#define RIGIDBODY_TOOL__CLASSSTR	"RigidBodyTool"

class RigidBodyTool : public FBTool
{
	//--- FiLMBOX Tool declaration.
	FBToolDeclare( RigidBodyTool, FBTool );

public:
	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

	virtual bool FbxStore		( HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat );
	virtual bool FbxRetrieve	( HFBFbxObject pFbxObject, kFbxObjectStore pStoreWhat );

private:
	void		EventButtonApplyClick( HISender pSender, HKEvent pEvent );
	void		EventButtonClearClick( HISender pSender, HKEvent pEvent );
	void		EventToolIdle		( HISender pSender, HKEvent pEvent );
	void		EventToolShow		( HISender pSender, HKEvent pEvent );
	void		EventToolPaint		( HISender pSender, HKEvent pEvent );
	void		EventToolResize		( HISender pSender, HKEvent pEvent );
	void		EventToolInput		( HISender pSender, HKEvent pEvent );

private:
	//-- control buttons
	FBButton			mButtonApply;	// add newton properties for selection
	FBButton			mButtonClear;	// remove all newton properties from selection

	FBLabel				mLabelMass;
	FBEditNumber		mEditMass;		// body mass
};

#endif
