#ifndef __ORIMPEXP_TOOL_MENU_H__
#define __ORIMPEXP_TOOL_MENU_H__


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

/**	\file	orimpexpcsv_menu.h
*	Custom menu item class declarations.
*	Custom menu item class declarations for two empty menu
*	items: ORCustomSceneImport & ORCustomSceneExport.
*/

//--- SDK include
#include <fbsdk/fbsdk.h>
#include "orimpexpcsm_engine.h"


/**	Custom Scene Import Menu Item.
*	This class creates a custom menu item, giving the developer the
*	ability to map their custom import functions to an option in the menu.
*/
class ORMenuItemCSMImport : public FBMenuItem
{
	//--- FiLMBOX Menu Item Declaration
	FBMenuItemDeclare( ORMenuItemCSMImport, FBMenuItem );

public:
	//--- FBMenuItem functions
	virtual bool FBCreate	();								//!< Overloaded FBMenuItem configuration function.

#ifdef OLD_FBMENU_INTERFACE
	virtual	bool Execute (char *pFilename=NULL);			//!< Overloaded FBMenuItem execution function.
#else
	virtual	bool Execute (const char *pFilename=NULL);			//!< Overloaded FBMenuItem execution function.
#endif

public:
    CSMEngine*    mEngine;
};

class ORMenuItemCSMExport : public FBMenuItem
{
	//--- FiLMBOX Menu Item Declaration
	FBMenuItemDeclare( ORMenuItemCSMExport, FBMenuItem );

public:
	//--- FBMenuItem functions
	virtual bool FBCreate	();								//!< Overloaded FBMenuItem configuration function.

#ifdef OLD_FBMENU_INTERFACE
	virtual	bool Execute (char *pFilename=NULL);			//!< Overloaded FBMenuItem execution function.
#else
	virtual	bool Execute (const char *pFilename=NULL);			//!< Overloaded FBMenuItem execution function.
#endif

public:
    CSMEngine*    mEngine;
};

#endif /* __ORIMPEXP_TOOL_MENU_H__ */
