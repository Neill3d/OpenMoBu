#ifndef __ORTOOLPERSISTENT_DATA_H__
#define __ORTOOLPERSISTENT_DATA_H__


/** \file ortoolpersistent_data.h
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

//! This class will make sure that the data related to the tool is persistent.
class BrushMovePersistentData : public FBUserObject {
    //--- FiLMBOX declaration.
    FBClassDeclare(BrushMovePersistentData, FBUserObject)
    FBDeclareUserObject(BrushMovePersistentData);

public:
    BrushMovePersistentData( const char *pName=NULL, HIObject pObject=NULL );

    //--- FiLMBOX Construction/Destruction,
    virtual bool FBCreate();        //!< FiLMBOX Creation function.
    virtual void FBDestroy();       //!< FiLMBOX Destruction function.

    virtual bool FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );
    virtual bool FbxRetrieve( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );

	FBPropertyBool			Override;	// override common settings
	FBPropertyDouble		Radius;
	FBPropertyDouble		Strength;
};

#endif /* __ORTOOL_PERSISTENT_DATA_H__ */
