#ifndef __ORTOOL_TEMPLATE_TOOL_H__
#define __ORTOOL_TEMPLATE_TOOL_H__


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

//--- Motion Code Library include
#include "mainmenu.h"

//--- Registration define
#define ORTOOLTEMPLATE__CLASSNAME	ORTool_Template
#define ORTOOLTEMPLATE__CLASSSTR	"ORTool_Template"

/**	Sample Tool.
*/
class ORTool_Template : public FBTool
{
	//--- FiLMBOX Tool declaration.
	FBToolDeclare( ORTool_Template, FBTool );

public:
	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

private:
	void		EventButtonTestClick( HISender pSender, HKEvent pEvent );
	void		EventToolIdle		( HISender pSender, HKEvent pEvent );
	void		EventToolShow		( HISender pSender, HKEvent pEvent );
	void		EventToolPaint		( HISender pSender, HKEvent pEvent );
	void		EventToolResize		( HISender pSender, HKEvent pEvent );
	void		EventToolInput		( HISender pSender, HKEvent pEvent );

private:
	ToolMenu		mMenu;
	MyMenuHandle	mMenuHandle; // handling submenu with a button
};

#endif /* __FB_TOOL_TEMPLATE_TOOL_H__ */
