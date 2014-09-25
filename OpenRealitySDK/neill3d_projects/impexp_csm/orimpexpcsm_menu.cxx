

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

//--- Class declarations
#include "orimpexpcsm_menu.h"


//--- FiLMBOX implementation and registration
FBMenuItemImplementation( ORMenuItemCSMImport );
FBRegisterMenuItem		( ORMenuItemCSMImport );

FBMenuItemImplementation( ORMenuItemCSMExport );
FBRegisterMenuItem		( ORMenuItemCSMExport );

/************************************************
 *	Import: Configuration Function
 ************************************************/
bool ORMenuItemCSMImport::FBCreate()
{
	// Configure the menu item.
	Type			= kFBMenuItemSceneImport;
	Extension		= "*.csm";
	Label			= "character studio marker data";
	Description		= "import optical marker data";

	return true;
}


/************************************************
 *	Import: Execution Function
 ************************************************/
bool ORMenuItemCSMImport::Execute(const char *pFilename)
{
	// Import data here into FiLMBOX
	mEngine->ImportFile(pFilename);
	return true;
}


/************************************************
 *	Export: Configuration Function
 ************************************************/
bool ORMenuItemCSMExport::FBCreate()
{
	// Configure the menu item.
	Type			= kFBMenuItemSceneExport;
	Extension		= "*.csm";
	Label			= "Character studio marker data";
	Description		= "export optical marker data";

	return true;
}


/************************************************
 *	Export: Execution Function
 ************************************************/
bool ORMenuItemCSMExport::Execute(const char *pFilename)
{
	// Import data here into FiLMBOX
	mEngine->ExportFile(pFilename);
	return true;
}