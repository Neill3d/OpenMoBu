
/** \file ortoolpersistent_tool.cxx
*/

// Class declaration
#include "BlendShapeToolkit_persistent_data.h"

//--- FiLMBOX Registration & Implementation.
FBClassImplementation(  BrushMovePersistentData    );
FBUserObjectImplement(  BrushMovePersistentData,
                        "Class object used to hold Move Brush data.",
                        FB_DEFAULT_SDK_ICON     );                                          //Register UserObject class
FBElementClassImplementation( BrushMovePersistentData, FB_DEFAULT_SDK_ICON );                  //Register to the asset system

/************************************************
 *  Constructor.
 ************************************************/
BrushMovePersistentData::BrushMovePersistentData( const char* pName, HIObject pObject ) 
	: FBUserObject( pName, pObject )
{
    FBClassInit;

	FBPropertyPublish( this, Override, "Override Common", nullptr, nullptr );
	FBPropertyPublish( this, Radius, "Radius", nullptr, nullptr );
	FBPropertyPublish( this, Strength, "Strength", nullptr, nullptr );

	Override = false;
	Radius = 5.0;
	Strength = 5.0;
}

/************************************************
 *  FiLMBOX Constructor.
 ************************************************/
bool BrushMovePersistentData::FBCreate()
{
    return true;
}

/************************************************
 *  FiLMBOX Destructor.
 ************************************************/
void BrushMovePersistentData::FBDestroy()
{
}

bool BrushMovePersistentData::FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
    return true;
}

bool BrushMovePersistentData::FbxRetrieve( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
    return true;
}
