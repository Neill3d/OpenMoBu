
/**	\file	orimpexpcsv_menu.cxx
*	Function defintions for menu item class.
*	Custom menu item function definitions for two empty custom menu
*	items: ORCustomSceneImport & FBCustomSceneExport.
*/

//--- Class declarations
#include "orimpexpcsv_menu.h"


//--- FiLMBOX implementation and registration
FBMenuItemImplementation( ORMenuItemCsvImport );
FBRegisterMenuItem		( ORMenuItemCsvImport );

/************************************************
 *	Import: Configuration Function
 ************************************************/
bool ORMenuItemCsvImport::FBCreate()
{
	// Configure the menu item.
	Type			= kFBMenuItemSceneImport;
	Extension		= "*.csv";
	Label			= "Rigid body data";
	Description		= "import optical rigid body data";

	return true;
}


/************************************************
 *	Import: Execution Function
 ************************************************/
bool ORMenuItemCsvImport::Execute(const char *pFilename)
{
	// Import data here into FiLMBOX
	mEngine->ImportFile(pFilename);
	return true;
}