#ifndef __SPLIT_STYLE_TOOL_H__
#define __SPLIT_STYLE_TOOL_H__


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
#define TOOLSPLITSTYLE__CLASSNAME	Tool_SplitStyle
#define TOOLSPLITSTYLE__CLASSSTR	"Tool_SplitStyle"

/**	Tool template.
*/
class Tool_SplitStyle : public FBTool
{
	//--- FiLMBOX Tool declaration.
	FBToolDeclare( Tool_SplitStyle, FBTool );

public:
	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

	virtual bool FbxStore		( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );
	virtual bool FbxRetrieve	( FBFbxObject *pFbxObject, kFbxObjectStore pStoreWhat );

private:
	void		EventToolIdle		( HISender pSender, HKEvent pEvent );
	void		EventToolShow		( HISender pSender, HKEvent pEvent );
	void		EventToolPaint		( HISender pSender, HKEvent pEvent );
	void		EventToolResize		( HISender pSender, HKEvent pEvent );
	void		EventToolInput		( HISender pSender, HKEvent pEvent );

private:
	FBLayout	mLayout0;
		FBLayoutRegion	mRegion0;
	FBLayout	mLayout1;
		FBLayoutRegion	mRegion1;
};

#endif /* __SPLIT_STYLE_TOOL_H__ */
